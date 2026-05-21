#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "registerwidget.h"
#include "mainwindow.h"
#include "module/homewidget.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 50));
    shadow->setOffset(0, 5);
    ui->cardWidget->setGraphicsEffect(shadow);

    QPalette pal = ui->leUsername->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->leUsername->setPalette(pal);
    ui->lePassword->setPalette(pal);

    connect(ui->lePassword, &QLineEdit::returnPressed, this, &LoginWidget::on_btnLogin_clicked);
    connect(ui->leUsername, &QLineEdit::returnPressed, this, &LoginWidget::on_btnLogin_clicked);
    connect(ui->btn_close, &QPushButton::clicked, this, &LoginWidget::on_btn_close_clicked);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_btn_close_clicked()
{
    QApplication::quit();
}

void LoginWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        QWidget *child = childAt(pos);
        if (child == ui->label_logo || pos.y() < 80) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void LoginWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void LoginWidget::showLoginError(const QString &message)
{
    QMessageBox::warning(this, "登录失败", message);
}

void LoginWidget::showLoginSuccess(const QString &message)
{
    QMessageBox::information(this, "登录成功", message);
}

void LoginWidget::navigateToHome(int userId, const QString &username, int role)
{
    this->hide();
    HomeWidget *homeWidget = new HomeWidget(userId, username, role);
    homeWidget->setAttribute(Qt::WA_DeleteOnClose);
    connect(homeWidget, &HomeWidget::destroyed, this, [this]() {
        this->show();
    });
    homeWidget->show();
}

void LoginWidget::showRegisterError(const QString &message)
{
    if (m_regWindow) m_regWindow->showRegisterError(message);
}

void LoginWidget::showRegisterSuccess(const QString &message)
{
    if (m_regWindow) m_regWindow->showRegisterSuccess(message);
}

void LoginWidget::showResetError(const QString &message)
{
    if (m_forgetWindow) m_forgetWindow->showResetError(message);
}

void LoginWidget::showResetSuccess(const QString &message)
{
    if (m_forgetWindow) m_forgetWindow->showResetSuccess(message);
}

void LoginWidget::on_btn_forgetPwd_clicked()
{
    this->hide();

    m_forgetWindow = new MainWindow(this);
    m_forgetWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(m_forgetWindow, &MainWindow::destroyed, this, [this]() {
        m_forgetWindow = nullptr;
        this->show();
    });

    connect(m_forgetWindow, &MainWindow::resetPasswordRequested,
            this, &LoginWidget::passwordResetSubmitted);

    m_forgetWindow->show();
}

void LoginWidget::on_btn_register_clicked()
{
    this->hide();

    m_regWindow = new RegisterWidget(0);
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;
        this->show();
    });

    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow) m_regWindow->close();
    });

    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    m_regWindow->show();
}

void LoginWidget::on_btn_registerAdmin_clicked()
{
    this->hide();

    m_regWindow = new RegisterWidget(1);
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);
    m_regWindow->setWindowTitle("注册管理员");

    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;
        this->show();
    });

    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow) m_regWindow->close();
    });

    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    m_regWindow->show();
}

void LoginWidget::on_btnLogin_clicked()
{
    QString username = ui->leUsername->text().trimmed();
    QString password = ui->lePassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }

    emit loginSubmitted(username, password);
}