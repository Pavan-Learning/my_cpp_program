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

std::string title_using_singleton() {
    return "Welcome to " + Settings::instance().application_name();
}

std::string title_using_supplied_name(const std::string& name) {
    return "Welcome to " + name;
}

void demonstrate_drawback() {
    // Drawback: this function hides its settings dependency. Two tests cannot
    // give it different names: it always reaches the one global Settings object.
    // We keep the singleton immutable rather than add unsafe mutation to show it.
    const auto first_test = title_using_singleton();
    const auto second_test = title_using_singleton();
    check(first_test == second_test && first_test == "Welcome to Pattern demo",
          "Both callers are tied to the same global configuration");

    // Supplying the needed value lets each caller choose its own configuration.
    check(title_using_supplied_name("Shop") == "Welcome to Shop", "First independent setting");
    check(title_using_supplied_name("Editor") == "Welcome to Editor", "Second independent setting");
    std::cout << "Drawback: global settings force the same title; "
                 "supplied settings allow Shop and Editor independently.\n";
}

int main() {
    const auto& first = Settings::instance();
    const auto& second = Settings::instance();
    check(&first == &second, "Both accesses must refer to the same instance");
    check(first.application_name() == "Pattern demo", "Settings are initialized");
    std::cout << first.application_name() << '\n';
    demonstrate_drawback();
}