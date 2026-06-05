/**
 * @file registerwidget.cpp
 * @brief 注册窗口实现：负责 UI 展示、输入合法性校验及注册信号发送
 */

#include "registerwidget.h"
#include "ui_registerwidget.h"
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStringList>

// 构造函数：初始化窗口样式、角色区分及输入框视觉设置
RegisterWidget::RegisterWidget(int role, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
    , m_role(role) // 保存用户角色（0：普通用户，1：管理员）
{
    ui->setupUi(this);
    // 设置关闭窗口时自动释放内存，防止内存泄漏
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 设置无边框透明窗口，实现自定义现代 UI
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 为卡片添加阴影效果，增强层次感
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 5);
    ui->cardWidget->setGraphicsEffect(shadow);

    // 根据角色（role）动态设置标题和按钮颜色
    if (role == 1) {
        ui->label_title->setText("👑 Admin Registration");
        ui->btn_RegisterSubmit->setStyleSheet(
            "QPushButton { background-color: #e6a23c; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #d4912e; }"
            );
    } else {
        ui->label_title->setText("📝 User Registration");
        ui->btn_RegisterSubmit->setStyleSheet(
            "QPushButton { background-color: #67c23a; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #5daf34; }"
            );
    }

    // 强制设置输入框文字为黑色，防止被全局深色样式覆盖
    QPalette pal = ui->le_Register_Username->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->le_Register_Username->setPalette(pal);
    ui->le_Register_Email->setPalette(pal);
    ui->le_Register_Phone->setPalette(pal);
    ui->le_Register_Password->setPalette(pal);
    ui->le_Register_ConfirmPwd->setPalette(pal);

    // 设置用户名输入框：只能输入字母、数字和下划线，且长度限制为20
    QRegularExpressionValidator *usernameValidator = new QRegularExpressionValidator(
        QRegularExpression("^[a-zA-Z_][a-zA-Z0-9_]*$"), this);
    ui->le_Register_Username->setValidator(usernameValidator);
    ui->le_Register_Username->setMaxLength(20);  // 限制最大输入长度为20
    ui->le_Register_Username->setPlaceholderText("Username (3-20 chars, start with letter or _)");

    // 连接关闭按钮槽函数
    connect(ui->btn_close, &QPushButton::clicked, this, &RegisterWidget::on_btn_close_clicked);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

// 关闭窗口槽函数
void RegisterWidget::on_btn_close_clicked()
{
    this->close();
}

// 窗口显示事件：确保窗口每次打开都居中于屏幕
void RegisterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    move(screenGeo.center() - rect().center());
}

// 鼠标按下事件：通过标题栏区域实现窗口拖动
void RegisterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        QWidget *child = childAt(pos);
        // 如果点击标题栏区域，记录初始偏移量
        if (child == ui->label_title || pos.y() < 80) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

// 鼠标移动事件：根据鼠标偏移量实时移动窗口
void RegisterWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

// 供控制器调用的错误提示接口
void RegisterWidget::showRegisterError(const QString &message)
{
    QMessageBox::warning(this, "Registration Failed", message);
}

// 供控制器调用的成功处理接口
void RegisterWidget::showRegisterSuccess(const QString &message)
{
    QMessageBox::information(this, "Registration Success", message);
    emit backToLogin();  // 信号：通知登录界面返回操作
    this->close();
}

// 返回登录按钮点击逻辑
void RegisterWidget::on_btn_BackToLogin_clicked()
{
    emit backToLogin();  // 信号：请求返回登录页
    this->close();
}

// 注册提交：前端数据校验层，确保逻辑层收到的都是合法数据
void RegisterWidget::on_btn_RegisterSubmit_clicked()
{
    // 获取并修剪输入内容
    QString username = ui->le_Register_Username->text().trimmed();
    QString email    = ui->le_Register_Email->text().trimmed();
    QString phone    = ui->le_Register_Phone->text().trimmed();
    QString password = ui->le_Register_Password->text();
    QString confirm  = ui->le_Register_ConfirmPwd->text();

    // 格式化校验逻辑
    if (username.isEmpty()) { QMessageBox::warning(this, "Tip", "Please enter username"); return; }
    if (username.length() < 3 || username.length() > 20) { QMessageBox::warning(this, "Tip", "Username must be 3-20 characters"); return; }

    // 校验用户名格式：只能包含字母、数字和下划线，且不能以数字开头
    QRegularExpression usernameRegex("^[a-zA-Z_][a-zA-Z0-9_]*$");
    if (!usernameRegex.match(username).hasMatch()) {
        QMessageBox::warning(this, "Tip", "Username can only contain letters, digits and underscores, and cannot start with a digit");
        return;
    }

    // 校验用户名是否包含保留字
    static const QStringList reservedNames = {
        "admin", "administrator", "root", "superuser", "system",
        "test", "testing", "guest", "user", "null", "undefined",
        "mod", "moderator", "support", "help", "info", "web",
        "master", "owner", "manager", "operator"
    };
    if (reservedNames.contains(username.toLower())) {
        QMessageBox::warning(this, "Tip", "This username is reserved, please choose another one");
        return;
    }
    if (email.isEmpty()) { QMessageBox::warning(this, "Tip", "Please enter email"); return; }
    if (!email.contains('@') || !email.contains('.')) { QMessageBox::warning(this, "Tip", "Please enter a valid email address"); return; }
    if (password.isEmpty()) { QMessageBox::warning(this, "Tip", "Please enter password"); return; }
    if (password.length() < 6 || password.length() > 20) { QMessageBox::warning(this, "Tip", "Password must be 6-20 characters"); return; }
    if (password != confirm) { QMessageBox::warning(this, "Tip", "Passwords do not match"); return; }

    // 核心信号：将所有经过校验的注册数据发射给控制器层
    emit registerSubmitted(username, email, phone, password, confirm, m_role);
}