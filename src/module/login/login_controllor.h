#ifndef LOGIN_CONTROLLOR_H
#define LOGIN_CONTROLLOR_H

#include <QObject>
#include <QString>

class LoginWidget;

class login_controllor : public QObject
{
    Q_OBJECT

public:
    explicit login_controllor(QObject *parent = nullptr);
    void setView(LoginWidget *view);

    bool login(const QString& username, const QString& password);
    bool registerUser(const QString& username, const QString& email,
                      const QString& phone, const QString& password);
    void logout();

    int getCurrentUserId() const { return m_currentUserId; }
    QString getCurrentUsername() const { return m_currentUsername; }
    bool isLoggedIn() const { return m_currentUserId > 0; }

signals:
    void loginSuccess(int userId, const QString& username);
    void loginFailed(const QString& error);

private:
    LoginWidget* m_view;
    int m_currentUserId;
    QString m_currentUsername;
};

#endif // LOGIN_CONTROLLOR_H