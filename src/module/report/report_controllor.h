#ifndef REPORT_CONTROLLOR_H
#define REPORT_CONTROLLOR_H

#include <QObject>
#include "databasemanager.h"

class ReportWidget;

class report_controllor : public QObject
{
    Q_OBJECT

public:
    explicit report_controllor(int userId, ReportWidget *widget, QObject *parent = nullptr);

private:
    int m_userId;
    ReportWidget *m_view;
};

#endif // REPORT_CONTROLLOR_H
