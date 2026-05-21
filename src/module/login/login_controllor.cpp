#include "login_controllor.h"
#include "loginwidget.h"
#include "common/databasemanager.h"
#include "common/hashsha.h"
#include <QDebug>

login_controllor::login_controllor(QObject *parent)
    : QObject(parent)
    , m_view(nullptr)
    , m_currentUserId(0)
{
}

void login_controllor::setView(LoginWidget *view)
{
    m_view = view;
}

bool login_controllor::login(const QString& username, const QString& password)
{
    UserInfo info = DatabaseManager::instance().findUserByAccount(username);

    if (info.id < 0) {
        emit loginFailed("用户不存在");
        return false;
    }

    if (!info.isActive) {
        emit loginFailed("账号已被禁用");
        return false;
    }

    QString hashedInput = HashSha::hashSha256(password);
    if (info.passwordHash != hashedInput) {
        emit loginFailed("密码错误");
        return false;
    }

    m_currentUserId = info.id;
    m_currentUsername = info.username;

    DatabaseManager::instance().updateLastLogin(info.id);
    emit loginSuccess(info.id, info.username);
    return true;
}

bool login_controllor::registerUser(const QString& username, const QString& email,
                                    const QString& phone, const QString& password)
{
    if (DatabaseManager::instance().userExists(username)) {
        return false;
    }

    QString hashedPassword = HashSha::hashSha256(password);
    return DatabaseManager::instance().registerUser(username, email, phone, hashedPassword, 0, nullptr);
}

void login_controllor::logout()
{
    m_currentUserId = 0;
    m_currentUsername.clear();
}