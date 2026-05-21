#include "cartwidget.h"
#include "ui_cartwidget.h"
#include "common/databasemanager.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QDebug>

CartWidget::CartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWidget)
    , m_currentUserId(0)
    , m_currentBalance(0.0)
{
    ui->setupUi(this);
    setupTables();

    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CartWidget::onProductComboChanged);
    connect(ui->addToCartButton, &QPushButton::clicked,
            this, &CartWidget::onAddToCartButtonClicked);
    connect(ui->clearCartButton, &QPushButton::clicked,
            this, &CartWidget::onClearCartButtonClicked);
    connect(ui->checkoutButton, &QPushButton::clicked,
            this, &CartWidget::onCheckoutButtonClicked);
    connect(ui->refreshOrdersButton, &QPushButton::clicked,
            this, &CartWidget::onRefreshOrdersButtonClicked);
    connect(ui->cartTable, &QTableWidget::itemDoubleClicked,
            this, &CartWidget::onCartTableDoubleClicked);

    updateStatus("就绪");
}

CartWidget::~CartWidget()
{
    delete ui;
}

void CartWidget::setCurrentUser(int userId, const QString &username)
{
    m_currentUserId = userId;
    m_currentUsername = username;
    refreshBalance();
    refreshProductList();
    refreshCart();
    refreshOrders();
}

void CartWidget::refreshBalance()
{
    AccountInfo account = DatabaseManager::instance().getAccountByUserId(m_currentUserId);
    m_currentBalance = account.balance;
    ui->balanceLabel->setText(QString("余额：¥ %1").arg(m_currentBalance, 0, 'f', 2));
}

