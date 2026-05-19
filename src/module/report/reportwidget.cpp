#include "reportwidget.h"
#include "ui_reportwidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "module/inventory/inventorywidget.h"
#include <QFileDialog>

ReportWidget::ReportWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReportWidget)
{
    ui->setupUi(this);
    setupTables();

    // 连接信号槽
    connect(ui->queryButton, &QPushButton::clicked, this, &ReportWidget::onQueryButtonClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &ReportWidget::onExportButtonClicked);

    // 设置默认日期范围（最近30天）
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-30);
    ui->startDateEdit->setDate(startDate);
    ui->endDateEdit->setDate(endDate);

    // 加载数据
    refreshReports(startDate, endDate);
    updateStatus("就绪");
}

ReportWidget::~ReportWidget()
{
    delete ui;
}

void ReportWidget::setupTables()
{
    // 销售报表表格
    QStringList salesHeaders = {"订单号", "商品名称", "数量", "单价", "总金额", "客户", "销售时间"};
    ui->salesTable->setColumnCount(salesHeaders.size());
    ui->salesTable->setHorizontalHeaderLabels(salesHeaders);
    ui->salesTable->setColumnWidth(0, 150);
    ui->salesTable->setColumnWidth(1, 150);
    ui->salesTable->setColumnWidth(2, 80);
    ui->salesTable->setColumnWidth(3, 100);
    ui->salesTable->setColumnWidth(4, 100);
    ui->salesTable->setColumnWidth(5, 120);
    ui->salesTable->setColumnWidth(6, 140);

    // 商品销售排行表格
    QStringList rankHeaders = {"排名", "商品ID", "商品名称", "销售数量", "销售金额"};
    ui->productSalesTable->setColumnCount(rankHeaders.size());
    ui->productSalesTable->setHorizontalHeaderLabels(rankHeaders);
    ui->productSalesTable->setColumnWidth(0, 60);
    ui->productSalesTable->setColumnWidth(1, 80);
    ui->productSalesTable->setColumnWidth(2, 200);
    ui->productSalesTable->setColumnWidth(3, 100);
    ui->productSalesTable->setColumnWidth(4, 120);

    // 日报表表格
    QStringList dailyHeaders = {"日期", "订单数", "销售额", "成本", "利润"};
    ui->dailySalesTable->setColumnCount(dailyHeaders.size());
    ui->dailySalesTable->setHorizontalHeaderLabels(dailyHeaders);
    ui->dailySalesTable->setColumnWidth(0, 120);
    ui->dailySalesTable->setColumnWidth(1, 100);
    ui->dailySalesTable->setColumnWidth(2, 120);
    ui->dailySalesTable->setColumnWidth(3, 120);
    ui->dailySalesTable->setColumnWidth(4, 120);

    // 设置表格属性
    QList<QTableWidget*> tables = {ui->salesTable, ui->productSalesTable, ui->dailySalesTable};
    for (QTableWidget *table : tables) {
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

// ========== 统计接口实现（TODO：接入后端） ==========

QList<SalesReportItem> ReportWidget::getSalesReport(const QDate &startDate, const QDate &endDate)
{
    QList<SalesReportItem> items;

    // TODO: 从数据库查询销售明细
    // SELECT id, order_no, product_name, quantity, price, total, customer, sale_time
    // FROM sales WHERE sale_time BETWEEN ? AND ?

    Q_UNUSED(startDate);
    Q_UNUSED(endDate);

    // 模拟数据
    SalesReportItem item1;
    item1.id = 1;
    item1.orderNo = "202405190001";
    item1.productName = "笔记本电脑";
    item1.quantity = 2;
    item1.price = 4999.00;
    item1.total = 9998.00;
    item1.customer = "张三";
    item1.saleTime = "2024-05-19 10:30:00";
    items.append(item1);

    SalesReportItem item2;
    item2.id = 2;
    item2.orderNo = "202405190002";
    item2.productName = "无线鼠标";
    item2.quantity = 5;
    item2.price = 39.00;
    item2.total = 195.00;
    item2.customer = "李四";
    item2.saleTime = "2024-05-19 14:20:00";
    items.append(item2);

    SalesReportItem item3;
    item3.id = 3;
    item3.orderNo = "202405180001";
    item3.productName = "机械键盘";
    item3.quantity = 1;
    item3.price = 249.00;
    item3.total = 249.00;
    item3.customer = "王五";
    item3.saleTime = "2024-05-18 09:15:00";
    items.append(item3);

    return items;
}

QList<ProductSalesStat> ReportWidget::getProductSalesRanking(const QDate &startDate, const QDate &endDate, int limit)
{
    QList<ProductSalesStat> stats;

    // TODO: 从数据库查询商品销售排行
    // SELECT product_id, product_name, SUM(quantity) as total_qty, SUM(total) as total_amount
    // FROM sales WHERE sale_time BETWEEN ? AND ?
    // GROUP BY product_id ORDER BY total_amount DESC LIMIT ?

    Q_UNUSED(startDate);
    Q_UNUSED(endDate);
    Q_UNUSED(limit);

    // 模拟数据
    ProductSalesStat stat1;
    stat1.productId = 1001;
    stat1.productName = "笔记本电脑";
    stat1.totalQuantity = 15;
    stat1.totalAmount = 74985.00;
    stats.append(stat1);

    ProductSalesStat stat2;
    stat2.productId = 1003;
    stat2.productName = "机械键盘";
    stat2.totalQuantity = 12;
    stat2.totalAmount = 2988.00;
    stats.append(stat2);

    ProductSalesStat stat3;
    stat3.productId = 1002;
    stat3.productName = "无线鼠标";
    stat3.totalQuantity = 45;
    stat3.totalAmount = 1755.00;
    stats.append(stat3);

    return stats;
}

QList<DailySalesStat> ReportWidget::getDailySalesReport(const QDate &startDate, const QDate &endDate)
{
    QList<DailySalesStat> stats;

    // TODO: 从数据库查询日报表
    // SELECT DATE(sale_time) as date, COUNT(*) as order_count, SUM(total) as total_sales
    // FROM sales WHERE sale_time BETWEEN ? AND ?
    // GROUP BY DATE(sale_time) ORDER BY date

    Q_UNUSED(startDate);
    Q_UNUSED(endDate);

    // 模拟数据
    DailySalesStat stat1;
    stat1.date = QDate(2024, 5, 18);
    stat1.orderCount = 3;
    stat1.totalSales = 3249.00;
    stat1.totalCost = 1850.00;
    stat1.profit = 1399.00;
    stats.append(stat1);

    DailySalesStat stat2;
    stat2.date = QDate(2024, 5, 19);
    stat2.orderCount = 5;
    stat2.totalSales = 10193.00;
    stat2.totalCost = 5900.00;
    stat2.profit = 4293.00;
    stats.append(stat2);

    return stats;
}

double ReportWidget::getTotalSales(const QDate &startDate, const QDate &endDate)
{
    // TODO: 从数据库查询
    Q_UNUSED(startDate);
    Q_UNUSED(endDate);
    return 13442.00;
}

double ReportWidget::getTotalCost(const QDate &startDate, const QDate &endDate)
{
    // TODO: 从数据库查询
    Q_UNUSED(startDate);
    Q_UNUSED(endDate);
    return 7750.00;
}

double ReportWidget::getTotalProfit(const QDate &startDate, const QDate &endDate)
{
    return getTotalSales(startDate, endDate) - getTotalCost(startDate, endDate);
}

int ReportWidget::getOrderCount(const QDate &startDate, const QDate &endDate)
{
    // TODO: 从数据库查询
    Q_UNUSED(startDate);
    Q_UNUSED(endDate);
    return 8;
}

void ReportWidget::refreshReports(const QDate &startDate, const QDate &endDate)
{
    // 刷新统计卡片
    updateStatisticsCards(startDate, endDate);

    // 刷新销售报表
    QList<SalesReportItem> salesItems = getSalesReport(startDate, endDate);
    double totalSales = getTotalSales(startDate, endDate);
    displaySalesReport(salesItems, totalSales);

    // 刷新商品销售排行
    QList<ProductSalesStat> ranking = getProductSalesRanking(startDate, endDate);
    displayProductSalesRanking(ranking);

    // 刷新日报表
    QList<DailySalesStat> dailyStats = getDailySalesReport(startDate, endDate);
    displayDailySalesReport(dailyStats);

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

    // 导出销售报表
    stream << "=== 销售报表 ===\n";
    stream << "订单号,商品名称,数量,单价,总金额,客户,销售时间\n";
    for (const SalesReportItem &item : m_currentSalesReport) {
        stream << item.orderNo << ","
               << item.productName << ","
               << item.quantity << ","
               << item.price << ","
               << item.total << ","
               << item.customer << ","
               << item.saleTime << "\n";
    }

    file.close();
    return true;
}

// ========== UI 槽函数实现 ==========

void ReportWidget::onQueryButtonClicked()
{
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

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

// ========== 内部辅助函数 ==========

void ReportWidget::displaySalesReport(const QList<SalesReportItem> &items, double total)
{
    ui->salesTable->setRowCount(0);
    m_currentSalesReport = items;

    for (int i = 0; i < items.size(); ++i) {
        ui->salesTable->insertRow(i);
        addSalesRowToTable(items[i], i);
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

void ReportWidget::displayDailySalesReport(const QList<DailySalesStat> &stats)
{
    ui->dailySalesTable->setRowCount(0);
    m_currentDailyReport = stats;

    for (int i = 0; i < stats.size(); ++i) {
        ui->dailySalesTable->insertRow(i);
        addDailySalesRowToTable(stats[i], i);
    }
}

void ReportWidget::addSalesRowToTable(const SalesReportItem &item, int row)
{
    ui->salesTable->setItem(row, 0, new QTableWidgetItem(item.orderNo));
    ui->salesTable->setItem(row, 1, new QTableWidgetItem(item.productName));
    ui->salesTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
    ui->salesTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.price, 'f', 2)));
    ui->salesTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.total, 'f', 2)));
    ui->salesTable->setItem(row, 5, new QTableWidgetItem(item.customer));
    ui->salesTable->setItem(row, 6, new QTableWidgetItem(item.saleTime));
}

