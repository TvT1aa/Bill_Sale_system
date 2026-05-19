#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class CartWidget; }
QT_END_NAMESPACE

// 购物车商品项
struct CartItem {
    int productId;
    QString productName;
    int quantity;
    double price;
    double total;
};

// 订单结构体
struct Order {
    int id;
    QString orderNo;
    double totalAmount;
    QString status;      // "待支付", "已支付", "已取消"
    QString createTime;
};

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(QWidget *parent = nullptr);
    ~CartWidget();

    // ========== 用户接口 ==========
    void setCurrentUser(int userId, const QString &username, double balance);
    void refreshProductList();
    void refreshCart();
    void refreshOrders();
    void refreshBalance();

    // ========== 购物车操作接口 ==========
    bool addToCart(int productId, int quantity);
    bool removeFromCart(int productId);
    bool updateCartQuantity(int productId, int quantity);
    bool clearCart();
    QList<CartItem> getCartItems();
    double getCartTotal();

    // ========== 订单操作接口 ==========
    bool checkout();
    QList<Order> getUserOrders();

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
    void displayCartItems(const QList<CartItem> &items);
    void displayOrders(const QList<Order> &orders);
    void addCartItemToTable(const CartItem &item, int row);
    void addOrderToTable(const Order &order, int row);

    // TODO: 接入后端时替换
    int getProductStock(int productId);
    double getProductPrice(int productId);
    bool deductBalance(int userId, double amount);
    bool updateProductStock(int productId, int quantity);

private:
    Ui::CartWidget *ui;
    int m_currentUserId;
    QString m_currentUsername;
    double m_currentBalance;
    QList<CartItem> m_cartItems;
    QList<Order> m_orders;
};

#endif // CARTWIDGET_H