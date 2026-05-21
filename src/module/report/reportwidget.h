#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QTabWidget>

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(int userId, QWidget *parent = nullptr);
    ~ReportWidget();

signals:
    // 向后端请求数据的信号
    void refreshRequested();
    void filterByDateRequested(const QDateTime& startDate, const QDateTime& endDate);
    void exportReportRequested(const QString& format, const QDateTime& startDate, const QDateTime& endDate);

public slots:
    // 后端调用的槽
    void onSalesSummaryLoaded(double totalSales, double totalProfit, int orderCount);
    void onDailySalesLoaded(const QList<QPair<QString, double>>& dailySales);
    void onTopProductsLoaded(const QList<QVariantMap>& topProducts);
    void onSalesOrdersLoaded(const QList<QVariantMap>& orders);
    void onOperationError(const QString& error);

private slots:
    void onFilterClicked();
    void onExportExcel();
    void onExportPdf();
    void onRefreshClicked();
    void onTabChanged(int index);

private:
    void setupUI();
    void updateSummaryDisplay();

    QTabWidget* m_tabWidget;

    // 概览页
    QLabel* m_totalSalesLabel;
    QLabel* m_totalProfitLabel;
    QLabel* m_orderCountLabel;

    // 销售趋势页
    QTableWidget* m_dailySalesTable;

    // 热销商品页
    QTableWidget* m_topProductsTable;

    // 订单明细页
    QTableWidget* m_ordersTable;

    // 日期筛选
    QDateTimeEdit* m_startDateEdit;
    QDateTimeEdit* m_endDateEdit;
    QPushButton* m_filterBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_exportExcelBtn;
    QPushButton* m_exportPdfBtn;

    int m_userId;
};

#endif // REPORTWIDGET_H