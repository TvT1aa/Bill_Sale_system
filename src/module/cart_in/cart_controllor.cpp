#include "cart_controllor.h"
#include "cartwidget.h"
#include <QDebug>

// 获取全量商品
static QList<ProductInfo> getAllProductList()
{
    return DatabaseManager::instance().getProductsByName("");
}

static QList<QVariantMap> getProductsWithStock()
{
    QList<QVariantMap> products;
    for (const ProductInfo &p : getAllProductList()) {
        QVariantMap m;
        m["id"] = p.id;
        m["name"] = p.name;
        m["category"] = p.category;
        m["purchasePrice"] = p.purchasePrice;
        m["salePrice"] = p.salePrice;
        m["unit"] = p.unit;
        m["remark"] = p.remark;
        m["quantity"] = DatabaseManager::instance().getInventoryByProductId(p.id).quantity;
        products.append(m);
    }
    return products;
}

static QVariantMap cartItemToMap(const CartItem &c)
{
    QVariantMap m;
    m["productId"] = c.productId;
    m["productName"] = c.productName;
    m["quantity"] = c.quantity;
    m["price"] = c.price;
    m["total"] = c.total;
    return m;
}

cart_controllor::cart_controllor(int userId, CartWidget *widget, QObject *parent)
    : QObject(parent)
    , m_userId(userId)
    , m_view(widget)
{
        Cart::instance().setCurrentUser(m_userId);

    connect(m_view, &CartWidget::refreshRequested, this, [this]() {
        // 只在用户ID变化时才重新设置（setCurrentUser 会清空购物车，不能重复调用）
        if (Cart::instance().userId() != m_userId) {
            Cart::instance().setCurrentUser(m_userId);
        }
        m_view->onProductsLoaded(getProductsWithStock());

        QList<QVariantMap> cartItems;
        for (const CartItem &c : Cart::instance().getCartItems())
            cartItems.append(cartItemToMap(c));
        m_view->onCartLoaded(cartItems);
    });

    connect(m_view, &CartWidget::searchProductRequested, this, [this](const QString &keyword) {
        QList<QVariantMap> results;
        for (const ProductInfo &p : DatabaseManager::instance().getProductsByName(keyword)) {
            QVariantMap m;
            m["id"] = p.id;
            m["name"] = p.name;
            m["category"] = p.category;
            m["purchasePrice"] = p.purchasePrice;
            m["salePrice"] = p.salePrice;
            m["unit"] = p.unit;
            m["remark"] = p.remark;
            m["quantity"] = DatabaseManager::instance().getInventoryByProductId(p.id).quantity;
            results.append(m);
        }
        m_view->onProductsLoaded(results);
    });

    connect(m_view, &CartWidget::addToCartRequested, this, [this](int productId, int quantity) {
        bool ok = Cart::instance().addToCart(productId, quantity);
        if (ok) {
            // ✅ 扣减数据库库存（实时反映到数据库）
            DatabaseManager::instance().updateProductStock(productId, quantity);
            // 刷新商品表格显示最新库存
            m_view->onProductsLoaded(getProductsWithStock());
            // 刷新购物车显示
            QList<QVariantMap> cartItems;
        for (const CartItem &c : Cart::instance().getCartItems())
            cartItems.append(cartItemToMap(c));
            m_view->onCartLoaded(cartItems);
            m_view->onCheckoutResult(true, "Added to cart");
        } else {
            m_view->onCheckoutResult(false, Cart::instance().lastError());
        }
    });

    connect(m_view, &CartWidget::removeFromCartRequested, this, [this](int productId) {
        // 移除前获取购物车中该商品的数量，用于恢复库存
        int qtyInCart = 0;
        for (const CartItem &c : Cart::instance().getCartItems()) {
            if (c.productId == productId) {
                qtyInCart = c.quantity;
                break;
            }
        }
        Cart::instance().removeFromCart(productId);
        if (qtyInCart > 0) {
            // ✅ 恢复数据库库存（传负数给 updateProductStock 会做加回）
            DatabaseManager::instance().updateProductStock(productId, -qtyInCart);
            m_view->onProductsLoaded(getProductsWithStock());
        }
        QList<QVariantMap> cartItems;
        for (const CartItem &c : Cart::instance().getCartItems())
            cartItems.append(cartItemToMap(c));
        m_view->onCartLoaded(cartItems);
    });

    // 注意：checkoutRequested 由 HomeWidget 处理，切换到结算页面
    // 实际的结算下单操作在 DeductWidget 的 submitOrderRequested 中完成
    // 因此这里不做任何处理，避免重复下单

    // 管理员：进货
    connect(m_view, &CartWidget::addToPurchaseRequested, this, [this](int productId, int quantity, double price) {
        ProductInfo product = DatabaseManager::instance().getProductById(productId);
        if (product.id < 0) {
            m_view->onPurchaseResult(false, "Product not found");
            return;
        }

        // 创建进货订单
        int orderId = -1;
        if (!DatabaseManager::instance().addPurchaseOrder(QString("进货: %1 x%2").arg(product.name).arg(quantity), &orderId)) {
            m_view->onPurchaseResult(false, "Failed to create purchase order");
            return;
        }

        // 添加进货明细
        if (!DatabaseManager::instance().addOrderItem(orderId, productId, quantity, price)) {
            m_view->onPurchaseResult(false, "Failed to add purchase item");
            return;
        }

        // 更新库存（增加库存）
        InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(productId);
        if (inv.id > 0) {
            DatabaseManager::instance().updateQuantity(inv.id, inv.quantity + quantity);
        } else {
            DatabaseManager::instance().addInventory(productId, quantity);
        }

        m_view->onPurchaseResult(true, QString("Purchase success: %1 x%2, Order ID: %3")
                                     .arg(product.name).arg(quantity).arg(orderId));
    });

    connect(m_view, &CartWidget::submitPurchaseRequested, this, [this](const QString &remark) {
        Q_UNUSED(remark);
        m_view->onPurchaseResult(true, "Purchase order submitted");
    });
}
