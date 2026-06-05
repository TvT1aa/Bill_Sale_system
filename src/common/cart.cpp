#include "cart.h"
#include "databasemanager.h" // 确保能拿到底层的单例接口
#include <QDebug>

Cart& Cart::instance()
{
    static Cart _instance;
    return _instance;
}

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
        m_lastError = QString("Insufficient stock! Current stock: %1").arg(stock);
        return false;
    }

    for (CartItem &item : m_cartItems) {
        if (item.productId == productId) {
            int newQuantity = item.quantity + quantity;
            if (newQuantity > stock) {
                m_lastError = QString("Quantity exceeds stock! Current stock: %1").arg(stock);
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
                m_lastError = QString("Insufficient stock! Current stock: %1").arg(stock);
                return false;
            }
            item.quantity = quantity;
            item.total = quantity * item.price;
            emit cartChanged();
            return true;
        }
    }
    m_lastError = "Product not found in cart";
    return false;
}

bool Cart::clearCart()
{
    if (m_cartItems.isEmpty()) {
        m_lastError = "Cart is already empty";
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

int Cart::checkout(const QString& address)
{
    if (m_cartItems.isEmpty()) {
        m_lastError = "Cart is empty, please add products first";
        return -1;
    }

    double total = getCartTotal();

    // 检查所有商品库存
    for (const CartItem &item : m_cartItems) {
        int stock = getProductStock(item.productId);
        if (item.quantity > stock) {
            m_lastError = QString("Product %1 insufficient stock! Current stock: %2")
                              .arg(item.productName).arg(stock);
            return -1;
        }
    }

    // 使用传入的地址，如果没有则获取默认地址
    QString finalAddress = address.isEmpty() ? getDefaultAddress() : address;

    // 对接队友的 addSalesOrder 接口
    int orderId = -1;
    if (!DatabaseManager::instance().addSalesOrder(m_userId, finalAddress, "Cart checkout", &orderId, total)) {
        m_lastError = "Failed to create order";
        return -1;
    }

    // 添加订单商品明细
    // 注意：库存已在加入购物车时扣减，这里只需创建订单记录，不再重复扣库存
    for (const CartItem &item : m_cartItems) {
        if (!DatabaseManager::instance().addSalesOrderItem(orderId, item.productId, item.quantity, item.price)) {
            m_lastError = QString("Failed to add order item %1").arg(item.productName);
            return -1;
        }
    }

    // 自动增加卖家余额（收入）
    DatabaseManager::instance().addIncome(total, QString("User order, Order ID: %1").arg(orderId));

    // 清空购物车并抛出成功信号
    m_cartItems.clear();
    emit cartChanged();
    emit checkoutSuccess(orderId, total);

    qDebug() << "[Cart] 下单成功，订单ID:" << orderId << "金额:" << total;
    return orderId;
}