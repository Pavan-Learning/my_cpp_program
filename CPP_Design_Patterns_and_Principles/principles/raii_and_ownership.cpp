#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>

class Connection {
public:
    Connection() { ++active_; }
    ~Connection() { --active_; }
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    static int active() { return active_; }

private:
    inline static int active_ = 0;
};

void failing_operation() {
    auto connection = std::make_unique<Connection>();
    check(Connection::active() == 1, "Resource acquired");
    throw std::runtime_error("Operation failed");
}

int main() {
    check(Connection::active() == 0, "No initial resources");
    {
        auto owner = std::make_unique<Connection>();
        auto new_owner = std::move(owner);
        check(!owner && new_owner && Connection::active() == 1, "Move transfers sole ownership");
    }
    check(Connection::active() == 0, "Normal scope exit releases resource");
    bool failed = false;
    try { failing_operation(); }
    catch (const std::runtime_error&) { failed = true; }
    check(failed && Connection::active() == 0, "Exception unwinding also releases resource");
    std::cout << "Resource released on normal and exceptional exits\n";
}