#ifndef PURCHASE_CONTROLLOR_H
#define PURCHASE_CONTROLLOR_H

#include <QObject>
#include <QList>
#include <QVariantMap>
#include "databasemanager.h"

class PurchaseWidget;

struct PurchaseItem {
    int productId;
    QString productName;
    int quantity;
    double unitPrice;
};

class purchase_controllor : public QObject
{
    Q_OBJECT

public:
    explicit purchase_controllor(int userId, PurchaseWidget *widget, QObject *parent = nullptr);

private:
    int m_userId;
    PurchaseWidget *m_view;
    QList<PurchaseItem> m_purchaseList;
};

#endif // PURCHASE_CONTROLLOR_H
