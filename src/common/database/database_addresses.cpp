#include "databasemanager.h"
#include "desutil.h"
#include <QSqlQuery>
#include <QDebug>

// 1. 按ID查找地址
AddressInfo DatabaseManager::getAddressById(int addressId)
{
    AddressInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, name, phone, province, city, district, detail, is_default, is_active, created_at, updated_at "
                  "FROM buyer_addresses WHERE id = ?");
    query.addBindValue(addressId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.userId = query.value("user_id").toInt();
        info.name = DESutil::decryptWithDefaultKey(query.value("name").toString());
        info.phone = DESutil::decryptWithDefaultKey(query.value("phone").toString());
        info.province = query.value("province").toString();
        info.city = query.value("city").toString();
        info.district = query.value("district").toString();
        info.detail = query.value("detail").toString();
        info.isDefault = query.value("is_default").toInt();
        info.isActive = query.value("is_active").toInt();
        info.createdAt = query.value("created_at").toString();
        info.updatedAt = query.value("updated_at").toString();
    }
    return info;
}

// 2. 按用户ID查找地址列表
QList<AddressInfo> DatabaseManager::getAddressesByUserId(int userId)
{
    QList<AddressInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, user_id, name, phone, province, city, district, detail, is_default, is_active, created_at, updated_at "
                  "FROM buyer_addresses WHERE user_id = ? AND is_active = 1");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            AddressInfo info;
            info.id = query.value("id").toInt();
            info.userId = query.value("user_id").toInt();
            info.name = DESutil::decryptWithDefaultKey(query.value("name").toString());
            info.phone = DESutil::decryptWithDefaultKey(query.value("phone").toString());
            info.province = query.value("province").toString();
            info.city = query.value("city").toString();
            info.district = query.value("district").toString();
            info.detail = query.value("detail").toString();
            info.isDefault = query.value("is_default").toInt();
            info.isActive = query.value("is_active").toInt();
            info.createdAt = query.value("created_at").toString();
            info.updatedAt = query.value("updated_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 3. 添加地址
bool DatabaseManager::addAddress(int userId, const QString& name, const QString& phone,
                                 const QString& province, const QString& city,
                                 const QString& district, const QString& detail,
                                 bool isDefault, int* outAddressId)
{
    if (!isConnected()) return false;

    // 加密姓名和手机号
    QString encryptedName = DESutil::encryptWithDefaultKey(name.trimmed());
    QString encryptedPhone = DESutil::encryptWithDefaultKey(phone.trimmed());

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO buyer_addresses (user_id, name, phone, province, city, district, detail, is_default) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(encryptedName);
    query.addBindValue(encryptedPhone);
    query.addBindValue(province);
    query.addBindValue(city);
    query.addBindValue(district);
    query.addBindValue(detail);
    query.addBindValue(isDefault ? 1 : 0);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outAddressId) *outAddressId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

// 5. 修改地址
bool DatabaseManager::updateAddress(int addressId, const QString& name, const QString& phone,
                                    const QString& province, const QString& city,
                                    const QString& district, const QString& detail)
{
    if (!isConnected()) return false;

    // 加密姓名和手机号
    QString encryptedName = DESutil::encryptWithDefaultKey(name.trimmed());
    QString encryptedPhone = DESutil::encryptWithDefaultKey(phone.trimmed());

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE buyer_addresses SET name = ?, phone = ?, province = ?, city = ?, district = ?, detail = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(encryptedName);
    query.addBindValue(encryptedPhone);
    query.addBindValue(province);
    query.addBindValue(city);
    query.addBindValue(district);
    query.addBindValue(detail);
    query.addBindValue(addressId);

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

// 6. 软删除地址
bool DatabaseManager::deleteAddress(int addressId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE buyer_addresses SET is_active = 0, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(addressId);

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

// 7. 设置默认地址
bool DatabaseManager::setDefaultAddress(int userId, int addressId)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);

    // 先取消该用户所有地址的默认状态
    query.prepare("UPDATE buyer_addresses SET is_default = 0 WHERE user_id = ?");
    query.addBindValue(userId);
    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    // 再设置指定地址为默认
    query.prepare("UPDATE buyer_addresses SET is_default = 1, updated_at = CURRENT_TIMESTAMP WHERE id = ? AND user_id = ?");
    query.addBindValue(addressId);
    query.addBindValue(userId);
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
