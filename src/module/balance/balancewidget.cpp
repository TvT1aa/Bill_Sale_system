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
    setupUI(role);
    emit refreshRequested();
}

BalanceWidget::~BalanceWidget()
{
}

void BalanceWidget::setupUI(int role)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 余额显示区域
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

    // 操作区域
    if (role == 0) {
        setupUserUI();
    } else {
        setupAdminUI();
    }

    // 交易记录表格
    QLabel* historyLabel = new QLabel("交易记录", this);
    historyLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(historyLabel);

    m_transactionTable = new QTableWidget(this);
    m_transactionTable->setColumnCount(5);
    QStringList headers = {"时间", "类型", "金额", "余额", "备注"};
    m_transactionTable->setHorizontalHeaderLabels(headers);
    m_transactionTable->horizontalHeader()->setStretchLastSection(true);
    m_transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_transactionTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_transactionTable);

    // 刷新按钮
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(80, 32);
    bottomLayout->addWidget(m_refreshBtn);
    mainLayout->addLayout(bottomLayout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &BalanceWidget::onRefreshClicked);
}

void BalanceWidget::setupUserUI()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());

    // 充值区域
    QWidget* rechargeWidget = new QWidget(this);
    rechargeWidget->setStyleSheet("QWidget { background-color: #F5F7FA; border-radius: 8px; }");
    QHBoxLayout* rechargeLayout = new QHBoxLayout(rechargeWidget);

    QLabel* amountLabel = new QLabel("充值金额:", this);
    m_rechargeEdit = new QLineEdit(this);
    m_rechargeEdit->setPlaceholderText("请输入充值金额");
    m_rechargeEdit->setFixedWidth(200);
    m_rechargeBtn = new QPushButton("确认充值", this);
    m_rechargeBtn->setFixedSize(100, 32);
    m_rechargeBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; }");

    rechargeLayout->addStretch();
    rechargeLayout->addWidget(amountLabel);
    rechargeLayout->addWidget(m_rechargeEdit);
    rechargeLayout->addWidget(m_rechargeBtn);
    rechargeLayout->addStretch();

    mainLayout->insertWidget(1, rechargeWidget);

    connect(m_rechargeBtn, &QPushButton::clicked, this, &BalanceWidget::onRecharge);
}

void BalanceWidget::setupAdminUI()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());

    // 余额调整区域
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

    mainLayout->insertWidget(1, adjustWidget);

    connect(m_adjustBtn, &QPushButton::clicked, this, &BalanceWidget::onAdjustBalance);
}

void BalanceWidget::onRecharge()
{
    double amount = m_rechargeEdit->text().toDouble();
    if (amount <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的充值金额");
        return;
    }
    emit rechargeRequested(amount);
    m_rechargeEdit->clear();
}

void BalanceWidget::onAdjustBalance()
{
    double amount = m_adjustAmountEdit->text().toDouble();
    if (amount == 0) {
        QMessageBox::warning(this, "提示", "请输入调整金额（非零）");
        return;
    }
    QString remark = m_adjustRemarkEdit->text().trimmed();
    if (remark.isEmpty()) {
        remark = "管理员手动调整";
    }
    emit adjustBalanceRequested(amount, remark);
    m_adjustAmountEdit->clear();
    m_adjustRemarkEdit->clear();
}

void BalanceWidget::onRefreshClicked()
{
    emit refreshRequested();
}

// ========== 后端调用的槽 ==========

void BalanceWidget::onBalanceLoaded(double balance, const QString& accountName)
{
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
    emit refreshRequested();
}

void BalanceWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "失败", error);
}