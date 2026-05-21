#include "registerwidget.h"
#include "ui_registerwidget.h"
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

RegisterWidget::RegisterWidget(int role, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
    , m_role(role)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 5);
    ui->cardWidget->setGraphicsEffect(shadow);

    if (role == 1) {
        ui->label_title->setText("👑 管理员注册");
        ui->btn_RegisterSubmit->setStyleSheet(
            "QPushButton { background-color: #e6a23c; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #d4912e; }"
            );
    } else {
        ui->label_title->setText("📝 用户注册");
        ui->btn_RegisterSubmit->setStyleSheet(
            "QPushButton { background-color: #67c23a; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #5daf34; }"
            );
    }

    QPalette pal = ui->le_Register_Username->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->le_Register_Username->setPalette(pal);
    ui->le_Register_Email->setPalette(pal);
    ui->le_Register_Phone->setPalette(pal);
    ui->le_Register_Password->setPalette(pal);
    ui->le_Register_ConfirmPwd->setPalette(pal);

    connect(ui->btn_close, &QPushButton::clicked, this, &RegisterWidget::on_btn_close_clicked);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::on_btn_close_clicked()
{
    this->close();
}

void RegisterWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    move(screenGeo.center() - rect().center());
}

void RegisterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        QWidget *child = childAt(pos);
        if (child == ui->label_title || pos.y() < 80) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void RegisterWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void RegisterWidget::showRegisterError(const QString &message)
{
    QMessageBox::warning(this, "注册失败", message);
}

void RegisterWidget::showRegisterSuccess(const QString &message)
{
    QMessageBox::information(this, "注册成功", message);
    emit backToLogin();
    this->close();
}

void RegisterWidget::on_btn_BackToLogin_clicked()
{
    emit backToLogin();
    this->close();
}

void RegisterWidget::on_btn_RegisterSubmit_clicked()
{
    QString username = ui->le_Register_Username->text().trimmed();
    QString email    = ui->le_Register_Email->text().trimmed();
    QString phone    = ui->le_Register_Phone->text().trimmed();
    QString password = ui->le_Register_Password->text();
    QString confirm  = ui->le_Register_ConfirmPwd->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名");
        return;
    }
    if (username.length() < 3 || username.length() > 20) {
        QMessageBox::warning(this, "提示", "用户名长度需在3-20位之间");
        return;
    }
    if (email.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入邮箱");
        return;
    }
    if (!email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "提示", "请输入有效的邮箱地址");
        return;
    }
    if (password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入密码");
        return;
    }
    if (password.length() < 6 || password.length() > 20) {
        QMessageBox::warning(this, "提示", "密码长度需在6-20位之间");
        return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致");
        return;
    }

    emit registerSubmitted(username, email, phone, password, confirm, m_role);
}