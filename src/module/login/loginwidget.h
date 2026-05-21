#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class LoginWidget;
}

class RegisterWidget;
class MainWindow;
class HomeWidget;

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public slots:
    void showLoginError(const QString &message);
    void showLoginSuccess(const QString &message);
    void navigateToHome(int userId, const QString &username, int role);
    void showRegisterError(const QString &message);
    void showRegisterSuccess(const QString &message);
    void showResetError(const QString &message);
    void showResetSuccess(const QString &message);

signals:
    void loginSubmitted(const QString &username, const QString &password);
    void registerSubmitted(const QString &username,
                           const QString &email,
                           const QString &phone,
                           const QString &password,
                           const QString &confirmPassword,
                           int role);
    void passwordResetSubmitted(const QString &account,
                                const QString &code,
                                const QString &newPassword,
                                const QString &confirmPassword);

private slots:
    void on_btn_forgetPwd_clicked();
    void on_btn_register_clicked();
    void on_btn_registerAdmin_clicked();
    void on_btnLogin_clicked();
    void on_btn_close_clicked();  // 添加关闭按钮槽函数

private:
    Ui::LoginWidget *ui;
    RegisterWidget *m_regWindow = nullptr;
    MainWindow     *m_forgetWindow = nullptr;
    QPoint m_dragPosition;
};

#endif // LOGINWIDGET_H