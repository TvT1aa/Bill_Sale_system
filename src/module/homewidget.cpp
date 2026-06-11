#include "homewidget.h"
#include "inventory/inventorywidget.h"
#include "cart_in/cartwidget.h"
#include "address/addresswidget.h"
#include "balance/balancewidget.h"
#include "deduct/deductwidget.h"
#include "report/reportwidget.h"
#include "purchase/purchasewidget.h"
#include "cart_in/cart_controllor.h"
#include "address/address_controllor.h"
#include "deduct/deduct_controllor.h"
#include "inventory/inventory_controllor.h"
#include "purchase/purchase_controllor.h"
#include "balance/balance_controllor.h"
#include "report/report_controllor.h"
#include "cart.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>

HomeWidget::HomeWidget(int userId, const QString& username, int role, QWidget *parent)
    : QWidget(parent)
    , m_userId(userId)
    , m_username(username)
    , m_role(role)
    , m_inventoryWidget(nullptr)
    , m_cartWidget(nullptr)
    , m_addressWidget(nullptr)
    , m_balanceWidget(nullptr)
    , m_deductWidget(nullptr)
    , m_reportWidget(nullptr)
    , m_purchaseWidget(nullptr)
{
    setupUI(role);
}

HomeWidget::~HomeWidget()
{
}

void HomeWidget::setupUI(int role)
{
    setWindowTitle(QString("Bill&Sale System - Welcome %1").arg(m_username));
    resize(1200, 700);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 左侧边栏 ==========
    QWidget* sidebar = new QWidget(this);
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet("QWidget { background-color: #304156; }");
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 20, 0, 0);
    sidebarLayout->setSpacing(0);

    m_userInfoLabel = new QLabel(QString("User: %1").arg(m_username), sidebar);
    m_userInfoLabel->setStyleSheet("QLabel { color: white; font-size: 14px; font-weight: bold; padding: 10px 15px; border-bottom: 1px solid #4B6A8A; }");
    m_userInfoLabel->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(m_userInfoLabel);

    m_menuList = new QListWidget(sidebar);
    m_menuList->setStyleSheet(
        "QListWidget { background-color: #304156; border: none; outline: none; }"
        "QListWidget::item { color: #E6E6E6; padding: 12px 15px; font-size: 14px; }"
        "QListWidget::item:selected { background-color: #1E2A36; color: #409EFF; border-left: 3px solid #409EFF; }"
        "QListWidget::item:hover { background-color: #263445; }"
        );
    m_menuList->setFocusPolicy(Qt::NoFocus);

    if (m_role == 0) {
        setupUserMenu();
    } else {
        setupAdminMenu();
    }

    sidebarLayout->addWidget(m_menuList);
    sidebarLayout->addStretch();

    QPushButton* logoutBtn = new QPushButton("Logout", sidebar);
    logoutBtn->setStyleSheet(
        "QPushButton { background-color: #F56C6C; color: white; border: none; padding: 10px; margin: 15px; border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #F78989; }"
        );
    connect(logoutBtn, &QPushButton::clicked, this, &HomeWidget::onLogout);
    sidebarLayout->addWidget(logoutBtn);

    // ========== 右侧内容区 ==========
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet("QStackedWidget { background-color: #F5F7FA; }");

    if (m_role == 0) {
        createUserPages();
    } else {
        createAdminPages();
    }

    m_menuList->setCurrentRow(0);
    onMenuClicked(0);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(m_stackedWidget, 1);
}

void HomeWidget::setupUserMenu()
{
    QList<QPair<QString, QString>> menus = {
        {"🛒 Cart", "cart"},
        {"📍 Checkout", "address"}
    };

    for (const auto& menu : menus) {
        m_menuList->addItem(menu.first);
    }

    connect(m_menuList, &QListWidget::currentRowChanged, this, &HomeWidget::onMenuClicked);
}

