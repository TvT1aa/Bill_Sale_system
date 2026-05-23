#ifndef BALANCE_CONTROLLOR_H
#define BALANCE_CONTROLLOR_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include "databasemanager.h"

class BalanceWidget;

class balance_controllor : public QObject
{
    Q_OBJECT

public:
    explicit balance_controllor(int userId, BalanceWidget *widget, QObject *parent = nullptr);

private:
    int m_userId;
    BalanceWidget *m_view;
};

#endif // BALANCE_CONTROLLOR_H
