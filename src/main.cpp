#include <QApplication>
#include <QDebug>
#include "common/databasemanager.h"
#include "module/login/loginwidget.h"
#include "module/login/login_controllor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!DatabaseManager::instance().connectToDatabase()) {
        qDebug() << "数据库连接失败：" << DatabaseManager::instance().getLastError();
        return -1;
    }
    qDebug() << "数据库连接成功";

    LoginWidget loginWidget;
    login_controllor controller;
    controller.setView(&loginWidget);
    loginWidget.show();

    return a.exec();
}