void ReportWidget::addProductSalesRowToTable(const ProductSalesStat &stat, int row, int rank)
{
    ui->productSalesTable->setItem(row, 0, new QTableWidgetItem(QString::number(rank)));
    ui->productSalesTable->setItem(row, 1, new QTableWidgetItem(QString::number(stat.productId)));
    ui->productSalesTable->setItem(row, 2, new QTableWidgetItem(stat.productName));
    ui->productSalesTable->setItem(row, 3, new QTableWidgetItem(QString::number(stat.totalQuantity)));
    ui->productSalesTable->setItem(row, 4, new QTableWidgetItem(QString::number(stat.totalAmount, 'f', 2)));
}

void ReportWidget::addDailySalesRowToTable(const DailySalesStat &stat, int row)
{
    ui->dailySalesTable->setItem(row, 0, new QTableWidgetItem(stat.date.toString("yyyy-MM-dd")));
    ui->dailySalesTable->setItem(row, 1, new QTableWidgetItem(QString::number(stat.orderCount)));
    ui->dailySalesTable->setItem(row, 2, new QTableWidgetItem(QString::number(stat.totalSales, 'f', 2)));
    ui->dailySalesTable->setItem(row, 3, new QTableWidgetItem(QString::number(stat.totalCost, 'f', 2)));
    ui->dailySalesTable->setItem(row, 4, new QTableWidgetItem(QString::number(stat.profit, 'f', 2)));
}

void ReportWidget::updateStatisticsCards(const QDate &startDate, const QDate &endDate)
{
    double totalSales = getTotalSales(startDate, endDate);
    double totalCost = getTotalCost(startDate, endDate);
    double totalProfit = getTotalProfit(startDate, endDate);
    int orderCount = getOrderCount(startDate, endDate);

    ui->totalSalesValue->setText(QString("¥ %1").arg(totalSales, 0, 'f', 2));
    ui->totalCostValue->setText(QString("¥ %1").arg(totalCost, 0, 'f', 2));
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