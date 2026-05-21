

#include "databasemanager.h"
#include "hashsha.h"
#include <QCoreApplication>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

static const char* DB_CONNECTION_NAME = "BillandSale_connection";

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

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::isConnected() const
{
    return m_connected && m_db.isOpen();
}

QString DatabaseManager::getLastError() const
{
    return m_lastError;
}

bool DatabaseManager::connectToDatabase()
{
    if (isConnected()) return true;

    m_dbPath = QCoreApplication::applicationDirPath() + "/BillandSale.db";
    m_db.setDatabaseName(m_dbPath);

    if (m_db.open()) {
        m_connected = true;
        QSqlQuery walQuery(m_db);
        walQuery.exec("PRAGMA journal_mode=WAL");
        walQuery.exec("PRAGMA foreign_keys=ON");
        return createAllTables();
    }
    m_lastError = m_db.lastError().text();
    return false;
}

void DatabaseManager::disconnectDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_connected = false;
}

// ========== 加密/解密辅助 ==========

QString DatabaseManager::encrypt(const QString& plainText)
{
    if (plainText.isEmpty()) return plainText;
    return DESutil::encryptWithDefaultKey(plainText);
}

QString DatabaseManager::decrypt(const QString& cipherText)
{
    if (cipherText.isEmpty()) return cipherText;
    return DESutil::decryptWithDefaultKey(cipherText);
}

// ========== SQL 执行辅助 ==========

bool DatabaseManager::execute(const QString& sql, const QList<QVariant>& args)
{
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : args) {
        query.addBindValue(arg);
    }
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "SQL Error:" << m_lastError << sql;
        return false;
    }
    return true;
}

QVariant DatabaseManager::getSingleValue(const QString& sql, const QList<QVariant>& args)
{
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : args) {
        query.addBindValue(arg);
    }
    if (query.exec() && query.next()) {
        return query.value(0);
    }
    return QVariant();
}

QVariantMap DatabaseManager::getSingleRecord(const QString& sql, const QList<QVariant>& args)
{
    QVariantMap result;
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : args) {
        query.addBindValue(arg);
    }
    if (query.exec() && query.next()) {
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            result[record.fieldName(i)] = query.value(i);
        }
    }
    return result;
}

QList<QVariantMap> DatabaseManager::getRecords(const QString& sql, const QList<QVariant>& args)
{
    QList<QVariantMap> results;
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& arg : args) {
        query.addBindValue(arg);
    }
    if (query.exec()) {
        QSqlRecord record = query.record();
        while (query.next()) {
            QVariantMap row;
            for (int i = 0; i < record.count(); ++i) {
                row[record.fieldName(i)] = query.value(i);
            }
            results.append(row);
        }
    }
    return results;
}

// ========== 创建所有表 ==========

bool DatabaseManager::createAllTables()
{
    return createUsersTable() &&
           createAccountsTable() &&
           createBuyerAddressesTable() &&
           createProductsTable() &&
           createInventoryTable() &&
           createSalesOrdersTable() &&
           createContainsTable() &&
           createTransactionsTable();
}

bool DatabaseManager::createUsersTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT,
            phone TEXT,
            passwordHash TEXT NOT NULL,
            role INTEGER DEFAULT 0,
            isActive INTEGER DEFAULT 1,
            createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,
            lastLogin DATETIME
        )
    )";
    if (!execute(sql)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)");
    return true;
}

bool DatabaseManager::createAccountsTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL,
            balance REAL DEFAULT 0,
            updatedAt DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    return execute(sql);
}

bool DatabaseManager::createBuyerAddressesTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS buyerAddresses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userId INTEGER NOT NULL,
            name TEXT NOT NULL,
            phone TEXT NOT NULL,
            province TEXT NOT NULL,
            city TEXT NOT NULL,
            district TEXT,
            detail TEXT NOT NULL,
            isDefault INTEGER DEFAULT 0,
            isActive INTEGER DEFAULT 1,
            createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,
            updatedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (userId) REFERENCES users(id) ON DELETE CASCADE
        )
    )";
    if (!execute(sql)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_addresses_userId ON buyerAddresses(userId)");
    return true;
}

