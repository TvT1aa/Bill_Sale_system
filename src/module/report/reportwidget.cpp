#include "reportwidget.h"
#include "ui_reportwidget.h"
#include "common/databasemanager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

ReportWidget::ReportWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReportWidget)
{
    ui->setupUi(this);
    setupTables();

    connect(ui->queryButton, &QPushButton::clicked, this, &ReportWidget::onQueryButtonClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &ReportWidget::onExportButtonClicked);

    QDateTime endDate = QDateTime::currentDateTime();
    QDateTime startDate = endDate.addDays(-30);
    ui->startDateEdit->setDateTime(startDate);
    ui->endDateEdit->setDateTime(endDate);

    refreshReports(startDate, endDate);
    updateStatus("就绪");
}

ReportWidget::~ReportWidget()
{
    delete ui;
}

void ReportWidget::setupTables()
{
    QStringList salesHeaders = {"订单号", "用户ID", "总金额", "地址", "备注", "下单时间"};
    ui->salesTable->setColumnCount(salesHeaders.size());
    ui->salesTable->setHorizontalHeaderLabels(salesHeaders);
    ui->salesTable->setColumnWidth(0, 100);
    ui->salesTable->setColumnWidth(1, 80);
    ui->salesTable->setColumnWidth(2, 100);
    ui->salesTable->setColumnWidth(3, 200);
    ui->salesTable->setColumnWidth(4, 150);
    ui->salesTable->setColumnWidth(5, 150);

    QStringList rankHeaders = {"排名", "商品ID", "商品名称", "销售数量", "销售金额"};
    ui->productSalesTable->setColumnCount(rankHeaders.size());
    ui->productSalesTable->setHorizontalHeaderLabels(rankHeaders);
    ui->productSalesTable->setColumnWidth(0, 60);
    ui->productSalesTable->setColumnWidth(1, 80);
    ui->productSalesTable->setColumnWidth(2, 200);
    ui->productSalesTable->setColumnWidth(3, 100);
    ui->productSalesTable->setColumnWidth(4, 120);

    QStringList dailyHeaders = {"日期", "销售额"};
    ui->dailySalesTable->setColumnCount(dailyHeaders.size());
    ui->dailySalesTable->setHorizontalHeaderLabels(dailyHeaders);
    ui->dailySalesTable->setColumnWidth(0, 120);
    ui->dailySalesTable->setColumnWidth(1, 120);

    QList<QTableWidget*> tables = {ui->salesTable, ui->productSalesTable, ui->dailySalesTable};
    for (QTableWidget *table : tables) {
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void ReportWidget::refreshReports(const QDateTime &startDate, const QDateTime &endDate)
{
    updateStatisticsCards(startDate, endDate);

    QList<SalesOrderInfo> orders = DatabaseManager::instance().getSalesReport(startDate, endDate);
    double totalSales = DatabaseManager::instance().getTotalSales(startDate, endDate);
    displaySalesReport(orders, totalSales);

    QList<ProductSalesStat> ranking = DatabaseManager::instance().getProductSalesRanking(startDate, endDate);
    displayProductSalesRanking(ranking);

    QList<QPair<QDate, double>> dailyReport = DatabaseManager::instance().getDailySalesReport(startDate, endDate);
    displayDailySalesReport(dailyReport);

    updateStatus(QString("已刷新报表：%1 至 %2")
                     .arg(startDate.toString("yyyy-MM-dd"))
                     .arg(endDate.toString("yyyy-MM-dd")));
}

bool ReportWidget::exportToCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("无法创建文件");
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    stream << "=== 销售报表 ===\n";
    stream << "订单号,用户ID,总金额,地址,备注,下单时间\n";
    for (const SalesOrderInfo &order : m_currentSalesReport) {
        stream << order.id << ","
               << order.userId << ","
               << order.totalAmount << ","
               << order.address << ","
               << order.remark << ","
               << order.createdAt.toString("yyyy-MM-dd hh:mm:ss") << "\n";
    }

    stream << "\n=== 商品销售排行 ===\n";
    stream << "排名,商品ID,商品名称,销售数量,销售金额\n";
    for (int i = 0; i < m_currentProductRanking.size(); ++i) {
        const ProductSalesStat &stat = m_currentProductRanking[i];
        stream << (i + 1) << ","
               << stat.productId << ","
               << stat.productName << ","
               << stat.totalQuantity << ","
               << stat.totalAmount << "\n";
    }

    file.close();
    return true;
}

void ReportWidget::onQueryButtonClicked()
{
    QDateTime startDate = ui->startDateEdit->dateTime();
    QDateTime endDate = ui->endDateEdit->dateTime();

    if (startDate > endDate) {
        showError("开始日期不能大于结束日期");
        return;
    }

    refreshReports(startDate, endDate);
}

void ReportWidget::onExportButtonClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出报表",
                                                    QString("报表_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
                                                    "CSV文件 (*.csv)");

    if (!filePath.isEmpty()) {
        if (exportToCSV(filePath)) {
            showSuccess(QString("报表已导出到：%1").arg(filePath));
        }
    }
}

