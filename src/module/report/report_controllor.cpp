#include "report_controllor.h"
#include "reportwidget.h"
#include <QDebug>

report_controllor::report_controllor(int userId, ReportWidget *widget, QObject *parent)
    : QObject(parent)
    , m_userId(userId)
    , m_view(widget)
{
    connect(m_view, &ReportWidget::refreshRequested, this, [this]() {
        // 计算总销售额
        double totalSales = 0;
        QList<SalesOrderInfo> orders = DatabaseManager::instance().getAllSalesOrders();
        for (const SalesOrderInfo &order : orders) {
            totalSales += order.totalAmount;
        }

        // 计算总利润（收入 - 支出）
        double totalIncome = 0;
        double totalExpense = 0;
        for (const TransactionInfo &t : DatabaseManager::instance().getAllTransactions()) {
            if (t.type == 1) {
                totalIncome += t.amount;
            } else {
                totalExpense += t.amount;
            }
        }
        double totalProfit = totalIncome - totalExpense;

        // 订单数量
        int orderCount = orders.size();

        m_view->onSalesSummaryLoaded(totalSales, totalProfit, orderCount);
    });

    // 初始加载
    emit m_view->refreshRequested();
}
