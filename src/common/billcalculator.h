#ifndef BILLCALCULATOR_H
#define BILLCALCULATOR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include "databasemanager.h"

/**
 * @brief 账单计算器 - 仅供管理员使用
 */
class BillCalculator : public QObject
{
    Q_OBJECT

public:
    explicit BillCalculator(QObject *parent = nullptr);

    void setCurrentUser(int userId);
    int userId() const { return m_userId; }

    // ==================== 余额操作（限管理员） ====================
    bool recharge(double amount);
    bool withdraw(double amount);
    double getCurrentBalance();
    double refreshBalance();

    // ==================== 交易流水 ====================
    // 使用统一的 TransactionInfo，确保你的 databasemanager.h 中已经定义了此结构体
    QList<TransactionInfo> getTransactionHistory(int limit = 50);
    QList<TransactionInfo> getTransactions(const QDateTime &start, const QDateTime &end);

    // ==================== 订单利润计算 ====================
    // 关键修正：确保这里引用的结构体名与 databasemanager.h 中一致
    double calcOrderProfit(int orderId);

    // ==================== 统计查询 ====================
    double getTotalSales(const QDateTime &start, const QDateTime &end) const;
    double getTotalProfit(const QDateTime &start, const QDateTime &end) const;
    int getOrderCount(const QDateTime &start, const QDateTime &end) const;

    QString lastError() const { return m_lastError; }

    signals:
        void balanceChanged(double newBalance);
    void transactionDone(const QString &type, double amount, double balance);

private:
    int m_userId = 0;
    QString m_lastError;

    // 内部权限判断辅助
    bool isAdmin() const { return m_userId == 1; }
};

#endif // BILLCALCULATOR_H