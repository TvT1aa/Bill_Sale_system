//
// Created by ASUS on 2026/5/15.
//

#include "databasemanager.h"
#include "inventory_controllor.h"
#include <QDebug>
InventoryControllor::InventoryControllor(QObject *parent)
    : QObject(parent)
    , m_view(nullptr) // 初始化视图指针为空，防止野指针
{
}
InventoryControllor::~InventoryControllor()
{
}
void InventoryControllor::bindWithView(InventoryWidget* view) {
    m_view = view;
    // ---- 建立信号与槽的连接 (双向奔赴) ----

    // 监听视图的“刷新”请求 -> 触发控制器的“处理刷新”业务
    connect(m_view, &InventoryWidget::refreshRequested,
            this, &InventoryControllor::handleRefresh);

    // 监听视图的“搜索”请求 -> 触发控制器的“处理搜索”业务
    connect(m_view, &InventoryWidget::searchRequested,
            this, &InventoryControllor::handleSearch);

    // 监听视图的“添加商品”请求 -> 触发控制器的“处理添加”业务
    connect(m_view, &InventoryWidget::addProductRequested,
            this, &InventoryControllor::handleAddProduct);

    // 监听视图的“修改商品”请求 -> 触发控制器的“处理更新”业务
    connect(m_view, &InventoryWidget::updateProductRequested,
            this, &InventoryControllor::handleUpdateProduct);

    // 监听视图的“删除商品”请求 -> 触发控制器的“处理删除”业务
    connect(m_view, &InventoryWidget::deleteProductRequested,
            this, &InventoryControllor::handleDeleteProduct);

    // 监听视图的“注册管理员”请求 -> 触发控制器的“身份提权”业务
    // 考虑到界面上信号参数可能跟槽不同，这里直接用 Lambda 表达式进行安全对接
    connect(m_view, &InventoryWidget::adminRegisterRequested, this, [this](const QString& code) {
        // 传入激活码，并默认传入当前用户的ID（这里暂定为1，实际可从全局用户管理中获取）
        this->handleAdminRegister(code, 1);
    });
}
void handleRefresh() {

}
void InventoryControllor::handleRefresh() {}
void InventoryControllor::handleSearch(const QString&) {}
void InventoryControllor::handleAddProduct(const QMap<QString, QVariant>&) {}
void InventoryControllor::handleUpdateProduct(int, const QMap<QString, QVariant>&) {}
void InventoryControllor::handleDeleteProduct(int) {}
void InventoryControllor::handleAdminRegister(const QString&, int) {}