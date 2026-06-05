/**
 * @file loginwidget.cpp
 * @brief 登录窗口实现文件
 *
 * 本文件实现了登录窗口的所有功能，包括：
 * - UI初始化与样式设置
 * - 窗口拖动功能
 * - 登录、注册、忘记密码的界面跳转
 * - 信号转发机制（将用户操作通过信号传递给外部业务层）
 */

#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "registerwidget.h"
#include "mainwindow.h"
#include "module/homewidget.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

// ==================== 构造函数 ====================
/**
 * @brief 登录窗口构造函数
 * @param parent 父窗口指针，默认为nullptr
 *
 * 功能：
 * 1. 加载UI文件并初始化界面
 * 2. 设置无边框透明窗口样式
 * 3. 添加卡片阴影效果
 * 4. 设置输入框文字颜色
 * 5. 连接信号槽（回车键登录、关闭按钮）
 */
LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)                    // 调用父类构造函数
    , ui(new Ui::LoginWidget)            // 创建UI对象（由Qt Designer生成）
{
    // 加载UI文件，创建所有界面控件
    ui->setupUi(this);

    // ========== 窗口样式设置 ==========
    // 移除窗口边框（实现自定义标题栏）
    setWindowFlags(Qt::FramelessWindowHint);
    // 启用透明背景（配合阴影效果实现圆角阴影窗口）
    setAttribute(Qt::WA_TranslucentBackground);

    // ========== 卡片阴影效果 ==========
    // 为 cardWidget 添加阴影，使其看起来像浮起来的卡片
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);           // 阴影模糊半径（20像素）
    shadow->setColor(QColor(0, 0, 0, 50)); // 阴影颜色：黑色，透明度50/255
    shadow->setOffset(0, 5);             // 阴影偏移：水平0，垂直5像素
    ui->cardWidget->setGraphicsEffect(shadow);

    // ========== 输入框文字颜色设置 ==========
    // 将用户名和密码输入框的文字颜色设为黑色
    // （因为全局样式可能设为了白色，需要覆盖）
    QPalette pal = ui->leUsername->palette();
    pal.setColor(QPalette::Text, Qt::black);
    ui->leUsername->setPalette(pal);
    ui->lePassword->setPalette(pal);

    // ========== 信号槽连接 ==========
    // 密码框按回车键 → 触发登录
    connect(ui->lePassword, &QLineEdit::returnPressed,
            this, &LoginWidget::on_btnLogin_clicked);
    // 用户名框按回车键 → 触发登录
    connect(ui->leUsername, &QLineEdit::returnPressed,
            this, &LoginWidget::on_btnLogin_clicked);
    // 关闭按钮点击 → 退出程序
    connect(ui->btn_close, &QPushButton::clicked,
            this, &LoginWidget::on_btn_close_clicked);
}

// ==================== 析构函数 ====================
/**
 * @brief 登录窗口析构函数
 *
 * 功能：释放 UI 对象占用的内存
 */
LoginWidget::~LoginWidget()
{
    delete ui;  // 删除UI对象，释放其管理的所有控件
}

// ==================== 私有槽函数 ====================

/**
 * @brief 关闭按钮点击响应
 *
 * 功能：退出整个应用程序
 * 注意：使用 QApplication::quit() 而不是 this->close()
 *       因为登录窗口可能不是主窗口，直接close可能不会退出程序
 */
void LoginWidget::on_btn_close_clicked()
{
    QApplication::quit();  // 退出应用程序
}

/**
 * @brief 忘记密码按钮点击响应
 *
 * 功能流程：
 * 1. 隐藏当前登录窗口
 * 2. 创建忘记密码窗口（MainWindow）
 * 3. 设置窗口关闭时自动删除
 * 4. 监听忘记密码窗口的销毁事件，销毁后重新显示登录窗口
 * 5. 连接忘记密码窗口的重置密码信号到本窗口的转发信号
 * 6. 显示忘记密码窗口
 */
void LoginWidget::on_btn_forgetPwd_clicked()
{
    this->hide();

    // 1. 创建窗口
    m_forgetWindow = new MainWindow(this);
    m_forgetWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 监听销毁事件
    connect(m_forgetWindow, &MainWindow::destroyed, this, [this]() {
        m_forgetWindow = nullptr;
        this->show();
    });

    // 3. 信号转发 (严格对应你的头文件声明)
    // 补上缺失的 requestVerificationCode 转发
    connect(m_forgetWindow, &MainWindow::requestVerificationCode,
            this, &LoginWidget::requestVerificationCode); // 如果这里报错，去 .h 里把这个信号加上

    // 使用你 .h 中已定义的 passwordResetSubmitted
    connect(m_forgetWindow, &MainWindow::resetPasswordRequested,
            this, &LoginWidget::passwordResetSubmitted);

    m_forgetWindow->show();
}
/**
 * @brief 用户注册按钮点击响应
 *
 * 功能流程：
 * 1. 隐藏当前登录窗口
 * 2. 创建用户注册窗口（role=0 表示普通用户）
 * 3. 设置窗口关闭时自动删除
 * 4. 监听注册窗口的销毁事件，销毁后重新显示登录窗口
 * 5. 连接注册窗口的返回登录信号，点击后关闭注册窗口
 * 6. 信号转发：将注册窗口的 registerSubmitted 信号转发出去
 * 7. 显示注册窗口
 */
