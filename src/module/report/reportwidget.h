#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(int userId, QWidget *parent = nullptr);
    ~ReportWidget();

signals:
    void refreshRequested();

public slots:
    void onSalesSummaryLoaded(double totalSales, double totalProfit, int orderCount);
    void onOperationError(const QString& error);

private slots:
    void onRefreshClicked();

private:
    void setupUI();

    QLabel* m_totalSalesLabel;
    QLabel* m_totalProfitLabel;
    QLabel* m_orderCountLabel;
    QPushButton* m_refreshBtn;

    int m_userId;
};

#endif // REPORTWIDGET_H
