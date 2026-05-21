#ifndef INVENTORYWIDGET_H
#define INVENTORYWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include "common/databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryWidget; }
QT_END_NAMESPACE

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(QWidget *parent = nullptr);
    ~InventoryWidget();

    void setCurrentUser(int userId, const QString &username);

    // ========== 商品管理接口 ==========
    void refreshInventoryData();
    bool addProduct(const ProductInfo &product);
    bool updateProduct(int productId, const ProductInfo &product);
    bool deleteProduct(int productId);
    bool updateStock(int productId, int newQuantity);
    void searchProductByName(const QString &name);
    int getCurrentSelectedProductId() const;
    QList<ProductInfo> getAllProducts() const;  // ← 添加这行

    // ========== 统计接口 ==========
    double getMonthlyIncome(int year, int month);
    double getMonthlySales(int year, int month);
    double getMonthlyPurchase(int year, int month);
    double getMonthlyProfit(int year, int month);
    void refreshStatistics(int year, int month);

    // ========== 明细查询接口 ==========
    QList<SalesOrderInfo> getMonthlySalesDetails(int year, int month);
    QList<ProductSalesStat> getProductSalesRanking(int year, int month, int limit = 10);

private slots:
    void onSearchButtonClicked();
    void onRefreshButtonClicked();
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onAdjustButtonClicked();
    void onTableItemDoubleClicked(QTableWidgetItem *item);
    void onQueryStatsButtonClicked();

private:
    void setupTable();
    void setupDetailTables();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void updateTotalDisplay();  // ← 添加这行（如果不需要可以删除实现）

    void displayProducts(const QList<ProductInfo> &products);
    void addRowToTable(const ProductInfo &product, int row);
    ProductInfo getProductFromCurrentRow() const;
    void clearTable();

    void displaySalesDetails(const QList<SalesOrderInfo> &orders);
    void displayProductSalesRanking(const QList<ProductSalesStat> &stats);
    void addSalesRowToTable(const SalesOrderInfo &order, int row);
    void addProductSalesRowToTable(const ProductSalesStat &stat, int row, int rank);

    bool showAddProductDialog(ProductInfo &product);
    bool showEditProductDialog(ProductInfo &product);
    bool showStockAdjustDialog(int productId, int currentQuantity, int &newQuantity);
    void getCurrentYearMonth(int &year, int &month);

private:
    Ui::InventoryWidget *ui;
    int m_currentUserId;
    QString m_currentUsername;
    QList<ProductInfo> m_currentProducts;
    QList<SalesOrderInfo> m_currentSalesDetails;
    QList<ProductSalesStat> m_currentProductSalesRank;
};

#endif // INVENTORYWIDGET_H