void CartWidget::setupTables()
{
    // 购物车表格
    QStringList cartHeaders = {"商品ID", "商品名称", "数量", "单价", "小计", "操作"};
    ui->cartTable->setColumnCount(cartHeaders.size());
    ui->cartTable->setHorizontalHeaderLabels(cartHeaders);
    ui->cartTable->setColumnWidth(0, 60);
    ui->cartTable->setColumnWidth(1, 200);
    ui->cartTable->setColumnWidth(2, 80);
    ui->cartTable->setColumnWidth(3, 100);
    ui->cartTable->setColumnWidth(4, 100);
    ui->cartTable->setColumnWidth(5, 100);

    // 订单表格
    QStringList orderHeaders = {"订单号", "总金额", "地址", "备注", "下单时间"};
    ui->ordersTable->setColumnCount(orderHeaders.size());
    ui->ordersTable->setHorizontalHeaderLabels(orderHeaders);
    ui->ordersTable->setColumnWidth(0, 100);
    ui->ordersTable->setColumnWidth(1, 100);
    ui->ordersTable->setColumnWidth(2, 200);
    ui->ordersTable->setColumnWidth(3, 150);
    ui->ordersTable->setColumnWidth(4, 150);

    ui->cartTable->setAlternatingRowColors(true);
    ui->cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->ordersTable->setAlternatingRowColors(true);
    ui->ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void CartWidget::refreshProductList()
{
    ui->productCombo->clear();
    ui->productCombo->addItem("请选择商品", -1);

    QList<ProductInfo> products = DatabaseManager::instance().getAllProducts();
    for (const ProductInfo &product : products) {
        ui->productCombo->addItem(product.name, product.id);
    }
    updateStatus(QString("加载了 %1 个商品").arg(products.size()));
}

void CartWidget::refreshCart()
{
    displayCartItems(m_cartItems);
    updateTotalDisplay();
}

void CartWidget::refreshOrders()
{
    QList<SalesOrderInfo> orders = DatabaseManager::instance().getOrdersByUserId(m_currentUserId);
    m_orders = orders;
    displayOrders(orders);
    updateStatus(QString("加载了 %1 条订单记录").arg(orders.size()));
}

bool CartWidget::addToCart(int productId, int quantity)
{
    int stock = getProductStock(productId);
    if (quantity > stock) {
        showError(QString("库存不足！当前库存：%1 件").arg(stock));
        return false;
    }

    for (CartTempItem &item : m_cartItems) {
        if (item.productId == productId) {
            int newQuantity = item.quantity + quantity;
            if (newQuantity > stock) {
                showError(QString("加入后数量超过库存！当前库存：%1 件").arg(stock));
                return false;
            }
            item.quantity = newQuantity;
            item.total = item.quantity * item.price;
            refreshCart();
            showSuccess(QString("已更新 %1 数量为 %2").arg(item.productName).arg(newQuantity));
            return true;
        }
    }

    ProductInfo product = DatabaseManager::instance().getProductById(productId);
    CartTempItem newItem;
    newItem.productId = productId;
    newItem.productName = product.name;
    newItem.quantity = quantity;
    newItem.price = product.salePrice;
    newItem.total = quantity * product.salePrice;
    m_cartItems.append(newItem);
    refreshCart();
    showSuccess(QString("已添加 %1 x %2 件").arg(product.name).arg(quantity));
    return true;
}

bool CartWidget::removeFromCart(int productId)
{
    for (int i = 0; i < m_cartItems.size(); ++i) {
        if (m_cartItems[i].productId == productId) {
            m_cartItems.removeAt(i);
            refreshCart();
            return true;
        }
    }
    return false;
}

bool CartWidget::updateCartQuantity(int productId, int quantity)
{
    for (CartTempItem &item : m_cartItems) {
        if (item.productId == productId) {
            if (quantity <= 0) {
                return removeFromCart(productId);
            }
            int stock = getProductStock(productId);
            if (quantity > stock) {
                showError(QString("库存不足！当前库存：%1 件").arg(stock));
                return false;
            }
            item.quantity = quantity;
            item.total = quantity * item.price;
            refreshCart();
            return true;
        }
    }
    return false;
}

bool CartWidget::clearCart()
{
    if (m_cartItems.isEmpty()) {
        showError("购物车已经是空的");
        return false;
    }
    m_cartItems.clear();
    refreshCart();
    updateStatus("购物车已清空");
    return true;
}

double CartWidget::getCartTotal()
{
    double total = 0;
    for (const CartTempItem &item : m_cartItems) {
        total += item.total;
    }
    return total;
}

bool CartWidget::checkout()
{
    if (m_cartItems.isEmpty()) {
        showError("购物车为空，请先添加商品");
        return false;
    }

    double total = getCartTotal();

    if (total > m_currentBalance) {
        showError(QString("余额不足！需要 ¥ %1，当前余额 ¥ %2")
                      .arg(total, 0, 'f', 2)
                      .arg(m_currentBalance, 0, 'f', 2));
        return false;
    }

    // 检查所有商品库存
    for (const CartTempItem &item : m_cartItems) {
        int stock = getProductStock(item.productId);
        if (item.quantity > stock) {
            showError(QString("商品 %1 库存不足！当前库存：%2 件")
                          .arg(item.productName).arg(stock));
            return false;
        }
    }

    // 确认对话框
    QString msg = QString("确认下单？\n\n订单总额：¥ %1\n支付后余额：¥ %2")
                      .arg(total, 0, 'f', 2)
                      .arg(m_currentBalance - total, 0, 'f', 2);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认下单", msg, QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return false;
    }

    // 获取默认地址
    QString address = getDefaultAddress();

    // 创建订单
    int orderId = DatabaseManager::instance().createSalesOrder(m_currentUserId, address, total, "购物车下单");
    if (orderId < 0) {
        showError("创建订单失败");
        return false;
    }

    // 添加订单商品明细并扣减库存
    for (const CartTempItem &item : m_cartItems) {
        if (!DatabaseManager::instance().addOrderItem(orderId, item.productId, item.quantity, item.price, item.total)) {
            showError(QString("添加订单商品 %1 失败").arg(item.productName));
            return false;
        }
        DatabaseManager::instance().updateProductStock(item.productId, item.quantity);
    }

    // 扣款
    if (!DatabaseManager::instance().updateBalance(m_currentUserId, -total)) {
        showError("扣款失败");
        return false;
    }

    // 清空购物车
    m_cartItems.clear();
    refreshCart();
    refreshBalance();
    refreshOrders();

    showSuccess(QString("下单成功！订单号：%1").arg(orderId));
    return true;
}

// ========== UI 槽函数 ==========

void CartWidget::onProductComboChanged(int index)
{
    if (index > 0) {
        int productId = ui->productCombo->currentData().toInt();
        int stock = getProductStock(productId);
        double price = getProductPrice(productId);

        ui->stockLabel->setText(QString("库存：%1 件").arg(stock));
        ui->priceLabel->setText(QString("单价：¥ %1").arg(price, 0, 'f', 2));
        ui->quantitySpin->setMaximum(stock);
    } else {
        ui->stockLabel->setText("库存：-- 件");
        ui->priceLabel->setText("单价：¥ --");
        ui->quantitySpin->setMaximum(999);
    }
}

