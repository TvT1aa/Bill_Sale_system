#include "balancewidget.h"
#include "ui_balancewidget.h"
#include "common/databasemanager.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

BalanceWidget::BalanceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BalanceWidget)
    , m_currentUserId(0)
{
    ui->setupUi(this);
    setupTable();

    connect(ui->rechargeButton, &QPushButton::clicked, this, &BalanceWidget::onRechargeButtonClicked);
    connect(ui->withdrawButton, &QPushButton::clicked, this, &BalanceWidget::onWithdrawButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &BalanceWidget::onRefreshButtonClicked);

    updateStatus("就绪");
}

BalanceWidget::~BalanceWidget()
{
    delete ui;
}

void BalanceWidget::setCurrentUser(int userId, const QString &username)
{
    m_currentUserId = userId;
    m_currentUsername = username;
    refreshBalance();
    refreshTransactionHistory();
}

void BalanceWidget::setupTable()
{
    QStringList headers = {"ID", "类型", "金额", "余额", "备注", "时间"};
    ui->historyTable->setColumnCount(headers.size());
    ui->historyTable->setHorizontalHeaderLabels(headers);

    ui->historyTable->setColumnWidth(0, 50);
    ui->historyTable->setColumnWidth(1, 80);
    ui->historyTable->setColumnWidth(2, 100);
    ui->historyTable->setColumnWidth(3, 100);
    ui->historyTable->setColumnWidth(4, 200);
    ui->historyTable->setColumnWidth(5, 150);

    ui->historyTable->setAlternatingRowColors(true);
    ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void BalanceWidget::refreshBalance()
{
    double balance = getCurrentBalance();
    ui->balanceAmountLabel->setText(QString("¥ %1").arg(balance, 0, 'f', 2));
}

void BalanceWidget::refreshTransactionHistory()
{
    QList<TransactionInfo> records = DatabaseManager::instance().getTransactionHistory(m_currentUserId);
    displayTransactions(records);
    updateStatus(QString("加载了 %1 条交易记录").arg(records.size()));
}

bool BalanceWidget::recharge(double amount)
{
    if (DatabaseManager::instance().updateBalance(m_currentUserId, amount)) {
        refreshBalance();
        refreshTransactionHistory();
        return true;
    }
    return false;
}

bool BalanceWidget::withdraw(double amount)
{
    double current = getCurrentBalance();
    if (amount > current) {
        showError("余额不足，无法提现");
        return false;
    }
    if (DatabaseManager::instance().updateBalance(m_currentUserId, -amount)) {
        refreshBalance();
        refreshTransactionHistory();
        return true;
    }
    return false;
}

double BalanceWidget::getCurrentBalance()
{
    AccountInfo account = DatabaseManager::instance().getAccountByUserId(m_currentUserId);
    return account.balance;
}

void BalanceWidget::onRechargeButtonClicked()
{
    bool ok;
    double amount = QInputDialog::getDouble(this, "充值", "请输入充值金额：", 0, 0, 999999, 2, &ok);
    if (ok && amount > 0) {
        if (recharge(amount)) {
            showSuccess(QString("充值成功！充值金额：¥ %1").arg(amount, 0, 'f', 2));
        } else {
            showError("充值失败");
        }
    }
}

void BalanceWidget::onWithdrawButtonClicked()
{
    bool ok;
    double current = getCurrentBalance();
    double amount = QInputDialog::getDouble(this, "提现", "请输入提现金额：", 0, 0, current, 2, &ok);
    if (ok && amount > 0) {
        if (withdraw(amount)) {
            showSuccess(QString("提现成功！提现金额：¥ %1").arg(amount, 0, 'f', 2));
        } else {
            showError("提现失败");
        }
    }
}

void BalanceWidget::onRefreshButtonClicked()
{
    refreshBalance();
    refreshTransactionHistory();
    updateStatus("数据已刷新");
}

void BalanceWidget::displayTransactions(const QList<TransactionInfo> &records)
{
    ui->historyTable->setRowCount(0);
    m_currentTransactions = records;

    for (int i = 0; i < records.size(); ++i) {
        ui->historyTable->insertRow(i);
        addTransactionToTable(records[i], i);
    }
}

void BalanceWidget::addTransactionToTable(const TransactionInfo &record, int row)
{
    QString typeText;
    if (record.type == "recharge") typeText = "充值";
    else if (record.type == "consume") typeText = "消费";
    else if (record.type == "withdraw") typeText = "提现";
    else typeText = record.type;

    ui->historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.id)));
    ui->historyTable->setItem(row, 1, new QTableWidgetItem(typeText));
    ui->historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.amount, 'f', 2)));
    ui->historyTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.balance, 'f', 2)));
    ui->historyTable->setItem(row, 4, new QTableWidgetItem(record.remark));
    ui->historyTable->setItem(row, 5, new QTableWidgetItem(record.createTime.toString("yyyy-MM-dd hh:mm:ss")));
}

void BalanceWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void BalanceWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void BalanceWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}