#include "billcalculator.h"
BillCalculator::BillCalculator(QObject *parent)
    : QObject(parent)
    , m_userId(0)
{
}
bool BillCalculator::recharge(double amount)
{
    // 只有管理员(role=1)才能操作，假设 m_userId 是当前登录的 ID
    UserInfo user = DatabaseManager::instance().getUserById(m_userId);
    if (user.role != 1) {
        m_lastError = "No permission: admin only";
        return false;
    }

    // 使用头文件已有的接口：addIncome (如果是充值)
    return DatabaseManager::instance().addIncome(amount, "System recharge");
}

bool BillCalculator::withdraw(double amount)
{
    UserInfo user = DatabaseManager::instance().getUserById(m_userId);
    if (user.role != 1) return false;

    // 使用头文件已有的接口：addExpense
    return DatabaseManager::instance().addExpense(amount, "Withdrawal");
}

double BillCalculator::getCurrentBalance()
{
    // 头文件定义：AccountInfo getAccount()
    return DatabaseManager::instance().getAccount().balance;
}

// 订单利润计算（对齐 SalesOrderItemInfo）
double BillCalculator::calcOrderProfit(int orderId)
{
    // 头文件定义：QList<SalesOrderItemInfo> getSalesOrderItems(int orderId)
    QList<SalesOrderItemInfo> items = DatabaseManager::instance().getSalesOrderItems(orderId);

    double totalRevenue = 0.0;
    double totalCost = 0.0;

    for (const auto &item : items) {
        totalRevenue += item.subtotal;
        // 获取商品信息计算成本
        ProductInfo product = DatabaseManager::instance().getProductById(item.productId);
        totalCost += item.quantity * product.purchasePrice;
    }
    return totalRevenue - totalCost;
}

// 统计接口：由于 DatabaseManager 里没写这三个函数，暂时返回 0
double BillCalculator::getTotalSales(const QDateTime&, const QDateTime&) const { return 0.0; }
double BillCalculator::getTotalProfit(const QDateTime&, const QDateTime&) const { return 0.0; }
int BillCalculator::getOrderCount(const QDateTime&, const QDateTime&) const { return 0; }