void CartWidget::onAddToCartButtonClicked()
{
    if (ui->productCombo->currentIndex() <= 0) {
        showError("请选择商品");
        return;
    }
    int productId = ui->productCombo->currentData().toInt();
    int quantity = ui->quantitySpin->value();
    addToCart(productId, quantity);
}

void CartWidget::onClearCartButtonClicked()
{
    if (m_cartItems.isEmpty()) {
        showError("购物车已经是空的");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认清空", "确定要清空购物车吗？",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        clearCart();
    }
}

void CartWidget::onCheckoutButtonClicked()
{
    checkout();
}

void CartWidget::onRefreshOrdersButtonClicked()
{
    refreshOrders();
    updateStatus("订单列表已刷新");
}

void CartWidget::onCartTableDoubleClicked(QTableWidgetItem *item)
{
    if (!item) return;
    int row = item->row();
    if (row < 0 || row >= m_cartItems.size()) return;
    int productId = m_cartItems[row].productId;
    int currentQuantity = m_cartItems[row].quantity;
    bool ok;
    int newQuantity = QInputDialog::getInt(this, "修改数量", "请输入新数量：", currentQuantity, 1, 999, 1, &ok);
    if (ok) {
        updateCartQuantity(productId, newQuantity);
    }
}

// ========== 内部辅助函数 ==========

void CartWidget::displayCartItems(const QList<CartTempItem> &items)
{
    ui->cartTable->setRowCount(0);
    for (int i = 0; i < items.size(); ++i) {
        ui->cartTable->insertRow(i);
        addCartItemToTable(items[i], i);
        QPushButton *deleteBtn = new QPushButton("删除");
        ui->cartTable->setCellWidget(i, 5, deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, productId = items[i].productId]() {
            removeFromCart(productId);
        });
    }
}

void CartWidget::addCartItemToTable(const CartTempItem &item, int row)
{
    ui->cartTable->setItem(row, 0, new QTableWidgetItem(QString::number(item.productId)));
    ui->cartTable->setItem(row, 1, new QTableWidgetItem(item.productName));
    ui->cartTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
    ui->cartTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.price, 'f', 2)));
    ui->cartTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.total, 'f', 2)));
}

void CartWidget::displayOrders(const QList<SalesOrderInfo> &orders)
{
    ui->ordersTable->setRowCount(0);
    for (int i = 0; i < orders.size(); ++i) {
        ui->ordersTable->insertRow(i);
        addOrderToTable(orders[i], i);
    }
}

void CartWidget::addOrderToTable(const SalesOrderInfo &order, int row)
{
    ui->ordersTable->setItem(row, 0, new QTableWidgetItem(QString::number(order.id)));
    ui->ordersTable->setItem(row, 1, new QTableWidgetItem(QString::number(order.totalAmount, 'f', 2)));
    ui->ordersTable->setItem(row, 2, new QTableWidgetItem(order.address));
    ui->ordersTable->setItem(row, 3, new QTableWidgetItem(order.remark));
    ui->ordersTable->setItem(row, 4, new QTableWidgetItem(order.createdAt.toString("yyyy-MM-dd hh:mm:ss")));
}

void CartWidget::updateTotalDisplay()
{
    double total = getCartTotal();
    ui->totalLabel->setText(QString("总计：¥ %1").arg(total, 0, 'f', 2));
}

void CartWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void CartWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void CartWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}

double CartWidget::getProductPrice(int productId)
{
    ProductInfo product = DatabaseManager::instance().getProductById(productId);
    return product.salePrice;
}

int CartWidget::getProductStock(int productId)
{
    InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(productId);
    return inv.quantity;
}

QString CartWidget::getDefaultAddress()
{
    QList<BuyerAddressInfo> addresses = DatabaseManager::instance().getAddressesByUserId(m_currentUserId);
    for (const BuyerAddressInfo &addr : addresses) {
        if (addr.isDefault) {
            return QString("%1 %2 %3 %4").arg(addr.province, addr.city, addr.district, addr.detail);
        }
    }
    if (!addresses.isEmpty()) {
        const BuyerAddressInfo &addr = addresses.first();
        return QString("%1 %2 %3 %4").arg(addr.province, addr.city, addr.district, addr.detail);
    }
    return "";
}