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
    explicit BalanceWidget(int userId, int role, QWidget *parent = nullptr);
    ~BalanceWidget();

signals:
    // 向后端请求数据的信号
    void refreshRequested();
    void rechargeRequested(double amount);           // 用户充值
    void adjustBalanceRequested(double amount, const QString& remark);  // 管理员调整余额

public slots:
    // 后端调用的槽
    void onBalanceLoaded(double balance, const QString& accountName);
    void onTransactionsLoaded(const QList<QVariantMap>& transactions);
    void onOperationSuccess(const QString& message);
    void onOperationError(const QString& error);

private slots:
    void onRecharge();
    void onAdjustBalance();
    void onRefreshClicked();

private:
    void setupUI(int role);
    void setupUserUI();     // 用户界面（查看余额+充值）
    void setupAdminUI();    // 管理员界面（查看库存账户余额+调整）

    QLabel* m_balanceLabel;
    QLabel* m_accountNameLabel;
    QTableWidget* m_transactionTable;
    QPushButton* m_refreshBtn;

    // 用户专用
    QLineEdit* m_rechargeEdit;
    QPushButton* m_rechargeBtn;

    // 管理员专用
    QLineEdit* m_adjustAmountEdit;
    QLineEdit* m_adjustRemarkEdit;
    QPushButton* m_adjustBtn;

    int m_userId;
    int m_role;
};

#endif // BALANCEWIDGET_H