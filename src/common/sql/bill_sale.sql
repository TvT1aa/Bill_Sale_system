-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 用户ID，主键，自增
    username VARCHAR(50) NOT NULL UNIQUE,   -- 用户名，唯一
    email VARCHAR(100) NOT NULL UNIQUE,     -- 邮箱，唯一
    phone VARCHAR(20),                      -- 手机号
    password_hash VARCHAR(255) NOT NULL,    -- 密码哈希值
    role INTEGER NOT NULL DEFAULT 0,        -- 角色（0=普通用户，1=管理员）
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 注册时间
    is_active INTEGER DEFAULT 1             -- 账号状态（1=启用，0=禁用）
);

CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);  -- 用户名索引
CREATE INDEX IF NOT EXISTS idx_users_role ON users(role);          -- 角色索引

-- 买家收货地址表
CREATE TABLE IF NOT EXISTS buyer_addresses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 地址ID，主键，自增
    user_id INTEGER NOT NULL,               -- 用户ID，外键关联users表
    name VARCHAR(100) NOT NULL,             -- 收货人姓名
    phone VARCHAR(20),                      -- 联系电话
    province VARCHAR(50),                   -- 省份
    city VARCHAR(50),                       -- 城市
    district VARCHAR(50),                   -- 区县
    detail VARCHAR(255),                    -- 详细地址
    is_default INTEGER DEFAULT 0,           -- 是否默认地址（1=默认，0=非默认）
    is_active INTEGER DEFAULT 1,            -- 是否启用（1=启用，0=禁用）
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 创建时间
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 更新时间
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE INDEX IF NOT EXISTS idx_buyer_addresses_user ON buyer_addresses(user_id);  -- 用户ID索引

-- 商品表
CREATE TABLE IF NOT EXISTS products (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 商品ID，主键，自增
    name VARCHAR(100) NOT NULL,             -- 商品名称
    category VARCHAR(50),                   -- 商品分类（如"水果"、"饮料"）
    purchase_price DECIMAL(10,2) DEFAULT 0, -- 进货单价
    sale_price DECIMAL(10,2) DEFAULT 0,     -- 销售单价
    unit VARCHAR(20),                       -- 计量单位（个、箱、斤等）
    remark VARCHAR(255),                    -- 备注
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 创建时间
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP   -- 更新时间
);

-- 库存表（仓库地址固定不变，直接存在库存表中）
CREATE TABLE IF NOT EXISTS inventory (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 库存ID，主键，自增
    product_id INTEGER NOT NULL,            -- 商品ID，外键关联products表
    quantity INTEGER DEFAULT 0,             -- 库存数量
    warehouse_address VARCHAR(255),         -- 仓库地址（固定不变）
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 最后更新时间
    FOREIGN KEY (product_id) REFERENCES products(id)
);

CREATE INDEX IF NOT EXISTS idx_inventory_product ON inventory(product_id);  -- 商品ID索引

-- 采购订单主表
CREATE TABLE IF NOT EXISTS purchase_orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 采购订单ID，主键，自增
    total_amount DECIMAL(10,2) DEFAULT 0,   -- 订单总金额
    remark VARCHAR(255),                    -- 备注
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP  -- 下单时间
);

-- 采购订单明细表
CREATE TABLE IF NOT EXISTS purchase_order_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 明细ID，主键，自增
    order_id INTEGER NOT NULL,              -- 采购订单ID，外键关联purchase_orders表
    product_id INTEGER NOT NULL,            -- 商品ID，外键关联products表
    quantity INTEGER NOT NULL,              -- 采购数量
    unit_price DECIMAL(10,2) NOT NULL,      -- 采购单价
    subtotal DECIMAL(10,2) NOT NULL,        -- 小计金额（数量×单价）
    FOREIGN KEY (order_id) REFERENCES purchase_orders(id),
    FOREIGN KEY (product_id) REFERENCES products(id)
);

CREATE INDEX IF NOT EXISTS idx_purchase_order_items_order ON purchase_order_items(order_id);  -- 订单ID索引

-- 销售订单主表
CREATE TABLE IF NOT EXISTS sales_orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 销售订单ID，主键，自增
    user_id INTEGER NOT NULL,               -- 下单用户ID，外键关联users表
    address VARCHAR(255),                   -- 收货地址
    total_amount DECIMAL(10,2) DEFAULT 0,   -- 订单总金额
    remark VARCHAR(255),                    -- 备注
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 下单时间
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE INDEX IF NOT EXISTS idx_sales_orders_user ON sales_orders(user_id);  -- 用户ID索引

-- 销售订单明细表
CREATE TABLE IF NOT EXISTS sales_order_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 明细ID，主键，自增
    order_id INTEGER NOT NULL,              -- 销售订单ID，外键关联sales_orders表
    product_id INTEGER NOT NULL,            -- 商品ID，外键关联products表
    quantity INTEGER NOT NULL,              -- 销售数量
    unit_price DECIMAL(10,2) NOT NULL,      -- 销售单价
    subtotal DECIMAL(10,2) NOT NULL,        -- 小计金额（数量×单价）
    FOREIGN KEY (order_id) REFERENCES sales_orders(id),
    FOREIGN KEY (product_id) REFERENCES products(id)
);

CREATE INDEX IF NOT EXISTS idx_sales_order_items_order ON sales_order_items(order_id);  -- 订单ID索引

-- 账户表（只有一个账户）
CREATE TABLE IF NOT EXISTS accounts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 账户ID，主键，自增
    name VARCHAR(50) NOT NULL,              -- 账户名称（如"主账户"）
    balance DECIMAL(12,2) DEFAULT 0,        -- 账户余额
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP  -- 最后更新时间
);

-- 收支流水表（type: 0=支出，1=收入）
CREATE TABLE IF NOT EXISTS transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- 流水ID，主键，自增
    type INTEGER NOT NULL,                  -- 类型（0=支出，1=收入）
    amount DECIMAL(10,2) NOT NULL,          -- 交易金额
    remark VARCHAR(255),                    -- 备注（如"采购苹果"、"销售订单#123"）
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP  -- 交易时间
);

CREATE INDEX IF NOT EXISTS idx_transactions_type ON transactions(type);  -- 类型索引