void HomeWidget::setupAdminMenu()
{
    QList<QPair<QString, QString>> menus = {
        {"📦 Inventory", "inventory"},
        {"📦 Purchase", "purchase"},
        {"📤 Dispatch", "deduct"},
        {"💰 Balance", "balance"},
        {"📋 Reports", "report"}
    };

    for (const auto& menu : menus) {
        m_menuList->addItem(menu.first);
    }

    connect(m_menuList, &QListWidget::currentRowChanged, this, &HomeWidget::onMenuClicked);
}
void HomeWidget::createUserPages()
{
    m_cartWidget = new CartWidget(m_userId, 0, this);
    m_stackedWidget->addWidget(m_cartWidget);

    m_deductWidget = new DeductWidget(m_userId, 0, this);
    m_stackedWidget->addWidget(m_deductWidget);

    m_addressWidget = new AddressWidget(m_userId, this);
    m_stackedWidget->addWidget(m_addressWidget);

    // 先创建所有控制器，确保信号连接就绪
    new cart_controllor(m_userId, m_cartWidget, this);
    new address_controllor(m_userId, m_addressWidget, this);
    new deduct_controllor(m_userId, 0, m_deductWidget, this);

    // 控制器连接完成后，再触发初始数据加载
    emit m_cartWidget->refreshRequested();
    emit m_deductWidget->loadCheckoutDataRequested();

    connect(m_cartWidget, &CartWidget::checkoutRequested, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_deductWidget);
        emit m_deductWidget->loadCheckoutDataRequested();
    });
}

void HomeWidget::createAdminPages()
{
    m_inventoryWidget = new InventoryWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_inventoryWidget);

    m_purchaseWidget = new PurchaseWidget(m_userId, this);
    m_stackedWidget->addWidget(m_purchaseWidget);

    m_deductWidget = new DeductWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_deductWidget);

    m_balanceWidget = new BalanceWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_balanceWidget);

    m_reportWidget = new ReportWidget(m_userId, this);
    m_stackedWidget->addWidget(m_reportWidget);

    // 创建控制器并连接信号
    InventoryControllor* inventoryCtrl = new InventoryControllor(this);
    inventoryCtrl->bindWithView(m_inventoryWidget);

    new purchase_controllor(m_userId, m_purchaseWidget, this);
    new deduct_controllor(m_userId, 1, m_deductWidget, this);
    new balance_controllor(m_userId, m_balanceWidget, this);
    new report_controllor(m_userId, m_reportWidget, this);

    // 初始加载数据
    emit m_inventoryWidget->refreshRequested();
}

void HomeWidget::onMenuClicked(int row)
{
    m_stackedWidget->setCurrentIndex(row);
}

void HomeWidget::onLogout()
{
    // 退出登录前清空购物车：恢复库存并清空购物车列表
    Cart &cart = Cart::instance();
    if (cart.userId() == m_userId) {
        // 遍历购物车中所有商品，将库存加回数据库
        for (const CartItem &item : cart.getCartItems()) {
            DatabaseManager::instance().updateProductStock(item.productId, -item.quantity);
        }
        cart.clearCart();
    }

    emit logoutRequested();
    this->close();
}

void HomeWidget::refreshCurrentPage()
{
    int currentIndex = m_stackedWidget->currentIndex();
    QWidget* currentWidget = m_stackedWidget->widget(currentIndex);

    if (currentWidget == m_inventoryWidget && m_inventoryWidget) {
        emit m_inventoryWidget->refreshRequested();
    } else if (currentWidget == m_cartWidget && m_cartWidget) {
        emit m_cartWidget->refreshRequested();
    } else if (currentWidget == m_purchaseWidget && m_purchaseWidget) {
        emit m_purchaseWidget->refreshRequested();
    } else if (m_role == 0 && currentWidget == m_addressWidget && m_addressWidget) {
        emit m_addressWidget->refreshRequested();
    } else if (m_role == 1 && currentWidget == m_balanceWidget && m_balanceWidget) {
        emit m_balanceWidget->refreshRequested();
    } else if (m_role == 0 && currentWidget == m_deductWidget && m_deductWidget) {
        emit m_deductWidget->loadCheckoutDataRequested();
    } else if (m_role == 1 && currentWidget == m_deductWidget && m_deductWidget) {
        emit m_deductWidget->refreshRequested();
    } else if (m_role == 1 && currentWidget == m_reportWidget && m_reportWidget) {
        emit m_reportWidget->refreshRequested();
    }
}
