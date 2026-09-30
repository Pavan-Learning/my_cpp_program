#include "support/check.hpp"

#include <iostream>
#include <string>

class Settings final {
public:
    static const Settings& instance() {
        static const Settings settings;
        return settings;
    }

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    const std::string& application_name() const { return name_; }

private:
    Settings() = default;
    std::string name_ = "Pattern demo";
};

int main() {
    const auto& first = Settings::instance();
    const auto& second = Settings::instance();
    check(&first == &second, "Both accesses must refer to the same instance");
    check(first.application_name() == "Pattern demo", "Settings are initialized");
    std::cout << first.application_name() << '\n';
}