bool DatabaseManager::createProductsTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS products (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            category TEXT,
            purchasePrice REAL DEFAULT 0,
            salePrice REAL DEFAULT 0,
            unit TEXT DEFAULT '件',
            remark TEXT,
            createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,
            updatedAt DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    return execute(sql);
}

bool DatabaseManager::createInventoryTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS inventory (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            productId INTEGER NOT NULL UNIQUE,
            quantity INTEGER DEFAULT 0,
            unitPrice REAL DEFAULT 0,
            subtotal REAL DEFAULT 0,
            FOREIGN KEY (productId) REFERENCES products(id) ON DELETE CASCADE
        )
    )";
    if (!execute(sql)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_inventory_productId ON inventory(productId)");
    return true;
}

bool DatabaseManager::createSalesOrdersTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS salesOrders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userId INTEGER NOT NULL,
            address TEXT NOT NULL,
            totalAmount REAL DEFAULT 0,
            remark TEXT,
            createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (userId) REFERENCES users(id) ON DELETE CASCADE
        )
    )";
    if (!execute(sql)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_orders_userId ON salesOrders(userId)");
    execute("CREATE INDEX IF NOT EXISTS idx_orders_createdAt ON salesOrders(createdAt)");
    return true;
}

bool DatabaseManager::createContainsTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS contains (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            orderId INTEGER NOT NULL,
            productId INTEGER NOT NULL,
            quantity INTEGER DEFAULT 0,
            unitPrice REAL DEFAULT 0,
            subtotal REAL DEFAULT 0,
            FOREIGN KEY (orderId) REFERENCES salesOrders(id) ON DELETE CASCADE,
            FOREIGN KEY (productId) REFERENCES products(id) ON DELETE CASCADE
        )
    )";
    if (!execute(sql)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_contains_orderId ON contains(orderId)");
    return true;
}

bool DatabaseManager::createTransactionsTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userId INTEGER NOT NULL,
            type TEXT NOT NULL,
            amount REAL NOT NULL,
            balance REAL NOT NULL,
            remark TEXT,
            createTime DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (userId) REFERENCES users(id) ON DELETE CASCADE
        )
    )";
    if (!execute(sql)) return false;
    execute("CREATE INDEX IF NOT EXISTS idx_transactions_userId ON transactions(userId)");
    execute("CREATE INDEX IF NOT EXISTS idx_transactions_createTime ON transactions(createTime)");
    return true;
}

// ========== 用户操作实现 ==========

UserInfo DatabaseManager::findUserByAccount(const QString& account)
{
    UserInfo info;
    QString encryptedAccount = encrypt(account.trimmed());

    QSqlQuery query(m_db);
    query.prepare("SELECT id, username, email, phone, passwordHash, role, isActive, createdAt, lastLogin "
                  "FROM users WHERE username = ? OR email = ? OR phone = ?");
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.username = decrypt(query.value("username").toString());
        info.email = decrypt(query.value("email").toString());
        info.phone = decrypt(query.value("phone").toString());
        info.passwordHash = query.value("passwordHash").toString();
        info.role = query.value("role").toInt();
        info.isActive = query.value("isActive").toBool();
        info.createdAt = query.value("createdAt").toDateTime();
        info.lastLogin = query.value("lastLogin").toDateTime();
    }
    return info;
}

bool DatabaseManager::userExists(const QString& username)
{
    QString encryptedUsername = encrypt(username);
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM users WHERE username = ?");
    query.addBindValue(encryptedUsername);
    return query.exec() && query.next();
}

int DatabaseManager::getUserIdByAccount(const QString& account)
{
    QString encryptedAccount = encrypt(account);
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE username = ? OR email = ? OR phone = ?");
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);
    query.addBindValue(encryptedAccount);
    if (query.exec() && query.next()) {
        return query.value("id").toInt();
    }
    return -1;
}

