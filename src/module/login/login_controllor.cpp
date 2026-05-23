#include "login_controllor.h"
#include "loginwidget.h"
#include "databasemanager.h"
#include "hashsha.h"
#include "registerwidget.h"
#include <QDebug>
#include <QRandomGenerator>

// 构造函数：初始化控制器，并配置好邮件发送服务的基础信息
login_controllor::login_controllor(QObject *parent)
    : QObject(parent)
    , m_view(nullptr)
    , m_currentUserId(0)
{
    // 配置 SMTP 服务器为 QQ 邮箱，端口 465（SSL 加密）
    m_emailsender.setServer("smtp.qq.com", 465, true);
    // 设置发件人身份及授权码（注意：这是发邮件的凭据）
    m_emailsender.setCredentials("3434816838@qq.com", "fhplplxboywudaid");
}

// 设置主登录窗口视图，并建立信号与槽的连接（UI 交互逻辑）
void login_controllor::setView(LoginWidget *view)
{
    m_view = view;
    // 绑定 UI 触发的动作与控制器的处理槽函数
    connect(m_view, &LoginWidget::loginSubmitted, this, &login_controllor::login);
    connect(m_view, &LoginWidget::registerSubmitted, this, &login_controllor::registerUser);

    // 绑定控制器结果回传给 UI 显示
    connect(this, &login_controllor::loginSuccess, m_view, &LoginWidget::navigateToHome);
    connect(this, &login_controllor::loginFailed, m_view, &LoginWidget::showLoginError);
    connect(this, &login_controllor::registerSuccess, m_view, &LoginWidget::showRegisterSuccess);
    connect(this, &login_controllor::registerFailed, m_view, &LoginWidget::showRegisterError);

    // 绑定找回密码相关的信号
    connect(m_view, &LoginWidget::requestVerificationCode, this, &login_controllor::onGetCodeRequested);
    connect(m_view, &LoginWidget::passwordResetSubmitted, this, &login_controllor::onResetPasswordSubmitted);
}

// 打开找回密码窗口的逻辑
void login_controllor::openResetWindow(QWidget *parent)
{
    // 1. 创建独立窗口，并设置关闭时自动销毁（防止内存泄漏）
    MainWindow *win = new MainWindow(parent);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->setAttribute(Qt::WA_TranslucentBackground);

    // 2. 连接窗口特有的信号到控制器，实现业务逻辑解耦
    connect(win, &MainWindow::requestVerificationCode, this, &login_controllor::onGetCodeRequested);
    connect(win, &MainWindow::resetPasswordRequested, this, &login_controllor::onResetPasswordSubmitted);

    win->show();
    qDebug() << ">>> [DEBUG] 找回密码窗口已创建并自动连接信号";
}

// 处理登录业务逻辑
bool login_controllor::login(const QString& username, const QString& password)
{
    // 从数据库查询用户记录
    UserInfo info = DatabaseManager::instance().findUserByAccount(username);

    // 校验：用户不存在
    if (info.id < 0) {
        emit loginFailed("用户不存在");
        return false;
    }

    // 校验：账号是否被封禁/禁用
    if (!info.isActive) {
        emit loginFailed("账号已被禁用");
        return false;
    }

    // 校验：密码加密后比对
    QString hashedInput = HashSha::hashSha256(password);
    if (info.passwordHash != hashedInput) {
        emit loginFailed("密码错误");
        return false;
    }

    // 更新登录状态及数据库记录
    m_currentUserId = info.id;
    m_currentUsername = info.username;
    DatabaseManager::instance().updateLastLogin(info.id);

    emit loginSuccess(info.id, info.username, info.role);
    return true;
}

// 处理注册业务逻辑
bool login_controllor::registerUser(const QString& username, const QString& email,
                                    const QString& phone, const QString& password,
                                    const QString& confirmPassword, int role)
{
    // 校验两次密码输入一致性
    if (password != confirmPassword) {
        emit registerFailed("两次输入的密码不一致");
        return false;
    }

    // 校验用户名是否已存在
    if (DatabaseManager::instance().userExists(username)) {
        emit registerFailed("用户名已存在");
        return false;
    }

    // 加密密码并写入数据库
    QString hashedPassword = HashSha::hashSha256(password);
    bool success = DatabaseManager::instance().registerUser(username, email, phone,
                                                             hashedPassword, role, nullptr);

    if (success) {
        emit registerSuccess("注册成功！");
    } else {
        emit registerFailed("注册失败，请稍后重试");
    }
    return success;
}

// 获取验证码的处理逻辑
void login_controllor::onGetCodeRequested(const QString &account)
{
    // 查询账户对应用户信息
    UserInfo user = DatabaseManager::instance().findUserByAccount(account);

    // 校验账号存在与邮箱绑定情况
    if (user.id == 0) {
        m_view->showResetError("该账号/手机号不存在");
        return;
    }
    if (user.email.isEmpty()) {
        m_view->showResetError("该账号未绑定邮箱，无法重置密码");
        return;
    }

    // 生成 6 位随机验证码并存入内存缓存（包含时间戳）
    QString code = QString::number(QRandomGenerator::global()->bounded(100000, 999999));
    m_codeCache[account] = {code, QDateTime::currentDateTime()};

    // 调用邮件发送模块
    m_emailsender.sendVerificationCode(user.email, code, user.username);
    qDebug() << ">>> [DEBUG] 成功获取邮箱:" << user.email << "准备发送验证码:" << code;
}

// 处理密码重置提交逻辑
void login_controllor::onResetPasswordSubmitted(const QString &account, const QString &code,
                                                const QString &newPwd, const QString &confirmPwd)
{
    // 1. 验证码校验：检查缓存是否存在、是否正确、是否超过 5 分钟有效期
    if (!m_codeCache.contains(account) ||
        m_codeCache[account].code != code ||
        m_codeCache[account].timestamp.secsTo(QDateTime::currentDateTime()) > 300) {

        m_view->showResetError("验证码错误或已过期");
        return;
    }

    // 2. 将新密码哈希化后更新至数据库
    QString newHash = HashSha::hashSha256(newPwd);
    bool success = DatabaseManager::instance().updatePassword(account, newHash);

    if (success) {
        m_codeCache.remove(account); // 成功后清理缓存
        m_view->showResetSuccess("密码已重置，请登录");
    } else {
        m_view->showResetError("重置失败，账号不存在");
    }
}