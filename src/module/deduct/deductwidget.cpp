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

DeductWidget::DeductWidget(int userId, int mode, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_mode(mode)
    , m_currentBalance(0)
    , m_currentTotal(0)
{
    setupUI(mode);
    if (mode == 0) {
        emit loadCheckoutDataRequested();
    } else {
        emit refreshRequested();
    }
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

// ========== 用户结算界面 ==========
void DeductWidget::setupCheckoutUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel* titleLabel = new QLabel("订单结算", this);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #303133; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 商品列表
    QLabel* itemsLabel = new QLabel("商品清单", this);
    itemsLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(itemsLabel);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({"商品名称", "单价", "数量", "小计"});
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setFixedHeight(200);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(m_tableWidget);

    // 收货地址
    QHBoxLayout* addressLayout = new QHBoxLayout();
    addressLayout->addWidget(new QLabel("收货地址:", this));
    m_addressCombo = new QComboBox(this);
    m_addressCombo->setMinimumWidth(400);
    addressLayout->addWidget(m_addressCombo);
    addressLayout->addStretch();
    mainLayout->addLayout(addressLayout);

    // 金额和余额
    QWidget* infoWidget = new QWidget(this);
    infoWidget->setStyleSheet("QWidget { background-color: #F5F7FA; border-radius: 8px; }");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);

    m_totalLabel = new QLabel("订单总额: ¥0.00", this);
    m_totalLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #F56C6C; }");

    m_balanceLabel = new QLabel("我的余额: ¥0.00", this);
    m_balanceLabel->setStyleSheet("QLabel { font-size: 16px; color: #606266; }");

    infoLayout->addWidget(m_totalLabel);
    infoLayout->addWidget(m_balanceLabel);
    mainLayout->addWidget(infoWidget);

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_submitBtn = new QPushButton("确认下单", this);
    m_submitBtn->setFixedSize(150, 40);
    m_submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    btnLayout->addWidget(m_submitBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(m_submitBtn, &QPushButton::clicked, this, &DeductWidget::onSubmitClicked);
}

// ========== 管理员出库界面 ==========
void DeductWidget::setupAdminUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 搜索栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索订单号/用户/商品...");
    m_searchEdit->setFixedHeight(32);
    m_searchBtn = new QPushButton("搜索", this);
    m_searchBtn->setFixedSize(80, 32);

    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_searchBtn);
    topLayout->addSpacing(20);

    QLabel* dateLabel = new QLabel("日期范围:", this);
    m_startDateEdit = new QDateTimeEdit(this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDateTime(QDateTime::currentDateTime().addDays(-30));
    m_endDateEdit = new QDateTimeEdit(this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDateTime(QDateTime::currentDateTime());
    m_filterBtn = new QPushButton("筛选", this);
    m_filterBtn->setFixedSize(80, 32);

    topLayout->addWidget(dateLabel);
    topLayout->addWidget(m_startDateEdit);
    topLayout->addWidget(m_endDateEdit);
    topLayout->addWidget(m_filterBtn);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // 操作按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_viewDetailBtn = new QPushButton("查看详情", this);
    m_updateStatusBtn = new QPushButton("更新状态", this);
    m_manualDeductBtn = new QPushButton("手动出库", this);
    m_refreshBtn = new QPushButton("刷新", this);

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
    statusLayout->addWidget(new QLabel("订单状态:", this));
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItems({"全部", "待处理", "已确认", "已发货", "已完成", "已取消"});
    statusLayout->addWidget(m_statusCombo);
    statusLayout->addStretch();
    mainLayout->addLayout(statusLayout);

    // 订单表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(7);
    QStringList headers = {"订单ID", "用户ID", "用户名称", "总金额", "状态", "创建时间", "操作"};
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

// ========== 用户结算槽 ==========
void DeductWidget::onSubmitClicked()
{
    if (m_addressCombo->currentData().isNull()) {
        QMessageBox::warning(this, "提示", "请选择收货地址");
        return;
    }
    if (m_currentBalance < m_currentTotal) {
        QMessageBox::warning(this, "余额不足",
                             QString("订单总额 ¥%1，余额 ¥%2，请先充值").arg(m_currentTotal, 0, 'f', 2).arg(m_currentBalance, 0, 'f', 2));
        return;
    }
    int addressId = m_addressCombo->currentData().toInt();
    emit submitOrderRequested(addressId, "");
}

void DeductWidget::onCartItemsLoaded(const QList<QVariantMap>& items, double total)
{
    m_currentTotal = total;
    m_tableWidget->setRowCount(items.size());
    for (int i = 0; i < items.size(); i++) {
        const QVariantMap& item = items[i];
        double subtotal = item["quantity"].toInt() * item["price"].toDouble();
        m_tableWidget->setItem(i, 0, new QTableWidgetItem(item["name"].toString()));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(QString("¥%1").arg(item["price"].toDouble(), 0, 'f', 2)));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(item["quantity"].toInt())));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(QString("¥%1").arg(subtotal, 0, 'f', 2)));
    }
    m_totalLabel->setText(QString("订单总额: ¥%1").arg(total, 0, 'f', 2));

    if (m_currentBalance < total && total > 0) {
        m_submitBtn->setEnabled(false);
        m_submitBtn->setStyleSheet("QPushButton { background-color: #C0C4CC; color: white; border-radius: 4px; font-size: 14px; }");
    } else {
        m_submitBtn->setEnabled(true);
        m_submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    }
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
    }
}

