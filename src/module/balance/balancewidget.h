#ifndef BALANCEWIDGET_H
#define BALANCEWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include "common/databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BalanceWidget; }
QT_END_NAMESPACE

class BalanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BalanceWidget(QWidget *parent = nullptr);
    ~BalanceWidget();

    void setCurrentUser(int userId, const QString &username);

    void refreshBalance();
    void refreshTransactionHistory();
    bool recharge(double amount);
    bool withdraw(double amount);
    double getCurrentBalance();

private slots:
    void onRechargeButtonClicked();
    void onWithdrawButtonClicked();
    void onRefreshButtonClicked();

private:
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void displayTransactions(const QList<TransactionInfo> &records);
    void addTransactionToTable(const TransactionInfo &record, int row);

private:
    Ui::BalanceWidget *ui;
    int m_currentUserId;
    QString m_currentUsername;
    QList<TransactionInfo> m_currentTransactions;
};

#endif // BALANCEWIDGET_H