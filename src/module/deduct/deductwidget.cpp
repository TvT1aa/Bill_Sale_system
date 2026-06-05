#include "deductwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QCompleter>
#include <QPushButton>
#include <QRegularExpression>

DeductWidget::DeductWidget(int userId, int mode, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_mode(mode)
    , m_currentTotal(0)
{
    setupUI(mode);
}

DeductWidget::~DeductWidget()
{
}

void DeductWidget::setupUI(int mode)
{
    if (mode == 0) {
        setupCheckoutUI();
    } else {
        setupAdminUI();
    }
}

// ========== 用户结算界面（去余额干净版） ==========
void DeductWidget::setupCheckoutUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel* titleLabel = new QLabel("Order Checkout", this);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #303133; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 商品列表
    QLabel* itemsLabel = new QLabel("Product List", this);
    itemsLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(itemsLabel);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({"Product Name", "Price", "Quantity", "Subtotal"});
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setFixedHeight(200);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(m_tableWidget);

    // 收货地址区域
    QLabel* addressLabel = new QLabel("Shipping Address", this);
    addressLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(addressLabel);

    QHBoxLayout* addressLayout = new QHBoxLayout();
    addressLayout->addWidget(new QLabel("Select Address:", this));
    m_addressCombo = new QComboBox(this);
    m_addressCombo->setMinimumWidth(400);
    addressLayout->addWidget(m_addressCombo);

    m_addAddressBtn = new QPushButton("➕ Add Address", this);
    m_addAddressBtn->setFixedSize(100, 32);
    m_addAddressBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 12px; }");

    m_refreshAddressBtn = new QPushButton("🔄 Refresh", this);
    m_refreshAddressBtn->setFixedSize(70, 32);
    m_refreshAddressBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; font-size: 12px; }");

    addressLayout->addWidget(m_addAddressBtn);
    addressLayout->addWidget(m_refreshAddressBtn);
    addressLayout->addStretch();
    mainLayout->addLayout(addressLayout);

    // 金额看板组件（移除我的余额，只留总额）
    QWidget* infoWidget = new QWidget(this);
    infoWidget->setStyleSheet("QWidget { background-color: #F5F7FA; border-radius: 8px; }");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);

    m_totalLabel = new QLabel("Order Total: ¥0.00", this);
    m_totalLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #F56C6C; }");
    infoLayout->addWidget(m_totalLabel);

    mainLayout->addWidget(infoWidget);

    // 确认按钮布局
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_submitBtn = new QPushButton("Place Order", this);
    m_submitBtn->setFixedSize(150, 40);
    m_submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    btnLayout->addWidget(m_submitBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(m_submitBtn, &QPushButton::clicked, this, &DeductWidget::onSubmitClicked);
    connect(m_addAddressBtn, &QPushButton::clicked, this, &DeductWidget::onAddNewAddress);
    connect(m_refreshAddressBtn, &QPushButton::clicked, this, &DeductWidget::onRefreshAddresses);
}

