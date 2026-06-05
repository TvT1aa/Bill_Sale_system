#include <QApplication>
#include <QStyleFactory>
#include "thememanager.h"
#include "databasemanager.h"
#include "module/login/loginwidget.h"
#include "module/login/login_controllor.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ThemeManager::forceLightTheme();
    if (!DatabaseManager::instance().connectToDatabase()) {
        qDebug() << "Database connection failed:" << DatabaseManager::instance().getLastError();
        return -1;
    }
    qDebug() << "Database connected successfully";

    LoginWidget loginWidget;
    login_controllor controller;
    controller.setView(&loginWidget);
    loginWidget.show();
    return a.exec();
}
