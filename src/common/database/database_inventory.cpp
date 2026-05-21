#include "databasemanager.h"
#include <QSqlQuery>
#include <QDebug>

static const char* DEFAULT_WAREHOUSE = "南昌大学青山湖校区1栋";

// 1. 按商品ID查找库存
InventoryInfo DatabaseManager::getInventoryByProductId(int productId)
{
    InventoryInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, product_id, quantity, warehouse_address, updated_at "
                  "FROM inventory WHERE product_id = ?");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.productId = query.value("product_id").toInt();
        info.quantity = query.value("quantity").toInt();
        info.warehouseAddress = query.value("warehouse_address").toString();
        info.updatedAt = query.value("updated_at").toString();
    }
    return info;
}

// 2. 获取所有库存列表
QList<InventoryInfo> DatabaseManager::getAllInventory()
{
    QList<InventoryInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, product_id, quantity, warehouse_address, updated_at "
                  "FROM inventory");

    if (query.exec()) {
        while (query.next()) {
            InventoryInfo info;
            info.id = query.value("id").toInt();
            info.productId = query.value("product_id").toInt();
            info.quantity = query.value("quantity").toInt();
            info.warehouseAddress = query.value("warehouse_address").toString();
            info.updatedAt = query.value("updated_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 3. 添加库存
bool DatabaseManager::addInventory(int productId, int quantity, int* outInventoryId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO inventory (product_id, quantity, warehouse_address) VALUES (?, ?, ?)");
    query.addBindValue(productId);
    query.addBindValue(quantity);
    query.addBindValue(DEFAULT_WAREHOUSE);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outInventoryId) *outInventoryId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

// 4. 修改库存数量
bool DatabaseManager::updateQuantity(int inventoryId, int quantity)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE inventory SET quantity = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(quantity);
    query.addBindValue(inventoryId);

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
