#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QDateTime>
#include "desutil.h"

// 前向声明
class User;

// ========== 数据结构体 ==========

struct UserInfo {
    int id = -1;
    QString username;
    QString email;
    QString phone;
    QString passwordHash;
    int role = 0;
    bool isActive = true;
    QDateTime createdAt;
    QDateTime lastLogin;
};

struct AccountInfo {
    int id = -1;
    QString name;
    double balance = 0.0;
    QDateTime updatedAt;
};

struct BuyerAddressInfo {
    int id = -1;
    int userId = -1;
    QString name;
    QString phone;
    QString province;
    QString city;
    QString district;
    QString detail;
    bool isDefault = false;
    bool isActive = true;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct ProductInfo {
    int id = -1;
    QString name;
    QString category;
    double purchasePrice = 0.0;
    double salePrice = 0.0;
    QString unit = "件";
    QString remark;
    int quantity = 0;  // 库存量（从 inventory 表关联）
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct InventoryInfo {
    int id = -1;
    int productId = -1;
    int quantity = 0;
    double unitPrice = 0.0;
    double subtotal = 0.0;
};

struct SalesOrderInfo {
    int id = -1;
    int userId = -1;
    QString address;
    double totalAmount = 0.0;
    QString remark;
    QDateTime createdAt;
};

struct ContainsInfo {
    int id = -1;
    int orderId = -1;
    int productId = -1;
    int quantity = 0;
    double unitPrice = 0.0;
    double subtotal = 0.0;
};

struct TransactionInfo {
    int id = -1;
    int userId = -1;
    QString type;
    double amount = 0.0;
    double balance = 0.0;
    QString remark;
    QDateTime createTime;
};

// 商品销售统计
struct ProductSalesStat {
    int productId = -1;
    QString productName;
    int totalQuantity = 0;
    double totalAmount = 0.0;
};

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool connectToDatabase();
    void disconnectDatabase();
    bool isConnected() const;
    QString getLastError() const;
    QSqlDatabase getDatabase() const { return m_db; }

    bool createAllTables();

    // ========== 用户操作 ==========
    UserInfo findUserByAccount(const QString& account);
    bool userExists(const QString& username);
    int getUserIdByAccount(const QString& account);
    bool registerUser(const QString& username, const QString& email,
                      const QString& phone, const QString& passwordHash,
                      int role = 0, int* outUserId = nullptr);
    bool updatePassword(int userId, const QString& newPasswordHash);
    bool updateLastLogin(int userId);
    bool deleteUserByAccount(const QString& account);
    QList<UserInfo> getAllUsers();

    // ========== 账户操作 ==========
    AccountInfo getAccountByUserId(int userId);
    AccountInfo getAccountByUsername(const QString& username);
    bool updateBalance(int userId, double amount);
    bool createAccount(int userId, const QString& username);
    QList<TransactionInfo> getTransactionHistory(int userId, int limit = 50);

    // ========== 地址操作 ==========
    QList<BuyerAddressInfo> getAddressesByUserId(int userId);
    BuyerAddressInfo getAddressById(int addressId);
    bool addAddress(const BuyerAddressInfo& address);
    bool updateAddress(const BuyerAddressInfo& address);
    bool deleteAddress(int addressId);
    bool setDefaultAddress(int userId, int addressId);

    // ========== 商品操作 ==========
    QList<ProductInfo> getAllProducts();
    ProductInfo getProductById(int productId);
    bool addProduct(const ProductInfo& product);
    bool updateProduct(const ProductInfo& product);
    bool deleteProduct(int productId);
    QList<ProductInfo> searchProductsByName(const QString& name);
    bool updateProductStock(int productId, int quantity);

    // ========== 库存操作 ==========
    InventoryInfo getInventoryByProductId(int productId);
    bool updateInventory(int productId, int quantity, double unitPrice);
    bool addInventory(int productId, int quantity, double unitPrice);

    // ========== 订单操作 ==========
    int createSalesOrder(int userId, const QString& address, double totalAmount, const QString& remark);
    bool addOrderItem(int orderId, int productId, int quantity, double unitPrice, double subtotal);
    QList<SalesOrderInfo> getOrdersByUserId(int userId);
    QList<ContainsInfo> getOrderItems(int orderId);
    QList<SalesOrderInfo> getAllOrders(const QDateTime& startDate, const QDateTime& endDate);

    // ========== 报表操作 ==========
    double getTotalSales(const QDateTime& startDate, const QDateTime& endDate);
    double getTotalProfit(const QDateTime& startDate, const QDateTime& endDate);
    int getOrderCount(const QDateTime& startDate, const QDateTime& endDate);
    QList<SalesOrderInfo> getSalesReport(const QDateTime& startDate, const QDateTime& endDate);
    QList<ProductSalesStat> getProductSalesRanking(const QDateTime& startDate, const QDateTime& endDate, int limit = 10);
    QList<QPair<QDate, double>> getDailySalesReport(const QDateTime& startDate, const QDateTime& endDate);

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initDatabase();
    bool execute(const QString& sql, const QList<QVariant>& args = QList<QVariant>());
    QVariant getSingleValue(const QString& sql, const QList<QVariant>& args = QList<QVariant>());
    QVariantMap getSingleRecord(const QString& sql, const QList<QVariant>& args = QList<QVariant>());
    QList<QVariantMap> getRecords(const QString& sql, const QList<QVariant>& args = QList<QVariant>());

    QString encrypt(const QString& plainText);
    QString decrypt(const QString& cipherText);

    bool createUsersTable();
    bool createAccountsTable();
    bool createBuyerAddressesTable();
    bool createProductsTable();
    bool createInventoryTable();
    bool createSalesOrdersTable();
    bool createContainsTable();
    bool createTransactionsTable();

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_connected;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H