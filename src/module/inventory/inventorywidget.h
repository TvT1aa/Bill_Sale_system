#ifndef INVENTORYWIDGET_H
#define INVENTORYWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVariantMap>
#include <QList>

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(int userId, int role, QWidget *parent = nullptr);
    ~InventoryWidget();

signals:
    // 向后端请求数据的信号
    void refreshRequested();
    void searchRequested(const QString& keyword);
    void addProductRequested(const QVariantMap& product);
    void updateProductRequested(int id, const QVariantMap& product);
    void deleteProductRequested(int id);
    void adminRegisterRequested(const QString& code);
    void productSelected(int productId);
    void logoutRequested();  // 添加这行：退出登录信号

public slots:
    void onInventoryLoaded(const QList<QVariantMap>& products);
    void onSearchResult(const QList<QVariantMap>& products);
    void onOperationSuccess(const QString& message);
    void onOperationError(const QString& error);
    void onAdminRegisterResult(bool success, const QString& message);

private slots:
    void onSearchClicked();
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRegisterAdminClicked();
    void onTableItemDoubleClicked(int row, int col);
    void onRefreshClicked();

private:
    void setupUI(int role);
    void setupUserUI();
    void setupAdminUI();

    QTableWidget* m_tableWidget;
    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QPushButton* m_refreshBtn;

    QPushButton* m_addBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_registerAdminBtn;

    int m_userId;
    int m_role;
};

#endif // INVENTORYWIDGET_H