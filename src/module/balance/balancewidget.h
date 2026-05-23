#ifndef BALANCEWIDGET_H
#define BALANCEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>

class BalanceWidget : public QWidget
{
    Q_OBJECT

public:
    // role 参数如果其他地方强依赖可以保留，但在内部我们不再根据角色分流 UI
    explicit BalanceWidget(int userId, int role, QWidget *parent = nullptr);
    ~BalanceWidget();

    signals:
        // 向后端请求数据的信号
        void refreshRequested();
    // 只保留管理员调整余额（进货资金支出/手动平账）的信号
    void adjustBalanceRequested(double amount, const QString& remark);

public slots:
    // 后端调用的槽（依然由后端把管理员的账户余额、账目流水喂给这个界面）
    void onBalanceLoaded(double balance, const QString& accountName);
    void onTransactionsLoaded(const QList<QVariantMap>& transactions);
    void onOperationSuccess(const QString& message);
    void onOperationError(const QString& error);

private slots:
    void onAdjustBalance();
    void onRefreshClicked();

private:
    void setupUI(); // 统一的 UI 初始化，不再区分用户/管理员

    // 核心显示控件
    QLabel* m_balanceLabel;
    QLabel* m_accountNameLabel;
    QTableWidget* m_transactionTable;
    QPushButton* m_refreshBtn;

    // 管理员专用：余额调整与流水账目备注
    QLineEdit* m_adjustAmountEdit;
    QLineEdit* m_adjustRemarkEdit;
    QPushButton* m_adjustBtn;

    int m_userId;
    int m_role;
};

#endif // BALANCEWIDGET_H