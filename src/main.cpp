#include <QApplication>
#include "module/homewidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 管理员窗口
    HomeWidget* adminHome = new HomeWidget(1, "管理员", 1);
    adminHome->setWindowTitle("管理员模式");
    adminHome->move(50, 50);
    adminHome->show();

    // 普通用户窗口
    HomeWidget* userHome = new HomeWidget(2, "普通用户", 0);
    userHome->setWindowTitle("普通用户模式");
    userHome->move(850, 50);
    userHome->show();

    return a.exec();
}