# ============================================================================
# 项目名称：BillandSale_system (进销存及销售系统)
# 维护架构：基于目标驱动的现代 CMake 体系 (Target-Based CMake)
# 适用平台：Windows 10/11 + CLion + Qt 6.10.2 (MinGW 工具链)
# 变更记录：追加 Qt6::Network 模块，完美支持安全邮件发送（QSslSocket）
# ============================================================================

# ----------------------------------------------------------------------------
# 【第一阶段】工具链强力注入 (解决 ABI 崩溃的“定海神针”)
# ----------------------------------------------------------------------------
# 设定 CMake 最低版本要求，3.16 是现代 Qt6 启动的底线版本
cmake_minimum_required(VERSION 3.16)

# 🔥 【全盘最核心的一战】：强制指定编译器路径
# 避坑起底：为了防止 CLion 调用其自带的低版本 MinGW 导致符号链接崩溃，
# 必须使用 CACHE PATH "" FORCE 强行踢掉默认缓存，死死锁住 Qt 2026 年最新原装高版本的 gcc 和 g++。
set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe" CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe" CACHE PATH "" FORCE)


# ----------------------------------------------------------------------------
# 【第二阶段】项目元信息声明与 C++ 编译器行为配置
# ----------------------------------------------------------------------------
# 声明项目名称、版本号，显式指定其为 C++ (CXX) 工程
project(BillandSale_system VERSION 0.1 LANGUAGES CXX)

# 开启 Qt 专属的“三大魔术前置编译器”自动探测
set(CMAKE_AUTOMOC ON)  # 自动处理元对象系统（扫描 Q_OBJECT 宏、翻译信号与槽）
set(CMAKE_AUTOUIC ON)  # 自动把 SKF 和全组画的 XML 格式 *.ui 文件翻译成 C++ 头文件
set(CMAKE_AUTORCC ON)  # 自动把静态资源（图片、Logo等）打包编译进二进制 exe 内部
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 允许自动包含当前构建目录，确保预生成的 ui_*.h 触手可及

# 强制定制 C++17 编译标准，且如果编译器不支持则直接报错中止（REQUIRED ON）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ----------------------------------------------------------------------------
# 【第三阶段】外部依赖依赖包定位 (招兵买马)
# ----------------------------------------------------------------------------
# 指示 CMake 去这个根目录下扫描 Qt 配置文件，正斜杠路径防范转义字符灾难
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64")

# 🌟【核心修复点】：在此处追加引入 Network 组件！
# 目的：让编译器能够抓取到 <QSslSocket>、<QTcpSocket> 等网络传输和加密相关的系统头文件
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network)


# ----------------------------------------------------------------------------
# 【第四阶段】收集全组开发源文件并创建生命主体 (构建 Target)
# ----------------------------------------------------------------------------
# 用一个变量 PROJECT_SOURCES 彻底归拢项目各模块。
# 组员以后若增加新模块（如报表统计、打印等），直接在对应注释下方添加源码路径即可。
set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui

        # 主框架导航模块
        src/module/homewidget.cpp
        src/module/homewidget.h
        src/module/homewidget.ui

        # ── Common 公共底层模块 ──
        # (包含 persistence 数据库层、DES加密、SHA哈希、实体类以及新引入的邮件发送器)
        src/common/destuil.cpp
        src/common/desutil.h
        src/common/hashsha.cpp
        src/common/hashsha.h
        src/common/user.cpp
        src/common/user.h
        src/common/goods.cpp
        src/common/goods.h
        src/common/billcalculator.cpp
        src/common/billcalculator.h
        src/common/emailsender.cpp       # 邮件逻辑实现文件
        src/common/emailsender.h         # 抛出 QSslSocket 的头文件
        # ── Login 登录验证模块 (贯彻 MVC 解耦，View 与 Controller 逻辑分离) ──

        src/module/login/login_controllor.cpp
        src/module/login/login_controllor.h
        src/module/login/loginwidget.cpp
        src/module/login/loginwidget.h
        src/module/login/loginwidget.ui
        src/module/login/registerwidget.cpp
        src/module/login/registerwidget.h
        src/module/login/registerwidget.ui

        # ── Address 地址管理模块 ──
        src/module/address/address_controllor.cpp
        src/module/address/address_controllor.h
        src/module/address/addresswidget.cpp
        src/module/address/addresswidget.h
        src/module/address/addresswidget.ui

        # ── Balance 余额与扣款模块 ──
        src/module/balance/balance_controllor.cpp
        src/module/balance/balance_controllor.h
        src/module/balance/balancewidget.cpp
        src/module/balance/balancewidget.h
        src/module/balance/balancewidget.ui

        # ── Cart_in 购物车模块 ──
        src/module/cart_in/cart_controllor.cpp
        src/module/cart_in/cart_controllor.h
        src/module/cart_in/cartwidget.cpp
        src/module/cart_in/cartwidget.h
        src/module/cart_in/cartwidget.ui
        src/common/cart.h
        src/common/cart.cpp

        # ── Deduct 扣减子业务模块 ──
        src/module/deduct/deduct_controllor.cpp
        src/module/deduct/deduct_controllor.h
        src/module/deduct/deductwidget.cpp
        src/module/deduct/deductwidget.h
        src/module/deduct/deductwidget.ui

        # ── Inventory 库存控制模块 ──
        src/module/inventory/inventory_controllor.cpp
        src/module/inventory/inventory_controllor.h
        src/module/inventory/inventorywidget.cpp
        src/module/inventory/inventorywidget.h
        src/module/inventory/inventorywidget.ui

        # ── Purchase 进货管理模块 ──
        src/module/purchase/purchase_controllor.cpp
        src/module/purchase/purchase_controllor.h
        src/module/purchase/purchasewidget.cpp
        src/module/purchase/purchasewidget.h

        # ── Report 进销存数据报表模块 ──
        src/module/report/report_controllor.cpp
        src/module/report/report_controllor.h
        src/module/report/reportwidget.cpp
        src/module/report/reportwidget.h
        src/module/report/reportwidget.ui
        src/common/thememanager.cpp
        src/common/thememanager.h
        src/common/database/database_accounts.cpp
        src/common/database/database_products.cpp
        src/common/database/database_sales_orders.cpp
        src/common/database/databasemanager.cpp
        src/common/database/database_addresses.cpp
        src/common/database/database_inventory.cpp
        src/common/database/database_purchase_orders.cpp
        src/common/database/database_user.cpp
        src/common/database/databasemanager.h
        src/common/database/databasemanager.cpp
)

