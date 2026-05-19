#include "inventorywidget.h"
#include "ui_inventorywidget.h"
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

// ========== 构造与析构 ==========

InventoryWidget::InventoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InventoryWidget)
{
    ui->setupUi(this);
    setupTable();
    setupDetailTables();

    // 连接信号槽
    connect(ui->searchButton, &QPushButton::clicked, this, &InventoryWidget::onSearchButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &InventoryWidget::onRefreshButtonClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &InventoryWidget::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &InventoryWidget::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &InventoryWidget::onDeleteButtonClicked);
    connect(ui->adjustButton, &QPushButton::clicked, this, &InventoryWidget::onAdjustButtonClicked);
    connect(ui->inventoryTable, &QTableWidget::itemDoubleClicked, this, &InventoryWidget::onTableItemDoubleClicked);
    connect(ui->queryStatsButton, &QPushButton::clicked, this, &InventoryWidget::onQueryStatsButtonClicked);

    // 初始化
    refreshInventoryData();

    // 设置默认月份为当前月份
    ui->monthSelect->setDate(QDate::currentDate());
    QDate currentDate = QDate::currentDate();
    refreshStatistics(currentDate.year(), currentDate.month());

    updateStatus("就绪");
}

InventoryWidget::~InventoryWidget()
{
    delete ui;
}

// ========== 表格初始化 ==========

