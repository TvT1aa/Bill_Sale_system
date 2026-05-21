#ifndef DEDUCTWIDGET_H
#define DEDUCTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include "common/databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DeductWidget; }
QT_END_NAMESPACE

class DeductWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeductWidget(QWidget *parent = nullptr);
    ~DeductWidget();

    void setCurrentUser(int userId, const QString &username);

    void refreshProductList();
    void refreshOrders();
    bool deductStock(int productId, int quantity, const QString &remark);

private slots:
    void onProductComboChanged(int index);
    void onConfirmButtonClicked();
    void onRefreshHistoryButtonClicked();  // 改为 UI 中实际存在的名称

private:
    void setupTable();
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void displayOrders(const QList<SalesOrderInfo> &orders);
    void addOrderToTable(const SalesOrderInfo &order, int row);

    int getProductStock(int productId);
    double getProductPrice(int productId);

private:
    Ui::DeductWidget *ui;
    int m_currentUserId;
    QString m_currentUsername;
    QList<SalesOrderInfo> m_orders;
};

#endif // DEDUCTWIDGET_H