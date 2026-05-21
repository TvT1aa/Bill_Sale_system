#include "databasemanager.h"
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>

// 1. 按ID查找商品
ProductInfo DatabaseManager::getProductById(int productId)
{
    ProductInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, category, purchase_price, sale_price, unit, remark, is_active, created_at, updated_at "
                  "FROM products WHERE id = ? AND is_active = 1");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        info.category = query.value("category").toString();
        info.purchasePrice = query.value("purchase_price").toDouble();
        info.salePrice = query.value("sale_price").toDouble();
        info.unit = query.value("unit").toString();
        info.remark = query.value("remark").toString();
        info.isActive = query.value("is_active").toInt();
        info.createdAt = query.value("created_at").toString();
        info.updatedAt = query.value("updated_at").toString();
    }
    return info;
}

// 2. 按名称查找商品
QList<ProductInfo> DatabaseManager::getProductsByName(const QString& name)
{
    QList<ProductInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, category, purchase_price, sale_price, unit, remark, is_active, created_at, updated_at "
                  "FROM products WHERE name LIKE ? AND is_active = 1");
    query.addBindValue("%" + name + "%");

    if (query.exec()) {
        while (query.next()) {
            ProductInfo info;
            info.id = query.value("id").toInt();
            info.name = query.value("name").toString();
            info.category = query.value("category").toString();
            info.purchasePrice = query.value("purchase_price").toDouble();
            info.salePrice = query.value("sale_price").toDouble();
            info.unit = query.value("unit").toString();
            info.remark = query.value("remark").toString();
            info.isActive = query.value("is_active").toInt();
            info.createdAt = query.value("created_at").toString();
            info.updatedAt = query.value("updated_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 3. 添加商品
bool DatabaseManager::addProduct(const QString& name, const QString& category,
                                 double purchasePrice, double salePrice,
                                 const QString& unit, const QString& remark,
                                 int* outProductId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO products (name, category, purchase_price, sale_price, unit, remark) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(category);
    query.addBindValue(purchasePrice);
    query.addBindValue(salePrice);
    query.addBindValue(unit);
    query.addBindValue(remark);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outProductId) *outProductId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

// 4. 修改商品信息
bool DatabaseManager::updateProduct(int productId, const QString& name, const QString& category,
                                    double purchasePrice, double salePrice,
                                    const QString& unit, const QString& remark)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE products SET name = ?, category = ?, purchase_price = ?, sale_price = ?, "
                  "unit = ?, remark = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(category);
    query.addBindValue(purchasePrice);
    query.addBindValue(salePrice);
    query.addBindValue(unit);
    query.addBindValue(remark);
    query.addBindValue(productId);

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

// 5. 软删除商品
bool DatabaseManager::deleteProduct(int productId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE products SET is_active = 0, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(productId);

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
