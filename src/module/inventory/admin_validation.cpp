#include "admin_validation.h"
#include "ui_admin_validation.h"
#include "module/inventory/inventorywidget.h"
#include <QMessageBox>
#include <QDebug>

AdminValidation::AdminValidation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminValidation)
{
    ui->setupUi(this);
    connect(ui->registerButton, &QPushButton::clicked, this, &AdminValidation::onRegisterButtonClicked);
    updateStatus("请填写管理员信息");
}

AdminValidation::~AdminValidation()
{
    delete ui;
}

bool AdminValidation::registerAdmin(const QString &username, const QString &password)
{
    // TODO: 将管理员信息存入数据库
    // INSERT INTO admins (username, password, create_time) VALUES (?, ?, ?)

    Q_UNUSED(username);
    Q_UNUSED(password);

    // 模拟注册成功
    return true;
}

void AdminValidation::onRegisterButtonClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmEdit->text();

    // 验证输入
    if (username.isEmpty()) {
        showError("请输入用户名");
        return;
    }

    if (password.isEmpty()) {
        showError("请输入密码");
        return;
    }

    if (password.length() < 6) {
        showError("密码长度不能少于6位");
        return;
    }

    if (password != confirm) {
        showError("两次输入的密码不一致");
        return;
    }

    // 注册管理员
    if (registerAdmin(username, password)) {
        showSuccess(QString("管理员 %1 注册成功！正在进入系统...").arg(username));

        // 关闭当前注册窗口
        this->close();

        // 打开库存管理主界面
        InventoryWidget *inventoryWidget = new InventoryWidget();
        inventoryWidget->show();
    } else {
        showError("注册失败，用户名可能已存在");
    }
}

void AdminValidation::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
}

void AdminValidation::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    updateStatus("错误: " + message);
}

void AdminValidation::showSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
    updateStatus(message);
}