bool DatabaseManager::registerUser(const QString& username, const QString& email,
                                   const QString& phone, const QString& passwordHash,
                                   int role, int* outUserId)
{
    if (!isConnected()) return false;

    QString encryptedUsername = encrypt(username);
    QString encryptedEmail = encrypt(email);
    QString encryptedPhone = encrypt(phone);

    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, email, phone, passwordHash, role, isActive) "
                  "VALUES (?, ?, ?, ?, ?, 1)");
    query.addBindValue(encryptedUsername);
    query.addBindValue(encryptedEmail);
    query.addBindValue(encryptedPhone);
    query.addBindValue(passwordHash);
    query.addBindValue(role);

    if (!query.exec()) {
        m_db.rollback();
        m_lastError = query.lastError().text();
        qDebug() << "注册用户失败:" << m_lastError;
        return false;
    }

    int userId = query.lastInsertId().toInt();
    if (outUserId) *outUserId = userId;

    if (!createAccount(userId, username)) {
        m_db.rollback();
        return false;
    }

    m_db.commit();
    return true;
}

bool DatabaseManager::updatePassword(int userId, const QString& newPasswordHash)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET passwordHash = ? WHERE id = ?");
    query.addBindValue(newPasswordHash);
    query.addBindValue(userId);
    return query.exec();
}

bool DatabaseManager::updateLastLogin(int userId)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET lastLogin = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(userId);
    return query.exec();
}

bool DatabaseManager::deleteUserByAccount(const QString& account)
{
    QString encryptedAccount = encrypt(account);
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE username = ?");
    query.addBindValue(encryptedAccount);
    return query.exec();
}

QList<UserInfo> DatabaseManager::getAllUsers()
{
    QList<UserInfo> users;
    QSqlQuery query("SELECT id, username, email, phone, passwordHash, role, isActive, createdAt, lastLogin FROM users");

    if (query.exec()) {
        while (query.next()) {
            UserInfo info;
            info.id = query.value("id").toInt();
            info.username = decrypt(query.value("username").toString());
            info.email = decrypt(query.value("email").toString());
            info.phone = decrypt(query.value("phone").toString());
            info.passwordHash = query.value("passwordHash").toString();
            info.role = query.value("role").toInt();
            info.isActive = query.value("isActive").toBool();
            info.createdAt = query.value("createdAt").toDateTime();
            info.lastLogin = query.value("lastLogin").toDateTime();
            users.append(info);
        }
    }
    return users;
}

// ========== 账户操作实现 ==========

AccountInfo DatabaseManager::getAccountByUserId(int userId)
{
    AccountInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, balance, updatedAt FROM accounts "
                  "WHERE name = (SELECT username FROM users WHERE id = ?)");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.name = decrypt(query.value("name").toString());
        info.balance = query.value("balance").toDouble();
        info.updatedAt = query.value("updatedAt").toDateTime();
    }
    return info;
}

AccountInfo DatabaseManager::getAccountByUsername(const QString& username)
{
    AccountInfo info;
    QString encryptedName = encrypt(username);
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, balance, updatedAt FROM accounts WHERE name = ?");
    query.addBindValue(encryptedName);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.name = decrypt(query.value("name").toString());
        info.balance = query.value("balance").toDouble();
        info.updatedAt = query.value("updatedAt").toDateTime();
    }
    return info;
}

bool DatabaseManager::updateBalance(int userId, double amount)
{
    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare("UPDATE accounts SET balance = balance + ?, updatedAt = CURRENT_TIMESTAMP "
                  "WHERE name = (SELECT username FROM users WHERE id = ?)");
    query.addBindValue(amount);
    query.addBindValue(userId);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    // 记录交易
    double newBalance = getAccountByUserId(userId).balance;
    QSqlQuery transQuery(m_db);
    transQuery.prepare("INSERT INTO transactions (userId, type, amount, balance, remark) "
                       "VALUES (?, ?, ?, ?, ?)");
    transQuery.addBindValue(userId);
    transQuery.addBindValue(amount > 0 ? "recharge" : (amount < 0 ? "consume" : "adjust"));
    transQuery.addBindValue(amount);
    transQuery.addBindValue(newBalance);
    transQuery.addBindValue(amount > 0 ? "在线充值" : "订单消费");
    transQuery.exec();

    m_db.commit();
    return true;
}

