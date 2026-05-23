#ifndef INVENTORY_CONTROLLOR_H
#define INVENTORY_CONTROLLOR_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include "inventorywidget.h" // 引入你的 View 视图

class InventoryControllor : public QObject
{
    Q_OBJECT
public:
    explicit InventoryControllor(QObject *parent = nullptr);
    ~InventoryControllor();

    // 核心桥梁：将前端 UI Widget 传入进行绑定
    void bindWithView(InventoryWidget* view);

private slots:
    // 响应前端 UI 请求的槽函数
    void handleRefresh();
    void handleSearch(const QString& keyword);
    void handleAddProduct(const QVariantMap& productData);
    void handleUpdateProduct(int id, const QVariantMap& productData);
    void handleDeleteProduct(int id);
    void handleAdminRegister(const QString& verifyCode, int userId);

private:
    InventoryWidget* m_view = nullptr; // 指向界面
};

#endif