void DeductWidget::onBalanceLoaded(double balance)
{
    m_currentBalance = balance;
    m_balanceLabel->setText(QString("我的余额: ¥%1").arg(balance, 0, 'f', 2));

    if (m_currentBalance < m_currentTotal && m_currentTotal > 0) {
        m_submitBtn->setEnabled(false);
        m_submitBtn->setStyleSheet("QPushButton { background-color: #C0C4CC; color: white; border-radius: 4px; font-size: 14px; }");
    } else if (m_currentTotal > 0) {
        m_submitBtn->setEnabled(true);
        m_submitBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; font-size: 14px; }");
    }
}

void DeductWidget::onOrderResult(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "下单成功", message);
        emit loadCheckoutDataRequested();
    } else {
        QMessageBox::warning(this, "下单失败", message);
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
        QMessageBox::warning(this, "提示", "请先选择订单");
        return;
    }
    showOrderDetailDialog(m_tableWidget->item(row, 0)->text().toInt());
}

void DeductWidget::onUpdateStatus()
{
    int row = m_tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择订单");
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
        case 0: statusStr = "待处理"; break;
        case 1: statusStr = "已确认"; break;
        case 2: statusStr = "已发货"; break;
        case 3: statusStr = "已完成"; break;
        case 4: statusStr = "已取消"; break;
        default: statusStr = "未知"; break;
        }
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(statusStr));
        m_tableWidget->setItem(i, 5, new QTableWidgetItem(order["createdAt"].toString()));

        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(5);
        QPushButton* detailBtn = new QPushButton("详情");
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
    QMessageBox::information(this, "成功", message);
    emit refreshRequested();
}

void DeductWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "失败", error);
}

void DeductWidget::showOrderDetailDialog(int orderId)
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString("订单详情 - ID: %1").arg(orderId));
    dialog.setFixedSize(600, 400);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTableWidget* itemsTable = new QTableWidget(&dialog);
    itemsTable->setColumnCount(4);
    itemsTable->setHorizontalHeaderLabels({"商品", "单价", "数量", "小计"});
    itemsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(itemsTable);
    QPushButton* closeBtn = new QPushButton("关闭", &dialog);
    closeBtn->setFixedSize(80, 32);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    emit viewOrderDetailRequested(orderId);
    dialog.exec();
}

void DeductWidget::showManualDeductDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("手动出库");
    dialog.setFixedSize(400, 250);
    QFormLayout* form = new QFormLayout(&dialog);
    QSpinBox* productIdSpin = new QSpinBox(&dialog);
    productIdSpin->setRange(1, 999999);
    QSpinBox* quantitySpin = new QSpinBox(&dialog);
    quantitySpin->setRange(1, 99999);
    QTextEdit* reasonEdit = new QTextEdit(&dialog);
    reasonEdit->setPlaceholderText("出库原因（如：销售出库、退货等）");
    reasonEdit->setFixedHeight(80);
    form->addRow("商品ID:", productIdSpin);
    form->addRow("出库数量:", quantitySpin);
    form->addRow("出库原因:", reasonEdit);
    QPushButton* submitBtn = new QPushButton("确认出库", &dialog);
    QPushButton* cancelBtn = new QPushButton("取消", &dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    form->addRow(btnLayout);
    connect(submitBtn, &QPushButton::clicked, [&]() {
        QString reason = reasonEdit->toPlainText().trimmed();
        if (reason.isEmpty()) reason = "手动出库";
        emit manualDeductRequested(productIdSpin->value(), quantitySpin->value(), reason);
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    dialog.exec();
}