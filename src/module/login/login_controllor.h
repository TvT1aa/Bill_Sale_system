// controllor.h
#ifndef LOGIN_CONTROLLOR_H
#define LOGIN_CONTROLLOR_H

#include <QObject>
#include "user.h"            // 必须连接：提供校验逻辑和哈希算法
#include "database/databasemanager.h" // 必须连接：提供数据库读写
#include "loginwidget.h"     // 必须连接：获取 UI 信号
#include "desutil.h"
#include "hashsha.h"
class login_controllor : public QObject {
    Q_OBJECT
public:
    explicit login_controllor(QObject *parent = nullptr);

    explicit login_controllor(LoginWidget *m_view) : m_view(m_view) {}
    // 构建核心：注入界面指针并绑定信号
    void setView(LoginWidget* view);

private slots:
    // 这里的参数必须对应 LoginWidget 的信号参数
    // controllor.h
private slots:
    void onHandleRegister(const QString &u, const QString &e,
                          const QString &ph, const QString &p,
                          const QString &cp);

    void onHandleLogin(const QString &username, const QString &password);

private:
    LoginWidget* m_view;

};

#endif