bool DatabaseManager::createAccount(int userId, const QString& username)
{
    QString encryptedName = encrypt(username);
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO accounts (name, balance) VALUES (?, 0)");
    query.addBindValue(encryptedName);
    return query.exec();
}

QList<TransactionInfo> DatabaseManager::getTransactionHistory(int userId, int limit)
{
    QList<TransactionInfo> transactions;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, type, amount, balance, remark, createTime "
                  "FROM transactions WHERE userId = ? ORDER BY createTime DESC LIMIT ?");
    query.addBindValue(userId);
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            TransactionInfo info;
            info.id = query.value("id").toInt();
            info.userId = userId;
            info.type = query.value("type").toString();
            info.amount = query.value("amount").toDouble();
            info.balance = query.value("balance").toDouble();
            info.remark = query.value("remark").toString();
            info.createTime = query.value("createTime").toDateTime();
            transactions.append(info);
        }
    }
    return transactions;
}

// ========== 地址操作实现 ==========

QList<BuyerAddressInfo> DatabaseManager::getAddressesByUserId(int userId)
{
    QList<BuyerAddressInfo> addresses;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, phone, province, city, district, detail, isDefault, isActive, createdAt, updatedAt "
                  "FROM buyerAddresses WHERE userId = ? AND isActive = 1 ORDER BY isDefault DESC, id DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            BuyerAddressInfo info;
            info.id = query.value("id").toInt();
            info.userId = userId;
            info.name = decrypt(query.value("name").toString());
            info.phone = decrypt(query.value("phone").toString());
            info.province = query.value("province").toString();
            info.city = query.value("city").toString();
            info.district = query.value("district").toString();
            info.detail = decrypt(query.value("detail").toString());
            info.isDefault = query.value("isDefault").toBool();
            info.isActive = query.value("isActive").toBool();
            info.createdAt = query.value("createdAt").toDateTime();
            info.updatedAt = query.value("updatedAt").toDateTime();
            addresses.append(info);
        }
    }
    return addresses;
}

BuyerAddressInfo DatabaseManager::getAddressById(int addressId)
{
    BuyerAddressInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT userId, name, phone, province, city, district, detail, isDefault "
                  "FROM buyerAddresses WHERE id = ?");
    query.addBindValue(addressId);

    if (query.exec() && query.next()) {
        info.id = addressId;
        info.userId = query.value("userId").toInt();
        info.name = decrypt(query.value("name").toString());
        info.phone = decrypt(query.value("phone").toString());
        info.province = query.value("province").toString();
        info.city = query.value("city").toString();
        info.district = query.value("district").toString();
        info.detail = decrypt(query.value("detail").toString());
        info.isDefault = query.value("isDefault").toBool();
    }
    return info;
}

bool DatabaseManager::addAddress(const BuyerAddressInfo& address)
{
    m_db.transaction();

    // 如果是默认地址，先取消其他默认
    if (address.isDefault) {
        QSqlQuery clearQuery(m_db);
        clearQuery.prepare("UPDATE buyerAddresses SET isDefault = 0 WHERE userId = ?");
        clearQuery.addBindValue(address.userId);
        clearQuery.exec();
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO buyerAddresses (userId, name, phone, province, city, district, detail, isDefault) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(address.userId);
    query.addBindValue(encrypt(address.name));
    query.addBindValue(encrypt(address.phone));
    query.addBindValue(address.province);
    query.addBindValue(address.city);
    query.addBindValue(address.district);
    query.addBindValue(encrypt(address.detail));
    query.addBindValue(address.isDefault ? 1 : 0);

    bool success = query.exec();
    if (success) {
        m_db.commit();
    } else {
        m_db.rollback();
        m_lastError = query.lastError().text();
    }
    return success;
}

bool DatabaseManager::updateAddress(const BuyerAddressInfo& address)
{
    m_db.transaction();

    if (address.isDefault) {
        QSqlQuery clearQuery(m_db);
        clearQuery.prepare("UPDATE buyerAddresses SET isDefault = 0 WHERE userId = ?");
        clearQuery.addBindValue(address.userId);
        clearQuery.exec();
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE buyerAddresses SET name = ?, phone = ?, province = ?, city = ?, district = ?, detail = ?, isDefault = ?, updatedAt = CURRENT_TIMESTAMP "
                  "WHERE id = ?");
    query.addBindValue(encrypt(address.name));
    query.addBindValue(encrypt(address.phone));
    query.addBindValue(address.province);
    query.addBindValue(address.city);
    query.addBindValue(address.district);
    query.addBindValue(encrypt(address.detail));
    query.addBindValue(address.isDefault ? 1 : 0);
    query.addBindValue(address.id);

    bool success = query.exec();
    if (success) {
        m_db.commit();
    } else {
        m_db.rollback();
    }
    return success;
}

bool DatabaseManager::deleteAddress(int addressId)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE buyerAddresses SET isActive = 0, updatedAt = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(addressId);
    return query.exec();
}

