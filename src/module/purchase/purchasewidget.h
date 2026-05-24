#ifndef PURCHASEWIDGET_H
#define PURCHASEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVariantMap>
#include <QList>

class PurchaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PurchaseWidget(int userId, QWidget *parent = nullptr);
    ~PurchaseWidget();

signals:
    void refreshRequested();
    void searchProductRequested(const QString& keyword);
    void addToPurchaseRequested(int productId, int quantity, double price);
    void removeFromPurchaseRequested(int itemId);
    void submitPurchaseRequested();

public slots:
    void onProductsLoaded(const QList<QVariantMap>& products);
    void onPurchaseListLoaded(const QList<QVariantMap>& items);
    void onOperationSuccess(const QString& message);
    void onOperationError(const QString& error);

private slots:
    void onSearchClicked();
    void onRefreshClicked();
    void onAddToPurchase();
    void onSubmitPurchase();

private:
    void setupUI();

    QTableWidget* m_productTable;
    QTableWidget* m_purchaseTable;
    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_addToPurchaseBtn;
    QPushButton* m_submitPurchaseBtn;
    QSpinBox* m_quantitySpin;
    QDoubleSpinBox* m_priceSpin;
    QLabel* m_totalLabel;

    int m_userId;
};

#endif // PURCHASEWIDGET_H
