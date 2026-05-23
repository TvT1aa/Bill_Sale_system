#ifndef LOGIN_CONTROLLOR_H
#define LOGIN_CONTROLLOR_H

#include <QObject>
#include <QString>
#include "emailsender.h"
#include "mainwindow.h"
class LoginWidget;
class MainWindow;

class login_controllor : public QObject
{
    Q_OBJECT

public:

    struct CodeInfo {
        QString code;
        QDateTime timestamp;
    };
    explicit login_controllor(QObject *parent = nullptr);
    void setView(LoginWidget *view);
    bool login(const QString& username, const QString& password);
    bool registerUser(const QString& username, const QString& email,
                      const QString& phone, const QString& password
                      , const QString& confirm, int role);
    void logout();

    int getCurrentUserId() const { return m_currentUserId; }
    QString getCurrentUsername() const { return m_currentUsername; }
    bool isLoggedIn() const { return m_currentUserId > 0; }
    void openResetWindow(QWidget *parent);
    void connectResetWindow(MainWindow *resetWindow);

signals:
    void loginSuccess(int userId, const QString& username, int role);
    void loginFailed(const QString& error);
    void registerSuccess(const QString& message);
    void registerFailed(const QString& error);

    void requestVerificationCode(const QString &account);
    void passwordResetSubmitted(const QString &account, const QString &code,
                                const QString &newPwd, const QString &confirmPwd);
private slots:
    // 处理获取验证码请求
    void onGetCodeRequested(const QString &account);
    // 处理重置密码请求
    void onResetPasswordSubmitted(const QString &account, const QString &code,
                                  const QString &newPwd, const QString &confirmPwd);

private:
    LoginWidget* m_view;
    int m_currentUserId;
    QString m_currentUsername;
    QMap<QString, CodeInfo> m_codeCache;
    EmailSender m_emailsender;
};

#endif // LOGIN_CONTROLLOR_H