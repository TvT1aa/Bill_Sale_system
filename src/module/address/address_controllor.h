#ifndef ADDRESS_CONTROLLOR_H
#define ADDRESS_CONTROLLOR_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include "databasemanager.h"

class AddressWidget;

class address_controllor : public QObject
{
    Q_OBJECT

public:
    explicit address_controllor(int userId, AddressWidget *widget, QObject *parent = nullptr);

private:
    int m_userId;
    AddressWidget *m_view;
};

#endif // ADDRESS_CONTROLLOR_H