bool DatabaseManager::setDefaultAddress(int userId, int addressId)
{
    m_db.transaction();

    QSqlQuery clearQuery(m_db);
    clearQuery.prepare("UPDATE buyerAddresses SET isDefault = 0 WHERE userId = ?");
    clearQuery.addBindValue(userId);
    clearQuery.exec();

    QSqlQuery setQuery(m_db);
    setQuery.prepare("UPDATE buyerAddresses SET isDefault = 1 WHERE id = ?");
    setQuery.addBindValue(addressId);

    bool success = setQuery.exec();
    if (success) {
        m_db.commit();
    } else {
        m_db.rollback();
    }
    return success;
}

// ========== 商品操作实现 ==========

QList<ProductInfo> DatabaseManager::getAllProducts()
{
    QList<ProductInfo> products;
    QSqlQuery query("SELECT id, name, category, purchasePrice, salePrice, unit, remark, createdAt, updatedAt FROM products");

    if (query.exec()) {
        while (query.next()) {
            ProductInfo info;
            info.id = query.value("id").toInt();
            info.name = query.value("name").toString();
            info.category = query.value("category").toString();
            info.purchasePrice = query.value("purchasePrice").toDouble();
            info.salePrice = query.value("salePrice").toDouble();
            info.unit = query.value("unit").toString();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("createdAt").toDateTime();
            info.updatedAt = query.value("updatedAt").toDateTime();
            products.append(info);
        }
    }
    return products;
}

ProductInfo DatabaseManager::getProductById(int productId)
{
    ProductInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT name, category, purchasePrice, salePrice, unit, remark FROM products WHERE id = ?");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        info.id = productId;
        info.name = query.value("name").toString();
        info.category = query.value("category").toString();
        info.purchasePrice = query.value("purchasePrice").toDouble();
        info.salePrice = query.value("salePrice").toDouble();
        info.unit = query.value("unit").toString();
        info.remark = query.value("remark").toString();
    }
    return info;
}

bool DatabaseManager::addProduct(const ProductInfo& product)
{
    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO products (name, category, purchasePrice, salePrice, unit, remark) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(product.name);
    query.addBindValue(product.category);
    query.addBindValue(product.purchasePrice);
    query.addBindValue(product.salePrice);
    query.addBindValue(product.unit);
    query.addBindValue(product.remark);

    if (!query.exec()) {
        m_db.rollback();
        return false;
    }

    int productId = query.lastInsertId().toInt();

    // 初始化库存
    QSqlQuery invQuery(m_db);
    invQuery.prepare("INSERT INTO inventory (productId, quantity, unitPrice, subtotal) VALUES (?, 0, ?, 0)");
    invQuery.addBindValue(productId);
    invQuery.addBindValue(product.purchasePrice);
    invQuery.exec();

    m_db.commit();
    return true;
}

bool DatabaseManager::updateProduct(const ProductInfo& product)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE products SET name = ?, category = ?, purchasePrice = ?, salePrice = ?, unit = ?, remark = ?, updatedAt = CURRENT_TIMESTAMP "
                  "WHERE id = ?");
    query.addBindValue(product.name);
    query.addBindValue(product.category);
    query.addBindValue(product.purchasePrice);
    query.addBindValue(product.salePrice);
    query.addBindValue(product.unit);
    query.addBindValue(product.remark);
    query.addBindValue(product.id);
    return query.exec();
}

