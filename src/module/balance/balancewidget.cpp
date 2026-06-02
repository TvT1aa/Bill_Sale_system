#include "balancewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDoubleSpinBox>

BalanceWidget::BalanceWidget(int userId, int role, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_role(role)
{
    // 统一初始化管理员界面，不再受 role 干扰
    setupUI();
    emit refreshRequested();
}

BalanceWidget::~BalanceWidget()
{
}

void BalanceWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // ================= 1. 余额显示区域 =================
    QWidget* balanceWidget = new QWidget(this);
    balanceWidget->setStyleSheet("QWidget { background-color: #E6F7FF; border-radius: 8px; }");
    QVBoxLayout* balanceLayout = new QVBoxLayout(balanceWidget);

    m_accountNameLabel = new QLabel(this);
    m_accountNameLabel->setStyleSheet("QLabel { font-size: 14px; color: #606266; }");

    m_balanceLabel = new QLabel("¥0.00", this);
    m_balanceLabel->setStyleSheet("QLabel { font-size: 32px; font-weight: bold; color: #F56C6C; }");
    m_balanceLabel->setAlignment(Qt::AlignCenter);

    balanceLayout->addWidget(m_accountNameLabel, 0, Qt::AlignCenter);
    balanceLayout->addWidget(m_balanceLabel);

    mainLayout->addWidget(balanceWidget);

    // ================= 2. 管理员余额调整区域 =================
    QWidget* adjustWidget = new QWidget(this);
    adjustWidget->setStyleSheet("QWidget { background-color: #F5F7FA; border-radius: 8px; }");
    QHBoxLayout* adjustLayout = new QHBoxLayout(adjustWidget);

    QLabel* amountLabel = new QLabel("调整金额:", this);
    m_adjustAmountEdit = new QLineEdit(this);
    m_adjustAmountEdit->setPlaceholderText("正数增加，负数减少");
    m_adjustAmountEdit->setFixedWidth(150);

    QLabel* remarkLabel = new QLabel("备注:", this);
    m_adjustRemarkEdit = new QLineEdit(this);
    m_adjustRemarkEdit->setPlaceholderText("调整原因");
    m_adjustRemarkEdit->setFixedWidth(200);

    m_adjustBtn = new QPushButton("确认调整", this);
    m_adjustBtn->setFixedSize(100, 32);
    m_adjustBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 4px; }");

    adjustLayout->addStretch();
    adjustLayout->addWidget(amountLabel);
    adjustLayout->addWidget(m_adjustAmountEdit);
    adjustLayout->addWidget(remarkLabel);
    adjustLayout->addWidget(m_adjustRemarkEdit);
    adjustLayout->addWidget(m_adjustBtn);
    adjustLayout->addStretch();

    mainLayout->addWidget(adjustWidget); // 直接作为核心组件加入布局

    // ================= 3. 交易记录表格 =================
    QLabel* historyLabel = new QLabel("资金流水明细", this);
    historyLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(historyLabel);

    m_transactionTable = new QTableWidget(this);
    m_transactionTable->setColumnCount(5);
    QStringList headers = {"时间", "类型", "变动金额", "当前变动后余额", "备注说明"};
    m_transactionTable->setHorizontalHeaderLabels(headers);
    m_transactionTable->horizontalHeader()->setStretchLastSection(true);
    m_transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_transactionTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_transactionTable);

    // ================= 4. 底部刷新按钮 =================
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(80, 32);
    bottomLayout->addWidget(m_refreshBtn);
    mainLayout->addLayout(bottomLayout);

    // 信号槽连接
    connect(m_adjustBtn, &QPushButton::clicked, this, &BalanceWidget::onAdjustBalance);
    connect(m_refreshBtn, &QPushButton::clicked, this, &BalanceWidget::onRefreshClicked);
}

void BalanceWidget::onAdjustBalance()
{
    bool ok = false;
    double amount = m_adjustAmountEdit->text().toDouble(&ok);
    if (!ok || amount == 0) {
        QMessageBox::warning(this, "提示", "请输入有效的调整金额（非零）");
        return;
    }

    // 限制最多两位小数
    QString amountText = m_adjustAmountEdit->text().trimmed();
    int dotPos = amountText.indexOf('.');
    if (dotPos >= 0 && amountText.length() - dotPos - 1 > 2) {
        QMessageBox::warning(this, "提示", "金额最多保留两位小数");
        return;
    }

    // 单次调整上限
    const double MAX_AMOUNT = 999999.99;
    if (qAbs(amount) > MAX_AMOUNT) {
        QMessageBox::warning(this, "提示", QString("单次调整金额不能超过 ¥%1").arg(MAX_AMOUNT, 0, 'f', 2));
        return;
    }

    // 扣减时不能低于0
    if (amount < 0 && m_currentBalance + amount < 0) {
        QMessageBox::warning(this, "提示", "余额不足，扣减后余额不能低于 ¥0.00");
        return;
    }

    QString remark = m_adjustRemarkEdit->text().trimmed();
    if (remark.isEmpty()) {
        remark = "管理员手动调整";
    }

    // 发出调整资金信号，由后端接收并改写商户账本
    emit adjustBalanceRequested(amount, remark);

    m_adjustAmountEdit->clear();
    m_adjustRemarkEdit->clear();
}

void BalanceWidget::onRefreshClicked()
{
    emit refreshRequested();
}

// ========== 后端调用的槽（完全保留，用于展示管理员数据） ==========

void BalanceWidget::onBalanceLoaded(double balance, const QString& accountName)
{
    m_currentBalance = balance;
    m_accountNameLabel->setText(QString("账户：%1").arg(accountName));
    m_balanceLabel->setText(QString("¥%1").arg(balance, 0, 'f', 2));
}

void BalanceWidget::onTransactionsLoaded(const QList<QVariantMap>& transactions)
{
    m_transactionTable->setRowCount(transactions.size());
    for (int i = 0; i < transactions.size(); i++) {
        const QVariantMap& trans = transactions[i];
        m_transactionTable->setItem(i, 0, new QTableWidgetItem(trans["createTime"].toString()));
        m_transactionTable->setItem(i, 1, new QTableWidgetItem(trans["type"].toString()));
        m_transactionTable->setItem(i, 2, new QTableWidgetItem(QString::number(trans["amount"].toDouble(), 'f', 2)));
        m_transactionTable->setItem(i, 3, new QTableWidgetItem(QString::number(trans["balance"].toDouble(), 'f', 2)));
        m_transactionTable->setItem(i, 4, new QTableWidgetItem(trans["remark"].toString()));
    }
}

void BalanceWidget::onOperationSuccess(const QString& message)
{
    QMessageBox::information(this, "成功", message);
    emit refreshRequested(); // 操作成功后自动刷新资产看板
}

void BalanceWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "失败", error);
}