void ReportWidget::displaySalesReport(const QList<SalesOrderInfo> &orders, double total)
{
    ui->salesTable->setRowCount(0);
    m_currentSalesReport = orders;

    for (int i = 0; i < orders.size(); ++i) {
        ui->salesTable->insertRow(i);
        addSalesRowToTable(orders[i], i);
    }

    ui->salesTotalLabel->setText(QString("合计：¥ %1").arg(total, 0, 'f', 2));
}

void ReportWidget::displayProductSalesRanking(const QList<ProductSalesStat> &stats)
{
    ui->productSalesTable->setRowCount(0);
    m_currentProductRanking = stats;

    for (int i = 0; i < stats.size(); ++i) {
        ui->productSalesTable->insertRow(i);
        addProductSalesRowToTable(stats[i], i, i + 1);
    }
}

void ReportWidget::displayDailySalesReport(const QList<QPair<QDate, double>> &report)
{
    ui->dailySalesTable->setRowCount(0);
    m_currentDailyReport = report;

    for (int i = 0; i < report.size(); ++i) {
        ui->dailySalesTable->insertRow(i);
        addDailySalesRowToTable(report[i], i);
    }
}

void ReportWidget::addSalesRowToTable(const SalesOrderInfo &order, int row)
{
    ui->salesTable->setItem(row, 0, new QTableWidgetItem(QString::number(order.id)));
    ui->salesTable->setItem(row, 1, new QTableWidgetItem(QString::number(order.userId)));
    ui->salesTable->setItem(row, 2, new QTableWidgetItem(QString::number(order.totalAmount, 'f', 2)));
    ui->salesTable->setItem(row, 3, new QTableWidgetItem(order.address));
    ui->salesTable->setItem(row, 4, new QTableWidgetItem(order.remark));
    ui->salesTable->setItem(row, 5, new QTableWidgetItem(order.createdAt.toString("yyyy-MM-dd hh:mm:ss")));
}

void ReportWidget::addProductSalesRowToTable(const ProductSalesStat &stat, int row, int rank)
{
    ui->productSalesTable->setItem(row, 0, new QTableWidgetItem(QString::number(rank)));
    ui->productSalesTable->setItem(row, 1, new QTableWidgetItem(QString::number(stat.productId)));
    ui->productSalesTable->setItem(row, 2, new QTableWidgetItem(stat.productName));
    ui->productSalesTable->setItem(row, 3, new QTableWidgetItem(QString::number(stat.totalQuantity)));
    ui->productSalesTable->setItem(row, 4, new QTableWidgetItem(QString::number(stat.totalAmount, 'f', 2)));
}

void ReportWidget::addDailySalesRowToTable(const QPair<QDate, double> &data, int row)
{
    ui->dailySalesTable->setItem(row, 0, new QTableWidgetItem(data.first.toString("yyyy-MM-dd")));
    ui->dailySalesTable->setItem(row, 1, new QTableWidgetItem(QString::number(data.second, 'f', 2)));
}

void ReportWidget::updateStatisticsCards(const QDateTime &startDate, const QDateTime &endDate)
{
    double totalSales = DatabaseManager::instance().getTotalSales(startDate, endDate);
    double totalProfit = DatabaseManager::instance().getTotalProfit(startDate, endDate);
    int orderCount = DatabaseManager::instance().getOrderCount(startDate, endDate);

    ui->totalSalesValue->setText(QString("¥ %1").arg(totalSales, 0, 'f', 2));
    ui->totalProfitValue->setText(QString("¥ %1").arg(totalProfit, 0, 'f', 2));
    ui->orderCountValue->setText(QString::number(orderCount));
}

void ReportWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void ReportWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void ReportWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}