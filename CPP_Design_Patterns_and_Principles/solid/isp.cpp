#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace before {
struct Machine {
    virtual ~Machine() = default;
    virtual std::string print() const = 0;
    virtual std::string scan() const = 0;
};

struct BasicPrinter final : Machine {
    std::string print() const override { return "printed"; }
    std::string scan() const override { throw std::logic_error("No scanner hardware"); }
};
}

namespace after {
struct Printer {
    virtual ~Printer() = default;
    virtual std::string print() const = 0;
};

struct Scanner {
    virtual ~Scanner() = default;
    virtual std::string scan() const = 0;
};

struct BasicPrinter final : Printer {
    std::string print() const override { return "printed"; }
};

struct OfficeMachine final : Printer, Scanner {
    std::string print() const override { return "printed"; }
    std::string scan() const override { return "scanned"; }
};

std::string print_job(const Printer& printer) { return printer.print(); }
}

int main() {
    bool unsupported = false;
    try { before::BasicPrinter{}.scan(); }
    catch (const std::logic_error&) { unsupported = true; }
    check(unsupported, "Fat interface forces an unsupported operation");
    const after::BasicPrinter basic;
    const after::OfficeMachine office;
    check(after::print_job(basic) == "printed", "Print-only client uses narrow interface");
    check(after::print_job(office) == "printed" && office.scan() == "scanned", "Capabilities can be combined");
    std::cout << "Print and scan capabilities separated\n";
}