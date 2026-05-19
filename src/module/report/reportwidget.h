#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H
#include "module/inventory/inventorywidget.h"
#include <QWidget>
#include <QTableWidgetItem>
#include <QDate>

QT_BEGIN_NAMESPACE
namespace Ui { class ReportWidget; }
QT_END_NAMESPACE

// 销售记录结构体
struct SalesReportItem {
    int id;
    QString orderNo;
    QString productName;
    int quantity;
    double price;
    double total;
    QString customer;
    QString saleTime;
};

// 日报表结构体
struct DailySalesStat {
    QDate date;
    int orderCount;
    double totalSales;
    double totalCost;
    double profit;
};

// 注意：ProductSalesStat 已在 inventorywidget.h 中定义，这里不再重复

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(QWidget *parent = nullptr);
    ~ReportWidget();

    // ========== 统计接口 ==========
    QList<SalesReportItem> getSalesReport(const QDate &startDate, const QDate &endDate);
    QList<ProductSalesStat> getProductSalesRanking(const QDate &startDate, const QDate &endDate, int limit = 20);
    QList<DailySalesStat> getDailySalesReport(const QDate &startDate, const QDate &endDate);

    double getTotalSales(const QDate &startDate, const QDate &endDate);
    double getTotalCost(const QDate &startDate, const QDate &endDate);
    double getTotalProfit(const QDate &startDate, const QDate &endDate);
    int getOrderCount(const QDate &startDate, const QDate &endDate);

    void refreshReports(const QDate &startDate, const QDate &endDate);
    bool exportToCSV(const QString &filePath);

private slots:
    void onQueryButtonClicked();
    void onExportButtonClicked();

private:
    void setupTables();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);

    void displaySalesReport(const QList<SalesReportItem> &items, double total);
    void displayProductSalesRanking(const QList<ProductSalesStat> &stats);
    void displayDailySalesReport(const QList<DailySalesStat> &stats);

    void addSalesRowToTable(const SalesReportItem &item, int row);
    void addProductSalesRowToTable(const ProductSalesStat &stat, int row, int rank);
    void addDailySalesRowToTable(const DailySalesStat &stat, int row);

    void updateStatisticsCards(const QDate &startDate, const QDate &endDate);

private:
    Ui::ReportWidget *ui;
    QList<SalesReportItem> m_currentSalesReport;
    QList<ProductSalesStat> m_currentProductRanking;
    QList<DailySalesStat> m_currentDailyReport;
};

#endif // REPORTWIDGET_H