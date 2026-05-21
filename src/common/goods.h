#ifndef GOODS_H
#define GOODS_H

#include <string>

class Goods {
public:
    // 构造函数
    Goods();
    Goods(int id, const std::string& name, int stock, double price);

    // 获取属性
    int getId() const;
    std::string getName() const;
    int getStock() const;
    double getPrice() const;

    // 设置属性
    void setName(const std::string& name);
    void setPrice(double price);
    void setStock(int stock);

    // 业务操作
    void addStock(int quantity);       // 入库
    bool deductStock(int quantity);    // 出库，返回是否成功
    bool isStockSufficient(int required) const;
    double calculateTotalPrice(int quantity) const;

private:
    int id_;
    std::string name_;
    int stock_;
    double price_;
};

#endif // GOODS_H