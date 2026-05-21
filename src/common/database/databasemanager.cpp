#include "databasemanager.h"
#include <QCoreApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

// 唯一的连接名，确保单例连接不冲突
static const char* DB_CONNECTION_NAME = "forgot_password_connection";

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}
// 1. 单例模式实现：解决你 main.cpp 中的 undefined reference 报错
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager _instance;
    return _instance;
}

DatabaseManager::DatabaseManager() : m_connected(false)
{
    if (QSqlDatabase::contains(DB_CONNECTION_NAME)) {
        m_db = QSqlDatabase::database(DB_CONNECTION_NAME);
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION_NAME);
    }
}

// 2. 状态检查
bool DatabaseManager::isConnected() const
{
    return m_connected && m_db.isOpen();
}

QString DatabaseManager::getLastError() const
{
    return m_db.lastError().text();
}

// 3. 数据库连接：统一命名为 connectToDatabase
bool DatabaseManager::connectToDatabase()
{
    if (isConnected()) return true;

    m_dbPath = QCoreApplication::applicationDirPath() + "/password.db";
    m_db.setDatabaseName(m_dbPath);

    if (m_db.open()) {
        m_connected = true;
        // 开启 WAL 模式提高并发性能
        QSqlQuery walQuery(m_db);
        walQuery.exec("PRAGMA journal_mode=WAL");
        return initDatabase();
    }
    return false;
}

// 4. 初始化表结构
bool DatabaseManager::initDatabase()
{
    QFile sqlFile(QCoreApplication::applicationDirPath() + "/../src/common/sql/bill_sale.sql");
    if (!sqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "无法打开 SQL 文件";
        return false;
    }

    QString sql = QTextStream(&sqlFile).readAll();
    sqlFile.close();

    // 移除注释，按分号分割执行
    sql.replace(QRegularExpression("--[^\n]*"), "");
    for (const QString& stmt : sql.split(';', Qt::SkipEmptyParts)) {
        QString trimmed = stmt.simplified();
        if (!trimmed.isEmpty() && !execute(trimmed)) {
            qCritical() << "执行 SQL 失败:" << trimmed;
            return false;
        }
    }

    qDebug() << "数据库表初始化成功";
    return true;
}

// 5. 通用执行函数
bool DatabaseManager::execute(const QString& sql, const QList<QVariant>& args)
{
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : args) {
        query.addBindValue(arg);
    }
    if (!query.exec()) {
        qDebug() << "SQL Error:" << m_db.lastError().text();
        return false;
    }
    return true;
}
