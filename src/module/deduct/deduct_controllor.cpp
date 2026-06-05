#include "deduct_controllor.h"
#include "deductwidget.h"
#include <QDebug>

static QVariantMap cartItemToMap(const CartItem &c)
{
    QVariantMap m;
    m["productId"] = c.productId;
    m["productName"] = c.productName;
    m["quantity"] = c.quantity;
    m["price"] = c.price;
    m["total"] = c.total;
    return m;
}

static QVariantMap addressToMap(const AddressInfo &a)
{
    QVariantMap m;
    m["id"] = a.id;
    m["userId"] = a.userId;
    m["name"] = a.name;
    m["phone"] = a.phone;
    m["province"] = a.province;
    m["city"] = a.city;
    m["district"] = a.district;
    m["detail"] = a.detail;
    m["isDefault"] = a.isDefault;
    return m;
}

static QVariantMap orderToMap(const SalesOrderInfo &o)
{
    QVariantMap m;
    m["id"] = o.id;
    m["userId"] = o.userId;
    m["address"] = o.address;
    m["totalAmount"] = o.totalAmount;
    m["remark"] = o.remark;
    m["createdAt"] = o.createdAt;
    // 获取用户名
    UserInfo user = DatabaseManager::instance().getUserById(o.userId);
    m["username"] = user.username;
    m["status"] = 0; // 默认状态：待处理
    return m;
}

static QList<SalesOrderInfo> filterOrdersByDate(const QList<SalesOrderInfo> &allOrders,
                                                 const QDateTime &start, const QDateTime &end)
{
    QList<SalesOrderInfo> filtered;
    for (const SalesOrderInfo &o : allOrders) {
        QDateTime dt = QDateTime::fromString(o.createdAt, Qt::ISODate);
        if (!dt.isValid())
            dt = QDateTime::fromString(o.createdAt, "yyyy-MM-dd hh:mm:ss");
        if (dt >= start && dt <= end)
            filtered.append(o);
    }
    return filtered;
}

deduct_controllor::deduct_controllor(int userId, int mode, DeductWidget *widget, QObject *parent)
    : QObject(parent)
    , m_userId(userId)
    , m_mode(mode)
    , m_view(widget)
{
    Cart::instance().setCurrentUser(m_userId);

    if (m_mode == 0) {
        // 用户结算模式
        connect(m_view, &DeductWidget::loadCheckoutDataRequested, this, [this]() {
            QList<QVariantMap> items;
            for (const CartItem &c : Cart::instance().getCartItems())
                items.append(cartItemToMap(c));
            m_view->onCartItemsLoaded(items, Cart::instance().getCartTotal());

            QList<QVariantMap> addresses;
            for (const AddressInfo &a : DatabaseManager::instance().getAddressesByUserId(m_userId))
                addresses.append(addressToMap(a));
            m_view->onAddressesLoaded(addresses);
        });

        connect(m_view, &DeductWidget::submitOrderRequested, this, [this](int addressId, const QString &remark) {
            // 根据地址ID获取完整地址字符串
            AddressInfo addr = DatabaseManager::instance().getAddressById(addressId);
            QString addressStr = QString("%1 %2 %3 %4（%5 %6）")
                .arg(addr.province, addr.city, addr.district, addr.detail, addr.name, addr.phone);

            int orderId = Cart::instance().checkout(addressStr);
            bool ok = (orderId > 0);
            m_view->onOrderResult(ok, ok ? QString("Order placed successfully, Order ID: %1").arg(orderId) : Cart::instance().lastError());
        });

        connect(m_view, &DeductWidget::addAddressRequested, this, [this](const QVariantMap &data) {
            bool ok = DatabaseManager::instance().addAddress(
                m_userId, data["name"].toString(), data["phone"].toString(),
                data["province"].toString(), data["city"].toString(),
                data["district"].toString(), data["detail"].toString(),
                data["isDefault"].toBool());
            m_view->onAddAddressResult(ok, ok ? "Address added" : "Failed to add address");
        });

        connect(m_view, &DeductWidget::refreshAddressesRequested, this, [this]() {
            QList<QVariantMap> addresses;
            for (const AddressInfo &a : DatabaseManager::instance().getAddressesByUserId(m_userId))
                addresses.append(addressToMap(a));
            m_view->onAddressesLoaded(addresses);
        });

    } else {
        // 管理员出库模式
        connect(m_view, &DeductWidget::refreshRequested, this, [this]() {
            QList<QVariantMap> orders;
            for (const SalesOrderInfo &o : DatabaseManager::instance().getAllSalesOrders())
                orders.append(orderToMap(o));
            m_view->onOrdersLoaded(orders);
        });

        connect(m_view, &DeductWidget::searchOrderRequested, this, [this](const QString &keyword) {
            Q_UNUSED(keyword);
            m_view->onOperationError("Search not implemented yet");
        });

        connect(m_view, &DeductWidget::filterByDateRequested, this, [this](const QDateTime &start, const QDateTime &end) {
            QList<QVariantMap> orders;
            for (const SalesOrderInfo &o : filterOrdersByDate(DatabaseManager::instance().getAllSalesOrders(), start, end))
                orders.append(orderToMap(o));
            m_view->onOrdersLoaded(orders);
        });

        connect(m_view, &DeductWidget::manualDeductRequested, this, [this](int productId, int quantity, const QString &reason) {
            InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(productId);
            if (quantity > inv.quantity) {
                m_view->onOperationError("Insufficient stock");
                return;
            }
            DatabaseManager::instance().updateProductStock(productId, quantity);
            ProductInfo p = DatabaseManager::instance().getProductById(productId);

            double amount = quantity * p.salePrice;
            int orderId = -1;
            if (DatabaseManager::instance().addSalesOrder(0, "Manual Dispatch",
                    QString("Dispatch: %1 x%2, %3").arg(p.name).arg(quantity).arg(reason), &orderId, amount)) {
                DatabaseManager::instance().addSalesOrderItem(orderId, productId, quantity, p.salePrice);

                // 自动增加卖家余额（收入）
                DatabaseManager::instance().addIncome(amount, QString("Manual dispatch, Order ID: %1").arg(orderId));

                m_view->onOperationSuccess(QString("Dispatch successful, Order ID: %1").arg(orderId));
            } else {
                m_view->onOperationError("Dispatch failed");
            }
        });

        // 查看订单详情
        connect(m_view, &DeductWidget::viewOrderDetailRequested, this, [this](int orderId) {
            SalesOrderInfo order = DatabaseManager::instance().getSalesOrderById(orderId);
            if (order.id < 0) {
                m_view->onOperationError("Order not found");
                return;
            }

            QVariantMap detail;
            detail["id"] = order.id;
            detail["userId"] = order.userId;
            detail["address"] = order.address;
            detail["totalAmount"] = order.totalAmount;
            detail["remark"] = order.remark;
            detail["createdAt"] = order.createdAt;

            UserInfo user = DatabaseManager::instance().getUserById(order.userId);
            detail["username"] = user.username;

            m_view->onOrderDetailLoaded(detail);
        });

        // 更新订单状态（数据库暂无此功能，提示用户）
        connect(m_view, &DeductWidget::updateOrderStatusRequested, this, [this](int orderId, int status) {
            Q_UNUSED(orderId);
            Q_UNUSED(status);
            m_view->onOperationError("Order status update not implemented yet");
        });
    }
}