void InventoryWidget::setupTable()
{
    QStringList headers = {"商品ID", "商品名称", "库存量", "警戒线", "进价", "售价", "最后更新"};
    ui->inventoryTable->setColumnCount(headers.size());
    ui->inventoryTable->setHorizontalHeaderLabels(headers);

    ui->inventoryTable->setColumnWidth(0, 80);
    ui->inventoryTable->setColumnWidth(1, 150);
    ui->inventoryTable->setColumnWidth(2, 80);
    ui->inventoryTable->setColumnWidth(3, 80);
    ui->inventoryTable->setColumnWidth(4, 100);
    ui->inventoryTable->setColumnWidth(5, 100);
    ui->inventoryTable->setColumnWidth(6, 140);

    ui->inventoryTable->setAlternatingRowColors(true);
    ui->inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->inventoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->inventoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void InventoryWidget::setupDetailTables()
{
    // 销售明细表格
    QStringList salesHeaders = {"ID", "商品名称", "数量", "单价", "总金额", "客户", "销售时间"};
    ui->salesDetailTable->setColumnCount(salesHeaders.size());
    ui->salesDetailTable->setHorizontalHeaderLabels(salesHeaders);
    ui->salesDetailTable->setColumnWidth(0, 50);
    ui->salesDetailTable->setColumnWidth(1, 150);
    ui->salesDetailTable->setColumnWidth(2, 80);
    ui->salesDetailTable->setColumnWidth(3, 100);
    ui->salesDetailTable->setColumnWidth(4, 100);
    ui->salesDetailTable->setColumnWidth(5, 120);
    ui->salesDetailTable->setColumnWidth(6, 140);

    // 进货明细表格
    QStringList purchaseHeaders = {"ID", "商品名称", "数量", "单价", "总金额", "供应商", "进货时间"};
    ui->purchaseDetailTable->setColumnCount(purchaseHeaders.size());
    ui->purchaseDetailTable->setHorizontalHeaderLabels(purchaseHeaders);
    ui->purchaseDetailTable->setColumnWidth(0, 50);
    ui->purchaseDetailTable->setColumnWidth(1, 150);
    ui->purchaseDetailTable->setColumnWidth(2, 80);
    ui->purchaseDetailTable->setColumnWidth(3, 100);
    ui->purchaseDetailTable->setColumnWidth(4, 100);
    ui->purchaseDetailTable->setColumnWidth(5, 120);
    ui->purchaseDetailTable->setColumnWidth(6, 140);

    // 商品销量排行表格
    QStringList rankHeaders = {"排名", "商品ID", "商品名称", "销售数量", "销售金额"};
    ui->productSalesTable->setColumnCount(rankHeaders.size());
    ui->productSalesTable->setHorizontalHeaderLabels(rankHeaders);
    ui->productSalesTable->setColumnWidth(0, 60);
    ui->productSalesTable->setColumnWidth(1, 80);
    ui->productSalesTable->setColumnWidth(2, 150);
    ui->productSalesTable->setColumnWidth(3, 100);
    ui->productSalesTable->setColumnWidth(4, 120);

    QList<QTableWidget*> tables = {ui->salesDetailTable, ui->purchaseDetailTable, ui->productSalesTable};
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
    // TODO: 调用后端接口获取所有商品数据
    QList<ProductInfo> products;

    // 模拟数据 - 后端接入后删除
    ProductInfo p1;
    p1.id = 1001;
    p1.name = "笔记本电脑";
    p1.quantity = 15;
    p1.warningLevel = 5;
    p1.purchasePrice = 4500.0;
    p1.salePrice = 4999.0;
    p1.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
    products.append(p1);

    ProductInfo p2;
    p2.id = 1002;
    p2.name = "无线鼠标";
    p2.quantity = 50;
    p2.warningLevel = 10;
    p2.purchasePrice = 25.0;
    p2.salePrice = 39.0;
    p2.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
    products.append(p2);

    ProductInfo p3;
    p3.id = 1003;
    p3.name = "机械键盘";
    p3.quantity = 23;
    p3.warningLevel = 8;
    p3.purchasePrice = 180.0;
    p3.salePrice = 249.0;
    p3.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
    products.append(p3);

    displayProducts(products);
    updateStatus(QString("加载了 %1 条商品记录").arg(products.size()));
}

bool InventoryWidget::addProduct(const ProductInfo &product)
{
    // TODO: 调用后端接口添加商品
    Q_UNUSED(product);
    refreshInventoryData();
    return true;
}

bool InventoryWidget::updateProduct(int productId, const ProductInfo &product)
{
    // TODO: 调用后端接口更新商品
    Q_UNUSED(productId);
    Q_UNUSED(product);
    refreshInventoryData();
    return true;
}

bool InventoryWidget::deleteProduct(int productId)
{
    // TODO: 调用后端接口删除商品
    Q_UNUSED(productId);
    refreshInventoryData();
    return true;
}

bool InventoryWidget::updateStock(int productId, int newQuantity)
{
    // TODO: 调用后端接口更新库存
    Q_UNUSED(productId);
    Q_UNUSED(newQuantity);
    refreshInventoryData();
    return true;
}

void InventoryWidget::searchProductByName(const QString &name)
{
    if (name.isEmpty()) {
        refreshInventoryData();
        return;
    }

    // TODO: 调用后端接口搜索
    QList<ProductInfo> results;
    for (const ProductInfo &product : m_currentProducts) {
        if (product.name.contains(name, Qt::CaseInsensitive)) {
            results.append(product);
        }
    }

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
    // TODO: 月收入 = 销售额 - 进货额
    return getMonthlySales(year, month) - getMonthlyPurchase(year, month);
}

double InventoryWidget::getMonthlySales(int year, int month)
{
    // TODO: 从数据库查询月销售额
    Q_UNUSED(year);
    Q_UNUSED(month);
    return 12580.00;  // 模拟数据
}

double InventoryWidget::getMonthlyPurchase(int year, int month)
{
    // TODO: 从数据库查询月进货额
    Q_UNUSED(year);
    Q_UNUSED(month);
    return 8230.00;  // 模拟数据
}

double InventoryWidget::getMonthlyProfit(int year, int month)
{
    // TODO: 利润需要从商品进价计算
    Q_UNUSED(year);
    Q_UNUSED(month);
    return 4350.00;  // 模拟数据
}

void InventoryWidget::refreshStatistics(int year, int month)
{
    double sales = getMonthlySales(year, month);
    double purchase = getMonthlyPurchase(year, month);
    double income = getMonthlyIncome(year, month);
    double profit = getMonthlyProfit(year, month);

    ui->salesValue->setText(QString("¥ %1").arg(sales, 0, 'f', 2));
    ui->purchaseValue->setText(QString("¥ %1").arg(purchase, 0, 'f', 2));
    ui->monthlyIncomeValue->setText(QString("¥ %1").arg(income, 0, 'f', 2));
    ui->profitValue->setText(QString("¥ %1").arg(profit, 0, 'f', 2));

    // 刷新明细表格
    displaySalesDetails(getMonthlySalesDetails(year, month));
    displayPurchaseDetails(getMonthlyPurchaseDetails(year, month));
    displayProductSalesRanking(getProductSalesRanking(year, month));

    updateStatus(QString("已刷新 %1年%2月 统计数据").arg(year).arg(month));
}

// ========== 明细查询接口实现 ==========

QList<SalesRecord> InventoryWidget::getMonthlySalesDetails(int year, int month)
{
    QList<SalesRecord> records;
    // TODO: 从数据库查询
    Q_UNUSED(year);
    Q_UNUSED(month);

    SalesRecord r1;
    r1.id = 1;
    r1.productName = "笔记本电脑";
    r1.quantity = 2;
    r1.price = 4999.00;
    r1.total = 9998.00;
    r1.customer = "张三";
    r1.saleTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    records.append(r1);

    SalesRecord r2;
    r2.id = 2;
    r2.productName = "无线鼠标";
    r2.quantity = 5;
    r2.price = 39.00;
    r2.total = 195.00;
    r2.customer = "李四";
    r2.saleTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    records.append(r2);

    return records;
}

QList<PurchaseRecord> InventoryWidget::getMonthlyPurchaseDetails(int year, int month)
{
    QList<PurchaseRecord> records;
    // TODO: 从数据库查询
    Q_UNUSED(year);
    Q_UNUSED(month);

    PurchaseRecord r1;
    r1.id = 1;
    r1.productName = "笔记本电脑";
    r1.quantity = 10;
    r1.price = 4500.00;
    r1.total = 45000.00;
    r1.supplier = "联想科技";
    r1.purchaseTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    records.append(r1);

    PurchaseRecord r2;
    r2.id = 2;
    r2.productName = "无线鼠标";
    r2.quantity = 50;
    r2.price = 25.00;
    r2.total = 1250.00;
    r2.supplier = "罗技电子";
    r2.purchaseTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    records.append(r2);

    return records;
}

QList<ProductSalesStat> InventoryWidget::getProductSalesRanking(int year, int month, int limit)
{
    QList<ProductSalesStat> stats;
    // TODO: 从数据库查询
    Q_UNUSED(year);
    Q_UNUSED(month);
    Q_UNUSED(limit);

    ProductSalesStat p1;
    p1.productId = 1001;
    p1.productName = "笔记本电脑";
    p1.totalQuantity = 15;
    p1.totalAmount = 74985.00;
    stats.append(p1);

    ProductSalesStat p2;
    p2.productId = 1002;
    p2.productName = "无线鼠标";
    p2.totalQuantity = 120;
    p2.totalAmount = 4680.00;
    stats.append(p2);

    ProductSalesStat p3;
    p3.productId = 1003;
    p3.productName = "机械键盘";
    p3.totalQuantity = 45;
    p3.totalAmount = 11205.00;
    stats.append(p3);

    return stats;
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

    ProductInfo product = getProductFromCurrentRow();
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
    int currentQuantity = ui->inventoryTable->item(row, 2)->text().toInt();
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

        if (products[i].quantity <= products[i].warningLevel) {
            for (int col = 0; col < ui->inventoryTable->columnCount(); ++col) {
                QTableWidgetItem *item = ui->inventoryTable->item(i, col);
                if (item) item->setForeground(Qt::red);
            }
        }
    }
}

