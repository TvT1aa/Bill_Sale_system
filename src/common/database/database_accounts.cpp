#include "databasemanager.h"
#include <QSqlQuery>
#include <QDebug>

// 1. 获取账户信息
AccountInfo DatabaseManager::getAccount()
{
    AccountInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, balance, updated_at FROM accounts LIMIT 1");

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        info.balance = query.value("balance").toDouble();
        info.updatedAt = query.value("updated_at").toString();
    }
    return info;
}

// 2. 更新余额
bool DatabaseManager::updateBalance(double balance)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE accounts SET balance = ?, updated_at = CURRENT_TIMESTAMP");
    query.addBindValue(balance);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// 3. 添加收入
bool DatabaseManager::addIncome(double amount, const QString& remark)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);

    // 更新余额
    query.prepare("UPDATE accounts SET balance = balance + ?, updated_at = CURRENT_TIMESTAMP");
    query.addBindValue(amount);
    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    // 记录流水
    query.prepare("INSERT INTO transactions (type, amount, remark) VALUES (1, ?, ?)");
    query.addBindValue(amount);
    query.addBindValue(remark);
    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// 4. 添加支出
bool DatabaseManager::addExpense(double amount, const QString& remark)
{
    if (!isConnected()) return false;

    m_db.transaction();
    QSqlQuery query(m_db);

    // 更新余额
    query.prepare("UPDATE accounts SET balance = balance - ?, updated_at = CURRENT_TIMESTAMP");
    query.addBindValue(amount);
    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    // 记录流水
    query.prepare("INSERT INTO transactions (type, amount, remark) VALUES (0, ?, ?)");
    query.addBindValue(amount);
    query.addBindValue(remark);
    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

// 5. 按ID查找流水
TransactionInfo DatabaseManager::getTransactionById(int transactionId)
{
    TransactionInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, type, amount, remark, created_at FROM transactions WHERE id = ?");
    query.addBindValue(transactionId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.type = query.value("type").toInt();
        info.amount = query.value("amount").toDouble();
        info.remark = query.value("remark").toString();
        info.createdAt = query.value("created_at").toString();
    }
    return info;
}

// 6. 获取所有流水列表
QList<TransactionInfo> DatabaseManager::getAllTransactions()
{
    QList<TransactionInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, type, amount, remark, created_at FROM transactions ORDER BY created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            TransactionInfo info;
            info.id = query.value("id").toInt();
            info.type = query.value("type").toInt();
            info.amount = query.value("amount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("created_at").toString();
            list.append(info);
        }
    }
    return list;
}

// 7. 按类型查找流水
QList<TransactionInfo> DatabaseManager::getTransactionsByType(int type)
{
    QList<TransactionInfo> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, type, amount, remark, created_at FROM transactions WHERE type = ? ORDER BY created_at DESC");
    query.addBindValue(type);

    if (query.exec()) {
        while (query.next()) {
            TransactionInfo info;
            info.id = query.value("id").toInt();
            info.type = query.value("type").toInt();
            info.amount = query.value("amount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("created_at").toString();
            list.append(info);
        }
    }
    return list;
}