// ========== 管理员出库界面（完全保留核心数据看板） ==========
void DeductWidget::setupAdminUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 搜索栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search order ID/user/product...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("Search", this);
    m_searchBtn->setFixedSize(80, 32);

    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_searchBtn);
    topLayout->addSpacing(20);

    QLabel* dateLabel = new QLabel("Date Range:", this);
    m_startDateEdit = new QDateTimeEdit(this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDateTime(QDateTime::currentDateTime().addDays(-30));
    m_endDateEdit = new QDateTimeEdit(this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDateTime(QDateTime::currentDateTime());
    m_filterBtn = new QPushButton("Filter", this);
    m_filterBtn->setFixedSize(80, 32);

    topLayout->addWidget(dateLabel);
    topLayout->addWidget(m_startDateEdit);
    topLayout->addWidget(m_endDateEdit);
    topLayout->addWidget(m_filterBtn);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // 操作按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_viewDetailBtn = new QPushButton("View Detail", this);
    m_updateStatusBtn = new QPushButton("Update Status", this);
    m_manualDeductBtn = new QPushButton("Manual Dispatch", this);
    m_refreshBtn = new QPushButton("Refresh", this);

    m_viewDetailBtn->setFixedSize(100, 32);
    m_updateStatusBtn->setFixedSize(100, 32);
    m_manualDeductBtn->setFixedSize(100, 32);
    m_refreshBtn->setFixedSize(80, 32);

    m_viewDetailBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; }");
    m_updateStatusBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 4px; }");
    m_manualDeductBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 4px; }");

    btnLayout->addWidget(m_viewDetailBtn);
    btnLayout->addWidget(m_updateStatusBtn);
    btnLayout->addWidget(m_manualDeductBtn);
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // 状态筛选
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->addWidget(new QLabel("Order Status:", this));
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItems({"All", "Pending", "Confirmed", "Shipped", "Completed", "Cancelled"});
    statusLayout->addWidget(m_statusCombo);
    statusLayout->addStretch();
    mainLayout->addLayout(statusLayout);

    // 订单表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(7);
    QStringList headers = {"Order ID", "User ID", "Username", "Total", "Status", "Created At", "Action"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(m_tableWidget);

    // 连接信号
    connect(m_searchBtn, &QPushButton::clicked, this, &DeductWidget::onSearchClicked);
    connect(m_filterBtn, &QPushButton::clicked, this, &DeductWidget::onFilterClicked);
    connect(m_viewDetailBtn, &QPushButton::clicked, this, &DeductWidget::onViewDetail);
    connect(m_updateStatusBtn, &QPushButton::clicked, this, &DeductWidget::onUpdateStatus);
    connect(m_manualDeductBtn, &QPushButton::clicked, this, &DeductWidget::onManualDeduct);
    connect(m_refreshBtn, &QPushButton::clicked, this, &DeductWidget::onRefreshClicked);
    connect(m_tableWidget, &QTableWidget::itemDoubleClicked,
            [this](QTableWidgetItem* item) {
                if (item) onTableItemDoubleClicked(item->row(), item->column());
            });
}

// ========== 添加地址对话框 ==========
void DeductWidget::showAddressDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add Address");
    dialog.setFixedSize(450, 550);
    dialog.setModal(true);

    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setSpacing(15);
    dialogLayout->setContentsMargins(20, 20, 20, 20);

    QFormLayout* form = new QFormLayout();
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Enter recipient name");
    nameEdit->setMinimumHeight(36);
    form->addRow("Name:", nameEdit);

    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setPlaceholderText("Enter phone number");
    phoneEdit->setMinimumHeight(36);
    form->addRow("Phone:", phoneEdit);

    QLineEdit* provinceEdit = new QLineEdit(&dialog);
    provinceEdit->setPlaceholderText("Enter province");
    provinceEdit->setMinimumHeight(36);
    QStringList provinces = {"北京市", "上海市", "广东省", "江苏省", "浙江省", "四川省", "湖北省", "湖南省", "福建省", "山东省", "河南省", "河北省", "安徽省", "陕西省", "重庆市"};
    QCompleter* provinceCompleter = new QCompleter(provinces, &dialog);
    provinceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    provinceEdit->setCompleter(provinceCompleter);
    form->addRow("Province:", provinceEdit);

    QLineEdit* cityEdit = new QLineEdit(&dialog);
    cityEdit->setPlaceholderText("Enter city");
    cityEdit->setMinimumHeight(36);
    form->addRow("City:", cityEdit);

    QLineEdit* districtEdit = new QLineEdit(&dialog);
    districtEdit->setPlaceholderText("Enter district");
    districtEdit->setMinimumHeight(36);
    form->addRow("District:", districtEdit);

    QTextEdit* detailEdit = new QTextEdit(&dialog);
    detailEdit->setPlaceholderText("Enter detailed address (street, community, door number)");
    detailEdit->setFixedHeight(80);
    form->addRow("Detail:", detailEdit);

    QComboBox* isDefaultCombo = new QComboBox(&dialog);
    isDefaultCombo->addItems({"No", "Yes"});
    isDefaultCombo->setMinimumHeight(36);
    form->addRow("Set Default:", isDefaultCombo);

    dialogLayout->addLayout(form);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);
    QPushButton* submitBtn = new QPushButton("Add", &dialog);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    submitBtn->setFixedSize(120, 40);
    cancelBtn->setFixedSize(80, 40);
    submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #909399; color: white; border-radius: 4px; font-size: 14px; }");
    btnLayout->addStretch();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();
    dialogLayout->addLayout(btnLayout);

    connect(submitBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Tip", "Please enter recipient name");
            return;
        }
        if (phoneEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Tip", "Please enter phone number");
            return;
        }
        QString phone = phoneEdit->text();
        if (phone.length() != 11 || !phone.contains(QRegularExpression("^1[3-9]\\d{9}$"))) {
            QMessageBox::warning(&dialog, "Tip", "Please enter a valid phone number");
            return;
        }
        if (detailEdit->toPlainText().isEmpty()) {
            QMessageBox::warning(&dialog, "Tip", "Please enter detailed address");
            return;
        }

        QVariantMap address;
        address["name"] = nameEdit->text();
        address["phone"] = phoneEdit->text();
        address["province"] = provinceEdit->text();
        address["city"] = cityEdit->text();
        address["district"] = districtEdit->text();
        address["detail"] = detailEdit->toPlainText();
        address["isDefault"] = (isDefaultCombo->currentIndex() == 1);
        address["userId"] = m_userId;

        emit addAddressRequested(address);
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

