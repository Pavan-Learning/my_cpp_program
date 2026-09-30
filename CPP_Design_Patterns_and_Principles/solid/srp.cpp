#include "support/check.hpp"

#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace before {
class Invoice {
public:
    explicit Invoice(std::vector<int> cents) : cents_(std::move(cents)) {}
    int total() const { return std::accumulate(cents_.begin(), cents_.end(), 0); }
    std::string csv() const { return "total_cents\n" + std::to_string(total()); }

private:
    std::vector<int> cents_;
};
}

namespace after {
class Invoice {
public:
    explicit Invoice(std::vector<int> cents) : cents_(std::move(cents)) {}
    int total() const { return std::accumulate(cents_.begin(), cents_.end(), 0); }

private:
    std::vector<int> cents_;
};

class CsvInvoiceFormatter {
public:
    std::string format(const Invoice& invoice) const {
        return "total_cents\n" + std::to_string(invoice.total());
    }
};
}

int main() {
    const before::Invoice coupled({100, 250});
    const after::Invoice invoice({100, 250});
    const after::CsvInvoiceFormatter formatter;
    check(invoice.total() == 350, "Calculation has its own responsibility");
    check(formatter.format(invoice) == coupled.csv(), "Refactor preserves output");
    check(after::Invoice({}).total() == 0, "Empty invoice");
    std::cout << formatter.format(invoice) << '\n';
}