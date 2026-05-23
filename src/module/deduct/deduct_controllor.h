#ifndef DEDUCT_CONTROLLOR_H
#define DEDUCT_CONTROLLOR_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include "cart.h"
#include "databasemanager.h"

class DeductWidget;

class deduct_controllor : public QObject
{
    Q_OBJECT

public:
    explicit deduct_controllor(int userId, int mode, DeductWidget *widget, QObject *parent = nullptr);

private:
    int m_userId;
    int m_mode;
    DeductWidget *m_view;
};

#endif // DEDUCT_CONTROLLOR_H
