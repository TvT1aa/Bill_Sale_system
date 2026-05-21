#include "addresswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>

AddressWidget::AddressWidget(int userId, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
{
    setupUI();
    emit refreshRequested();
}

AddressWidget::~AddressWidget()
{
}

void AddressWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 顶部按钮栏
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_addBtn = new QPushButton("新增地址", this);
    m_editBtn = new QPushButton("编辑地址", this);
    m_deleteBtn = new QPushButton("删除地址", this);
    m_setDefaultBtn = new QPushButton("设为默认", this);
    m_refreshBtn = new QPushButton("刷新", this);

    m_addBtn->setFixedSize(100, 32);
    m_editBtn->setFixedSize(100, 32);
    m_deleteBtn->setFixedSize(100, 32);
    m_setDefaultBtn->setFixedSize(100, 32);
    m_refreshBtn->setFixedSize(80, 32);

    m_addBtn->setStyleSheet("QPushButton { background-color: #67C23A; color: white; border-radius: 4px; }");
    m_editBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 4px; }");
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 4px; }");
    m_setDefaultBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 4px; }");

    topLayout->addWidget(m_addBtn);
    topLayout->addWidget(m_editBtn);
    topLayout->addWidget(m_deleteBtn);
    topLayout->addWidget(m_setDefaultBtn);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 地址表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(7);
    QStringList headers = {"ID", "收货人", "电话", "省/市/区", "详细地址", "默认", "操作"};
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    mainLayout->addWidget(m_tableWidget);

    // 连接信号
    connect(m_addBtn, &QPushButton::clicked, this, &AddressWidget::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &AddressWidget::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &AddressWidget::onDeleteClicked);
    connect(m_setDefaultBtn, &QPushButton::clicked, this, &AddressWidget::onSetDefaultClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &AddressWidget::onRefreshClicked);
}

void AddressWidget::showAddressDialog(int addressId, const QVariantMap& data)
{
    QDialog dialog(this);
    dialog.setWindowTitle(addressId < 0 ? "新增地址" : "编辑地址");
    dialog.setFixedSize(400, 450);

    QFormLayout* form = new QFormLayout(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("收货人姓名");
    if (data.contains("name")) nameEdit->setText(data["name"].toString());

    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setPlaceholderText("手机号码");
    if (data.contains("phone")) phoneEdit->setText(data["phone"].toString());

    QComboBox* provinceCombo = new QComboBox(&dialog);
    provinceCombo->setEditable(true);
    provinceCombo->addItems({"北京市", "上海市", "广东省", "江苏省", "浙江省", "四川省", "湖北省", "湖南省"});
    if (data.contains("province")) provinceCombo->setCurrentText(data["province"].toString());

    QComboBox* cityCombo = new QComboBox(&dialog);
    cityCombo->setEditable(true);
    if (data.contains("city")) cityCombo->setCurrentText(data["city"].toString());

    QComboBox* districtCombo = new QComboBox(&dialog);
    districtCombo->setEditable(true);
    if (data.contains("district")) districtCombo->setCurrentText(data["district"].toString());

    QTextEdit* detailEdit = new QTextEdit(&dialog);
    detailEdit->setPlaceholderText("详细地址（街道、小区、门牌号）");
    detailEdit->setFixedHeight(80);
    if (data.contains("detail")) detailEdit->setPlainText(data["detail"].toString());

    QComboBox* isDefaultCombo = new QComboBox(&dialog);
    isDefaultCombo->addItems({"否", "是"});
    if (data.contains("isDefault") && data["isDefault"].toBool()) {
        isDefaultCombo->setCurrentIndex(1);
    }

    form->addRow("收货人:", nameEdit);
    form->addRow("手机号:", phoneEdit);
    form->addRow("省份:", provinceCombo);
    form->addRow("城市:", cityCombo);
    form->addRow("区/县:", districtCombo);
    form->addRow("详细地址:", detailEdit);
    form->addRow("设为默认:", isDefaultCombo);

    QPushButton* submitBtn = new QPushButton("确定", &dialog);
    QPushButton* cancelBtn = new QPushButton("取消", &dialog);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    form->addRow(btnLayout);

    connect(submitBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请输入收货人姓名");
            return;
        }
        if (phoneEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请输入手机号");
            return;
        }
        if (detailEdit->toPlainText().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "请输入详细地址");
            return;
        }

        QVariantMap address;
        address["name"] = nameEdit->text();
        address["phone"] = phoneEdit->text();
        address["province"] = provinceCombo->currentText();
        address["city"] = cityCombo->currentText();
        address["district"] = districtCombo->currentText();
        address["detail"] = detailEdit->toPlainText();
        address["isDefault"] = (isDefaultCombo->currentIndex() == 1);
        address["userId"] = m_userId;

        if (addressId < 0) {
            emit addAddressRequested(address);
        } else {
            emit updateAddressRequested(addressId, address);
        }
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void AddressWidget::onAddClicked()
{
    showAddressDialog();
}

void AddressWidget::onEditClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的地址");
        return;
    }

    int addressId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QVariantMap data;
    data["name"] = m_tableWidget->item(currentRow, 1)->text();
    data["phone"] = m_tableWidget->item(currentRow, 2)->text();
    data["province"] = m_tableWidget->item(currentRow, 3)->text().split("/").value(0);
    data["city"] = m_tableWidget->item(currentRow, 3)->text().split("/").value(1);
    data["district"] = m_tableWidget->item(currentRow, 3)->text().split("/").value(2);
    data["detail"] = m_tableWidget->item(currentRow, 4)->text();
    data["isDefault"] = (m_tableWidget->item(currentRow, 5)->text() == "是");

    showAddressDialog(addressId, data);
}

