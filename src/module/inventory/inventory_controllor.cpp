#include "databasemanager.h"
#include "inventory_controllor.h"
#include <QDebug>
#include <QRegularExpression>

InventoryControllor::InventoryControllor(QObject *parent)
    : QObject(parent)
    , m_view(nullptr)
{
}

InventoryControllor::~InventoryControllor()
{
}

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
        m["remark"] = p.remark;
        m["quantity"] = DatabaseManager::instance().getInventoryByProductId(p.id).quantity;
        products.append(m);
    }
    return products;
}

void InventoryControllor::bindWithView(InventoryWidget* view) {
    m_view = view;

    // 刷新
    connect(m_view, &InventoryWidget::refreshRequested,
            this, &InventoryControllor::handleRefresh);

    // 搜索
    connect(m_view, &InventoryWidget::searchRequested,
            this, &InventoryControllor::handleSearch);

    // 添加商品
    connect(m_view, &InventoryWidget::addProductRequested,
            this, &InventoryControllor::handleAddProduct);

    // 修改商品
    connect(m_view, &InventoryWidget::updateProductRequested,
            this, &InventoryControllor::handleUpdateProduct);

    // 删除商品
    connect(m_view, &InventoryWidget::deleteProductRequested,
            this, &InventoryControllor::handleDeleteProduct);

    // 注册管理员
    connect(m_view, &InventoryWidget::adminRegisterRequested, this, [this](const QString& code) {
        this->handleAdminRegister(code, 1);
    });
}

void InventoryControllor::handleRefresh()
{
    m_view->onInventoryLoaded(getProductsWithStock());
}

void InventoryControllor::handleSearch(const QString& keyword)
{
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
    m_view->onSearchResult(results);
}

void InventoryControllor::handleAddProduct(const QVariantMap& productData)
{
    QString name = productData["name"].toString();
    QString category = productData["category"].toString();
    double salePrice = productData["salePrice"].toDouble();
    int quantity = productData["quantity"].toInt();
    QString unit = productData["unit"].toString();

    // 后端校验：单位只能包含字母
    QRegularExpression unitRegex("^[a-zA-Z]+$");
    if (!unitRegex.match(unit).hasMatch()) {
        m_view->onOperationError("Unit must contain only letters (e.g., pcs, kg, box)");
        return;
    }

    // 后端校验：类别不能为空
    if (category.isEmpty()) {
        m_view->onOperationError("Category cannot be empty");
        return;
    }

    int productId = -1;
    bool ok = DatabaseManager::instance().addProduct(name, category, 0, salePrice, unit, "", &productId);
    if (!ok) {
        m_view->onOperationError("Failed to add product");
        return;
    }

    // 添加库存
    if (quantity > 0) {
        DatabaseManager::instance().addInventory(productId, quantity);
    }

    m_view->onOperationSuccess(QString("Product added successfully: %1").arg(name));
    handleRefresh();
}

void InventoryControllor::handleUpdateProduct(int id, const QVariantMap& productData)
{
    ProductInfo product = DatabaseManager::instance().getProductById(id);
    if (product.id < 0) {
        m_view->onOperationError("Product not found");
        return;
    }

    QString name = productData["name"].toString();
    QString category = productData["category"].toString();
    double salePrice = productData["salePrice"].toDouble();
    int quantity = productData["quantity"].toInt();
    QString unit = productData["unit"].toString();

    // 后端校验：单位只能包含字母
    QRegularExpression unitRegex("^[a-zA-Z]+$");
    if (!unitRegex.match(unit).hasMatch()) {
        m_view->onOperationError("Unit must contain only letters (e.g., pcs, kg, box)");
        return;
    }

    // 后端校验：类别不能为空
    if (category.isEmpty()) {
        m_view->onOperationError("Category cannot be empty");
        return;
    }

    bool ok = DatabaseManager::instance().updateProduct(id, name, category,
                                                         product.purchasePrice, salePrice,
                                                         unit, product.remark);
    if (!ok) {
        m_view->onOperationError("Failed to update product");
        return;
    }

    // 更新库存
    InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(id);
    if (inv.id > 0) {
        DatabaseManager::instance().updateQuantity(inv.id, quantity);
    } else {
        DatabaseManager::instance().addInventory(id, quantity);
    }

    m_view->onOperationSuccess(QString("Product updated successfully: %1").arg(name));
    handleRefresh();
}

void InventoryControllor::handleDeleteProduct(int id)
{
    ProductInfo product = DatabaseManager::instance().getProductById(id);
    if (product.id < 0) {
        m_view->onOperationError("Product not found");
        return;
    }

    bool ok = DatabaseManager::instance().deleteProduct(id);
    if (!ok) {
        m_view->onOperationError("Failed to delete product");
        return;
    }

    m_view->onOperationSuccess(QString("Product deleted successfully: %1").arg(product.name));
    handleRefresh();
}

void InventoryControllor::handleAdminRegister(const QString& verifyCode, int userId)
{
    Q_UNUSED(userId);
    // 管理员验证码：简单实现，验证码为 "admin888"
    if (verifyCode == "admin888") {
        m_view->onAdminRegisterResult(true, "Admin registered successfully");
    } else {
        m_view->onAdminRegisterResult(false, "Incorrect verification code");
    }
}