#include "billcalculator.h"
#include <QDebug>

BillCalculator::BillCalculator(QObject *parent)
    : QObject(parent)
    , m_userId(0)
{
}

void BillCalculator::setCurrentUser(int userId)
{
    m_userId = userId;
}

// ==================== 余额操作（与 BalanceWidget 一致） ====================

bool BillCalculator::recharge(double amount)
{
    if (m_userId <= 0) {
        m_lastError = "请先登录";
        return false;
    }
    if (amount <= 0) {
        m_lastError = "充值金额必须大于 0";
        return false;
    }

    if (!DatabaseManager::instance().updateBalance(m_userId, amount)) {
        m_lastError = "充值失败";
        return false;
    }

    double newBalance = getCurrentBalance();
    emit balanceChanged(newBalance);
    emit transactionDone("recharge", amount, newBalance);
    return true;
}

bool BillCalculator::withdraw(double amount)
{
    if (m_userId <= 0) {
        m_lastError = "请先登录";
        return false;
    }
    if (amount <= 0) {
        m_lastError = "提现金额必须大于 0";
        return false;
    }

    double current = getCurrentBalance();
    if (amount > current) {
        m_lastError = "余额不足，无法提现";
        return false;
    }

    if (!DatabaseManager::instance().updateBalance(m_userId, -amount)) {
        m_lastError = "提现失败";
        return false;
    }

    double newBalance = getCurrentBalance();
    emit balanceChanged(newBalance);
    emit transactionDone("withdraw", -amount, newBalance);
    return true;
}

double BillCalculator::getCurrentBalance()
{
    if (m_userId <= 0) return 0.0;
    AccountInfo account = DatabaseManager::instance().getAccountByUserId(m_userId);
    return account.balance;
}

double BillCalculator::refreshBalance()
{
    double balance = getCurrentBalance();
    emit balanceChanged(balance);
    return balance;
}

// ==================== 交易流水 ====================

QList<TransactionInfo> BillCalculator::getTransactionHistory(int limit)
{
    if (m_userId <= 0) return {};
    return DatabaseManager::instance().getTransactionHistory(m_userId, limit);
}

QList<TransactionInfo> BillCalculator::getTransactions(const QDateTime &start, const QDateTime &end)
{
    if (m_userId <= 0) return {};
    // 通过订单关联获取该用户时间范围内的交易
    QSqlQuery query(DatabaseManager::instance().getDatabase());
    query.prepare("SELECT * FROM transactions WHERE userId = ? AND createTime BETWEEN ? AND ? ORDER BY createTime DESC");
    query.addBindValue(m_userId);
    query.addBindValue(start);
    query.addBindValue(end);

    QList<TransactionInfo> list;
    if (query.exec()) {
        while (query.next()) {
            TransactionInfo t;
            t.id = query.value("id").toInt();
            t.userId = query.value("userId").toInt();
            t.type = query.value("type").toString();
            t.amount = query.value("amount").toDouble();
            t.balance = query.value("balance").toDouble();
            t.remark = query.value("remark").toString();
            t.createTime = query.value("createTime").toDateTime();
            list.append(t);
        }
    }
    return list;
}

// ==================== 订单利润计算 ====================

double BillCalculator::calcOrderProfit(int orderId)
{
    QList<ContainsInfo> items = DatabaseManager::instance().getOrderItems(orderId);
    if (items.isEmpty()) {
        m_lastError = QString("订单 %1 无明细").arg(orderId);
        return 0.0;
    }

    double totalRevenue = 0.0;
    double totalCost = 0.0;

    for (const ContainsInfo &item : items) {
        ProductInfo product = DatabaseManager::instance().getProductById(item.productId);
        totalRevenue += item.subtotal;
        totalCost += item.quantity * product.purchasePrice;
    }

    double profit = totalRevenue - totalCost;
    qDebug() << "[BillCalculator] 订单利润 - 订单ID:" << orderId
             << "收入:" << totalRevenue
             << "成本:" << totalCost
             << "利润:" << profit;
    return profit;
}

// ==================== 统计查询 ====================

double BillCalculator::getTotalSales(const QDateTime &start, const QDateTime &end) const
{
    return DatabaseManager::instance().getTotalSales(start, end);
}

double BillCalculator::getTotalProfit(const QDateTime &start, const QDateTime &end) const
{
    return DatabaseManager::instance().getTotalProfit(start, end);
}

int BillCalculator::getOrderCount(const QDateTime &start, const QDateTime &end) const
{
    return DatabaseManager::instance().getOrderCount(start, end);
}
