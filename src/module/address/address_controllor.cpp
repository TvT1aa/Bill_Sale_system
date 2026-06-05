#include "address_controllor.h"
#include "addresswidget.h"
#include <QDebug>

static QVariantMap addressToMap(const AddressInfo &a)
{
    QVariantMap m;
    m["id"] = a.id;
    m["userId"] = a.userId;
    m["name"] = a.name;
    m["phone"] = a.phone;
    m["province"] = a.province;
    m["city"] = a.city;
    m["district"] = a.district;
    m["detail"] = a.detail;
    m["isDefault"] = a.isDefault;
    return m;
}

address_controllor::address_controllor(int userId, AddressWidget *widget, QObject *parent)
    : QObject(parent)
    , m_userId(userId)
    , m_view(widget)
{
    connect(m_view, &AddressWidget::refreshRequested, this, [this]() {
        QList<QVariantMap> addresses;
        for (const AddressInfo &a : DatabaseManager::instance().getAddressesByUserId(m_userId))
            addresses.append(addressToMap(a));
        m_view->onAddressLoaded(addresses);
    });

    connect(m_view, &AddressWidget::addAddressRequested, this, [this](const QVariantMap &data) {
        bool ok = DatabaseManager::instance().addAddress(
            m_userId, data["name"].toString(), data["phone"].toString(),
            data["province"].toString(), data["city"].toString(),
            data["district"].toString(), data["detail"].toString(),
            data["isDefault"].toBool());
        if (ok) {
            m_view->onOperationSuccess("Address added successfully");
            QList<QVariantMap> addresses;
            for (const AddressInfo &a : DatabaseManager::instance().getAddressesByUserId(m_userId))
                addresses.append(addressToMap(a));
            m_view->onAddressLoaded(addresses);
        } else {
            m_view->onOperationError("Failed to add address");
        }
    });

    connect(m_view, &AddressWidget::updateAddressRequested, this, [this](int id, const QVariantMap &data) {
        bool ok = DatabaseManager::instance().updateAddress(
            id, data["name"].toString(), data["phone"].toString(),
            data["province"].toString(), data["city"].toString(),
            data["district"].toString(), data["detail"].toString());
        if (ok)
            m_view->onOperationSuccess("Address updated successfully");
        else
            m_view->onOperationError("Failed to update address");
    });

    connect(m_view, &AddressWidget::deleteAddressRequested, this, [this](int id) {
        bool ok = DatabaseManager::instance().deleteAddress(id);
        if (ok) {
            m_view->onOperationSuccess("Address deleted successfully");
            QList<QVariantMap> addresses;
            for (const AddressInfo &a : DatabaseManager::instance().getAddressesByUserId(m_userId))
                addresses.append(addressToMap(a));
            m_view->onAddressLoaded(addresses);
        } else {
            m_view->onOperationError("Failed to delete address");
        }
    });

    connect(m_view, &AddressWidget::setDefaultAddressRequested, this, [this](int id) {
        bool ok = DatabaseManager::instance().setDefaultAddress(m_userId, id);
        if (ok) {
            m_view->onOperationSuccess("Default address set successfully");
            QList<QVariantMap> addresses;
            for (const AddressInfo &a : DatabaseManager::instance().getAddressesByUserId(m_userId))
                addresses.append(addressToMap(a));
            m_view->onAddressLoaded(addresses);
        } else {
            m_view->onOperationError("Failed to set default address");
        }
    });
}
