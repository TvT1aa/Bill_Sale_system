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
    loadSampleProducts();
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
    m_totalLabel = new QLabel("总计: ¥0.00", this);
    m_totalLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #F56C6C; }");
    bottomLayout->addWidget(m_totalLabel);
    bottomLayout->addStretch();

    if (role == 0) {
        m_checkoutBtn = new QPushButton("结算", this);
        m_checkoutBtn->setFixedSize(100, 36);
        m_checkoutBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
        bottomLayout->addWidget(m_checkoutBtn);
        connect(m_checkoutBtn, &QPushButton::clicked, this, &CartWidget::onCheckout);
    } else {
        m_submitPurchaseBtn = new QPushButton("提交进货", this);
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
    m_searchEdit->setPlaceholderText("搜索商品...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("搜索", this);
    m_searchBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(80, 32);
    m_addToCartBtn = new QPushButton("加入购物车", this);
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
    QStringList headers = {"商品ID", "商品名称", "单价", "库存", "操作", ""};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setFixedHeight(250);

    // 购物车商品表格
    QLabel* cartTitle = new QLabel("我的购物车", this);
    cartTitle->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #303133; margin-top: 5px; }");
    mainLayout->addWidget(cartTitle);

    m_cartTable = new QTableWidget(this);
    m_cartTable->setColumnCount(5);
    QStringList cartHeaders = {"商品ID", "商品名称", "单价", "数量", "小计"};
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
    m_searchEdit->setPlaceholderText("搜索商品并加入进货单...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("搜索", this);
    m_searchBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(80, 32);

    // 数量/价格输入
    QLabel* qtyLabel = new QLabel("数量:", this);
    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(1, 99999);
    m_quantitySpin->setFixedWidth(80);
    QLabel* priceLabel = new QLabel("进价:", this);
    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0, 999999);
    m_priceSpin->setPrefix("¥");
    m_priceSpin->setFixedWidth(100);

    m_addToPurchaseBtn = new QPushButton("加入进货单", this);
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
    QStringList headers = {"商品ID", "商品名称", "进价", "数量", "小计", "操作"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    connect(m_searchBtn, &QPushButton::clicked, this, &CartWidget::onSearchProduct);
    connect(m_refreshBtn, &QPushButton::clicked, this, &CartWidget::onRefreshClicked);
    connect(m_addToPurchaseBtn, &QPushButton::clicked, this, &CartWidget::onAddToPurchase);
}

void CartWidget::loadSampleProducts()
{
    QList<QVariantMap> sampleProducts;

    QVariantMap p1;
    p1["id"] = 1;
    p1["name"] = "华为 Mate 60 Pro";
    p1["category"] = "手机";
    p1["salePrice"] = 6999.00;
    p1["quantity"] = 50;
    p1["unit"] = "台";
    sampleProducts.append(p1);

    QVariantMap p2;
    p2["id"] = 2;
    p2["name"] = "iPhone 15 Pro";
    p2["category"] = "手机";
    p2["salePrice"] = 7999.00;
    p2["quantity"] = 30;
    p2["unit"] = "台";
    sampleProducts.append(p2);

    QVariantMap p3;
    p3["id"] = 3;
    p3["name"] = "联想 ThinkPad X1";
    p3["category"] = "电脑";
    p3["salePrice"] = 8999.00;
    p3["quantity"] = 20;
    p3["unit"] = "台";
    sampleProducts.append(p3);

    QVariantMap p4;
    p4["id"] = 4;
    p4["name"] = "罗技 MX Master 3S";
    p4["category"] = "鼠标";
    p4["salePrice"] = 599.00;
    p4["quantity"] = 100;
    p4["unit"] = "个";
    sampleProducts.append(p4);

    QVariantMap p5;
    p5["id"] = 5;
    p5["name"] = "机械键盘 K845";
    p5["category"] = "键盘";
    p5["salePrice"] = 399.00;
    p5["quantity"] = 80;
    p5["unit"] = "个";
    sampleProducts.append(p5);

    QVariantMap p6;
    p6["id"] = 6;
    p6["name"] = "小米手环 8";
    p6["category"] = "穿戴";
    p6["salePrice"] = 249.00;
    p6["quantity"] = 200;
    p6["unit"] = "个";
    sampleProducts.append(p6);

    onProductsLoaded(sampleProducts);
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
        QMessageBox::warning(this, "提示", "请先选择商品");
        return;
    }

    int productId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QString productName = m_tableWidget->item(currentRow, 1)->text();
    double price = m_tableWidget->item(currentRow, 2)->text().replace("¥", "").toDouble();
    int stock = m_tableWidget->item(currentRow, 3)->text().toInt();

    bool ok;
    int quantity = QInputDialog::getInt(this, "加入购物车",
                                        QString("商品：%1\n单价：¥%2\n库存：%3\n\n请输入数量:")
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
        QMessageBox::warning(this, "提示", "请先选择商品");
        return;
    }

    int productId = m_tableWidget->item(currentRow, 0)->text().toInt();
    int quantity = m_quantitySpin->value();
    double price = m_priceSpin->value();

    if (quantity <= 0) {
        QMessageBox::warning(this, "提示", "数量必须大于0");
        return;
    }
    if (price <= 0) {
        QMessageBox::warning(this, "提示", "进价必须大于0");
        return;
    }

    emit addToPurchaseRequested(productId, quantity, price);
}

void CartWidget::onRemoveItem()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的项目");
        return;
    }
    QMessageBox::information(this, "提示", "请在购物车表格中点击删除按钮");
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
        QMessageBox::warning(this, "提示", "进货单为空，请先添加商品");
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
    m_totalLabel->setText(QString("购物车总计: ¥%1").arg(total, 0, 'f', 2));
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

        QPushButton* addBtn = new QPushButton("➕ 加入购物车");
        addBtn->setFixedSize(100, 28);
        addBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; font-size: 11px; }");
        int productId = p["id"].toInt();
        int stock = p["quantity"].toInt();
        double price = p["salePrice"].toDouble();
        QString name = p["name"].toString();
        connect(addBtn, &QPushButton::clicked, [this, productId, name, price, stock]() {
            bool ok;
            int qty = QInputDialog::getInt(this, "加入购物车",
                                           QString("商品：%1\n单价：¥%2\n库存：%3\n\n请输入数量:")
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

        QPushButton* removeBtn = new QPushButton("删除");
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
        QMessageBox::information(this, "提示", message);
        emit refreshRequested();
    } else {
        QMessageBox::warning(this, "错误", message);
    }
}

void CartWidget::onPurchaseResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "进货成功", message);
        emit refreshRequested();
    } else {
        QMessageBox::warning(this, "进货失败", message);
    }
}

void CartWidget::onBalanceInfo(double balance, const QString& message)
{
    Q_UNUSED(balance)
    QMessageBox::information(this, "余额信息", message);
}