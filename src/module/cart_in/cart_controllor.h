#ifndef CART_CONTROLLOR_H
#define CART_CONTROLLOR_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include "cart.h"
#include "databasemanager.h"

class CartWidget;

class cart_controllor : public QObject
{
    Q_OBJECT

public:
    explicit cart_controllor(int userId, CartWidget *widget, QObject *parent = nullptr);

private:
    int m_userId;
    CartWidget *m_view;
};

#endif // CART_CONTROLLOR_H
