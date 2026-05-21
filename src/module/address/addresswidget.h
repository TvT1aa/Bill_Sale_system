#ifndef ADDRESSWIDGET_H
#define ADDRESSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>

class AddressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddressWidget(int userId, QWidget *parent = nullptr);
    ~AddressWidget();

signals:
    // 向后端请求数据的信号
    void refreshRequested();                        // 请求刷新地址列表
    void addAddressRequested(const QVariantMap& address);      // 添加地址
    void updateAddressRequested(int id, const QVariantMap& address);  // 更新地址
    void deleteAddressRequested(int id);            // 删除地址
    void setDefaultAddressRequested(int id);        // 设为默认地址

public slots:
    // 后端调用的槽（填充数据）
    void onAddressLoaded(const QList<QVariantMap>& addresses);
    void onOperationSuccess(const QString& message);
    void onOperationError(const QString& error);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSetDefaultClicked();
    void onRefreshClicked();

private:
    void setupUI();
    void showAddressDialog(int addressId = -1, const QVariantMap& data = QVariantMap());

    QTableWidget* m_tableWidget;
    QPushButton* m_addBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_setDefaultBtn;
    QPushButton* m_refreshBtn;

    int m_userId;
};

#endif // ADDRESSWIDGET_H