#include "cart.h"
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

// ==================== 辅助方法（与 CartWidget 一致） ====================

double Cart::getProductPrice(int productId)
{
    return DatabaseManager::instance().getProductById(productId).salePrice;
}

int Cart::getProductStock(int productId)
{
    return DatabaseManager::instance().getInventoryByProductId(productId).quantity;
}

QString Cart::getDefaultAddress()
{
    QList<BuyerAddressInfo> addresses = DatabaseManager::instance().getAddressesByUserId(m_userId);
    for (const BuyerAddressInfo &addr : addresses) {
        if (addr.isDefault) {
            return QString("%1 %2 %3 %4").arg(addr.province, addr.city, addr.district, addr.detail);
        }
    }
    if (!addresses.isEmpty()) {
        const BuyerAddressInfo &addr = addresses.first();
        return QString("%1 %2 %3 %4").arg(addr.province, addr.city, addr.district, addr.detail);
    }
    return "";
}

// ==================== 购物车操作（与 CartWidget 完全一致） ====================

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

// ==================== 结算下单（与 CartWidget 完全一致） ====================

int Cart::checkout()
{
    if (m_cartItems.isEmpty()) {
        m_lastError = "购物车为空，请先添加商品";
        return -1;
    }

    double total = getCartTotal();
    double balance = DatabaseManager::instance().getAccountByUserId(m_userId).balance;

    if (total > balance) {
        m_lastError = QString("余额不足！需要 ¥%1，当前余额 ¥%2")
                          .arg(total, 0, 'f', 2)
                          .arg(balance, 0, 'f', 2);
        return -1;
    }

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

    // 创建订单
    int orderId = DatabaseManager::instance().createSalesOrder(m_userId, address, total, "购物车下单");
    if (orderId < 0) {
        m_lastError = "创建订单失败";
        return -1;
    }

    // 添加订单商品明细并扣减库存
    for (const CartItem &item : m_cartItems) {
        if (!DatabaseManager::instance().addOrderItem(orderId, item.productId, item.quantity, item.price, item.total)) {
            m_lastError = QString("添加订单商品 %1 失败").arg(item.productName);
            return -1;
        }
        DatabaseManager::instance().updateProductStock(item.productId, item.quantity);
    }

    // 扣款
    if (!DatabaseManager::instance().updateBalance(m_userId, -total)) {
        m_lastError = "扣款失败";
        return -1;
    }

    // 清空购物车
    m_cartItems.clear();
    emit cartChanged();
    emit checkoutSuccess(orderId, total);

    qDebug() << "[Cart] 下单成功，订单ID:" << orderId << "金额:" << total;
    return orderId;
}
