#include "reportwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>

ReportWidget::ReportWidget(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    setupUI();
    emit refreshRequested();
}

ReportWidget::~ReportWidget()
{
}

void ReportWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部筛选栏
    QHBoxLayout* topLayout = new QHBoxLayout();

    QLabel* dateLabel = new QLabel("日期范围:", this);
    m_startDateEdit = new QDateTimeEdit(this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDateTime(QDateTime::currentDateTime().addDays(-30));
    m_endDateEdit = new QDateTimeEdit(this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDateTime(QDateTime::currentDateTime());

    m_filterBtn = new QPushButton("筛选", this);
    m_filterBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(80, 32);

    m_exportExcelBtn = new QPushButton("导出Excel", this);
    m_exportExcelBtn->setFixedSize(100, 32);
    m_exportExcelBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; }");

    m_exportPdfBtn = new QPushButton("导出PDF", this);
    m_exportPdfBtn->setFixedSize(100, 32);
    m_exportPdfBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 4px; }");

    topLayout->addWidget(dateLabel);
    topLayout->addWidget(m_startDateEdit);
    topLayout->addWidget(m_endDateEdit);
    topLayout->addWidget(m_filterBtn);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addStretch();
    topLayout->addWidget(m_exportExcelBtn);
    topLayout->addWidget(m_exportPdfBtn);

    mainLayout->addLayout(topLayout);

    // 标签页
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    // ========== 概览页 ==========
    QWidget* summaryWidget = new QWidget();
    QVBoxLayout* summaryLayout = new QVBoxLayout(summaryWidget);

    QGroupBox* summaryGroup = new QGroupBox("销售概览");
    QGridLayout* summaryGrid = new QGridLayout(summaryGroup);

    QLabel* salesLabel = new QLabel("总销售额:");
    m_totalSalesLabel = new QLabel("¥0.00");
    m_totalSalesLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #F56C6C; }");

    QLabel* profitLabel = new QLabel("总利润:");
    m_totalProfitLabel = new QLabel("¥0.00");
    m_totalProfitLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #67C23A; }");

    QLabel* countLabel = new QLabel("订单数量:");
    m_orderCountLabel = new QLabel("0");
    m_orderCountLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #409EFF; }");

    summaryGrid->addWidget(salesLabel, 0, 0);
    summaryGrid->addWidget(m_totalSalesLabel, 0, 1);
    summaryGrid->addWidget(profitLabel, 1, 0);
    summaryGrid->addWidget(m_totalProfitLabel, 1, 1);
    summaryGrid->addWidget(countLabel, 2, 0);
    summaryGrid->addWidget(m_orderCountLabel, 2, 1);

    summaryLayout->addWidget(summaryGroup);
    summaryLayout->addStretch();

    m_tabWidget->addTab(summaryWidget, "📊 销售概览");

    // ========== 销售趋势页 ==========
    QWidget* trendWidget = new QWidget();
    QVBoxLayout* trendLayout = new QVBoxLayout(trendWidget);

    m_dailySalesTable = new QTableWidget();
    m_dailySalesTable->setColumnCount(2);
    m_dailySalesTable->setHorizontalHeaderLabels({"日期", "销售额"});
    m_dailySalesTable->horizontalHeader()->setStretchLastSection(true);
    m_dailySalesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dailySalesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    trendLayout->addWidget(m_dailySalesTable);
    m_tabWidget->addTab(trendWidget, "📈 销售趋势");

    // ========== 热销商品页 ==========
    QWidget* topProductsWidget = new QWidget();
    QVBoxLayout* topProductsLayout = new QVBoxLayout(topProductsWidget);

    m_topProductsTable = new QTableWidget();
    m_topProductsTable->setColumnCount(4);
    m_topProductsTable->setHorizontalHeaderLabels({"排名", "商品名称", "销量", "销售额"});
    m_topProductsTable->horizontalHeader()->setStretchLastSection(true);
    m_topProductsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_topProductsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    topProductsLayout->addWidget(m_topProductsTable);
    m_tabWidget->addTab(topProductsWidget, "🏆 热销商品");

    // ========== 订单明细页 ==========
    QWidget* ordersWidget = new QWidget();
    QVBoxLayout* ordersLayout = new QVBoxLayout(ordersWidget);

    m_ordersTable = new QTableWidget();
    m_ordersTable->setColumnCount(6);
    m_ordersTable->setHorizontalHeaderLabels({"订单ID", "用户", "总金额", "状态", "创建时间", "备注"});
    m_ordersTable->horizontalHeader()->setStretchLastSection(true);
    m_ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ordersLayout->addWidget(m_ordersTable);
    m_tabWidget->addTab(ordersWidget, "📋 订单明细");

    // 连接信号
    connect(m_filterBtn, &QPushButton::clicked, this, &ReportWidget::onFilterClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ReportWidget::onRefreshClicked);
    connect(m_exportExcelBtn, &QPushButton::clicked, this, &ReportWidget::onExportExcel);
    connect(m_exportPdfBtn, &QPushButton::clicked, this, &ReportWidget::onExportPdf);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &ReportWidget::onTabChanged);
}

