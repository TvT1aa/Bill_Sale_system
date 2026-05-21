#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include "common/databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CartWidget; }
QT_END_NAMESPACE

// 购物车临时商品项（内存中）
struct CartTempItem {
    int productId;
    QString productName;
    int quantity;
    double price;
    double total;
};

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(QWidget *parent = nullptr);
    ~CartWidget();

    void setCurrentUser(int userId, const QString &username);
    void refreshBalance();

    // 购物车操作
    void refreshProductList();
    void refreshCart();
    void refreshOrders();
    bool addToCart(int productId, int quantity);
    bool removeFromCart(int productId);
    bool updateCartQuantity(int productId, int quantity);
    bool clearCart();
    double getCartTotal();
    bool checkout();

private slots:
    void onProductComboChanged(int index);
    void onAddToCartButtonClicked();
    void onClearCartButtonClicked();
    void onCheckoutButtonClicked();
    void onRefreshOrdersButtonClicked();
    void onCartTableDoubleClicked(QTableWidgetItem *item);

private:
    void setupTables();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void updateTotalDisplay();

    void displayCartItems(const QList<CartTempItem> &items);
    void displayOrders(const QList<SalesOrderInfo> &orders);
    void addCartItemToTable(const CartTempItem &item, int row);
    void addOrderToTable(const SalesOrderInfo &order, int row);

    // 辅助方法
    double getProductPrice(int productId);
    int getProductStock(int productId);
    QString getDefaultAddress();

private:
    Ui::CartWidget *ui;
    int m_currentUserId;
    QString m_currentUsername;
    double m_currentBalance;
    QList<CartTempItem> m_cartItems;
    QList<SalesOrderInfo> m_orders;
};

#endif // CARTWIDGET_H