// ========== 用户结算槽（删除了所有余额判断防护） ==========
void DeductWidget::onSubmitClicked()
{
    if (m_addressCombo->currentData().isNull()) {
        QMessageBox::warning(this, "Tip", "Please select a shipping address");
        return;
    }

    // 直接放行，允许生成线下结算/待后台确认的订单
    int addressId = m_addressCombo->currentData().toInt();
    emit submitOrderRequested(addressId, "");
}

void DeductWidget::onAddNewAddress()
{
    showAddressDialog();
}

void DeductWidget::onRefreshAddresses()
{
    emit refreshAddressesRequested();
}

void DeductWidget::onCartItemsLoaded(const QList<QVariantMap>& items, double total)
{
    m_currentTotal = total;
    m_tableWidget->setRowCount(items.size());
    for (int i = 0; i < items.size(); i++) {
        const QVariantMap& item = items[i];
        double subtotal = item["quantity"].toInt() * item["price"].toDouble();
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(item["productName"].toString()));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(QString("¥%1").arg(item["price"].toDouble(), 0, 'f', 2)));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(item["quantity"].toInt())));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(QString("¥%1").arg(subtotal, 0, 'f', 2)));
    }
    m_totalLabel->setText(QString("Order Total: ¥%1").arg(total, 0, 'f', 2));

    // 只要有商品就开启下单功能，不再管账户内有没有余额
    m_submitBtn->setEnabled(true);
    m_submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
}

void DeductWidget::onAddressesLoaded(const QList<QVariantMap>& addresses)
{
    m_addressCombo->clear();
    for (const auto& addr : addresses) {
        QString fullAddr = QString("%1 %2 %3 %4")
            .arg(addr["province"].toString())
            .arg(addr["city"].toString())
            .arg(addr["district"].toString())
            .arg(addr["detail"].toString());
        m_addressCombo->addItem(fullAddr, addr["id"].toInt());

        if (addr["isDefault"].toBool()) {
            int index = m_addressCombo->count() - 1;
            m_addressCombo->setCurrentIndex(index);
        }
    }
}

void DeductWidget::onOrderResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "Order Success", message);
        emit loadCheckoutDataRequested();
    } else {
        QMessageBox::warning(this, "Order Failed", message);
    }
}

void DeductWidget::onAddAddressResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "Success", message);
        emit refreshAddressesRequested();
    } else {
        QMessageBox::warning(this, "Failed", message);
    }
}

// ========== 管理员槽 ==========
void DeductWidget::onSearchClicked()
{
    emit searchOrderRequested(m_searchEdit->text().trimmed());
}

void DeductWidget::onFilterClicked()
{
    emit filterByDateRequested(m_startDateEdit->dateTime(), m_endDateEdit->dateTime());
}

void DeductWidget::onViewDetail()
{
    int row = m_tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Tip", "Please select an order first");
        return;
    }
    showOrderDetailDialog(m_tableWidget->item(row, 0)->text().toInt());
}

