# Bill_Sale_system

---
项目：需要完成的模块有七个，分别是**login(登录)**,**inventory(库存管理)**,**cart_in(购物车)**,**deduct(扣减，出库)**,**address(地址)**,**balance(余额)**,**report(报表)**

---

项目结构：
BillandSale_system/  
├── common/                  # 【工具层】全项目通用的“基石”  
│   ├── databasemanager.h/cpp # 数据库连接与配置  
│   ├── desutil.h/cpp        # DES 加密工具  
│   ├── hashsha.h/cpp        # SHA-256 哈希工具  
│   └── user.h/cpp           # 用户基础数据模型  
├── module/                  # 【业务层】按功能拆分的独立模块  
│   ├── address/             # 地址管理  
│   ├── balance/             # 账务余额  
│   ├── cart_in/             # 购物车逻辑  
│   ├── deduct/              # 销售/库存扣减  
│   ├── inventory/           # 库存盘点与查询  
│   ├── login/               # 登录/注册模块（当前核心）  
│   │   ├── login_controllor.h/cpp # 业务逻辑控制  
│   │   ├── loginwidget.h/cpp/ui   # 登录界面  
│   │   └── registerwidget.h/cpp/ui# 注册界面  
│   └── report/              # 数据报表统计  
├── main.cpp                 # 程序入口，初始化主控逻辑  
├── mainwindow.h/cpp/ui      # 主框架窗口  
└── CMakeLists.txt           # 项目构建的"总指挥部"