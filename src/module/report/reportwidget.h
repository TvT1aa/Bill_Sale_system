#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QDateTime>
#include <QPair>
#include "common/databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ReportWidget; }
QT_END_NAMESPACE

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(QWidget *parent = nullptr);
    ~ReportWidget();

    void refreshReports(const QDateTime &startDate, const QDateTime &endDate);
    bool exportToCSV(const QString &filePath);

private slots:
    void onQueryButtonClicked();
    void onExportButtonClicked();

private:
    void setupTables();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);

    void displaySalesReport(const QList<SalesOrderInfo> &orders, double total);
    void displayProductSalesRanking(const QList<ProductSalesStat> &stats);
    void displayDailySalesReport(const QList<QPair<QDate, double>> &report);

    void addSalesRowToTable(const SalesOrderInfo &order, int row);
    void addProductSalesRowToTable(const ProductSalesStat &stat, int row, int rank);
    void addDailySalesRowToTable(const QPair<QDate, double> &data, int row);

    void updateStatisticsCards(const QDateTime &startDate, const QDateTime &endDate);

private:
    Ui::ReportWidget *ui;
    QList<SalesOrderInfo> m_currentSalesReport;
    QList<ProductSalesStat> m_currentProductRanking;
    QList<QPair<QDate, double>> m_currentDailyReport;
};

#endif // REPORTWIDGET_H