void DeductWidget::onUpdateStatus()
{
    int row = m_tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Tip", "Please select an order first");
        return;
    }
    emit updateOrderStatusRequested(m_tableWidget->item(row, 0)->text().toInt(), m_statusCombo->currentIndex());
}

void DeductWidget::onManualDeduct()
{
    showManualDeductDialog();
}

void DeductWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void DeductWidget::onTableItemDoubleClicked(int row, int col)
{
    Q_UNUSED(col)
    showOrderDetailDialog(m_tableWidget->item(row, 0)->text().toInt());
}

void DeductWidget::onOrdersLoaded(const QList<QVariantMap>& orders)
{
    m_tableWidget->setRowCount(orders.size());
    for (int i = 0; i < orders.size(); i++) {
        const QVariantMap& order = orders[i];
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(order["id"].toInt())));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(order["userId"].toInt())));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(order["username"].toString()));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(order["totalAmount"].toDouble(), 'f', 2)));

        QString statusStr;
        int status = order["status"].toInt();
        switch(status) {
        case 0: statusStr = "Pending"; break;
        case 1: statusStr = "Confirmed"; break;
        case 2: statusStr = "Shipped"; break;
        case 3: statusStr = "Completed"; break;
        case 4: statusStr = "Cancelled"; break;
        default: statusStr = "Unknown"; break;
        }
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(statusStr));
        m_tableWidget->setItem(i, 5, new QTableWidgetItem(order["createdAt"].toString()));

        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(5);
        QPushButton* detailBtn = new QPushButton("Detail");
        detailBtn->setFixedSize(50, 25);
        detailBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 3px; font-size: 11px; }");
        int orderId = order["id"].toInt();
        connect(detailBtn, &QPushButton::clicked, [this, orderId]() {
            showOrderDetailDialog(orderId);
        });
        actionLayout->addWidget(detailBtn);
        actionLayout->addStretch();
        m_tableWidget->setCellWidget(i, 6, actionWidget);
    }
}

void DeductWidget::onOrderDetailLoaded(const QVariantMap& orderDetail)
{
    Q_UNUSED(orderDetail)
}

void DeductWidget::onOperationSuccess(const QString& message)
{
    QMessageBox::information(this, "Success", message);
    emit refreshRequested();
}

void DeductWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "Failed", error);
}

void DeductWidget::showOrderDetailDialog(int orderId)
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString("Order Detail - ID: %1").arg(orderId));
    dialog.setFixedSize(600, 400);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTableWidget* itemsTable = new QTableWidget(&dialog);
    itemsTable->setColumnCount(4);
    itemsTable->setHorizontalHeaderLabels({"Product", "Price", "Quantity", "Subtotal"});
    itemsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(itemsTable);
    QPushButton* closeBtn = new QPushButton("Close", &dialog);
    closeBtn->setFixedSize(80, 32);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    emit viewOrderDetailRequested(orderId);
    dialog.exec();
}

void DeductWidget::showManualDeductDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Manual Dispatch");
    dialog.setFixedSize(400, 250);
    QFormLayout* form = new QFormLayout(&dialog);
    QSpinBox* productIdSpin = new QSpinBox(&dialog);
    productIdSpin->setRange(1, 999999);
    QSpinBox* quantitySpin = new QSpinBox(&dialog);
    quantitySpin->setRange(1, 99999);
    QTextEdit* reasonEdit = new QTextEdit(&dialog);
    reasonEdit->setPlaceholderText("Dispatch reason (e.g. sales, return, etc.)");
    reasonEdit->setFixedHeight(80);
    form->addRow("Product ID:", productIdSpin);
    form->addRow("Quantity:", quantitySpin);
    form->addRow("Reason:", reasonEdit);
    QPushButton* submitBtn = new QPushButton("Confirm Dispatch", &dialog);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    form->addRow(btnLayout);
    connect(submitBtn, &QPushButton::clicked, [&]() {
        QString reason = reasonEdit->toPlainText().trimmed();
        if (reason.isEmpty()) reason = "Manual dispatch";
        emit manualDeductRequested(productIdSpin->value(), quantitySpin->value(), reason);
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    dialog.exec();
}