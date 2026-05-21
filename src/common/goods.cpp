#include "goods.h"

Goods::Goods() {
    id_ = -1;
    stock_ = 0;
    price_ = 0.0;
}

Goods::Goods(int id, const std::string& name, int stock, double price) {
    id_ = id;
    name_ = name;
    if (stock >= 0)
        stock_ = stock;
    else
        stock_ = 0;

    if (price >= 0)
        price_ = price;
    else
        price_ = 0.0;
}

int Goods::getId() const {
    return id_;
}

std::string Goods::getName() const {
    return name_;
}

int Goods::getStock() const {
    return stock_;
}

double Goods::getPrice() const {
    return price_;
}

void Goods::setName(const std::string& name) {
    name_ = name;
}

void Goods::setPrice(double price) {
    if (price >= 0)
        price_ = price;
}

void Goods::setStock(int stock) {
    if (stock >= 0)
        stock_ = stock;
}

void Goods::addStock(int quantity) {
    if (quantity > 0) {
        stock_ += quantity;
    }
}

bool Goods::deductStock(int quantity) {
    if (quantity > 0 && stock_ >= quantity) {
        stock_ -= quantity;
        return true;
    }
    return false;
}

bool Goods::isStockSufficient(int required) const {
    if (required <= 0) return false;
    return stock_ >= required;
}

double Goods::calculateTotalPrice(int quantity) const {
    if (quantity <= 0) return 0.0;
    return price_ * quantity;
}