#include "purchasewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

PurchaseWidget::PurchaseWidget(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    setupUI();
}

PurchaseWidget::~PurchaseWidget()
{
}

void PurchaseWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部搜索栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search products...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("Search", this);
    m_searchBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("Refresh", this);
    m_refreshBtn->setFixedSize(80, 32);

    QLabel* qtyLabel = new QLabel("Qty:", this);
    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(1, 99999);
    m_quantitySpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_quantitySpin->setFixedWidth(80);
    QLabel* priceLabel = new QLabel("Cost:", this);
    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0, 999999);
    m_priceSpin->setPrefix("¥");
    m_priceSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
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
    mainLayout->addLayout(topLayout);

    // 商品表格
    QLabel* productLabel = new QLabel("Product List", this);
    productLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; }");
    mainLayout->addWidget(productLabel);

    m_productTable = new QTableWidget(this);
    m_productTable->setColumnCount(5);
    m_productTable->setHorizontalHeaderLabels({"Product ID", "Product Name", "Category", "Price", "Stock"});
    m_productTable->horizontalHeader()->setStretchLastSection(true);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_productTable->setAlternatingRowColors(true);
    m_productTable->setFixedHeight(200);
    mainLayout->addWidget(m_productTable);

    // 进货单表格
    QLabel* purchaseLabel = new QLabel("Purchase List", this);
    purchaseLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; margin-top: 5px; }");
    mainLayout->addWidget(purchaseLabel);

    m_purchaseTable = new QTableWidget(this);
    m_purchaseTable->setColumnCount(6);
    m_purchaseTable->setHorizontalHeaderLabels({"Product ID", "Product Name", "Cost", "Quantity", "Subtotal", "Action"});
    m_purchaseTable->horizontalHeader()->setStretchLastSection(true);
    m_purchaseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_purchaseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_purchaseTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_purchaseTable);

    // 底部合计和提交按钮
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    m_totalLabel = new QLabel("Purchase Total: ¥0.00", this);
    m_totalLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #F56C6C; }");
    bottomLayout->addWidget(m_totalLabel);
    bottomLayout->addStretch();

    m_submitPurchaseBtn = new QPushButton("Submit Purchase", this);
    m_submitPurchaseBtn->setFixedSize(100, 36);
    m_submitPurchaseBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    bottomLayout->addWidget(m_submitPurchaseBtn);
    mainLayout->addLayout(bottomLayout);

    // 连接信号
    connect(m_searchBtn, &QPushButton::clicked, this, &PurchaseWidget::onSearchClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &PurchaseWidget::onRefreshClicked);
    connect(m_addToPurchaseBtn, &QPushButton::clicked, this, &PurchaseWidget::onAddToPurchase);
    connect(m_submitPurchaseBtn, &QPushButton::clicked, this, &PurchaseWidget::onSubmitPurchase);
}

void PurchaseWidget::onSearchClicked()
{
    QString keyword = m_searchEdit->text().trimmed();
    emit searchProductRequested(keyword);
}

void PurchaseWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void PurchaseWidget::onAddToPurchase()
{
    int currentRow = m_productTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Tip", "Please select a product first");
        return;
    }

    int productId = m_productTable->item(currentRow, 0)->text().toInt();
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

void PurchaseWidget::onSubmitPurchase()
{
    if (m_purchaseTable->rowCount() == 0) {
        QMessageBox::warning(this, "Tip", "Purchase list is empty, please add products first");
        return;
    }
    emit submitPurchaseRequested();
}

void PurchaseWidget::onProductsLoaded(const QList<QVariantMap>& products)
{
    m_productTable->setRowCount(products.size());
    for (int i = 0; i < products.size(); i++) {
        const QVariantMap& p = products[i];
        m_productTable->setItem(i, 0, new QTableWidgetItem(QString::number(p["id"].toInt())));
        m_productTable->setItem(i, 1, new QTableWidgetItem(p["name"].toString()));
        m_productTable->setItem(i, 2, new QTableWidgetItem(p["category"].toString()));
        m_productTable->setItem(i, 3, new QTableWidgetItem(QString("¥%1").arg(p["salePrice"].toDouble(), 0, 'f', 2)));
        m_productTable->setItem(i, 4, new QTableWidgetItem(QString::number(p["quantity"].toInt())));
    }
}

void PurchaseWidget::onPurchaseListLoaded(const QList<QVariantMap>& items)
{
    m_purchaseTable->setRowCount(items.size());
    double total = 0;
    for (int i = 0; i < items.size(); i++) {
        const QVariantMap& item = items[i];
        double subtotal = item["quantity"].toInt() * item["unitPrice"].toDouble();
        total += subtotal;

        m_purchaseTable->setItem(i, 0, new QTableWidgetItem(QString::number(item["productId"].toInt())));
        m_purchaseTable->setItem(i, 1, new QTableWidgetItem(item["productName"].toString()));
        m_purchaseTable->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(item["unitPrice"].toDouble(), 0, 'f', 2)));
        m_purchaseTable->setItem(i, 3, new QTableWidgetItem(QString::number(item["quantity"].toInt())));
        m_purchaseTable->setItem(i, 4, new QTableWidgetItem(QString("¥%1").arg(subtotal, 0, 'f', 2)));

        // 删除按钮
        QPushButton* removeBtn = new QPushButton("Delete");
        removeBtn->setFixedSize(60, 25);
        removeBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 3px; font-size: 11px; }");
        int productId = item["productId"].toInt();
        connect(removeBtn, &QPushButton::clicked, [this, productId]() {
            emit removeFromPurchaseRequested(productId);
        });
        m_purchaseTable->setCellWidget(i, 5, removeBtn);
    }
    m_totalLabel->setText(QString("Purchase Total: ¥%1").arg(total, 0, 'f', 2));
}

void PurchaseWidget::onOperationSuccess(const QString& message)
{
    QMessageBox::information(this, "Success", message);
}

void PurchaseWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "Failed", error);
}
