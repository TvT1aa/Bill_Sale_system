#include "inventorywidget.h"
#include "ui_inventorywidget.h"
#include "common/databasemanager.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDate>
#include <QDateTime>
#include <QDebug>

InventoryWidget::InventoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InventoryWidget)
    , m_currentUserId(0)
{
    ui->setupUi(this);
    setupTable();
    setupDetailTables();

    connect(ui->searchButton, &QPushButton::clicked, this, &InventoryWidget::onSearchButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &InventoryWidget::onRefreshButtonClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &InventoryWidget::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &InventoryWidget::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &InventoryWidget::onDeleteButtonClicked);
    connect(ui->adjustButton, &QPushButton::clicked, this, &InventoryWidget::onAdjustButtonClicked);
    connect(ui->inventoryTable, &QTableWidget::itemDoubleClicked, this, &InventoryWidget::onTableItemDoubleClicked);
    connect(ui->queryStatsButton, &QPushButton::clicked, this, &InventoryWidget::onQueryStatsButtonClicked);

    refreshInventoryData();

    ui->monthSelect->setDate(QDate::currentDate());
    QDate currentDate = QDate::currentDate();
    refreshStatistics(currentDate.year(), currentDate.month());

    updateStatus("就绪");
}

InventoryWidget::~InventoryWidget()
{
    delete ui;
}

void InventoryWidget::setCurrentUser(int userId, const QString &username)
{
    m_currentUserId = userId;
    m_currentUsername = username;
}