# 真正下发施工图纸，命令编译器将上述所有源码揉成名为 "BillandSale_system" 的二进制可执行对象
qt_add_executable(BillandSale_system
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
)


# ----------------------------------------------------------------------------
# 【第五阶段】精细化喂养：配置头文件搜索路径 (编译期使用)
# ----------------------------------------------------------------------------
# PRIVATE 关键字确保路径不外泄。
# 目的：将全组错综复杂的子模块目录彻底平铺，让代码可以极其优雅地直接 #include "goods.h" 或 "cart.h"
target_include_directories(BillandSale_system PRIVATE
        src
        src/common
        src/common/database
        src/module
        src/module/login
        src/module/address
        src/module/balance
        src/module/cart_in
        src/module/deduct
        src/module/inventory
        src/module/purchase
        src/module/report
        ${CMAKE_CURRENT_BINARY_DIR} # 存放自动化 MOC/UIC 影子文件的老巢，绝对不能漏
)


# ----------------------------------------------------------------------------
# 【第六阶段】铁板装配：硬核链接外部库 (链接期使用)
# ----------------------------------------------------------------------------
# 🌟【核心修复点】：在此处装配上 Qt6::Network 动态链接库！
# 目的：把第四阶段揉出来的二进制 obj 砖块，与官方编译好的网络/加密二进制大文件进行最后的符号焊接，
# 彻底斩断链接期由于缺少邮件和安全套接字支持而引发的 "undefined reference to QSslSocket" 灾难。
target_link_libraries(BillandSale_system PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Network
)


# ----------------------------------------------------------------------------
# 【第七阶段】运行期后置护航 (Windows 专属：自动搬运 DLL 补丁)
# ----------------------------------------------------------------------------
# 当我们在 Windows 系统下双击运行独立程序时生效，用于自备运行期依赖。
if(WIN32 AND NOT CONAN_EXPORT_STAGE)
    # 从内存变量里反向抓取 qmake 的绝对路径并剥离出 bin 目录
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    # 精准定位到 Windows 部署神器 windeployqt.exe
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    # 核心后置命令：当 exe 生成完的瞬间，立刻后台自动把运行所缺的库（包括新增的 Qt6Network.dll）拷贝过来
    add_custom_command(TARGET BillandSale_system POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            \"$<TARGET_FILE:BillandSale_system>\"
            COMMENT "Windows 专属护航：正在自动搬运新引入的 Network 模块及常规运行期 DLL 补丁..."
    )
endif()


# ----------------------------------------------------------------------------
# 【第八阶段】成品外包装修饰与最终封顶
# ----------------------------------------------------------------------------
# 💡【Lead 架构调整说明】：
# 此处特意配置为 FALSE。这意味着在调试和编写代码阶段，双击或在 CLion 中运行程序时，
# 操作系统会强制附带弹出一个黑色的 cmd 终端控制台。
# 目的：方便全组人员通过终端实时查看代码中通过 qDebug() << "xxxxx"; 输出的断点日志、SQL报错或邮件状态。
# 注意：在项目结项需要完美交付给用户时，再将其改回 TRUE 即可隐藏黑窗口。
set_target_properties(BillandSale_system PROPERTIES
        WIN32_EXECUTABLE FALSE
)

# 对整个 Qt6 编译目标进行最后的静态优化、资源绑定以及最后的完美收尾
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BillandSale_system)
endif()