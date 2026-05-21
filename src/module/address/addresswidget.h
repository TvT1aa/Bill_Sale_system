#ifndef ADDRESSWIDGET_H
#define ADDRESSWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include "common/databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AddressWidget; }
QT_END_NAMESPACE

class AddressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddressWidget(QWidget *parent = nullptr);
    ~AddressWidget();

    void setCurrentUser(int userId);

    void refreshAddressList();
    bool addAddress(const BuyerAddressInfo &address);
    bool updateAddress(const BuyerAddressInfo &address);
    bool deleteAddress(int addressId);
    bool setDefaultAddress(int addressId);

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onSetDefaultButtonClicked();
    void onTableDoubleClicked(QTableWidgetItem *item);

private:
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void displayAddresses(const QList<BuyerAddressInfo> &addresses);
    void addAddressToTable(const BuyerAddressInfo &address, int row);
    bool showAddressDialog(BuyerAddressInfo &address, bool isEdit = false);

private:
    Ui::AddressWidget *ui;
    int m_currentUserId;
    QList<BuyerAddressInfo> m_currentAddresses;
};

#endif // ADDRESSWIDGET_H