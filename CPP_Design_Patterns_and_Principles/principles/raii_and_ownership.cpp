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

struct ConnectionNode {
    Connection resource;
    std::shared_ptr<ConnectionNode> next;
    std::weak_ptr<ConnectionNode> previous;
};

void demonstrate_drawback() {
    check(Connection::active() == 0, "Cycle example starts with no live resources");
    std::weak_ptr<ConnectionNode> observed;
    {
        auto first = std::make_shared<ConnectionNode>();
        auto second = std::make_shared<ConnectionNode>();
        first->next = second;
        second->next = first;
        observed = first;
    }
    check(Connection::active() == 2 && !observed.expired(), "Strong ownership cycle survives external owners leaving");

    // Drawback: each node keeps the other alive, so neither reference count reaches
    // zero. RAII cannot release these resources until the ownership cycle is broken.
    // Break it deliberately so this teaching program does NOT leave a leak.
    auto rescued = observed.lock();
    check(static_cast<bool>(rescued), "Cycle keeps the observed node alive");
    rescued->next.reset();
    rescued.reset();
    check(Connection::active() == 0 && observed.expired(), "Breaking the cycle releases both resources");

    {
        auto first = std::make_shared<ConnectionNode>();
        auto second = std::make_shared<ConnectionNode>();
        first->next = second;
        second->previous = first;
        // The backward link observes instead of owning, so no strong cycle exists.
    }
    check(Connection::active() == 0, "Weak backward link permits automatic cleanup");
    std::cout << "Drawback: strong ownership cycle retained two connections; "
                 "breaking the cycle or using a weak backward link releases them.\n";
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
    demonstrate_drawback();
}