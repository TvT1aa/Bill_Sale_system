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
    void refreshRequested();
    void addToCartRequested(int productId, int quantity);
    void removeFromCartRequested(int cartItemId);
    void updateCartQuantityRequested(int cartItemId, int quantity);
    void checkoutRequested();
    void searchProductRequested(const QString& keyword);
    void addToPurchaseRequested(int productId, int quantity, double price);
    void removeFromPurchaseRequested(int purchaseItemId);
    void submitPurchaseRequested(const QString& remark);

public slots:
    void onCartLoaded(const QList<QVariantMap>& cartItems);
    void onProductsLoaded(const QList<QVariantMap>& products);
    void onPurchaseLoaded(const QList<QVariantMap>& purchaseItems);
    void onCheckoutResult(bool success, const QString& message);
    void onPurchaseResult(bool success, const QString& message);
    void onBalanceInfo(double balance, const QString& message);
    // 删除了 onRechargeResult 和 onRecharge

private slots:
    void onSearchProduct();
    void onAddToCart();
    void onRemoveItem();
    void onCheckout();
    void onAddToPurchase();
    void onRemovePurchaseItem();
    void onSubmitPurchase();
    void onRefreshClicked();

private:
    void setupUI(int role);
    void setupUserUI();
    void setupAdminUI();
    void loadSampleProducts();

    QTableWidget* m_tableWidget;
    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_refreshBtn;
    QLabel* m_totalLabel;

    QPushButton* m_addToCartBtn;
    QPushButton* m_removeBtn;
    QPushButton* m_checkoutBtn;
    QComboBox* m_addressCombo;

    QPushButton* m_addToPurchaseBtn;
    QPushButton* m_removePurchaseBtn;
    QPushButton* m_submitPurchaseBtn;
    QSpinBox* m_quantitySpin;
    QDoubleSpinBox* m_priceSpin;

    int m_userId;
    int m_role;
};

#endif // CARTWIDGET_H