#include "deductwidget.h"
#include "ui_deductwidget.h"
#include "common/databasemanager.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

DeductWidget::DeductWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeductWidget)
    , m_currentUserId(0)
{
    ui->setupUi(this);
    setupTable();

    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeductWidget::onProductComboChanged);
    connect(ui->confirmButton, &QPushButton::clicked,
            this, &DeductWidget::onConfirmButtonClicked);
    // 如果 UI 中没有 refreshHistoryButton，注释掉下面这行
    // connect(ui->refreshHistoryButton, &QPushButton::clicked,
    //         this, &DeductWidget::onRefreshHistoryButtonClicked);

    updateStatus("就绪");
}

DeductWidget::~DeductWidget()
{
    delete ui;
}

void DeductWidget::setCurrentUser(int userId, const QString &username)
{
    m_currentUserId = userId;
    m_currentUsername = username;
    refreshProductList();
    refreshOrders();
}

void DeductWidget::setupTable()
{
    // 使用 historyTable（你 UI 中实际存在的表格）
    if (ui->historyTable) {
        QStringList headers = {"订单号", "总金额", "地址", "备注", "下单时间"};
        ui->historyTable->setColumnCount(headers.size());
        ui->historyTable->setHorizontalHeaderLabels(headers);
        ui->historyTable->setColumnWidth(0, 100);
        ui->historyTable->setColumnWidth(1, 100);
        ui->historyTable->setColumnWidth(2, 200);
        ui->historyTable->setColumnWidth(3, 150);
        ui->historyTable->setColumnWidth(4, 150);

        ui->historyTable->setAlternatingRowColors(true);
        ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void DeductWidget::refreshProductList()
{
    ui->productCombo->clear();
    ui->productCombo->addItem("请选择商品", -1);

    QList<ProductInfo> products = DatabaseManager::instance().getAllProducts();
    for (const ProductInfo &product : products) {
        ui->productCombo->addItem(product.name, product.id);
    }
}

void DeductWidget::refreshOrders()
{
    QList<SalesOrderInfo> orders = DatabaseManager::instance().getOrdersByUserId(m_currentUserId);
    m_orders = orders;
    displayOrders(orders);
    updateStatus(QString("加载了 %1 条订单记录").arg(orders.size()));
}

bool DeductWidget::deductStock(int productId, int quantity, const QString &remark)
{
    int currentStock = getProductStock(productId);
    if (quantity > currentStock) {
        showError(QString("库存不足！当前库存：%1 件").arg(currentStock));
        return false;
    }

    InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(productId);
    if (!DatabaseManager::instance().updateInventory(productId, currentStock - quantity, inv.unitPrice)) {
        showError("库存扣减失败");
        return false;
    }

    double price = getProductPrice(productId);
    double total = price * quantity;
    ProductInfo product = DatabaseManager::instance().getProductById(productId);

    int orderId = DatabaseManager::instance().createSalesOrder(
        m_currentUserId, "线下出库", total, QString("出库: %1 x %2, %3").arg(product.name).arg(quantity).arg(remark));

    if (orderId < 0) {
        showError("创建出库记录失败");
        return false;
    }

    if (!DatabaseManager::instance().addOrderItem(orderId, productId, quantity, price, total)) {
        showError("添加出库明细失败");
        return false;
    }

    refreshOrders();
    refreshProductList();
    return true;
}

void DeductWidget::onProductComboChanged(int index)
{
    // 只更新数量上限，不访问 stockLabel
    if (index > 0 && ui->quantitySpin) {
        int productId = ui->productCombo->currentData().toInt();
        int stock = getProductStock(productId);
        ui->quantitySpin->setMaximum(stock);

        // 可选：显示提示信息
        updateStatus(QString("当前库存：%1 件").arg(stock));
    } else if (ui->quantitySpin) {
        ui->quantitySpin->setMaximum(999);
        updateStatus("请选择商品");
    }
}

void DeductWidget::onConfirmButtonClicked()
{
    if (ui->productCombo->currentIndex() <= 0) {
        showError("请选择商品");
        return;
    }

    int productId = ui->productCombo->currentData().toInt();
    int quantity = ui->quantitySpin ? ui->quantitySpin->value() : 1;

    if (quantity <= 0) {
        showError("请输入有效的出库数量");
        return;
    }

    // 检查库存
    int stock = getProductStock(productId);
    if (quantity > stock) {
        showError(QString("库存不足！当前库存：%1 件").arg(stock));
        return;
    }

    QString remark = ui->remarkEdit ? ui->remarkEdit->toPlainText().trimmed() : "";

    QString msg = QString("确认出库？\n\n商品：%1\n数量：%2 件\n备注：%3")
                      .arg(ui->productCombo->currentText())
                      .arg(quantity)
                      .arg(remark.isEmpty() ? "无" : remark);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认出库", msg, QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (deductStock(productId, quantity, remark)) {
            showSuccess("出库成功");
            if (ui->remarkEdit) {
                ui->remarkEdit->clear();
            }
            if (ui->quantitySpin) {
                ui->quantitySpin->setValue(1);
            }
            // 刷新商品列表以更新库存显示
            refreshProductList();
        }
    }
}

void DeductWidget::onRefreshHistoryButtonClicked()
{
    refreshOrders();
    updateStatus("订单列表已刷新");
}

void DeductWidget::displayOrders(const QList<SalesOrderInfo> &orders)
{
    if (!ui->historyTable) return;

    ui->historyTable->setRowCount(0);
    for (int i = 0; i < orders.size(); ++i) {
        ui->historyTable->insertRow(i);
        addOrderToTable(orders[i], i);
    }
}

void DeductWidget::addOrderToTable(const SalesOrderInfo &order, int row)
{
    if (!ui->historyTable) return;

    ui->historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(order.id)));
    ui->historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(order.totalAmount, 'f', 2)));
    ui->historyTable->setItem(row, 2, new QTableWidgetItem(order.address));
    ui->historyTable->setItem(row, 3, new QTableWidgetItem(order.remark));
    ui->historyTable->setItem(row, 4, new QTableWidgetItem(order.createdAt.toString("yyyy-MM-dd hh:mm:ss")));
}

int DeductWidget::getProductStock(int productId)
{
    InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(productId);
    return inv.quantity;
}

double DeductWidget::getProductPrice(int productId)
{
    ProductInfo product = DatabaseManager::instance().getProductById(productId);
    return product.salePrice;
}

void DeductWidget::updateStatus(const QString &message)
{
    if (ui->statusLabel) {
        ui->statusLabel->setText(message);
    }
}

void DeductWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void DeductWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}