void InventoryWidget::addRowToTable(const ProductInfo &product, int row)
{
    ui->inventoryTable->setItem(row, 0, new QTableWidgetItem(QString::number(product.id)));
    ui->inventoryTable->setItem(row, 1, new QTableWidgetItem(product.name));
    ui->inventoryTable->setItem(row, 2, new QTableWidgetItem(QString::number(product.quantity)));
    ui->inventoryTable->setItem(row, 3, new QTableWidgetItem(QString::number(product.warningLevel)));
    ui->inventoryTable->setItem(row, 4, new QTableWidgetItem(QString::number(product.purchasePrice, 'f', 2)));
    ui->inventoryTable->setItem(row, 5, new QTableWidgetItem(QString::number(product.salePrice, 'f', 2)));
    ui->inventoryTable->setItem(row, 6, new QTableWidgetItem(product.lastUpdate));
}

ProductInfo InventoryWidget::getProductFromCurrentRow() const
{
    ProductInfo product;
    int row = ui->inventoryTable->currentRow();
    if (row >= 0) {
        product.id = ui->inventoryTable->item(row, 0)->text().toInt();
        product.name = ui->inventoryTable->item(row, 1)->text();
        product.quantity = ui->inventoryTable->item(row, 2)->text().toInt();
        product.warningLevel = ui->inventoryTable->item(row, 3)->text().toInt();
        product.purchasePrice = ui->inventoryTable->item(row, 4)->text().toDouble();
        product.salePrice = ui->inventoryTable->item(row, 5)->text().toDouble();
        product.lastUpdate = ui->inventoryTable->item(row, 6)->text();
    }
    return product;
}

