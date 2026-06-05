#include "cartwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>

CartWidget::CartWidget(int userId, int role, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_role(role)
{
    setupUI(role);
}

CartWidget::~CartWidget()
{
}

void CartWidget::setupUI(int role)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    if (role == 0) {
        setupUserUI();
    } else {
        setupAdminUI();
    }

    mainLayout->addWidget(m_tableWidget);

    // 底部合计和操作按钮
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    m_totalLabel = new QLabel("Total: ¥0.00", this);
    m_totalLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #F56C6C; }");
    bottomLayout->addWidget(m_totalLabel);
    bottomLayout->addStretch();

    if (role == 0) {
        m_checkoutBtn = new QPushButton("Checkout", this);
        m_checkoutBtn->setFixedSize(100, 36);
        m_checkoutBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
        bottomLayout->addWidget(m_checkoutBtn);
        connect(m_checkoutBtn, &QPushButton::clicked, this, &CartWidget::onCheckout);
    } else {
        m_submitPurchaseBtn = new QPushButton("Submit Purchase", this);
        m_submitPurchaseBtn->setFixedSize(100, 36);
        m_submitPurchaseBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
        bottomLayout->addWidget(m_submitPurchaseBtn);
        connect(m_submitPurchaseBtn, &QPushButton::clicked, this, &CartWidget::onSubmitPurchase);
    }

    mainLayout->addLayout(bottomLayout);
}

void CartWidget::setupUserUI()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());

    // 搜索栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search products...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("Search", this);
    m_searchBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("Refresh", this);
    m_refreshBtn->setFixedSize(80, 32);
    m_addToCartBtn = new QPushButton("Add to Cart", this);
    m_addToCartBtn->setFixedSize(100, 32);
    m_addToCartBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; }");

    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_searchBtn);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addWidget(m_addToCartBtn);
    topLayout->addStretch();

    mainLayout->insertLayout(0, topLayout);

    // 商品表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(6);
    QStringList headers = {"Product ID", "Product Name", "Price", "Stock", "Action", ""};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setFixedHeight(250);

    // 购物车商品表格
    QLabel* cartTitle = new QLabel("My Cart", this);
    cartTitle->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #303133; margin-top: 5px; }");
    mainLayout->addWidget(cartTitle);

    m_cartTable = new QTableWidget(this);
    m_cartTable->setColumnCount(5);
    QStringList cartHeaders = {"Product ID", "Product Name", "Price", "Quantity", "Subtotal"};
    m_cartTable->setHorizontalHeaderLabels(cartHeaders);
    m_cartTable->horizontalHeader()->setStretchLastSection(true);
    m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cartTable->setAlternatingRowColors(true);
    m_cartTable->setFixedHeight(150);

    connect(m_searchBtn, &QPushButton::clicked, this, &CartWidget::onSearchProduct);
    connect(m_refreshBtn, &QPushButton::clicked, this, &CartWidget::onRefreshClicked);
    connect(m_addToCartBtn, &QPushButton::clicked, this, &CartWidget::onAddToCart);
}

void CartWidget::setupAdminUI()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());

    // 顶部搜索栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search products and add to purchase...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("Search", this);
    m_searchBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("Refresh", this);
    m_refreshBtn->setFixedSize(80, 32);

    // 数量/价格输入
    QLabel* qtyLabel = new QLabel("Qty:", this);
    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(1, 99999);
    m_quantitySpin->setFixedWidth(80);
    QLabel* priceLabel = new QLabel("Cost:", this);
    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0, 999999);
    m_priceSpin->setPrefix("¥");
    m_priceSpin->setFixedWidth(100);

    m_addToPurchaseBtn = new QPushButton("Add to Purchase", this);
    m_addToPurchaseBtn->setFixedSize(100, 32);
    m_addToPurchaseBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; }");

    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_searchBtn);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addWidget(qtyLabel);
    topLayout->addWidget(m_quantitySpin);
    topLayout->addWidget(priceLabel);
    topLayout->addWidget(m_priceSpin);
    topLayout->addWidget(m_addToPurchaseBtn);
    topLayout->addStretch();

    mainLayout->insertLayout(0, topLayout);

    // 进货单表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(6);
    QStringList headers = {"Product ID", "Product Name", "Cost", "Quantity", "Subtotal", "Action"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    connect(m_searchBtn, &QPushButton::clicked, this, &CartWidget::onSearchProduct);
    connect(m_refreshBtn, &QPushButton::clicked, this, &CartWidget::onRefreshClicked);
    connect(m_addToPurchaseBtn, &QPushButton::clicked, this, &CartWidget::onAddToPurchase);
}

void CartWidget::onSearchProduct()
{
    QString keyword = m_searchEdit->text().trimmed();
    emit searchProductRequested(keyword);
}

void CartWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void CartWidget::onAddToCart()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select a product first");
        return;
    }

    int productId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QString productName = m_tableWidget->item(currentRow, 1)->text();
    double price = m_tableWidget->item(currentRow, 2)->text().replace("¥", "").toDouble();
    int stock = m_tableWidget->item(currentRow, 3)->text().toInt();

    bool ok;
    int quantity = QInputDialog::getInt(this, "Add to Cart",
                                        QString("Product: %1\nPrice: ¥%2\nStock: %3\n\nEnter quantity:")
                                            .arg(productName).arg(price, 0, 'f', 2).arg(stock),
                                        1, 1, stock, 1, &ok);
    if (ok && quantity > 0) {
        emit addToCartRequested(productId, quantity);
    }
}