bool DatabaseManager::deleteProduct(int productId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM products WHERE id = ?");
    query.addBindValue(productId);
    return query.exec();
}

QList<ProductInfo> DatabaseManager::searchProductsByName(const QString& name)
{
    QList<ProductInfo> products;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, category, purchasePrice, salePrice, unit, remark FROM products WHERE name LIKE ?");
    query.addBindValue("%" + name + "%");

    if (query.exec()) {
        while (query.next()) {
            ProductInfo info;
            info.id = query.value("id").toInt();
            info.name = query.value("name").toString();
            info.category = query.value("category").toString();
            info.purchasePrice = query.value("purchasePrice").toDouble();
            info.salePrice = query.value("salePrice").toDouble();
            info.unit = query.value("unit").toString();
            info.remark = query.value("remark").toString();
            products.append(info);
        }
    }
    return products;
}

bool DatabaseManager::updateProductStock(int productId, int quantity)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE inventory SET quantity = quantity - ?, subtotal = quantity * unitPrice WHERE productId = ?");
    query.addBindValue(quantity);
    query.addBindValue(productId);
    return query.exec();
}

// ========== 库存操作实现 ==========

InventoryInfo DatabaseManager::getInventoryByProductId(int productId)
{
    InventoryInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, quantity, unitPrice, subtotal FROM inventory WHERE productId = ?");
    query.addBindValue(productId);

    if (query.exec() && query.next()) {
        info.id = query.value("id").toInt();
        info.productId = productId;
        info.quantity = query.value("quantity").toInt();
        info.unitPrice = query.value("unitPrice").toDouble();
        info.subtotal = query.value("subtotal").toDouble();
    }
    return info;
}

bool DatabaseManager::updateInventory(int productId, int quantity, double unitPrice)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE inventory SET quantity = ?, unitPrice = ?, subtotal = quantity * unitPrice WHERE productId = ?");
    query.addBindValue(quantity);
    query.addBindValue(unitPrice);
    query.addBindValue(productId);
    return query.exec();
}

bool DatabaseManager::addInventory(int productId, int quantity, double unitPrice)
{
    InventoryInfo existing = getInventoryByProductId(productId);
    if (existing.id > 0) {
        return updateInventory(productId, existing.quantity + quantity, unitPrice);
    } else {
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO inventory (productId, quantity, unitPrice, subtotal) VALUES (?, ?, ?, ?)");
        query.addBindValue(productId);
        query.addBindValue(quantity);
        query.addBindValue(unitPrice);
        query.addBindValue(quantity * unitPrice);
        return query.exec();
    }
}

// ========== 订单操作实现 ==========

int DatabaseManager::createSalesOrder(int userId, const QString& address, double totalAmount, const QString& remark)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO salesOrders (userId, address, totalAmount, remark, createdAt) "
                  "VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)");
    query.addBindValue(userId);
    query.addBindValue(encrypt(address));
    query.addBindValue(totalAmount);
    query.addBindValue(remark);

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }
    return -1;
}

bool DatabaseManager::addOrderItem(int orderId, int productId, int quantity, double unitPrice, double subtotal)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO contains (orderId, productId, quantity, unitPrice, subtotal) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(orderId);
    query.addBindValue(productId);
    query.addBindValue(quantity);
    query.addBindValue(unitPrice);
    query.addBindValue(subtotal);
    return query.exec();
}

QList<SalesOrderInfo> DatabaseManager::getOrdersByUserId(int userId)
{
    QList<SalesOrderInfo> orders;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, address, totalAmount, remark, createdAt FROM salesOrders WHERE userId = ? ORDER BY createdAt DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            SalesOrderInfo info;
            info.id = query.value("id").toInt();
            info.userId = userId;
            info.address = decrypt(query.value("address").toString());
            info.totalAmount = query.value("totalAmount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("createdAt").toDateTime();
            orders.append(info);
        }
    }
    return orders;
}