void LoginWidget::on_btn_register_clicked()
{
    // 隐藏登录窗口
    this->hide();

    // 创建注册窗口，参数 0 表示普通用户注册
    m_regWindow = new RegisterWidget(0);
    // 设置窗口关闭时自动释放内存
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);

    // 监听注册窗口的销毁事件
    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;  // 窗口已销毁，指针置空
        this->show();           // 重新显示登录窗口
    });

    // 连接注册窗口的返回登录信号
    // 当用户在注册窗口点击"返回登录"时，关闭注册窗口
    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow)
            m_regWindow->close();  // 关闭注册窗口（会触发destroyed信号）
    });

    // 信号转发：将注册窗口的 registerSubmitted 信号
    // 转发为 LoginWidget 的 registerSubmitted 信号
    // 这样外部只需要连接 LoginWidget 的信号即可
    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    // 显示注册窗口
    m_regWindow->show();
}

/**
 * @brief 管理员注册按钮点击响应
 *
 * 功能流程：
 * 1. 隐藏当前登录窗口
 * 2. 创建管理员注册窗口（role=1 表示管理员）
 * 3. 设置窗口标题为"注册管理员"
 * 4. 其他逻辑与用户注册相同
 */
void LoginWidget::on_btn_registerAdmin_clicked()
{
    // 隐藏登录窗口
    this->hide();

    // 创建注册窗口，参数 1 表示管理员注册
    m_regWindow = new RegisterWidget(1);
    // 设置窗口关闭时自动释放内存
    m_regWindow->setAttribute(Qt::WA_DeleteOnClose);
    // 设置窗口标题（区分用户注册）
    m_regWindow->setWindowTitle("Register Admin");

    // 监听注册窗口的销毁事件
    connect(m_regWindow, &RegisterWidget::destroyed, this, [this]() {
        m_regWindow = nullptr;  // 窗口已销毁，指针置空
        this->show();           // 重新显示登录窗口
    });

    // 连接注册窗口的返回登录信号
    connect(m_regWindow, &RegisterWidget::backToLogin, this, [this]() {
        if (m_regWindow)
            m_regWindow->close();
    });

    // 信号转发：将注册窗口的 registerSubmitted 信号转发出去
    connect(m_regWindow, &RegisterWidget::registerSubmitted,
            this, &LoginWidget::registerSubmitted);

    // 显示注册窗口
    m_regWindow->show();
}

/**
 * @brief 登录按钮点击响应
 *
 * 功能流程：
 * 1. 获取用户名和密码输入框的内容
 * 2. 去除用户名首尾空格
 * 3. 校验用户名和密码是否为空
 * 4. 校验通过后，发射 loginSubmitted 信号
 *    （由外部业务层处理实际登录请求）
 */
void LoginWidget::on_btnLogin_clicked()
{
    // 获取输入框内容
    // trimmed() 去除首尾空格
    QString username = ui->leUsername->text().trimmed();
    // 密码不去空格（密码可能包含空格）
    QString password = ui->lePassword->text();

    // 前端校验：用户名和密码不能为空
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Tip", "Please enter username and password");
        return;  // 校验失败，不发射登录信号
    }

    // 发射登录信号，携带用户名和密码
    // 外部（如 MainWindow 或网络模块）会连接此信号并处理实际登录
    emit loginSubmitted(username, password);
}

// ==================== 鼠标事件（窗口拖动功能） ====================

/**
 * @brief 鼠标按下事件
 * @param event 鼠标事件对象
 *
 * 功能：实现窗口拖动功能
 * 流程：
 * 1. 判断是否按下左键
 * 2. 获取鼠标点击的位置
 * 3. 判断点击区域是否允许拖动（点击标题栏区域）
 *    - 点击了 label_logo 控件
 *    - 或者 Y 坐标 < 80（标题栏区域）
 * 4. 记录拖动起始位置（全局坐标 - 窗口左上角坐标）
 */
