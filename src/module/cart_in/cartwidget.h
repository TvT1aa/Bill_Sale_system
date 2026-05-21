#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(int userId, int role, QWidget *parent = nullptr);
    ~CartWidget();

signals:
    // 通用信号
    void refreshRequested();

    // 用户购物车信号
    void addToCartRequested(int productId, int quantity);
    void removeFromCartRequested(int cartItemId);
    void updateCartQuantityRequested(int cartItemId, int quantity);
    void checkoutRequested();  // 跳转到结算页面
    void rechargeRequested(double amount);  // 充值请求

    // 管理员进货信号
    void searchProductRequested(const QString& keyword);
    void addToPurchaseRequested(int productId, int quantity, double price);
    void removeFromPurchaseRequested(int purchaseItemId);
    void submitPurchaseRequested(const QString& remark);

public slots:
    // 后端调用的槽
    void onCartLoaded(const QList<QVariantMap>& cartItems);
    void onProductsLoaded(const QList<QVariantMap>& products);
    void onPurchaseLoaded(const QList<QVariantMap>& purchaseItems);
    void onCheckoutResult(bool success, const QString& message);
    void onPurchaseResult(bool success, const QString& message);
    void onBalanceInfo(double balance, const QString& message);
    void onRechargeResult(bool success, const QString& message);  // 充值结果

private slots:
    void onSearchProduct();
    void onAddToCart();
    void onRemoveItem();
    void onCheckout();
    void onRecharge();  // 充值按钮点击
    void onAddToPurchase();
    void onRemovePurchaseItem();
    void onSubmitPurchase();
    void onRefreshClicked();

private:
    void setupUI(int role);
    void setupUserUI();
    void setupAdminUI();
    void loadSampleProducts();  // 加载示例商品

    QTableWidget* m_tableWidget;
    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_refreshBtn;
    QLabel* m_totalLabel;
    QLabel* m_balanceLabel;  // 余额显示

    // 用户专用
    QPushButton* m_addToCartBtn;
    QPushButton* m_removeBtn;
    QPushButton* m_checkoutBtn;
    QPushButton* m_rechargeBtn;  // 充值按钮
    QComboBox* m_addressCombo;

    // 管理员专用
    QPushButton* m_addToPurchaseBtn;
    QPushButton* m_removePurchaseBtn;
    QPushButton* m_submitPurchaseBtn;
    QSpinBox* m_quantitySpin;
    QDoubleSpinBox* m_priceSpin;

    int m_userId;
    int m_role;
    double m_currentBalance;  // 当前余额
};

#endif // CARTWIDGET_H