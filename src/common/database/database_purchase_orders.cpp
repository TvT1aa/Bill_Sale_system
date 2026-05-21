#include "databasemanager.h"
#include <QSqlQuery>
#include <QDebug>

// 1. 按ID查找采购订单
PurchaseOrderInfo DatabaseManager::getPurchaseOrderById(int orderId)
{
    PurchaseOrderInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, total_amount, remark, created_at "
                  "FROM purchase_orders WHERE id = ?");
    query.addBindValue(orderId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.totalAmount = query.value("total_amount").toDouble();
        info.remark = query.value("remark").toString();
        info.createdAt = query.value("created_at").toString();
    }
    return info;
}

// 2. 获取所有采购订单列表
QList<PurchaseOrderInfo> DatabaseManager::getAllPurchaseOrders()
{
    QList<PurchaseOrderInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, total_amount, remark, created_at "
                  "FROM purchase_orders ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            PurchaseOrderInfo info;
            info.id = query.value("id").toInt();
            info.totalAmount = query.value("total_amount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("created_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 3. 添加采购订单
bool DatabaseManager::addPurchaseOrder(const QString& remark, int* outOrderId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO purchase_orders (remark) VALUES (?)");
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

// 4. 删除采购订单
bool DatabaseManager::deletePurchaseOrder(int orderId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM purchase_orders WHERE id = ?");
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

// 5. 按订单ID查找明细
QList<PurchaseOrderItemInfo> DatabaseManager::getOrderItemsByOrderId(int orderId)
{
    QList<PurchaseOrderItemInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, order_id, product_id, quantity, unit_price, subtotal "
                  "FROM purchase_order_items WHERE order_id = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            PurchaseOrderItemInfo info;
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

// 6. 添加采购订单明细
bool DatabaseManager::addOrderItem(int orderId, int productId, int quantity, double unitPrice, int* outItemId)
{
    if (!isConnected()) return false;

    double subtotal = quantity * unitPrice;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO purchase_order_items (order_id, product_id, quantity, unit_price, subtotal) "
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

// 7. 修改采购订单明细
bool DatabaseManager::updateOrderItem(int itemId, int quantity, double unitPrice)
{
    if (!isConnected()) return false;

    double subtotal = quantity * unitPrice;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE purchase_order_items SET quantity = ?, unit_price = ?, subtotal = ? WHERE id = ?");
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

// 8. 删除采购订单明细
bool DatabaseManager::deleteOrderItem(int itemId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM purchase_order_items WHERE id = ?");
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
