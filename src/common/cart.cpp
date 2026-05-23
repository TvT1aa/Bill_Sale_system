#include "cart.h"
#include "databasemanager.h" // 确保能拿到底层的单例接口
#include <QDebug>

Cart::Cart(QObject *parent)
    : QObject(parent)
    , m_userId(0)
{
}

void Cart::setCurrentUser(int userId)
{
    m_userId = userId;
    m_username.clear();
    clearCart();
}

// ==================== 辅助方法 ====================

double Cart::getProductPrice(int productId)
{
    // 对齐：底层返回的是 ProductInfo 结构体
    return DatabaseManager::instance().getProductById(productId).salePrice;
}

int Cart::getProductStock(int productId)
{
    // 对齐：底层返回的是 InventoryInfo 或通过商品ID查库存
    return DatabaseManager::instance().getInventoryByProductId(productId).quantity;
}

QString Cart::getDefaultAddress()
{
    // 🌟【对齐修复 1】：将错误的 BuyerAddressInfo 改为底层统一定义的 AddressInfo
    QList<AddressInfo> addresses = DatabaseManager::instance().getAddressesByUserId(m_userId);
    for (const AddressInfo &addr : addresses) {
        if (addr.isDefault) {
            return QString("%1 %2 %3 %4").arg(addr.province, addr.city, addr.district, addr.detail);
        }
    }
    if (!addresses.isEmpty()) {
        const AddressInfo &addr = addresses.first();
        return QString("%1 %2 %3 %4").arg(addr.province, addr.city, addr.district, addr.detail);
    }
    return "";
}

// ==================== 购物车操作 ====================

bool Cart::addToCart(int productId, int quantity)
{
    int stock = getProductStock(productId);
    if (quantity > stock) {
        m_lastError = QString("库存不足！当前库存：%1 件").arg(stock);
        return false;
    }

    for (CartItem &item : m_cartItems) {
        if (item.productId == productId) {
            int newQuantity = item.quantity + quantity;
            if (newQuantity > stock) {
                m_lastError = QString("加入后数量超过库存！当前库存：%1 件").arg(stock);
                return false;
            }
            item.quantity = newQuantity;
            item.total = item.quantity * item.price;
            emit cartChanged();
            return true;
        }
    }

    ProductInfo product = DatabaseManager::instance().getProductById(productId);
    CartItem newItem;
    newItem.productId = productId;
    newItem.productName = product.name;
    newItem.quantity = quantity;
    newItem.price = product.salePrice;
    newItem.total = quantity * product.salePrice;
    m_cartItems.append(newItem);
    emit cartChanged();
    return true;
}

bool Cart::removeFromCart(int productId)
{
    for (int i = 0; i < m_cartItems.size(); ++i) {
        if (m_cartItems[i].productId == productId) {
            m_cartItems.removeAt(i);
            emit cartChanged();
            return true;
        }
    }
    return false;
}

bool Cart::updateCartQuantity(int productId, int quantity)
{
    for (CartItem &item : m_cartItems) {
        if (item.productId == productId) {
            if (quantity <= 0) {
                return removeFromCart(productId);
            }
            int stock = getProductStock(productId);
            if (quantity > stock) {
                m_lastError = QString("库存不足！当前库存：%1 件").arg(stock);
                return false;
            }
            item.quantity = quantity;
            item.total = quantity * item.price;
            emit cartChanged();
            return true;
        }
    }
    m_lastError = "购物车中未找到该商品";
    return false;
}

bool Cart::clearCart()
{
    if (m_cartItems.isEmpty()) {
        m_lastError = "购物车已经是空的";
        return false;
    }
    m_cartItems.clear();
    emit cartChanged();
    return true;
}

double Cart::getCartTotal()
{
    double total = 0;
    for (const CartItem &item : m_cartItems) {
        total += item.total;
    }
    return total;
}

// ==================== 结算下单 ====================

int Cart::checkout()
{
    if (m_cartItems.isEmpty()) {
        m_lastError = "购物车为空，请先添加商品";
        return -1;
    }

    double total = getCartTotal();

    // 检查所有商品库存
    for (const CartItem &item : m_cartItems) {
        int stock = getProductStock(item.productId);
        if (item.quantity > stock) {
            m_lastError = QString("商品 %1 库存不足！当前库存：%2 件")
                              .arg(item.productName).arg(stock);
            return -1;
        }
    }

    // 获取默认地址
    QString address = getDefaultAddress();

    // 对接队友的 addSalesOrder 接口
    int orderId = -1;
    if (!DatabaseManager::instance().addSalesOrder(m_userId, address, "购物车下单", &orderId)) {
        m_lastError = "创建订单失败";
        return -1;
    }

    // 添加订单商品明细并扣减库存
    for (const CartItem &item : m_cartItems) {
        // 对接队友的 addSalesOrderItem 接口
        if (!DatabaseManager::instance().addSalesOrderItem(orderId, item.productId, item.quantity, item.price)) {
            m_lastError = QString("添加订单商品 %1 失败").arg(item.productName);
            return -1;
        }

        // 调用库存扣减函数
        if (!DatabaseManager::instance().updateProductStock(item.productId, item.quantity)) {
            m_lastError = QString("商品 %1 库存扣减失败").arg(item.productName);
            return -1;
        }
    }

    // 清空购物车并抛出成功信号
    m_cartItems.clear();
    emit cartChanged();
    emit checkoutSuccess(orderId, total);

    qDebug() << "[Cart] 下单成功，订单ID:" << orderId << "金额:" << total;
    return orderId;
}