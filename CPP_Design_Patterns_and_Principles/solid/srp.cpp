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

// Deliberate over-splitting: these helpers each wrap one tiny expression, even
// though number conversion and joining are parts of the SAME CSV-formatting job.
namespace over_split {
struct TotalReader {
    int read(const after::Invoice& invoice) const { return invoice.total(); }
};
struct NumberWriter {
    std::string write(int total) const { return std::to_string(total); }
};
struct CsvJoiner {
    std::string join(const std::string& total) const { return "total_cents\n" + total; }
};
}

void demonstrate_drawback() {
    const after::Invoice invoice({100, 250});
    const over_split::TotalReader reader;
    const over_split::NumberWriter writer;
    const over_split::CsvJoiner joiner;
    const auto scattered = joiner.join(writer.write(reader.read(invoice)));
    const auto focused = after::CsvInvoiceFormatter{}.format(invoice);
    check(scattered == focused, "Three tiny helpers produce no additional behavior");

    // Drawback of over-applying SRP: callers must find and connect three helpers
    // for one formatting task. SRP means separate reasons to change, not one
    // expression per class. The existing formatter is the simpler boundary here.
    std::cout << "Drawback: over-splitting needs three helpers for the same CSV "
                 "that one focused formatter produces.\n";
}

int main() {
    const before::Invoice coupled({100, 250});
    const after::Invoice invoice({100, 250});
    const after::CsvInvoiceFormatter formatter;
    check(invoice.total() == 350, "Calculation has its own responsibility");
    check(formatter.format(invoice) == coupled.csv(), "Refactor preserves output");
    check(after::Invoice({}).total() == 0, "Empty invoice");
    std::cout << formatter.format(invoice) << '\n';
    demonstrate_drawback();
}