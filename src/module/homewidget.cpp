#include "homewidget.h"
#include "inventory/inventorywidget.h"
#include "cart_in/cartwidget.h"
#include "address/addresswidget.h"
#include "balance/balancewidget.h"
#include "deduct/deductwidget.h"
#include "report/reportwidget.h"
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
{
    setupUI(role);
}

HomeWidget::~HomeWidget()
{
}

void HomeWidget::setupUI(int role)
{
    setWindowTitle(QString("Bill&Sale 系统 - 欢迎 %1").arg(m_username));
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

    m_userInfoLabel = new QLabel(QString("用户：%1").arg(m_username), sidebar);
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

    QPushButton* logoutBtn = new QPushButton("退出登录", sidebar);
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
        {"🛒 购物车", "cart"},
        {"📍 地址", "address"},
        {"💰 余额", "balance"}
    };

    for (const auto& menu : menus) {
        m_menuList->addItem(menu.first);
    }

    connect(m_menuList, &QListWidget::currentRowChanged, this, &HomeWidget::onMenuClicked);
}

void HomeWidget::setupAdminMenu()
{
    QList<QPair<QString, QString>> menus = {
        {"📦 库存管理", "inventory"},
        {"📦 进货管理", "cart"},
        {"📤 出库管理", "deduct"},
        {"💰 库存余额", "balance"},
        {"📋 报表", "report"}
    };

    for (const auto& menu : menus) {
        m_menuList->addItem(menu.first);
    }

    connect(m_menuList, &QListWidget::currentRowChanged, this, &HomeWidget::onMenuClicked);
}

void HomeWidget::createUserPages()
{
    // 购物车页面
    m_cartWidget = new CartWidget(m_userId, 0, this);
    m_stackedWidget->addWidget(m_cartWidget);

    // 结算页面（使用 deductwidget，mode=0 用户结算模式）
    m_deductWidget = new DeductWidget(m_userId, 0, this);
    m_stackedWidget->addWidget(m_deductWidget);

    // 地址页面
    m_addressWidget = new AddressWidget(m_userId, this);
    m_stackedWidget->addWidget(m_addressWidget);

    // 余额页面
    m_balanceWidget = new BalanceWidget(m_userId, 0, this);
    m_stackedWidget->addWidget(m_balanceWidget);

    // 购物车结算跳转到结算页面
    connect(m_cartWidget, &CartWidget::checkoutRequested, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_deductWidget);
        emit m_deductWidget->loadCheckoutDataRequested();
    });
}

void HomeWidget::createAdminPages()
{
    // 库存页面
    m_inventoryWidget = new InventoryWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_inventoryWidget);

    // 进货页面
    m_cartWidget = new CartWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_cartWidget);

    // 出库页面（mode=1 管理员模式）
    m_deductWidget = new DeductWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_deductWidget);

    // 余额页面
    m_balanceWidget = new BalanceWidget(m_userId, 1, this);
    m_stackedWidget->addWidget(m_balanceWidget);

    // 报表页面
    m_reportWidget = new ReportWidget(m_userId, this);
    m_stackedWidget->addWidget(m_reportWidget);
}

void HomeWidget::onMenuClicked(int row)
{
    m_stackedWidget->setCurrentIndex(row);
}

void HomeWidget::onLogout()
{
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
    } else if (m_role == 0 && currentWidget == m_addressWidget && m_addressWidget) {
        emit m_addressWidget->refreshRequested();
    } else if (currentWidget == m_balanceWidget && m_balanceWidget) {
        emit m_balanceWidget->refreshRequested();
    } else if (m_role == 0 && currentWidget == m_deductWidget && m_deductWidget) {
        emit m_deductWidget->loadCheckoutDataRequested();
    } else if (m_role == 1 && currentWidget == m_deductWidget && m_deductWidget) {
        emit m_deductWidget->refreshRequested();
    } else if (m_role == 1 && currentWidget == m_reportWidget && m_reportWidget) {
        emit m_reportWidget->refreshRequested();
    }
}