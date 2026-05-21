#include "databasemanager.h"
#include "desutil.h"
#include <QSqlQuery>
#include <QDebug>
#include <QCryptographicHash>

// 1. 查重逻辑：注册前调用
bool DatabaseManager::userExists(const QString& username)
{
    QSqlQuery query(m_db);
    // 数据库存的是密文，所以查重也要用密文去查
    query.prepare("SELECT 1 FROM users WHERE username = ?");
    query.addBindValue(username);
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

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, email, phone, password_hash, role) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(email);
    query.addBindValue(phone);
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

// 10. SHA256哈希
QString DatabaseManager::hashSha256(const QString& input)
{
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}