void InventoryWidget::setupTable()
{
    QStringList headers = {"商品ID", "商品名称", "分类", "库存量", "进价", "售价", "单位"};
    ui->inventoryTable->setColumnCount(headers.size());
    ui->inventoryTable->setHorizontalHeaderLabels(headers);

    ui->inventoryTable->setColumnWidth(0, 80);
    ui->inventoryTable->setColumnWidth(1, 150);
    ui->inventoryTable->setColumnWidth(2, 100);
    ui->inventoryTable->setColumnWidth(3, 80);
    ui->inventoryTable->setColumnWidth(4, 100);
    ui->inventoryTable->setColumnWidth(5, 100);
    ui->inventoryTable->setColumnWidth(6, 60);

    ui->inventoryTable->setAlternatingRowColors(true);
    ui->inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->inventoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->inventoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void InventoryWidget::setupDetailTables()
{
    QStringList salesHeaders = {"订单号", "总金额", "地址", "备注", "下单时间"};
    ui->salesDetailTable->setColumnCount(salesHeaders.size());
    ui->salesDetailTable->setHorizontalHeaderLabels(salesHeaders);
    ui->salesDetailTable->setColumnWidth(0, 150);
    ui->salesDetailTable->setColumnWidth(1, 100);
    ui->salesDetailTable->setColumnWidth(2, 200);
    ui->salesDetailTable->setColumnWidth(3, 150);
    ui->salesDetailTable->setColumnWidth(4, 140);

    QStringList rankHeaders = {"排名", "商品ID", "商品名称", "销售数量", "销售金额"};
    ui->productSalesTable->setColumnCount(rankHeaders.size());
    ui->productSalesTable->setHorizontalHeaderLabels(rankHeaders);
    ui->productSalesTable->setColumnWidth(0, 60);
    ui->productSalesTable->setColumnWidth(1, 80);
    ui->productSalesTable->setColumnWidth(2, 200);
    ui->productSalesTable->setColumnWidth(3, 100);
    ui->productSalesTable->setColumnWidth(4, 120);

    QList<QTableWidget*> tables = {ui->salesDetailTable, ui->productSalesTable};
    for (QTableWidget *table : tables) {
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void InventoryWidget::clearTable()
{
    ui->inventoryTable->setRowCount(0);
}

// ========== 商品管理接口实现 ==========

void InventoryWidget::refreshInventoryData()
{
    QList<ProductInfo> products = DatabaseManager::instance().getAllProducts();

    for (ProductInfo &product : products) {
        InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(product.id);
        product.quantity = inv.quantity;
    }

    displayProducts(products);
    updateStatus(QString("加载了 %1 条商品记录").arg(products.size()));
}

bool InventoryWidget::addProduct(const ProductInfo &product)
{
    if (DatabaseManager::instance().addProduct(product)) {
        refreshInventoryData();
        showSuccess("商品添加成功");
        return true;
    }
    showError("商品添加失败");
    return false;
}

bool InventoryWidget::updateProduct(int productId, const ProductInfo &product)
{
    if (DatabaseManager::instance().updateProduct(product)) {
        refreshInventoryData();
        showSuccess("商品更新成功");
        return true;
    }
    showError("商品更新失败");
    return false;
}

bool InventoryWidget::deleteProduct(int productId)
{
    if (DatabaseManager::instance().deleteProduct(productId)) {
        refreshInventoryData();
        showSuccess("商品删除成功");
        return true;
    }
    showError("商品删除失败");
    return false;
}

bool InventoryWidget::updateStock(int productId, int newQuantity)
{
    InventoryInfo inv = DatabaseManager::instance().getInventoryByProductId(productId);
    if (DatabaseManager::instance().updateInventory(productId, newQuantity, inv.unitPrice)) {
        refreshInventoryData();
        showSuccess(QString("库存已更新为 %1 件").arg(newQuantity));
        return true;
    }
    showError("库存更新失败");
    return false;
}

void InventoryWidget::searchProductByName(const QString &name)
{
    if (name.isEmpty()) {
        refreshInventoryData();
        return;
    }

    QList<ProductInfo> results = DatabaseManager::instance().searchProductsByName(name);
    displayProducts(results);
    updateStatus(QString("找到 %1 条匹配记录").arg(results.size()));
}

int InventoryWidget::getCurrentSelectedProductId() const
{
    int currentRow = ui->inventoryTable->currentRow();
    if (currentRow < 0) return -1;
    return ui->inventoryTable->item(currentRow, 0)->text().toInt();
}

QList<ProductInfo> InventoryWidget::getAllProducts() const
{
    return m_currentProducts;
}

// ========== 统计接口实现 ==========

double InventoryWidget::getMonthlyIncome(int year, int month)
{
    QDateTime startDate(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime endDate = startDate.addMonths(1).addSecs(-1);
    return DatabaseManager::instance().getTotalSales(startDate, endDate);
}

double InventoryWidget::getMonthlySales(int year, int month)
{
    QDateTime startDate(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime endDate = startDate.addMonths(1).addSecs(-1);
    return DatabaseManager::instance().getTotalSales(startDate, endDate);
}

double InventoryWidget::getMonthlyPurchase(int year, int month)
{
    return 0; // TODO: 实现采购统计
}

double InventoryWidget::getMonthlyProfit(int year, int month)
{
    QDateTime startDate(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime endDate = startDate.addMonths(1).addSecs(-1);
    return DatabaseManager::instance().getTotalProfit(startDate, endDate);
}

void InventoryWidget::refreshStatistics(int year, int month)
{
    double sales = getMonthlySales(year, month);
    double profit = getMonthlyProfit(year, month);

    ui->salesValue->setText(QString("¥ %1").arg(sales, 0, 'f', 2));
    ui->profitValue->setText(QString("¥ %1").arg(profit, 0, 'f', 2));

    displaySalesDetails(getMonthlySalesDetails(year, month));
    displayProductSalesRanking(getProductSalesRanking(year, month));

    updateStatus(QString("已刷新 %1年%2月 统计数据").arg(year).arg(month));
}

// ========== 明细查询接口实现 ==========

QList<SalesOrderInfo> InventoryWidget::getMonthlySalesDetails(int year, int month)
{
    QDateTime startDate(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime endDate = startDate.addMonths(1).addSecs(-1);
    return DatabaseManager::instance().getSalesReport(startDate, endDate);
}

QList<ProductSalesStat> InventoryWidget::getProductSalesRanking(int year, int month, int limit)
{
    QDateTime startDate(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime endDate = startDate.addMonths(1).addSecs(-1);
    return DatabaseManager::instance().getProductSalesRanking(startDate, endDate, limit);
}

// ========== UI 槽函数实现 ==========

void InventoryWidget::onSearchButtonClicked()
{
    QString keyword = ui->searchLineEdit->text().trimmed();
    searchProductByName(keyword);
}

void InventoryWidget::onRefreshButtonClicked()
{
    ui->searchLineEdit->clear();
    refreshInventoryData();
    updateStatus("数据已刷新");
}

void InventoryWidget::onAddButtonClicked()
{
    ProductInfo newProduct;
    if (showAddProductDialog(newProduct)) {
        addProduct(newProduct);
    }
}

void InventoryWidget::onEditButtonClicked()
{
    int productId = getCurrentSelectedProductId();
    if (productId < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的商品");
        return;
    }

    ProductInfo product = DatabaseManager::instance().getProductById(productId);
    if (showEditProductDialog(product)) {
        updateProduct(productId, product);
    }
}

void InventoryWidget::onDeleteButtonClicked()
{
    int row = ui->inventoryTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    QString productName = ui->inventoryTable->item(row, 1)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除商品 \"%1\" 吗？").arg(productName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int productId = ui->inventoryTable->item(row, 0)->text().toInt();
        deleteProduct(productId);
    }
}

void InventoryWidget::onAdjustButtonClicked()
{
    int row = ui->inventoryTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择要盘点的商品");
        return;
    }

    int productId = ui->inventoryTable->item(row, 0)->text().toInt();
    int currentQuantity = ui->inventoryTable->item(row, 3)->text().toInt();
    QString productName = ui->inventoryTable->item(row, 1)->text();

    int newQuantity;
    if (showStockAdjustDialog(productId, currentQuantity, newQuantity)) {
        updateStock(productId, newQuantity);
    }
}

void InventoryWidget::onTableItemDoubleClicked(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    onEditButtonClicked();
}

void InventoryWidget::onQueryStatsButtonClicked()
{
    QDate selectedDate = ui->monthSelect->date();
    refreshStatistics(selectedDate.year(), selectedDate.month());
}

// ========== 内部辅助函数 ==========

void InventoryWidget::displayProducts(const QList<ProductInfo> &products)
{
    clearTable();
    m_currentProducts = products;

    for (int i = 0; i < products.size(); ++i) {
        ui->inventoryTable->insertRow(i);
        addRowToTable(products[i], i);
    }
}

void InventoryWidget::addRowToTable(const ProductInfo &product, int row)
{
    ui->inventoryTable->setItem(row, 0, new QTableWidgetItem(QString::number(product.id)));
    ui->inventoryTable->setItem(row, 1, new QTableWidgetItem(product.name));
    ui->inventoryTable->setItem(row, 2, new QTableWidgetItem(product.category));
    ui->inventoryTable->setItem(row, 3, new QTableWidgetItem(QString::number(product.quantity)));
    ui->inventoryTable->setItem(row, 4, new QTableWidgetItem(QString::number(product.purchasePrice, 'f', 2)));
    ui->inventoryTable->setItem(row, 5, new QTableWidgetItem(QString::number(product.salePrice, 'f', 2)));
    ui->inventoryTable->setItem(row, 6, new QTableWidgetItem(product.unit));
}

ProductInfo InventoryWidget::getProductFromCurrentRow() const
{
    ProductInfo product;
    int row = ui->inventoryTable->currentRow();
    if (row >= 0) {
        product.id = ui->inventoryTable->item(row, 0)->text().toInt();
        product.name = ui->inventoryTable->item(row, 1)->text();
        product.category = ui->inventoryTable->item(row, 2)->text();
        product.quantity = ui->inventoryTable->item(row, 3)->text().toInt();
        product.purchasePrice = ui->inventoryTable->item(row, 4)->text().toDouble();
        product.salePrice = ui->inventoryTable->item(row, 5)->text().toDouble();
        product.unit = ui->inventoryTable->item(row, 6)->text();
    }
    return product;
}

void InventoryWidget::displaySalesDetails(const QList<SalesOrderInfo> &orders)
{
    ui->salesDetailTable->setRowCount(0);
    m_currentSalesDetails = orders;

    for (int i = 0; i < orders.size(); ++i) {
        ui->salesDetailTable->insertRow(i);
        addSalesRowToTable(orders[i], i);
    }
}

void InventoryWidget::addSalesRowToTable(const SalesOrderInfo &order, int row)
{
    ui->salesDetailTable->setItem(row, 0, new QTableWidgetItem(QString::number(order.id)));
    ui->salesDetailTable->setItem(row, 1, new QTableWidgetItem(QString::number(order.totalAmount, 'f', 2)));
    ui->salesDetailTable->setItem(row, 2, new QTableWidgetItem(order.address));
    ui->salesDetailTable->setItem(row, 3, new QTableWidgetItem(order.remark));
    ui->salesDetailTable->setItem(row, 4, new QTableWidgetItem(order.createdAt.toString("yyyy-MM-dd")));
}

void InventoryWidget::displayProductSalesRanking(const QList<ProductSalesStat> &stats)
{
    ui->productSalesTable->setRowCount(0);
    m_currentProductSalesRank = stats;

    for (int i = 0; i < stats.size(); ++i) {
        ui->productSalesTable->insertRow(i);
        addProductSalesRowToTable(stats[i], i, i + 1);
    }
}

void InventoryWidget::addProductSalesRowToTable(const ProductSalesStat &stat, int row, int rank)
{
    ui->productSalesTable->setItem(row, 0, new QTableWidgetItem(QString::number(rank)));
    ui->productSalesTable->setItem(row, 1, new QTableWidgetItem(QString::number(stat.productId)));
    ui->productSalesTable->setItem(row, 2, new QTableWidgetItem(stat.productName));
    ui->productSalesTable->setItem(row, 3, new QTableWidgetItem(QString::number(stat.totalQuantity)));
    ui->productSalesTable->setItem(row, 4, new QTableWidgetItem(QString::number(stat.totalAmount, 'f', 2)));
}

void InventoryWidget::getCurrentYearMonth(int &year, int &month)
{
    QDate current = ui->monthSelect->date();
    year = current.year();
    month = current.month();
}

void InventoryWidget::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void InventoryWidget::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void InventoryWidget::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}

// ========== 对话框实现 ==========

bool InventoryWidget::showAddProductDialog(ProductInfo &product)
{
    QDialog dialog(this);
    dialog.setWindowTitle("新增商品");
    dialog.setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *nameEdit = new QLineEdit();
    QLineEdit *categoryEdit = new QLineEdit();
    QLineEdit *purchaseEdit = new QLineEdit();
    QLineEdit *saleEdit = new QLineEdit();
    QLineEdit *unitEdit = new QLineEdit("件");

    purchaseEdit->setValidator(new QDoubleValidator(0, 999999, 2, purchaseEdit));
    saleEdit->setValidator(new QDoubleValidator(0, 999999, 2, saleEdit));

    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("分类:", categoryEdit);
    formLayout->addRow("进价:", purchaseEdit);
    formLayout->addRow("售价:", saleEdit);
    formLayout->addRow("单位:", unitEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(this, "提示", "商品名称不能为空");
            return false;
        }
        product.name = nameEdit->text();
        product.category = categoryEdit->text();
        product.purchasePrice = purchaseEdit->text().toDouble();
        product.salePrice = saleEdit->text().toDouble();
        product.unit = unitEdit->text();
        return true;
    }
    return false;
}

bool InventoryWidget::showEditProductDialog(ProductInfo &product)
{
    QDialog dialog(this);
    dialog.setWindowTitle("编辑商品");
    dialog.setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *nameEdit = new QLineEdit(product.name);
    QLineEdit *categoryEdit = new QLineEdit(product.category);
    QLineEdit *purchaseEdit = new QLineEdit(QString::number(product.purchasePrice));
    QLineEdit *saleEdit = new QLineEdit(QString::number(product.salePrice));
    QLineEdit *unitEdit = new QLineEdit(product.unit);

    purchaseEdit->setValidator(new QDoubleValidator(0, 999999, 2, purchaseEdit));
    saleEdit->setValidator(new QDoubleValidator(0, 999999, 2, saleEdit));

    formLayout->addRow("商品ID:", new QLabel(QString::number(product.id)));
    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("分类:", categoryEdit);
    formLayout->addRow("进价:", purchaseEdit);
    formLayout->addRow("售价:", saleEdit);
    formLayout->addRow("单位:", unitEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(this, "提示", "商品名称不能为空");
            return false;
        }
        product.name = nameEdit->text();
        product.category = categoryEdit->text();
        product.purchasePrice = purchaseEdit->text().toDouble();
        product.salePrice = saleEdit->text().toDouble();
        product.unit = unitEdit->text();
        return true;
    }
    return false;
}

bool InventoryWidget::showStockAdjustDialog(int productId, int currentQuantity, int &newQuantity)
{
    QDialog dialog(this);
    dialog.setWindowTitle("库存盘点");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    formLayout->addRow("商品ID:", new QLabel(QString::number(productId)));
    formLayout->addRow("当前库存:", new QLabel(QString::number(currentQuantity)));

    QLineEdit *newQuantityEdit = new QLineEdit();
    newQuantityEdit->setValidator(new QIntValidator(0, 99999, newQuantityEdit));
    newQuantityEdit->setPlaceholderText("请输入盘点后的实际数量");
    formLayout->addRow("盘点后库存:", newQuantityEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        newQuantity = newQuantityEdit->text().toInt();
        return true;
    }
    return false;
}