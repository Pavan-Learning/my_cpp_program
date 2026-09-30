#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

class Request {
public:
    class Builder;
    const std::string& url() const { return url_; }
    int timeout() const { return timeout_ms_; }
    bool authenticated() const { return authenticated_; }

private:
    Request(std::string url, int timeout_ms, bool authenticated)
        : url_(std::move(url)), timeout_ms_(timeout_ms), authenticated_(authenticated) {}
    std::string url_;
    int timeout_ms_;
    bool authenticated_;
};

class Request::Builder {
public:
    Builder& url(std::string value) { url_ = std::move(value); return *this; }
    Builder& timeout(int value) { timeout_ms_ = value; return *this; }
    Builder& authenticate() { authenticated_ = true; return *this; }

    Request build() const {
        if (url_.empty() || timeout_ms_ <= 0) {
            throw std::invalid_argument("URL and positive timeout required");
        }
        return Request(url_, timeout_ms_, authenticated_);
    }

private:
    std::string url_;
    int timeout_ms_ = 1000;
    bool authenticated_ = false;
};

class RequestDirector {
public:
    Request health_check(std::string url) const {
        return Request::Builder{}.url(std::move(url)).timeout(200).build();
    }
};

int main() {
    const auto request = Request::Builder{}.url("/orders").timeout(500).authenticate().build();
    check(request.url() == "/orders" && request.timeout() == 500 && request.authenticated(),
          "Builder must preserve selected options");
    const auto health = RequestDirector{}.health_check("/health");
    check(health.timeout() == 200 && !health.authenticated(), "Director applies a recipe");
    bool rejected = false;
    try { Request::Builder{}.build(); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Incomplete request must be rejected");
    rejected = false;
    try { Request::Builder{}.url("/orders").timeout(0).build(); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "Invalid timeout must be rejected");
    std::cout << request.url() << " timeout=" << request.timeout() << '\n';
}