void LoginWidget::mousePressEvent(QMouseEvent *event)
{
    // 只处理左键按下
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();              // 获取鼠标在窗口内的坐标
        QWidget *child = childAt(pos);          // 获取点击位置下的子控件

        // 判断是否点击了可拖动区域
        // 条件1：点击了 label_logo 控件
        // 条件2：或者点击位置的Y坐标小于80（标题栏区域）
        if (child == ui->label_logo || pos.y() < 80) {
            // 计算拖动起始位置 = 鼠标全局坐标 - 窗口左上角坐标
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();  // 事件已被处理
        }
    }
}

/**
 * @brief 鼠标移动事件
 * @param event 鼠标事件对象
 *
 * 功能：实现窗口拖动功能
 * 流程：
 * 1. 判断是否按住左键并且有记录的拖动起始位置
 * 2. 移动窗口到新的位置
 */
void LoginWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 按住左键 && 有记录的拖动起始位置
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        // 计算新位置 = 鼠标全局坐标 - 记录的偏移量
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();  // 事件已被处理
    }
}

// ==================== 公共槽函数（供外部调用） ====================

/**
 * @brief 显示登录失败提示
 * @param message 错误信息文本
 *
 * 功能：弹出一个警告对话框显示登录失败的原因
 * 调用时机：外部业务层验证登录失败后调用
 */
void LoginWidget::showLoginError(const QString &message)
{
    QMessageBox::warning(this, "Login Failed", message);
}

/**
 * @brief 显示登录成功提示
 * @param message 成功信息文本
 *
 * 功能：弹出一个信息对话框显示登录成功
 * 调用时机：外部业务层验证登录成功后调用
 */
void LoginWidget::showLoginSuccess(const QString &message)
{
    QMessageBox::information(this, "Login Success", message);
}

/**
 * @brief 导航到主界面
 * @param userId 用户ID
 * @param username 用户名
 * @param role 角色（0=普通用户，1=管理员）
 *
 * 功能：
 * 1. 隐藏当前登录窗口
 * 2. 创建主界面（HomeWidget）
 * 3. 设置主界面关闭时自动删除
 * 4. 监听主界面销毁事件，销毁后重新显示登录窗口
 * 5. 显示主界面
 *
 * 调用时机：登录成功后由外部调用
 */
void LoginWidget::navigateToHome(int userId, const QString &username, int role)
{
    // 隐藏登录窗口
    this->hide();

    // 创建主界面，传入用户信息
    HomeWidget *homeWidget = new HomeWidget(userId, username, role);
    // 设置主界面关闭时自动释放内存
    homeWidget->setAttribute(Qt::WA_DeleteOnClose);

    // 监听主界面的销毁事件
    // 当主界面关闭时，重新显示登录窗口
    connect(homeWidget, &HomeWidget::destroyed, this, [this]() {
        this->show();  // 重新显示登录窗口
    });

    // 显示主界面
    homeWidget->show();
}

/**
 * @brief 显示注册错误提示（转发给注册窗口）
 * @param message 错误信息文本
 *
 * 功能：将注册失败的错误信息转发给当前打开的注册窗口显示
 * 调用时机：外部业务层注册失败后调用
 */
void LoginWidget::showRegisterError(const QString &message)
{
    if (m_regWindow)
        m_regWindow->showRegisterError(message);  // 调用注册窗口的错误显示函数
}

/**
 * @brief 显示注册成功提示（转发给注册窗口）
 * @param message 成功信息文本
 *
 * 功能：将注册成功的提示转发给当前打开的注册窗口显示
 * 调用时机：外部业务层注册成功后调用
 */
void LoginWidget::showRegisterSuccess(const QString &message)
{
    if (m_regWindow)
        m_regWindow->showRegisterSuccess(message);  // 调用注册窗口的成功显示函数
}

/**
 * @brief 显示重置密码错误提示（转发给忘记密码窗口）
 * @param message 错误信息文本
 *
 * 功能：将重置密码失败的错误信息转发给忘记密码窗口显示
 * 调用时机：外部业务层重置密码失败后调用
 */
void LoginWidget::showResetError(const QString &message)
{
    if (m_forgetWindow)
        m_forgetWindow->showResetError(message);  // 调用忘记密码窗口的错误显示函数
}

/**
 * @brief 显示重置密码成功提示（转发给忘记密码窗口）
 * @param message 成功信息文本
 *
 * 功能：将重置密码成功的提示转发给忘记密码窗口显示
 * 调用时机：外部业务层重置密码成功后调用
 */
void LoginWidget::showResetSuccess(const QString &message)
{
    if (m_forgetWindow)
        m_forgetWindow->showResetSuccess(message);  // 调用忘记密码窗口的成功显示函数
}