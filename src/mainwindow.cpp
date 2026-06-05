/**
 * @file mainwindow.cpp
 * @brief 密码重置窗口实现：负责找回密码流程的交互与反馈
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>

// 构造函数：初始化 UI 并统一设置输入框文字颜色（覆盖深色主题）
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 将输入框的文字颜色强制设为黑色，确保在不同主题下清晰可见
    QPalette pal = ui->edit_account->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->edit_account->setPalette(pal);
    ui->edit_code->setPalette(pal);
    ui->edit_newPwd->setPalette(pal);
    ui->edit_confirmPwd->setPalette(pal);
}

// 析构函数：自动释放 UI 对象资源
MainWindow::~MainWindow()
{
    delete ui;
}

// ========== 后端调用的公共槽函数 (Controller 反馈接口) ==========

// 接收控制器发来的重置失败信息
void MainWindow::showResetError(const QString &message)
{
    QMessageBox::warning(this, "Reset Failed", message);
}

// 接收控制器发来的重置成功信息
void MainWindow::showResetSuccess(const QString &message)
{
    QMessageBox::information(this, "Reset Success", message);
    emit closed(); // 通知登录窗口流程结束
    this->close();
}

// ========== 界面交互逻辑 ==========

// 返回登录：关闭当前窗口并通知上层
void MainWindow::on_btn_back_clicked()
{
    emit closed();
    this->close();
}

// 获取验证码逻辑
void MainWindow::on_btn_getCode_clicked()
{
    QString account = ui->edit_account->text().trimmed();

    // 空值校验
    if (account.isEmpty()) {
        QMessageBox::warning(this, "Tip", "Please enter account/phone number");
        return;
    }

    // 关键点：发射信号给 Controller。界面不负责逻辑，只负责请求
    qDebug() << ">>> [DEBUG] 正在发射请求验证码信号，账号:" << account;
    emit requestVerificationCode(account);

    QMessageBox::information(this, "Tip", "Verification code request sent, please wait...");
}

// 提交重置密码逻辑
void MainWindow::on_btn_submit_clicked()
{
    QString account = ui->edit_account->text().trimmed();
    QString code = ui->edit_code->text().trimmed();
    QString newPwd = ui->edit_newPwd->text();
    QString confirmPwd = ui->edit_confirmPwd->text();

    // 前端基础校验：检查输入完整性
    if (account.isEmpty() || code.isEmpty() || newPwd.isEmpty() || confirmPwd.isEmpty()) {
        QMessageBox::warning(this, "Tip", "Please fill in all fields");
        return;
    }
    // 密码一致性二次核对
    if (newPwd != confirmPwd) {
        QMessageBox::warning(this, "Tip", "Passwords do not match");
        return;
    }

    // 关键点：将数据包发送给 Controller 进行数据库层面的重置
    qDebug() << ">>> [DEBUG] 正在发射重置密码信号...";
    emit resetPasswordRequested(account, code, newPwd, confirmPwd);
}

// ========== 输入限制逻辑 ==========

// 限制账号输入长度为 11 位（常见手机号长度）
void MainWindow::on_edit_account_textChanged(const QString &text)
{
    if (text.length() > 11) {
        ui->edit_account->setText(text.left(11));
    }
}

// 限制验证码只能输入数字
void MainWindow::on_edit_code_textChanged(const QString &text)
{
    if (!text.isEmpty() && !text.back().isDigit()) {
        ui->edit_code->setText(text.chopped(1));
    }
}

// 实时校验：动态监测密码一致性，并给出视觉提示
void MainWindow::on_edit_newPwd_textChanged(const QString &text)
{
    if (!ui->edit_confirmPwd->text().isEmpty()) {
        if (text != ui->edit_confirmPwd->text()) {
            ui->label_tip->setText("Passwords do not match");
        } else {
            ui->label_tip->clear();
        }
    }
}

void MainWindow::on_edit_confirmPwd_textChanged(const QString &text)
{
    if (text != ui->edit_newPwd->text()) {
        ui->label_tip->setText("Passwords do not match");
    } else {
        ui->label_tip->clear();
    }
}