void CartWidget::onAddToPurchase()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select a product first");
        return;
    }

    int productId = m_tableWidget->item(currentRow, 0)->text().toInt();
    int quantity = m_quantitySpin->value();
    double price = m_priceSpin->value();

    if (quantity <= 0) {
        QMessageBox::warning(this, "Tip", "Quantity must be greater than 0");
        return;
    }
    if (price <= 0) {
        QMessageBox::warning(this, "Tip", "Cost price must be greater than 0");
        return;
    }

    emit addToPurchaseRequested(productId, quantity, price);
}

void CartWidget::onRemoveItem()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select an item to delete");
        return;
    }
    QMessageBox::information(this, "Tip", "Please click the delete button in the cart table");
}

void CartWidget::onRemovePurchaseItem()
{
    onRemoveItem();
}

void CartWidget::onCheckout()
{
    emit checkoutRequested();
}

void CartWidget::onSubmitPurchase()
{
    if (m_tableWidget->rowCount() == 0) {
        QMessageBox::warning(this, "Tip", "Purchase list is empty, please add products first");
        return;
    }
    emit submitPurchaseRequested("");
}

// ========== 后端调用的槽 ==========

void CartWidget::onCartLoaded(const QList<QVariantMap>& cartItems)
{
    // 填充购物车商品表格
    if (!m_cartTable) return;

    m_cartTable->setRowCount(cartItems.size());
    double total = 0;
    for (int i = 0; i < cartItems.size(); i++) {
        const QVariantMap& item = cartItems[i];
        double subtotal = item["quantity"].toInt() * item["price"].toDouble();
        total += subtotal;

        m_cartTable->setItem(i, 0, new QTableWidgetItem(QString::number(item["productId"].toInt())));
        m_cartTable->setItem(i, 1, new QTableWidgetItem(item["productName"].toString()));
        m_cartTable->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(item["price"].toDouble(), 0, 'f', 2)));
        m_cartTable->setItem(i, 3, new QTableWidgetItem(QString::number(item["quantity"].toInt())));
        m_cartTable->setItem(i, 4, new QTableWidgetItem(QString("¥%1").arg(subtotal, 0, 'f', 2)));
    }
    m_totalLabel->setText(QString("Cart Total: ¥%1").arg(total, 0, 'f', 2));
}

void CartWidget::onProductsLoaded(const QList<QVariantMap>& products)
{
    m_tableWidget->setRowCount(products.size());
    for (int i = 0; i < products.size(); i++) {
        const QVariantMap& p = products[i];
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(p["id"].toInt())));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(p["name"].toString()));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(p["salePrice"].toDouble(), 0, 'f', 2)));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(p["quantity"].toInt())));

        QPushButton* addBtn = new QPushButton("➕ Add to Cart");
        addBtn->setFixedSize(100, 28);
        addBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; font-size: 11px; }");
        int productId = p["id"].toInt();
        int stock = p["quantity"].toInt();
        double price = p["salePrice"].toDouble();
        QString name = p["name"].toString();
        connect(addBtn, &QPushButton::clicked, [this, productId, name, price, stock]() {
            bool ok;
            int qty = QInputDialog::getInt(this, "Add to Cart",
                                           QString("Product: %1\nPrice: ¥%2\nStock: %3\n\nEnter quantity:")
                                               .arg(name).arg(price, 0, 'f', 2).arg(stock),
                                           1, 1, stock, 1, &ok);
            if (ok && qty > 0) {
                emit addToCartRequested(productId, qty);
            }
        });
        m_tableWidget->setCellWidget(i, 4, addBtn);

        m_tableWidget->setCellWidget(i, 5, new QWidget());
    }
}

void CartWidget::onPurchaseLoaded(const QList<QVariantMap>& purchaseItems)
{
    m_tableWidget->setRowCount(purchaseItems.size());
    double total = 0;
    for (int i = 0; i < purchaseItems.size(); i++) {
        const QVariantMap& item = purchaseItems[i];
        double subtotal = item["quantity"].toInt() * item["unitPrice"].toDouble();
        total += subtotal;

        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(item["productId"].toInt())));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(item["productName"].toString()));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(item["unitPrice"].toDouble())));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(item["quantity"].toInt())));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(subtotal)));

        QPushButton* removeBtn = new QPushButton("Delete");
        removeBtn->setFixedSize(60, 25);
        removeBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 3px; font-size: 11px; }");
        connect(removeBtn, &QPushButton::clicked, [this, id = item["id"].toInt()]() {
            emit removeFromPurchaseRequested(id);
        });
        m_tableWidget->setCellWidget(i, 5, removeBtn);
    }
    m_totalLabel->setText(QString("总计: ¥%1").arg(total, 0, 'f', 2));
}

void CartWidget::onCheckoutResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "Tip", message);
        emit refreshRequested();
    } else {
        QMessageBox::warning(this, "Error", message);
    }
}

void CartWidget::onPurchaseResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "Purchase Success", message);
        emit refreshRequested();
    } else {
        QMessageBox::warning(this, "Purchase Failed", message);
    }
}

void CartWidget::onBalanceInfo(double balance, const QString& message)
{
    Q_UNUSED(balance)
    QMessageBox::information(this, "Balance Info", message);
}