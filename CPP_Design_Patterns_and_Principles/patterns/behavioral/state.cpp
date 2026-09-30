#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <string>

struct SignalState {
    virtual ~SignalState() = default;
    virtual std::string name() const = 0;
    virtual bool can_go() const = 0;
    virtual std::unique_ptr<SignalState> next() const = 0;
};

struct Red final : SignalState {
    std::string name() const override { return "red"; }
    bool can_go() const override { return false; }
    std::unique_ptr<SignalState> next() const override;
};

struct Green final : SignalState {
    std::string name() const override { return "green"; }
    bool can_go() const override { return true; }
    std::unique_ptr<SignalState> next() const override;
};

struct Amber final : SignalState {
    std::string name() const override { return "amber"; }
    bool can_go() const override { return false; }
    std::unique_ptr<SignalState> next() const override;
};

std::unique_ptr<SignalState> Red::next() const { return std::make_unique<Green>(); }
std::unique_ptr<SignalState> Green::next() const { return std::make_unique<Amber>(); }
std::unique_ptr<SignalState> Amber::next() const { return std::make_unique<Red>(); }

class TrafficSignal {
public:
    std::string name() const { return state_->name(); }
    bool can_go() const { return state_->can_go(); }
    void advance() {
        auto next = state_->next();
        state_ = std::move(next);
    }

private:
    std::unique_ptr<SignalState> state_ = std::make_unique<Red>();
};

int main() {
    TrafficSignal signal;
    check(signal.name() == "red" && !signal.can_go(), "Initial state");
    signal.advance();
    check(signal.name() == "green" && signal.can_go(), "Behavior changes with state");
    signal.advance();
    check(signal.name() == "amber" && !signal.can_go(), "Intermediate state");
    signal.advance();
    check(signal.name() == "red", "Full transition cycle");
    std::cout << "red -> green -> amber -> red\n";
}