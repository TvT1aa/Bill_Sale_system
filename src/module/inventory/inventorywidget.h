#ifndef INVENTORYWIDGET_H
#define INVENTORYWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryWidget; }
QT_END_NAMESPACE

// 商品数据结构
struct ProductInfo {
    int id;
    QString name;
    int quantity;
    int warningLevel;
    double purchasePrice;
    double salePrice;
    QString lastUpdate;
};

// 销售记录结构体
struct SalesRecord {
    int id;
    QString productName;
    int quantity;
    double price;
    double total;
    QString customer;
    QString saleTime;
};

// 进货记录结构体
struct PurchaseRecord {
    int id;
    QString productName;
    int quantity;
    double price;
    double total;
    QString supplier;
    QString purchaseTime;
};

// 商品销售统计
struct ProductSalesStat {
    int productId;
    QString productName;
    int totalQuantity;
    double totalAmount;
};

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);
    ~InventoryWidget();

    // ========== 商品管理接口 ==========
    void refreshInventoryData();
    bool addProduct(const ProductInfo &product);
    bool updateProduct(int productId, const ProductInfo &product);
    bool deleteProduct(int productId);
    bool updateStock(int productId, int newQuantity);
    void searchProductByName(const QString &name);
    int getCurrentSelectedProductId() const;
    QList<ProductInfo> getAllProducts() const;

    // ========== 统计接口 ==========
    double getMonthlyIncome(int year, int month);      // 月收入 = 销售额 - 进货额
    double getMonthlySales(int year, int month);       // 月销售额
    double getMonthlyPurchase(int year, int month);    // 月进货额
    double getMonthlyProfit(int year, int month);      // 月利润
    void refreshStatistics(int year, int month);       // 刷新所有统计

    // ========== 明细查询接口 ==========
    QList<SalesRecord> getMonthlySalesDetails(int year, int month);
    QList<PurchaseRecord> getMonthlyPurchaseDetails(int year, int month);
    QList<ProductSalesStat> getProductSalesRanking(int year, int month, int limit = 10);

private slots:
    // ========== UI 槽函数 ==========
    void onSearchButtonClicked();
    void onRefreshButtonClicked();
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onAdjustButtonClicked();
    void onTableItemDoubleClicked(QTableWidgetItem *item);
    void onQueryStatsButtonClicked();

private:
    // ========== 内部辅助函数 ==========
    void setupTable();
    void setupDetailTables();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);

    // 表格操作
    void displayProducts(const QList<ProductInfo> &products);
    void addRowToTable(const ProductInfo &product, int row);
    ProductInfo getProductFromCurrentRow() const;
    void clearTable();

    // 明细表格显示
    void displaySalesDetails(const QList<SalesRecord> &records);
    void displayPurchaseDetails(const QList<PurchaseRecord> &records);
    void displayProductSalesRanking(const QList<ProductSalesStat> &stats);

    // 对话框
    bool showAddProductDialog(ProductInfo &product);
    bool showEditProductDialog(ProductInfo &product);
    bool showStockAdjustDialog(int productId, int currentQuantity, int &newQuantity);

    // 获取当前选中的年月
    void getCurrentYearMonth(int &year, int &month);

private:
    Ui::InventoryWidget *ui;
    QList<ProductInfo> m_currentProducts;
    QList<SalesRecord> m_currentSalesDetails;
    QList<PurchaseRecord> m_currentPurchaseDetails;
    QList<ProductSalesStat> m_currentProductSalesRank;
};

#endif // INVENTORYWIDGET_H