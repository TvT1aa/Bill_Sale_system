#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

class InventoryWidget;
class CartWidget;
class AddressWidget;
class BalanceWidget;
class DeductWidget;
class ReportWidget;
class PurchaseWidget;

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWidget(int userId, const QString& username, int role, QWidget *parent = nullptr);
    ~HomeWidget();

signals:
    void logoutRequested();

public slots:
    void refreshCurrentPage();

private slots:
    void onMenuClicked(int row);
    void onLogout();

private:
    void setupUI(int role);
    void setupUserMenu();
    void setupAdminMenu();
    void createUserPages();
    void createAdminPages();

    QListWidget* m_menuList;
    QStackedWidget* m_stackedWidget;
    QLabel* m_userInfoLabel;

    int m_userId;
    QString m_username;
    int m_role;

    InventoryWidget* m_inventoryWidget;
    CartWidget* m_cartWidget;
    AddressWidget* m_addressWidget;
    BalanceWidget* m_balanceWidget;
    DeductWidget* m_deductWidget;
    ReportWidget* m_reportWidget;
    PurchaseWidget* m_purchaseWidget;
};

#endif // HOMEWIDGET_H