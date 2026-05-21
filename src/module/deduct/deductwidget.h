#ifndef DEDUCTWIDGET_H
#define DEDUCTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QLabel>

class DeductWidget : public QWidget
{
    Q_OBJECT

public:
    // mode: 0=用户结算模式, 1=管理员出库管理模式
    explicit DeductWidget(int userId, int mode, QWidget *parent = nullptr);
    ~DeductWidget();

signals:
    // 用户结算信号
    void loadCheckoutDataRequested();           // 加载结算页面数据
    void submitOrderRequested(int addressId, const QString& remark);

    // 管理员信号
    void refreshRequested();
    void searchOrderRequested(const QString& keyword);
    void filterByDateRequested(const QDateTime& startDate, const QDateTime& endDate);
    void viewOrderDetailRequested(int orderId);
    void updateOrderStatusRequested(int orderId, int status);
    void manualDeductRequested(int productId, int quantity, const QString& reason);

public slots:
    // 用户结算槽
    void onCartItemsLoaded(const QList<QVariantMap>& items, double total);
    void onAddressesLoaded(const QList<QVariantMap>& addresses);
    void onBalanceLoaded(double balance);
    void onOrderResult(bool success, const QString& message);

    // 管理员槽
    void onOrdersLoaded(const QList<QVariantMap>& orders);
    void onOrderDetailLoaded(const QVariantMap& orderDetail);
    void onOperationSuccess(const QString& message);
    void onOperationError(const QString& error);

private slots:
    // 用户结算
    void onSubmitClicked();

    // 管理员
    void onSearchClicked();
    void onFilterClicked();
    void onViewDetail();
    void onUpdateStatus();
    void onManualDeduct();
    void onRefreshClicked();
    void onTableItemDoubleClicked(int row, int col);

private:
    void setupUI(int mode);
    void setupCheckoutUI();     // 用户结算界面
    void setupAdminUI();        // 管理员出库界面
    void showOrderDetailDialog(int orderId);
    void showManualDeductDialog();

    QTableWidget* m_tableWidget;
    QLabel* m_totalLabel;
    QLabel* m_balanceLabel;
    QComboBox* m_addressCombo;
    QPushButton* m_submitBtn;

    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_filterBtn;
    QPushButton* m_viewDetailBtn;
    QPushButton* m_updateStatusBtn;
    QPushButton* m_manualDeductBtn;
    QPushButton* m_refreshBtn;
    QDateTimeEdit* m_startDateEdit;
    QDateTimeEdit* m_endDateEdit;
    QComboBox* m_statusCombo;

    int m_userId;
    int m_mode;  // 0=用户结算, 1=管理员
    double m_currentBalance;
    double m_currentTotal;
};

#endif // DEDUCTWIDGET_H