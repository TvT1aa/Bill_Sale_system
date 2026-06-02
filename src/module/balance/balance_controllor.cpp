#include "balance_controllor.h"
#include "balancewidget.h"
#include <QDebug>

static QVariantMap transactionToMap(const TransactionInfo &t, double balance)
{
    QVariantMap m;
    m["id"] = t.id;
    m["type"] = t.type;
    m["amount"] = t.amount;
    m["remark"] = t.remark;
    m["createTime"] = t.createdAt;
    m["balance"] = balance;
    return m;
}

balance_controllor::balance_controllor(int userId, BalanceWidget *widget, QObject *parent)
    : QObject(parent)
    , m_userId(userId)
    , m_view(widget)
{
    connect(m_view, &BalanceWidget::refreshRequested, this, [this]() {
        AccountInfo account = DatabaseManager::instance().getAccount();
        m_view->onBalanceLoaded(account.balance, account.name);

        QList<TransactionInfo> allTransactions = DatabaseManager::instance().getAllTransactions();
        QList<QVariantMap> transactions;
        double runningBalance = account.balance;
        for (const TransactionInfo &t : allTransactions) {
            transactions.append(transactionToMap(t, runningBalance));
            // 反推上一笔时的余额（当前倒序，收入减回、支出加回）
            if (t.type == 1) {
                runningBalance -= t.amount;
            } else {
                runningBalance += t.amount;
            }
        }
        m_view->onTransactionsLoaded(transactions);
    });

    connect(m_view, &BalanceWidget::adjustBalanceRequested, this, [this](double amount, const QString &remark) {
        bool ok = false;
        if (amount > 0) {
            ok = DatabaseManager::instance().addIncome(amount, remark);
        } else {
            ok = DatabaseManager::instance().addExpense(-amount, remark);
        }

        if (ok) {
            m_view->onOperationSuccess("余额已调整");
            AccountInfo updated = DatabaseManager::instance().getAccount();
            m_view->onBalanceLoaded(updated.balance, updated.name);
        } else {
            m_view->onOperationError("余额调整失败");
        }
    });
}