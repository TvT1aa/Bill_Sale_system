cmake_minimum_required(VERSION 3.16)

# ===== 强制指定编译器路径 =====
set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe" CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe" CACHE PATH "" FORCE)
# ==============================

project(BillandSale_system VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui

        # Common 模块
        src/common/databasemanager.cpp
        src/common/databasemanager.h
        src/common/destuil.cpp
        src/common/desutil.h
        src/common/hashsha.cpp
        src/common/hashsha.h
        src/common/user.cpp
        src/common/user.h

        # Login 模块 (View & Controller)
        src/module/login/login_controllor.cpp
        src/module/login/login_controllor.h
        src/module/login/loginwidget.cpp
        src/module/login/loginwidget.h
        src/module/login/loginwidget.ui
        src/module/login/registerwidget.cpp
        src/module/login/registerwidget.h

        # Address 模块
        src/module/address/address_controllor.cpp
        src/module/address/address_controllor.h
        src/module/address/addresswidget.cpp
        src/module/address/addresswidget.h
        src/module/address/addresswidget.ui

        # Balance 模块
        src/module/balance/balance_controllor.cpp
        src/module/balance/balance_controllor.h
        src/module/balance/balancewidget.cpp
        src/module/balance/balancewidget.h
        src/module/balance/balancewidget.ui

        # Cart_in 模块（包含入库控制器和购物车页面）
        src/module/cart_in/cart_controllor.cpp
        src/module/cart_in/cart_controllor.h
        src/module/cart_in/cartwidget.cpp
        src/module/cart_in/cartwidget.h
        src/module/cart_in/cartwidget.ui

        # Deduct 模块
        src/module/deduct/deduct_controllor.cpp
        src/module/deduct/deduct_controllor.h
        src/module/deduct/deductwidget.cpp
        src/module/deduct/deductwidget.h
        src/module/deduct/deductwidget.ui

        # Inventory 模块
        src/module/inventory/inventory_controllor.cpp
        src/module/inventory/inventory_controllor.h
        src/module/inventory/inventorywidget.cpp
        src/module/inventory/inventorywidget.h
        src/module/inventory/inventorywidget.ui
        src/module/inventory/admin_validation.cpp
        src/module/inventory/admin_validation.h
        src/module/inventory/admin_validation.ui

        # Report 模块
        src/module/report/reportwidget.cpp
        src/module/report/reportwidget.h
        src/module/report/reportwidget.ui
        )

qt_add_executable(BillandSale_system
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

target_include_directories(BillandSale_system PRIVATE
        src
        src/common
        src/module/login
        ${CMAKE_CURRENT_BINARY_DIR}
        )

target_link_libraries(BillandSale_system PRIVATE
        Qt6::Widgets
        Qt6::Sql
        )

if(WIN32 AND NOT CONAN_EXPORT_STAGE)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET BillandSale_system POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            \"$<TARGET_FILE:BillandSale_system>\"
            COMMENT "Running windeployqt..."
            )
endif()

set_target_properties(BillandSale_system PROPERTIES
        WIN32_EXECUTABLE TRUE
        )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BillandSale_system)
endif()