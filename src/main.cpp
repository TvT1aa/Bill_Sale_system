#include <QApplication>
#include <QMetaObject>
#include <QMessageBox>
#include <QRandomGenerator>
#include "login_controllor.h"
#include "databasemanager.h"
#include "module/inventory/inventorywidget.h"
#include "module/cart_in/cartwidget.h"
#include "module/deduct/deductwidget.h"
#include "module/balance/balancewidget.h"
#include "module/address/addresswidget.h"
#include "module/inventory/admin_validation.h"
#include "module/report/reportwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!DatabaseManager::instance().connectToDatabase()) {
        qDebug() << "数据库连接失败：" << DatabaseManager::instance().getLastError();
        return -1;
    }
    qDebug() << "数据库连接成功";

    LoginWidget loginWidget;
    login_controllor con;
    con.setView(&loginWidget);
    loginWidget.show();

    InventoryWidget inventoryWidget;
    CartWidget cartWidget;
    DeductWidget deductWidget;
    BalanceWidget balanceWidget;
    AddressWidget addressWidget;

    // 显示所有窗口
    // inventoryWidget.show();
    // cartWidget.show();
    // deductWidget.show();
    // balanceWidget.show();
    // addressWidget.show();

    AdminValidation w;
    w.show();

    ReportWidget reportWidget;
    reportWidget.show();



    return a.exec();
}