void InventoryWidget::displaySalesDetails(const QList<SalesRecord> &records)
{
    ui->salesDetailTable->setRowCount(0);
    m_currentSalesDetails = records;

    for (int i = 0; i < records.size(); ++i) {
        const SalesRecord &record = records[i];
        ui->salesDetailTable->insertRow(i);
        ui->salesDetailTable->setItem(i, 0, new QTableWidgetItem(QString::number(record.id)));
        ui->salesDetailTable->setItem(i, 1, new QTableWidgetItem(record.productName));
        ui->salesDetailTable->setItem(i, 2, new QTableWidgetItem(QString::number(record.quantity)));
        ui->salesDetailTable->setItem(i, 3, new QTableWidgetItem(QString::number(record.price, 'f', 2)));
        ui->salesDetailTable->setItem(i, 4, new QTableWidgetItem(QString::number(record.total, 'f', 2)));
        ui->salesDetailTable->setItem(i, 5, new QTableWidgetItem(record.customer));
        ui->salesDetailTable->setItem(i, 6, new QTableWidgetItem(record.saleTime));
    }
}

void InventoryWidget::displayPurchaseDetails(const QList<PurchaseRecord> &records)
{
    ui->purchaseDetailTable->setRowCount(0);
    m_currentPurchaseDetails = records;

    for (int i = 0; i < records.size(); ++i) {
        const PurchaseRecord &record = records[i];
        ui->purchaseDetailTable->insertRow(i);
        ui->purchaseDetailTable->setItem(i, 0, new QTableWidgetItem(QString::number(record.id)));
        ui->purchaseDetailTable->setItem(i, 1, new QTableWidgetItem(record.productName));
        ui->purchaseDetailTable->setItem(i, 2, new QTableWidgetItem(QString::number(record.quantity)));
        ui->purchaseDetailTable->setItem(i, 3, new QTableWidgetItem(QString::number(record.price, 'f', 2)));
        ui->purchaseDetailTable->setItem(i, 4, new QTableWidgetItem(QString::number(record.total, 'f', 2)));
        ui->purchaseDetailTable->setItem(i, 5, new QTableWidgetItem(record.supplier));
        ui->purchaseDetailTable->setItem(i, 6, new QTableWidgetItem(record.purchaseTime));
    }
}

