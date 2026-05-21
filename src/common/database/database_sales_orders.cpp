#include "databasemanager.h"
#include <QSqlQuery>
#include <QDebug>

// 1. 按ID查找销售订单
SalesOrderInfo DatabaseManager::getSalesOrderById(int orderId)
{
    SalesOrderInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, address, total_amount, remark, created_at "
                  "FROM sales_orders WHERE id = ?");
    query.addBindValue(orderId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.userId = query.value("user_id").toInt();
        info.address = query.value("address").toString();
        info.totalAmount = query.value("total_amount").toDouble();
        info.remark = query.value("remark").toString();
        info.createdAt = query.value("created_at").toString();
    }
    return info;
}

// 2. 按用户ID查找销售订单列表
QList<SalesOrderInfo> DatabaseManager::getSalesOrdersByUserId(int userId)
{
    QList<SalesOrderInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, address, total_amount, remark, created_at "
                  "FROM sales_orders WHERE user_id = ? ORDER BY created_at DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            SalesOrderInfo info;
            info.id = query.value("id").toInt();
            info.userId = query.value("user_id").toInt();
            info.address = query.value("address").toString();
            info.totalAmount = query.value("total_amount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("created_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 3. 获取所有销售订单列表
QList<SalesOrderInfo> DatabaseManager::getAllSalesOrders()
{
    QList<SalesOrderInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, address, total_amount, remark, created_at "
                  "FROM sales_orders ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            SalesOrderInfo info;
            info.id = query.value("id").toInt();
            info.userId = query.value("user_id").toInt();
            info.address = query.value("address").toString();
            info.totalAmount = query.value("total_amount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("created_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 4. 添加销售订单
bool DatabaseManager::addSalesOrder(int userId, const QString& address, const QString& remark, int* outOrderId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO sales_orders (user_id, address, remark) VALUES (?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(address);
    query.addBindValue(remark);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outOrderId) *outOrderId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

// 5. 删除销售订单
bool DatabaseManager::deleteSalesOrder(int orderId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM sales_orders WHERE id = ?");
    query.addBindValue(orderId);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// 6. 按订单ID查找明细
QList<SalesOrderItemInfo> DatabaseManager::getSalesOrderItems(int orderId)
{
    QList<SalesOrderItemInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, order_id, product_id, quantity, unit_price, subtotal "
                  "FROM sales_order_items WHERE order_id = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            SalesOrderItemInfo info;
            info.id = query.value("id").toInt();
            info.orderId = query.value("order_id").toInt();
            info.productId = query.value("product_id").toInt();
            info.quantity = query.value("quantity").toInt();
            info.unitPrice = query.value("unit_price").toDouble();
            info.subtotal = query.value("subtotal").toDouble();
            list.append(info);
        }
    }
    return list;
}

// 7. 添加销售订单明细
bool DatabaseManager::addSalesOrderItem(int orderId, int productId, int quantity, double unitPrice, int* outItemId)
{
    if (!isConnected()) return false;

    double subtotal = quantity * unitPrice;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO sales_order_items (order_id, product_id, quantity, unit_price, subtotal) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(orderId);
    query.addBindValue(productId);
    query.addBindValue(quantity);
    query.addBindValue(unitPrice);
    query.addBindValue(subtotal);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outItemId) *outItemId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

// 8. 修改销售订单明细
bool DatabaseManager::updateSalesOrderItem(int itemId, int quantity, double unitPrice)
{
    if (!isConnected()) return false;

    double subtotal = quantity * unitPrice;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE sales_order_items SET quantity = ?, unit_price = ?, subtotal = ? WHERE id = ?");
    query.addBindValue(quantity);
    query.addBindValue(unitPrice);
    query.addBindValue(subtotal);
    query.addBindValue(itemId);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// 9. 删除销售订单明细
bool DatabaseManager::deleteSalesOrderItem(int itemId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM sales_order_items WHERE id = ?");
    query.addBindValue(itemId);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}
