#ifndef BILLCALCULATOR_H
#define BILLCALCULATOR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include "databasemanager.h"

/**
 * @brief 账单计算器 - 参照 BalanceWidget 的充值/提现/查余额/查流水逻辑
 */
class BillCalculator : public QObject
{
    Q_OBJECT

public:
    explicit BillCalculator(QObject *parent = nullptr);

    void setCurrentUser(int userId);
    int userId() const { return m_userId; }

    // ==================== 余额操作（与 BalanceWidget 接口一致） ====================

    /// 充值（内部调 updateBalance）
    bool recharge(double amount);

    /// 提现（检查余额是否足够）
    bool withdraw(double amount);

    /// 获取当前余额
    double getCurrentBalance();

    /// 刷新余额（返回当前余额）
    double refreshBalance();

    // ==================== 交易流水 ====================

    /// 获取交易流水
    QList<TransactionInfo> getTransactionHistory(int limit = 50);

    /// 获取某段时间的交易流水
    QList<TransactionInfo> getTransactions(const QDateTime &start, const QDateTime &end);

    // ==================== 订单利润计算 ====================

    /// 计算订单利润（收入 - 成本）
    double calcOrderProfit(int orderId);

    // ==================== 统计查询 ====================

    /// 获取某段时间的总销售额
    double getTotalSales(const QDateTime &start, const QDateTime &end) const;

    /// 获取某段时间的总利润
    double getTotalProfit(const QDateTime &start, const QDateTime &end) const;

    /// 获取某段时间的订单总数
    int getOrderCount(const QDateTime &start, const QDateTime &end) const;

    /// 获取最后错误信息
    QString lastError() const { return m_lastError; }

signals:
    void balanceChanged(double newBalance);
    void transactionDone(const QString &type, double amount, double balance);

private:
    int m_userId = 0;
    QString m_lastError;
};

#endif // BILLCALCULATOR_H