void InventoryWidget::displayProductSalesRanking(const QList<ProductSalesStat> &stats)
{
    ui->productSalesTable->setRowCount(0);
    m_currentProductSalesRank = stats;

    for (int i = 0; i < stats.size(); ++i) {
        const ProductSalesStat &stat = stats[i];
        ui->productSalesTable->insertRow(i);
        ui->productSalesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        ui->productSalesTable->setItem(i, 1, new QTableWidgetItem(QString::number(stat.productId)));
        ui->productSalesTable->setItem(i, 2, new QTableWidgetItem(stat.productName));
        ui->productSalesTable->setItem(i, 3, new QTableWidgetItem(QString::number(stat.totalQuantity)));
        ui->productSalesTable->setItem(i, 4, new QTableWidgetItem(QString::number(stat.totalAmount, 'f', 2)));
    }
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
    QLineEdit *quantityEdit = new QLineEdit();
    QLineEdit *warningEdit = new QLineEdit();
    QLineEdit *purchaseEdit = new QLineEdit();
    QLineEdit *saleEdit = new QLineEdit();

    quantityEdit->setValidator(new QIntValidator(0, 99999, quantityEdit));
    warningEdit->setValidator(new QIntValidator(0, 99999, warningEdit));
    purchaseEdit->setValidator(new QDoubleValidator(0, 999999, 2, purchaseEdit));
    saleEdit->setValidator(new QDoubleValidator(0, 999999, 2, saleEdit));

    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("初始库存:", quantityEdit);
    formLayout->addRow("警戒线:", warningEdit);
    formLayout->addRow("进价:", purchaseEdit);
    formLayout->addRow("售价:", saleEdit);

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
        product.quantity = quantityEdit->text().toInt();
        product.warningLevel = warningEdit->text().toInt();
        product.purchasePrice = purchaseEdit->text().toDouble();
        product.salePrice = saleEdit->text().toDouble();
        product.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
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
    QLineEdit *quantityEdit = new QLineEdit(QString::number(product.quantity));
    QLineEdit *warningEdit = new QLineEdit(QString::number(product.warningLevel));
    QLineEdit *purchaseEdit = new QLineEdit(QString::number(product.purchasePrice));
    QLineEdit *saleEdit = new QLineEdit(QString::number(product.salePrice));

    quantityEdit->setValidator(new QIntValidator(0, 99999, quantityEdit));
    warningEdit->setValidator(new QIntValidator(0, 99999, warningEdit));
    purchaseEdit->setValidator(new QDoubleValidator(0, 999999, 2, purchaseEdit));
    saleEdit->setValidator(new QDoubleValidator(0, 999999, 2, saleEdit));

    formLayout->addRow("商品ID:", new QLabel(QString::number(product.id)));
    formLayout->addRow("商品名称:", nameEdit);
    formLayout->addRow("库存量:", quantityEdit);
    formLayout->addRow("警戒线:", warningEdit);
    formLayout->addRow("进价:", purchaseEdit);
    formLayout->addRow("售价:", saleEdit);

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
        product.quantity = quantityEdit->text().toInt();
        product.warningLevel = warningEdit->text().toInt();
        product.purchasePrice = purchaseEdit->text().toDouble();
        product.salePrice = saleEdit->text().toDouble();
        product.lastUpdate = QDate::currentDate().toString("yyyy-MM-dd");
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

    QLabel *tipLabel = new QLabel("提示：\n• 盘盈：新库存 > 当前库存\n• 盘亏：新库存 < 当前库存");
    tipLabel->setStyleSheet("color: gray; font-size: 11px;");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(tipLabel);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        newQuantity = newQuantityEdit->text().toInt();
        int diff = newQuantity - currentQuantity;
        if (diff > 0) {
            QMessageBox::information(&dialog, "盘盈", QString("盘盈数量: +%1").arg(diff));
        } else if (diff < 0) {
            QMessageBox::information(&dialog, "盘亏", QString("盘亏数量: %1").arg(diff));
        }
        return true;
    }
    return false;
}