#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantMap>

struct UserInfo {
    int id = -1;
    QString username;
    QString email;
    QString phone;
    QString passwordHash;
    int role = 0;          // 0=普通用户，1=管理员
    bool isActive = true;
    QString createdAt;
};

struct ProductInfo {
    int id = -1;
    QString name;
    QString category;
    double purchasePrice = 0.0;
    double salePrice = 0.0;
    QString unit;
    QString remark;
    bool isActive = true;
    QString createdAt;
    QString updatedAt;
};

struct AddressInfo {
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
    QString createdAt;
    QString updatedAt;
};

struct InventoryInfo {
    int id = -1;
    int productId = -1;
    int quantity = 0;
    QString warehouseAddress;
    QString updatedAt;
};

struct PurchaseOrderInfo {
    int id = -1;
    double totalAmount = 0.0;
    QString remark;
    QString createdAt;
};

struct PurchaseOrderItemInfo {
    int id = -1;
    int orderId = -1;
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
    QString createdAt;
};

struct SalesOrderItemInfo {
    int id = -1;
    int orderId = -1;
    int productId = -1;
    int quantity = 0;
    double unitPrice = 0.0;
    double subtotal = 0.0;
};

struct AccountInfo {
    int id = -1;
    QString name;
    double balance = 0.0;
    QString updatedAt;
};

struct TransactionInfo {
    int id = -1;
    int type = 0;          // 0=支出，1=收入
    double amount = 0.0;
    QString remark;
    QString createdAt;
};

class DatabaseManager
{
public:
    // 单例访问
    static DatabaseManager& instance();
    // 在 class DatabaseManager 的 public: 块中添加：
    bool updateProductStock(int productId, int quantityChange);
    // 数据库连接
    bool connectToDatabase();
    bool connectToSQLite();
    void disconnectDatabase();
    bool isConnected() const;
    QString getLastError() const;

    // 用户查找
    UserInfo findUserByAccount(const QString& account);
    UserInfo getUserById(int userId);
    bool userExists(const QString& account);
    // 通过 ID 和 手机号 获取邮箱
    QString getEmailByIdAndPhone(int userId, const QString& phone);
    // 用户注册
    bool registerUser(const QString& username, const QString& email,
                      const QString& phone, const QString& passwordHash,
                      int role = 0, int* outUserId = nullptr);

    // 密码更新
    bool updatePassword(int userId, const QString& newPasswordHash);
    bool updatePassword(const QString& account, const QString& newPasswordHash);
    // 修改用户信息
    bool updateUserInfo(int userId, const QString& email, const QString& phone);
    // 精确删除用户
    bool deleteUserByAccount(const QString& account);
    // 更新最后登录时间
    bool updateLastLogin(int userId);
    // 商品查找
    ProductInfo getProductById(int productId);
    QList<ProductInfo> getProductsByName(const QString& name);
    // 添加商品
    bool addProduct(const QString& name, const QString& category,
                    double purchasePrice, double salePrice,
                    const QString& unit, const QString& remark,
                    int* outProductId = nullptr);
    // 修改商品信息
    bool updateProduct(int productId, const QString& name, const QString& category,
                       double purchasePrice, double salePrice,
                       const QString& unit, const QString& remark);
    // 软删除商品
    bool deleteProduct(int productId);

    // 地址查找
    AddressInfo getAddressById(int addressId);
    QList<AddressInfo> getAddressesByUserId(int userId);
    // 添加地址
    bool addAddress(int userId, const QString& name, const QString& phone,
                    const QString& province, const QString& city,
                    const QString& district, const QString& detail,
                    bool isDefault = false, int* outAddressId = nullptr);
    // 修改地址
    bool updateAddress(int addressId, const QString& name, const QString& phone,
                       const QString& province, const QString& city,
                       const QString& district, const QString& detail);
    // 软删除地址
    bool deleteAddress(int addressId);
    // 设置默认地址
    bool setDefaultAddress(int userId, int addressId);

    // 库存查找
    InventoryInfo getInventoryByProductId(int productId);
    QList<InventoryInfo> getAllInventory();
    // 添加库存
    bool addInventory(int productId, int quantity, int* outInventoryId = nullptr);
    // 修改库存数量
    bool updateQuantity(int inventoryId, int quantity);

    // 采购订单查找
    PurchaseOrderInfo getPurchaseOrderById(int orderId);
    QList<PurchaseOrderInfo> getAllPurchaseOrders();
    // 添加采购订单
    bool addPurchaseOrder(const QString& remark, int* outOrderId = nullptr);
    // 删除采购订单
    bool deletePurchaseOrder(int orderId);

    // 采购订单明细查找
    QList<PurchaseOrderItemInfo> getOrderItemsByOrderId(int orderId);
    // 添加采购订单明细
    bool addOrderItem(int orderId, int productId, int quantity, double unitPrice, int* outItemId = nullptr);
    // 修改采购订单明细
    bool updateOrderItem(int itemId, int quantity, double unitPrice);
    // 删除采购订单明细
    bool deleteOrderItem(int itemId);

    // 销售订单查找
    SalesOrderInfo getSalesOrderById(int orderId);
    QList<SalesOrderInfo> getSalesOrdersByUserId(int userId);
    QList<SalesOrderInfo> getAllSalesOrders();
    // 添加销售订单
    bool addSalesOrder(int userId, const QString& address, const QString& remark, int* outOrderId = nullptr, double totalAmount = 0);
    // 删除销售订单
    bool deleteSalesOrder(int orderId);

    // 销售订单明细查找
    QList<SalesOrderItemInfo> getSalesOrderItems(int orderId);
    // 添加销售订单明细
    bool addSalesOrderItem(int orderId, int productId, int quantity, double unitPrice, int* outItemId = nullptr);
    // 修改销售订单明细
    bool updateSalesOrderItem(int itemId, int quantity, double unitPrice);
    // 删除销售订单明细
    bool deleteSalesOrderItem(int itemId);

    // 账户查找
    AccountInfo getAccount();
    // 更新余额
    bool updateBalance(double balance);
    // 添加收入
    bool addIncome(double amount, const QString& remark);
    // 添加支出
    bool addExpense(double amount, const QString& remark);

    // 收支流水查找
    TransactionInfo getTransactionById(int transactionId);
    QList<TransactionInfo> getAllTransactions();
    QList<TransactionInfo> getTransactionsByType(int type);

    // 工具
    static QString hashSha256(const QString& input);

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initDatabase();

    // 内部 SQL 执行
    bool execute(const QString& sql, const QVariantList& params = QVariantList());
    QVariant getSingleValue(const QString& sql, const QVariantList& params = QVariantList());
    QVariantMap getSingleRecord(const QString& sql, const QVariantList& params = QVariantList());

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_connected;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H
