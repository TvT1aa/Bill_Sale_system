#ifndef CART_H
#define CART_H

#include <QObject>
#include <QString>
#include <QList>
#include "databasemanager.h"

/**
 * @brief 购物车商品项（与 CartWidget 中的 CartTempItem 一致）
 */
struct CartItem {
    int productId;
    QString productName;
    int quantity;
    double price;
    double total;
};

/**
 * @brief 购物车业务类 - 提取自 CartWidget 的购物车逻辑
 *
 * 接口与 CartWidget 的购物车操作方法一一对应。
 * 前端调这些方法即可，内部完成数据校验和数据库写入。
 */
class Cart : public QObject
{
    Q_OBJECT

public:
    explicit Cart(QObject *parent = nullptr);

    void setCurrentUser(int userId);
    int userId() const { return m_userId; }
    QString username() const { return m_username; }

    // ==================== 购物车操作（与 CartWidget 接口一致） ====================

    /// 添加商品到购物车（检查库存，已存在则累加数量）
    bool addToCart(int productId, int quantity);

    /// 从购物车移除商品
    bool removeFromCart(int productId);

    /// 修改购物车中商品数量（<=0 则删除）
    bool updateCartQuantity(int productId, int quantity);

    /// 清空购物车
    bool clearCart();

    /// 获取购物车总金额
    double getCartTotal();

    /// 获取购物车商品列表
    QList<CartItem> getCartItems() const { return m_cartItems; }

    /// 获取购物车商品种类数
    int getItemCount() const { return m_cartItems.size(); }

    // ==================== 结算下单 ====================

    /// 结算（创建订单、扣余额、扣库存），成功返回订单ID，失败返回-1
    int checkout();

    /// 获取最后错误信息
    QString lastError() const { return m_lastError; }

signals:
    void cartChanged();
    void checkoutSuccess(int orderId, double totalAmount);

private:
    // 辅助方法（与 CartWidget 一致）
    double getProductPrice(int productId);
    int getProductStock(int productId);
    QString getDefaultAddress();

    int m_userId = 0;
    QString m_username;
    QList<CartItem> m_cartItems;
    QString m_lastError;
};

#endif // CART_H
