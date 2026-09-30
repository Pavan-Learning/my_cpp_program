#include "support/check.hpp"

#include <iostream>
#include <map>
#include <string>
#include <utility>

namespace before {
class SqlStock {
public:
    int available(const std::string& sku) const { return sku == "book" ? 3 : 0; }
};

class Warehouse {
public:
    bool can_ship(const std::string& sku) const { return database_.available(sku) > 0; }

private:
    SqlStock database_;
};
}

namespace after {
struct StockReader {
    virtual ~StockReader() = default;
    virtual int available(const std::string& sku) const = 0;
};

class Warehouse {
public:
    explicit Warehouse(const StockReader& stock) : stock_(stock) {}
    bool can_ship(const std::string& sku) const { return stock_.available(sku) > 0; }

private:
    const StockReader& stock_;
};

class MemoryStock final : public StockReader {
public:
    explicit MemoryStock(std::map<std::string, int> items) : items_(std::move(items)) {}
    int available(const std::string& sku) const override {
        const auto found = items_.find(sku);
        return found == items_.end() ? 0 : found->second;
    }

private:
    std::map<std::string, int> items_;
};
}

int main() {
    check(before::Warehouse{}.can_ship("book"), "Hardwired demo dependency");
    const after::MemoryStock available({{"book", 3}});
    const after::MemoryStock empty({});
    const after::Warehouse ready(available);
    const after::Warehouse unavailable(empty);
    check(ready.can_ship("book"), "Policy works with supplied stock reader");
    check(!unavailable.can_ship("book") && !ready.can_ship("missing"), "Failures test without SQL");
    std::cout << "Warehouse policy tested through an abstraction\n";
}