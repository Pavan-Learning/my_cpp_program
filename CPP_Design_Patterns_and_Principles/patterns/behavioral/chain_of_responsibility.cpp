#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

struct Request {
    bool authenticated;
    bool within_quota;
};

class Handler {
public:
    virtual ~Handler() = default;
    Handler& then(std::unique_ptr<Handler> next) {
        if (!next) { throw std::invalid_argument("Missing handler"); }
        next_ = std::move(next);
        return *next_;
    }
    virtual std::string handle(const Request& request) const {
        return next_ ? next_->handle(request) : "accepted";
    }

private:
    std::unique_ptr<Handler> next_;
};

class Authentication final : public Handler {
public:
    std::string handle(const Request& request) const override {
        return request.authenticated ? Handler::handle(request) : "unauthorized";
    }
};

class Quota final : public Handler {
public:
    std::string handle(const Request& request) const override {
        return request.within_quota ? Handler::handle(request) : "quota exceeded";
    }
};

int main() {
    Authentication chain;
    chain.then(std::make_unique<Quota>());
    check(chain.handle({false, false}) == "unauthorized", "First failure short-circuits");
    check(chain.handle({true, false}) == "quota exceeded", "Second handler can reject");
    check(chain.handle({true, true}) == "accepted", "All handlers pass");
    std::cout << chain.handle({true, true}) << '\n';
}