void AddressWidget::onDeleteClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的地址");
        return;
    }

    int addressId = m_tableWidget->item(currentRow, 0)->text().toInt();
    QString name = m_tableWidget->item(currentRow, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除地址「%1」吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        emit deleteAddressRequested(addressId);
    }
}

void AddressWidget::onSetDefaultClicked()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要设为默认的地址");
        return;
    }

    int addressId = m_tableWidget->item(currentRow, 0)->text().toInt();
    emit setDefaultAddressRequested(addressId);
}

void AddressWidget::onRefreshClicked()
{
    emit refreshRequested();
}

// ========== 后端调用的槽 ==========

void AddressWidget::onAddressLoaded(const QList<QVariantMap>& addresses)
{
    m_tableWidget->setRowCount(addresses.size());
    for (int i = 0; i < addresses.size(); i++) {
        const QVariantMap& addr = addresses[i];
        QString fullLocation = QString("%1/%2/%3")
                                   .arg(addr["province"].toString())
                                   .arg(addr["city"].toString())
                                   .arg(addr["district"].toString());

        m_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(addr["id"].toInt())));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(addr["name"].toString()));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(addr["phone"].toString()));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(fullLocation));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(addr["detail"].toString()));
        m_tableWidget->setItem(i, 5, new QTableWidgetItem(addr["isDefault"].toBool() ? "是" : "否"));

        // 操作按钮
        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(5);

        QPushButton* editBtn = new QPushButton("编辑");
        QPushButton* deleteBtn = new QPushButton("删除");
        QPushButton* defaultBtn = new QPushButton("默认");

        editBtn->setFixedSize(50, 25);
        deleteBtn->setFixedSize(50, 25);
        defaultBtn->setFixedSize(50, 25);

        editBtn->setStyleSheet("QPushButton { background-color: #409EFF; color: white; border-radius: 3px; font-size: 11px; }");
        deleteBtn->setStyleSheet("QPushButton { background-color: #F56C6C; color: white; border-radius: 3px; font-size: 11px; }");
        defaultBtn->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 3px; font-size: 11px; }");

        int addrId = addr["id"].toInt();
        connect(editBtn, &QPushButton::clicked, [this, addrId, addr]() {
            showAddressDialog(addrId, addr);
        });
        connect(deleteBtn, &QPushButton::clicked, [this, addrId]() {
            emit deleteAddressRequested(addrId);
        });
        connect(defaultBtn, &QPushButton::clicked, [this, addrId]() {
            emit setDefaultAddressRequested(addrId);
        });

        actionLayout->addWidget(editBtn);
        actionLayout->addWidget(deleteBtn);
        actionLayout->addWidget(defaultBtn);
        actionLayout->addStretch();

        m_tableWidget->setCellWidget(i, 6, actionWidget);
    }
}

void AddressWidget::onOperationSuccess(const QString& message)
{
    QMessageBox::information(this, "成功", message);
    emit refreshRequested();
}

void AddressWidget::onOperationError(const QString& error)
{
    QMessageBox::warning(this, "失败", error);
}