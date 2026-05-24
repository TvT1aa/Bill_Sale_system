#include "reportwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>

ReportWidget::ReportWidget(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    setupUI();
}

ReportWidget::~ReportWidget()
{
}

void ReportWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel* titleLabel = new QLabel("销售报表", this);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #303133; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 销售概览
    QGroupBox* summaryGroup = new QGroupBox("销售概览", this);
    summaryGroup->setStyleSheet("QGroupBox { font-size: 16px; font-weight: bold; }");
    QGridLayout* summaryGrid = new QGridLayout(summaryGroup);
    summaryGrid->setSpacing(20);

    QLabel* salesLabel = new QLabel("总销售额:", this);
    salesLabel->setStyleSheet("QLabel { font-size: 16px; color: #606266; }");
    m_totalSalesLabel = new QLabel("¥0.00", this);
    m_totalSalesLabel->setStyleSheet("QLabel { font-size: 32px; font-weight: bold; color: #F56C6C; }");

    QLabel* profitLabel = new QLabel("总利润:", this);
    profitLabel->setStyleSheet("QLabel { font-size: 16px; color: #606266; }");
    m_totalProfitLabel = new QLabel("¥0.00", this);
    m_totalProfitLabel->setStyleSheet("QLabel { font-size: 32px; font-weight: bold; color: #67C23A; }");

    QLabel* countLabel = new QLabel("订单数量:", this);
    countLabel->setStyleSheet("QLabel { font-size: 16px; color: #606266; }");
    m_orderCountLabel = new QLabel("0", this);
    m_orderCountLabel->setStyleSheet("QLabel { font-size: 32px; font-weight: bold; color: #409EFF; }");

    summaryGrid->addWidget(salesLabel, 0, 0, Qt::AlignRight);
    summaryGrid->addWidget(m_totalSalesLabel, 0, 1);
    summaryGrid->addWidget(profitLabel, 1, 0, Qt::AlignRight);
    summaryGrid->addWidget(m_totalProfitLabel, 1, 1);
    summaryGrid->addWidget(countLabel, 2, 0, Qt::AlignRight);
    summaryGrid->addWidget(m_orderCountLabel, 2, 1);

    mainLayout->addWidget(summaryGroup);
    mainLayout->addStretch();

    // 刷新按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(100, 36);
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; font-size: 14px; }");
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &ReportWidget::onRefreshClicked);
}

void ReportWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void ReportWidget::onSalesSummaryLoaded(double totalSales, double totalProfit, int orderCount)
{
    m_totalSalesLabel->setText(QString("¥%1").arg(totalSales, 0, 'f', 2));
    m_totalProfitLabel->setText(QString("¥%1").arg(totalProfit, 0, 'f', 2));
    m_orderCountLabel->setText(QString::number(orderCount));
}

void ReportWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "错误", error);
}
