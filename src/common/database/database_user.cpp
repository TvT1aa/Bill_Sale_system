#include "databasemanager.h"
#include "desutil.h"
#include <QSqlQuery>
#include <QDebug>
#include <QCryptographicHash>

// 1. 查重逻辑：注册前调用
bool DatabaseManager::userExists(const QString& username)
{
    QSqlQuery query(m_db);
    QString encryptedUsername = DESutil::encryptWithDefaultKey(username.trimmed());
    query.prepare("SELECT 1 FROM users WHERE username = ?");
    query.addBindValue(encryptedUsername);  //
    return query.exec() && query.next();
}

// 2. 核心查询：findUserByAccount
UserInfo DatabaseManager::findUserByAccount(const QString& account)
{
    UserInfo info;
    // 步骤 A: 把用户输入的明文变成密文，才能去数据库里匹配
    QString encryptedSearch = DESutil::encryptWithDefaultKey(account.trimmed());

    QSqlQuery query(m_db);
    query.prepare("SELECT id, username, email, phone, password_hash, role, is_active "
                  "FROM users WHERE username = ? OR email = ? OR phone = ?");
    query.addBindValue(encryptedSearch);
    query.addBindValue(encryptedSearch);
    query.addBindValue(encryptedSearch);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();

        // 步骤 B: 数据库取出的是密文，这里使用解密还原成明文给 UI 使用
        info.username = DESutil::decryptWithDefaultKey(query.value("username").toString());
        info.email = DESutil::decryptWithDefaultKey(query.value("email").toString());

        info.passwordHash = query.value("password_hash").toString(); // Hash不需要解密
        info.role = query.value("role").toInt();
        info.isActive = query.value("is_active").toInt();

        qDebug() << "成功查获并解密用户:" << info.username;
    }
    return info;
}

// 3. 按ID查找用户
UserInfo DatabaseManager::getUserById(int userId)
{
    UserInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, username, email, phone, password_hash, role, is_active "
                  "FROM users WHERE id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.username = DESutil::decryptWithDefaultKey(query.value("username").toString());
        info.email = DESutil::decryptWithDefaultKey(query.value("email").toString());
        info.passwordHash = query.value("password_hash").toString();
        info.role = query.value("role").toInt();
        info.isActive = query.value("is_active").toInt();

        qDebug() << "成功查获并解密用户:" << info.username;
    }
    return info;
}

// 4. 写入操作
bool DatabaseManager::registerUser(const QString& username, const QString& email,
                                   const QString& phone, const QString& passwordHash,
                                   int role, int* outUserId)
{
    if (!isConnected()) return false;

    //  添加加密
    QString encryptedUsername = DESutil::encryptWithDefaultKey(username.trimmed());
    QString encryptedEmail = DESutil::encryptWithDefaultKey(email.trimmed());
    QString encryptedPhone = DESutil::encryptWithDefaultKey(phone.trimmed());

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, email, phone, password_hash, role) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(encryptedUsername);
    query.addBindValue(encryptedEmail);
    query.addBindValue(encryptedPhone);
    query.addBindValue(passwordHash);
    query.addBindValue(role);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    if (m_db.commit()) {
        if (outUserId) *outUserId = query.lastInsertId().toInt();
        return true;
    }
    return false;
}

// 5. 删除用户
bool DatabaseManager::deleteUserByAccount(const QString& account)
{
    if (!isConnected()) return false;

    // 先加密账号再去数据库匹配
    QString encryptedAccount = DESutil::encryptWithDefaultKey(account.trimmed());

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE username = ? OR email = ? OR phone = ?");
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    // 检查是否真的删除了记录
    if (query.numRowsAffected() == 0) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// 6. 修改密码（按用户ID）
bool DatabaseManager::updatePassword(int userId, const QString& newPasswordHash)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password_hash = ? WHERE id = ?");
    query.addBindValue(newPasswordHash);
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

// 7. 修改密码（按账号）
bool DatabaseManager::updatePassword(const QString& account, const QString& newPasswordHash)
{
    if (!isConnected()) return false;

    // 先加密账号再去数据库匹配
    QString encryptedAccount = DESutil::encryptWithDefaultKey(account.trimmed());

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password_hash = ? WHERE username = ? OR email = ? OR phone = ?");
    query.addBindValue(newPasswordHash);
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);

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

// 8. 修改用户信息（邮箱、手机号）
bool DatabaseManager::updateUserInfo(int userId, const QString& email, const QString& phone)
{
    if (!isConnected()) return false;

    // 加密邮箱和手机号
    QString encryptedEmail = DESutil::encryptWithDefaultKey(email.trimmed());
    QString encryptedPhone = DESutil::encryptWithDefaultKey(phone.trimmed());

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET email = ?, phone = ? WHERE id = ?");
    query.addBindValue(encryptedEmail);
    query.addBindValue(encryptedPhone);
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

// 9. 更新最后登录时间
bool DatabaseManager::updateLastLogin(int userId)
{
    if (!isConnected()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(userId);

    return query.exec();
}
QString DatabaseManager::getEmailByIdAndPhone(int userId, const QString& phone)
{
    if (!isConnected()) return QString();

    // 1. 手机号在数据库中是加密存储的，匹配前必须先加密
    QString encryptedPhone = DESutil::encryptWithDefaultKey(phone.trimmed());

    QSqlQuery query(m_db);
    // 2. 根据 ID 和加密后的手机号精准查询
    query.prepare("SELECT email FROM users WHERE id = ? AND phone = ?");
    query.addBindValue(userId);
    query.addBindValue(encryptedPhone);

    if (query.exec() && query.next()) {
        // 3. 取出的 email 也是加密的，需要解密还原
        QString encryptedEmail = query.value("email").toString();
        return DESutil::decryptWithDefaultKey(encryptedEmail);
    }

    // 未找到匹配记录，返回空字符串
    return QString();
}
