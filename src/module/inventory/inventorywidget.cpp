#include "inventorywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>

InventoryWidget::InventoryWidget(int userId, int role, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_role(role)
{
    setupUI(role);
}

InventoryWidget::~InventoryWidget()
{
}

void InventoryWidget::setupUI(int role)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部搜索栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索商品名称...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("搜索", this);
    m_searchBtn->setFixedSize(80, 32);
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setFixedSize(80, 32);

    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_searchBtn);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addStretch();

    if (role == 0) {
        // 普通用户：注册管理员按钮
        m_registerAdminBtn = new QPushButton("注册管理员", this);
        m_registerAdminBtn->setFixedSize(120, 32);
        m_registerAdminBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 4px; }");
        topLayout->addWidget(m_registerAdminBtn);
        connect(m_registerAdminBtn, &QPushButton::clicked, this, &InventoryWidget::onRegisterAdminClicked);
    } else {
        // 管理员：增删改按钮
        m_addBtn = new QPushButton("添加商品", this);
        m_editBtn = new QPushButton("编辑商品", this);
        m_deleteBtn = new QPushButton("删除商品", this);
        m_addBtn->setFixedSize(100, 32);
        m_editBtn->setFixedSize(100, 32);
        m_deleteBtn->setFixedSize(100, 32);
        m_addBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; }");
        m_editBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; }");
        m_deleteBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 4px; }");
        topLayout->addWidget(m_addBtn);
        topLayout->addWidget(m_editBtn);
        topLayout->addWidget(m_deleteBtn);
        connect(m_addBtn, &QPushButton::clicked, this, &InventoryWidget::onAddClicked);
        connect(m_editBtn, &QPushButton::clicked, this, &InventoryWidget::onEditClicked);
        connect(m_deleteBtn, &QPushButton::clicked, this, &InventoryWidget::onDeleteClicked);
    }

    mainLayout->addLayout(topLayout);

    // 表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(6);
    QStringList headers = {"ID", "商品名称", "分类", "售价", "库存", "单位"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(m_tableWidget);

    // 连接信号 - 修复 itemDoubleClicked 使用 lambda
    connect(m_searchBtn, &QPushButton::clicked, this, &InventoryWidget::onSearchClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &InventoryWidget::onRefreshClicked);

    // 修复：使用 lambda 连接 itemDoubleClicked 信号
    connect(m_tableWidget, &QTableWidget::itemDoubleClicked,
            [this](QTableWidgetItem* item) {
                if (item) {
                    onTableItemDoubleClicked(item->row(), item->column());
                }
            });

    // 初始化时请求数据
    emit refreshRequested();
}

void InventoryWidget::onSearchClicked()
{
    QString keyword = m_searchEdit->text().trimmed();
    emit searchRequested(keyword);
}

void InventoryWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void InventoryWidget::onAddClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("添加商品");
    dialog.setFixedSize(350, 300);

    QFormLayout* form = new QFormLayout(&dialog);
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameEdit->setMaxLength(30);
    QLineEdit* categoryEdit = new QLineEdit(&dialog);
    categoryEdit->setMaxLength(30);
    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0, 999999);
    priceSpin->setPrefix("¥");
    priceSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QSpinBox* stockSpin = new QSpinBox(&dialog);
    stockSpin->setRange(0, 99999);
    stockSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QLineEdit* unitEdit = new QLineEdit(&dialog);
    unitEdit->setText("件");
    unitEdit->setMaxLength(5);

    form->addRow("商品名称:", nameEdit);
    form->addRow("分类:", categoryEdit);
    form->addRow("售价:", priceSpin);
    form->addRow("库存:", stockSpin);
    form->addRow("单位:", unitEdit);

    QPushButton* submitBtn = new QPushButton("确定", &dialog);
    QPushButton* cancelBtn = new QPushButton("取消", &dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    form->addRow(btnLayout);

    connect(submitBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请输入商品名称");
            return;
        }
        QVariantMap product;
        product["name"] = nameEdit->text();
        product["category"] = categoryEdit->text();
        product["salePrice"] = priceSpin->value();
        product["quantity"] = stockSpin->value();
        product["unit"] = unitEdit->text();
        emit addProductRequested(product);
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void InventoryWidget::onEditClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的商品");
        return;
    }

    int productId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QString currentName = m_tableWidget->item(currentRow, 1)->text();
    QString currentCategory = m_tableWidget->item(currentRow, 2)->text();
    double currentPrice = m_tableWidget->item(currentRow, 3)->text().toDouble();
    int currentStock = m_tableWidget->item(currentRow, 4)->text().toInt();
    QString currentUnit = m_tableWidget->item(currentRow, 5)->text();

    QDialog dialog(this);
    dialog.setWindowTitle("编辑商品");
    dialog.setFixedSize(350, 300);

    QFormLayout* form = new QFormLayout(&dialog);
    QLineEdit* nameEdit = new QLineEdit(currentName, &dialog);
    nameEdit->setMaxLength(30);
    QLineEdit* categoryEdit = new QLineEdit(currentCategory, &dialog);
    categoryEdit->setMaxLength(30);
    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0, 999999);
    priceSpin->setPrefix("¥");
    priceSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    priceSpin->setValue(currentPrice);
    QSpinBox* stockSpin = new QSpinBox(&dialog);
    stockSpin->setRange(0, 99999);
    stockSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    stockSpin->setValue(currentStock);
    QLineEdit* unitEdit = new QLineEdit(currentUnit, &dialog);
    unitEdit->setMaxLength(5);

    form->addRow("商品名称:", nameEdit);
    form->addRow("分类:", categoryEdit);
    form->addRow("售价:", priceSpin);
    form->addRow("库存:", stockSpin);
    form->addRow("单位:", unitEdit);

    QPushButton* submitBtn = new QPushButton("确定", &dialog);
    QPushButton* cancelBtn = new QPushButton("取消", &dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    form->addRow(btnLayout);

    connect(submitBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请输入商品名称");
            return;
        }
        QVariantMap product;
        product["name"] = nameEdit->text();
        product["category"] = categoryEdit->text();
        product["salePrice"] = priceSpin->value();
        product["quantity"] = stockSpin->value();
        product["unit"] = unitEdit->text();
        emit updateProductRequested(productId, product);
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void InventoryWidget::onDeleteClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    int productId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QString productName = m_tableWidget->item(currentRow, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除商品「%1」吗？").arg(productName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        emit deleteProductRequested(productId);
    }
}

void InventoryWidget::onRegisterAdminClicked()
{
    bool ok;
    QString code = QInputDialog::getText(this, "注册管理员",
                                         "请输入管理员验证码:",
                                         QLineEdit::Password,
                                         "", &ok);
    if (ok && !code.isEmpty()) {
        emit adminRegisterRequested(code);
    }
}

void InventoryWidget::onTableItemDoubleClicked(int row, int col)
{
    Q_UNUSED(col)
    int productId = m_tableWidget->item(row, 0)->text().toInt();
    emit productSelected(productId);
}

// ========== 后端调用的槽 ==========

void InventoryWidget::onInventoryLoaded(const QList<QVariantMap>& products)
{
    m_tableWidget->setRowCount(products.size());
    for (int i = 0; i < products.size(); i++) {
        const QVariantMap& p = products[i];
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(p["id"].toInt())));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(p["name"].toString()));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(p["category"].toString()));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(p["salePrice"].toDouble())));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(p["quantity"].toInt())));
        m_tableWidget->setItem(i, 5, new QTableWidgetItem(p["unit"].toString()));
    }
}

void InventoryWidget::onSearchResult(const QList<QVariantMap>& products)
{
    onInventoryLoaded(products);
}

void InventoryWidget::onOperationSuccess(const QString& message)
{
    QMessageBox::information(this, "成功", message);
    emit refreshRequested();
}

void InventoryWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "失败", error);
}

void InventoryWidget::onAdminRegisterResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "注册成功", "注册管理员成功，请重新登录");
        emit logoutRequested();
    } else {
        QMessageBox::warning(this, "注册失败", message);
    }
}