QList<ContainsInfo> DatabaseManager::getOrderItems(int orderId)
{
    QList<ContainsInfo> items;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, productId, quantity, unitPrice, subtotal FROM contains WHERE orderId = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            ContainsInfo info;
            info.id = query.value("id").toInt();
            info.orderId = orderId;
            info.productId = query.value("productId").toInt();
            info.quantity = query.value("quantity").toInt();
            info.unitPrice = query.value("unitPrice").toDouble();
            info.subtotal = query.value("subtotal").toDouble();
            items.append(info);
        }
    }
    return items;
}

QList<SalesOrderInfo> DatabaseManager::getAllOrders(const QDateTime& startDate, const QDateTime& endDate)
{
    QList<SalesOrderInfo> orders;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, userId, address, totalAmount, remark, createdAt FROM salesOrders "
                  "WHERE createdAt BETWEEN ? AND ? ORDER BY createdAt DESC");
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (query.exec()) {
        while (query.next()) {
            SalesOrderInfo info;
            info.id = query.value("id").toInt();
            info.userId = query.value("userId").toInt();
            info.address = decrypt(query.value("address").toString());
            info.totalAmount = query.value("totalAmount").toDouble();
            info.remark = query.value("remark").toString();
            info.createdAt = query.value("createdAt").toDateTime();
            orders.append(info);
        }
    }
    return orders;
}

// ========== 报表操作实现 ==========

double DatabaseManager::getTotalSales(const QDateTime& startDate, const QDateTime& endDate)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COALESCE(SUM(totalAmount), 0) FROM salesOrders WHERE createdAt BETWEEN ? AND ?");
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double DatabaseManager::getTotalProfit(const QDateTime& startDate, const QDateTime& endDate)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COALESCE(SUM(subtotal - (quantity * (SELECT purchasePrice FROM products WHERE id = productId))), 0) "
                  "FROM contains WHERE orderId IN (SELECT id FROM salesOrders WHERE createdAt BETWEEN ? AND ?)");
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

int DatabaseManager::getOrderCount(const QDateTime& startDate, const QDateTime& endDate)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM salesOrders WHERE createdAt BETWEEN ? AND ?");
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QList<SalesOrderInfo> DatabaseManager::getSalesReport(const QDateTime& startDate, const QDateTime& endDate)
{
    return getAllOrders(startDate, endDate);
}

QList<ProductSalesStat> DatabaseManager::getProductSalesRanking(const QDateTime& startDate, const QDateTime& endDate, int limit)
{
    QList<ProductSalesStat> stats;
    QSqlQuery query(m_db);
    query.prepare("SELECT p.id, p.name, SUM(c.quantity) as totalQuantity, SUM(c.subtotal) as totalAmount "
                  "FROM products p "
                  "JOIN contains c ON p.id = c.productId "
                  "JOIN salesOrders o ON c.orderId = o.id "
                  "WHERE o.createdAt BETWEEN ? AND ? "
                  "GROUP BY p.id ORDER BY totalAmount DESC LIMIT ?");
    query.addBindValue(startDate);
    query.addBindValue(endDate);
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            ProductSalesStat stat;
            stat.productId = query.value("id").toInt();
            stat.productName = query.value("name").toString();
            stat.totalQuantity = query.value("totalQuantity").toInt();
            stat.totalAmount = query.value("totalAmount").toDouble();
            stats.append(stat);
        }
    }
    return stats;
}
QList<QPair<QDate, double>> DatabaseManager::getDailySalesReport(const QDateTime& startDate, const QDateTime& endDate)
{
    QList<QPair<QDate, double>> report;
    QSqlQuery query(m_db);
    query.prepare("SELECT DATE(createdAt) as date, COALESCE(SUM(totalAmount), 0) as total "
                  "FROM salesOrders WHERE createdAt BETWEEN ? AND ? GROUP BY DATE(createdAt) ORDER BY date");
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (query.exec()) {
        while (query.next()) {
            QDate date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
            double total = query.value("total").toDouble();
            report.append(qMakePair(date, total));
        }
    }
    return report;
}