#include "purchase_controllor.h"
#include "purchasewidget.h"
#include <QDebug>

static QList<QVariantMap> getProductsWithStock()
{
    QList<QVariantMap> products;
    for (const ProductInfo &p : DatabaseManager::instance().getProductsByName("")) {
        QVariantMap m;
        m["id"] = p.id;
        m["name"] = p.name;
        m["category"] = p.category;
        m["purchasePrice"] = p.purchasePrice;
        m["salePrice"] = p.salePrice;
        m["unit"] = p.unit;
        m["quantity"] = DatabaseManager::instance().getInventoryByProductId(p.id).quantity;
        products.append(m);
    }
    return products;
}

static QList<QVariantMap> purchaseListToMap(const QList<PurchaseItem> &list)
{
    QList<QVariantMap> result;
    for (const PurchaseItem &item : list) {
        QVariantMap m;
        m["productId"] = item.productId;
        m["productName"] = item.productName;
        m["quantity"] = item.quantity;
        m["unitPrice"] = item.unitPrice;
        result.append(m);
    }
    return result;
}

purchase_controllor::purchase_controllor(int userId, PurchaseWidget *widget, QObject *parent)
    : QObject(parent)
    , m_userId(userId)
    , m_view(widget)
{
    // 刷新商品列表
    connect(m_view, &PurchaseWidget::refreshRequested, this, [this]() {
        m_view->onProductsLoaded(getProductsWithStock());
    });

    // 搜索商品
    connect(m_view, &PurchaseWidget::searchProductRequested, this, [this](const QString &keyword) {
        QList<QVariantMap> results;
        for (const ProductInfo &p : DatabaseManager::instance().getProductsByName(keyword)) {
            QVariantMap m;
            m["id"] = p.id;
            m["name"] = p.name;
            m["category"] = p.category;
            m["purchasePrice"] = p.purchasePrice;
            m["salePrice"] = p.salePrice;
            m["unit"] = p.unit;
            m["quantity"] = DatabaseManager::instance().getInventoryByProductId(p.id).quantity;
            results.append(m);
        }
        m_view->onProductsLoaded(results);
    });

    // 加入进货单
    connect(m_view, &PurchaseWidget::addToPurchaseRequested, this, [this](int productId, int quantity, double price) {
        ProductInfo product = DatabaseManager::instance().getProductById(productId);
        if (product.id < 0) {
            m_view->onOperationError("未找到该商品");
            return;
        }

        // 检查是否已在进货单中
        for (int i = 0; i < m_purchaseList.size(); i++) {
            if (m_purchaseList[i].productId == productId) {
                // 已存在，更新数量和价格
                m_purchaseList[i].quantity += quantity;
                m_purchaseList[i].unitPrice = price;
                m_view->onPurchaseListLoaded(purchaseListToMap(m_purchaseList));
                m_view->onOperationSuccess(QString("已更新进货单: %1 x%2").arg(product.name).arg(m_purchaseList[i].quantity));
                return;
            }
        }

        // 新增进货项
        PurchaseItem item;
        item.productId = productId;
        item.productName = product.name;
        item.quantity = quantity;
        item.unitPrice = price;
        m_purchaseList.append(item);

        m_view->onPurchaseListLoaded(purchaseListToMap(m_purchaseList));
        m_view->onOperationSuccess(QString("已加入进货单: %1 x%2").arg(product.name).arg(quantity));
    });

    // 删除进货项
    connect(m_view, &PurchaseWidget::removeFromPurchaseRequested, this, [this](int productId) {
        for (int i = 0; i < m_purchaseList.size(); i++) {
            if (m_purchaseList[i].productId == productId) {
                QString name = m_purchaseList[i].productName;
                m_purchaseList.removeAt(i);
                m_view->onPurchaseListLoaded(purchaseListToMap(m_purchaseList));
                m_view->onOperationSuccess(QString("已从进货单移除: %1").arg(name));
                return;
            }
        }
    });

    // 提交进货
    connect(m_view, &PurchaseWidget::submitPurchaseRequested, this, [this]() {
        if (m_purchaseList.isEmpty()) {
            m_view->onOperationError("进货单为空");
            return;
        }

        // 创建进货订单
        int orderId = -1;
        QString remark = QString("批量进货，共%1种商品").arg(m_purchaseList.size());
        if (!DatabaseManager::instance().addPurchaseOrder(remark, &orderId)) {
            m_view->onOperationError("创建进货单失败");
            return;
        }

        // 逐个添加进货明细并更新库存
        bool allSuccess = true;
        double totalCost = 0;
        for (const PurchaseItem &item : m_purchaseList) {
            // 添加进货明细
            if (!DatabaseManager::instance().addOrderItem(orderId, item.productId, item.quantity, item.unitPrice)) {
                allSuccess = false;
                continue;
            }

            // 更新库存
            InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(item.productId);
            if (inv.id > 0) {
                DatabaseManager::instance().updateQuantity(inv.id, inv.quantity + item.quantity);
            } else {
                DatabaseManager::instance().addInventory(item.productId, item.quantity);
            }

            totalCost += item.quantity * item.unitPrice;
        }

        // 自动扣减卖家余额（支出）
        DatabaseManager::instance().addExpense(totalCost, QString("进货采购，订单ID: %1").arg(orderId));

        // 清空进货单
        m_purchaseList.clear();
        m_view->onPurchaseListLoaded(purchaseListToMap(m_purchaseList));

        // 刷新商品列表显示最新库存
        m_view->onProductsLoaded(getProductsWithStock());

        if (allSuccess) {
            m_view->onOperationSuccess(QString("进货成功，订单ID: %1").arg(orderId));
        } else {
            m_view->onOperationSuccess(QString("部分商品进货失败，订单ID: %1").arg(orderId));
        }
    });

    // 初始加载商品列表
    m_view->onProductsLoaded(getProductsWithStock());
}
