#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

class Report {
public:
    virtual ~Report() = default;
    std::string generate(int total) const {
        if (total < 0) { throw std::invalid_argument("Negative total"); }
        const auto heading = header();
        const auto content = body(total);
        const auto ending = footer();
        return heading + content + ending;
    }

protected:
    virtual std::string header() const = 0;
    virtual std::string body(int total) const = 0;
    virtual std::string footer() const { return "\n"; }
};

class CsvReport final : public Report {
protected:
    std::string header() const override { return "total\n"; }
    std::string body(int total) const override { return std::to_string(total); }
};

class HtmlReport final : public Report {
protected:
    std::string header() const override { return "<p>"; }
    std::string body(int total) const override { return std::to_string(total); }
    std::string footer() const override { return "</p>\n"; }
};

class TracedReport final : public Report {
public:
    const std::string& trace() const { return trace_; }

protected:
    std::string header() const override { trace_ += 'H'; return {}; }
    std::string body(int) const override { trace_ += 'B'; return {}; }
    std::string footer() const override { trace_ += 'F'; return {}; }

private:
    mutable std::string trace_;
};

int main() {
    const CsvReport csv;
    const HtmlReport html;
    const TracedReport traced;
    traced.generate(42);
    check(traced.trace() == "HBF", "Hooks execute in the documented order");
    check(csv.generate(42) == "total\n42\n", "CSV follows the skeleton");
    check(html.generate(42) == "<p>42</p>\n", "HTML overrides the optional hook");
    bool rejected = false;
    try { html.generate(-1); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Common validation cannot be skipped through the base algorithm");
    try { traced.generate(-1); }
    catch (const std::invalid_argument&) {}
    check(traced.trace() == "HBF", "Invalid input executes no hooks");
    std::cout << html.generate(42);
}