void ReportWidget::onFilterClicked()
{
    QDateTime startDate = m_startDateEdit->dateTime();
    QDateTime endDate = m_endDateEdit->dateTime();
    emit filterByDateRequested(startDate, endDate);
}

void ReportWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void ReportWidget::onExportExcel()
{
    QDateTime startDate = m_startDateEdit->dateTime();
    QDateTime endDate = m_endDateEdit->dateTime();
    emit exportReportRequested("excel", startDate, endDate);
}

void ReportWidget::onExportPdf()
{
    QDateTime startDate = m_startDateEdit->dateTime();
    QDateTime endDate = m_endDateEdit->dateTime();
    emit exportReportRequested("pdf", startDate, endDate);
}

void ReportWidget::onTabChanged(int index)
{
    Q_UNUSED(index)
    // 切换标签页时刷新对应数据
    emit refreshRequested();
}

// ========== 后端调用的槽 ==========

void ReportWidget::onSalesSummaryLoaded(double totalSales, double totalProfit, int orderCount)
{
    m_totalSalesLabel->setText(QString("¥%1").arg(totalSales, 0, 'f', 2));
    m_totalProfitLabel->setText(QString("¥%1").arg(totalProfit, 0, 'f', 2));
    m_orderCountLabel->setText(QString::number(orderCount));
}

void ReportWidget::onDailySalesLoaded(const QList<QPair<QString, double>>& dailySales)
{
    m_dailySalesTable->setRowCount(dailySales.size());
    for (int i = 0; i < dailySales.size(); i++) {
        m_dailySalesTable->setItem(i, 0, new QTableWidgetItem(dailySales[i].first));
        m_dailySalesTable->setItem(i, 1, new QTableWidgetItem(QString("¥%1").arg(dailySales[i].second, 0, 'f', 2)));
    }
}

void ReportWidget::onTopProductsLoaded(const QList<QVariantMap>& topProducts)
{
    m_topProductsTable->setRowCount(topProducts.size());
    for (int i = 0; i < topProducts.size(); i++) {
        const QVariantMap& product = topProducts[i];
        m_topProductsTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_topProductsTable->setItem(i, 1, new QTableWidgetItem(product["name"].toString()));
        m_topProductsTable->setItem(i, 2, new QTableWidgetItem(QString::number(product["totalQuantity"].toInt())));
        m_topProductsTable->setItem(i, 3, new QTableWidgetItem(QString("¥%1").arg(product["totalAmount"].toDouble(), 0, 'f', 2)));
    }
}

void ReportWidget::onSalesOrdersLoaded(const QList<QVariantMap>& orders)
{
    m_ordersTable->setRowCount(orders.size());
    for (int i = 0; i < orders.size(); i++) {
        const QVariantMap& order = orders[i];
        m_ordersTable->setItem(i, 0, new QTableWidgetItem(QString::number(order["id"].toInt())));
        m_ordersTable->setItem(i, 1, new QTableWidgetItem(order["username"].toString()));
        m_ordersTable->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(order["totalAmount"].toDouble(), 0, 'f', 2)));
        m_ordersTable->setItem(i, 3, new QTableWidgetItem(order["status"].toString()));
        m_ordersTable->setItem(i, 4, new QTableWidgetItem(order["createdAt"].toString()));
        m_ordersTable->setItem(i, 5, new QTableWidgetItem(order["remark"].toString()));
    }
}

void ReportWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "错误", error);
}