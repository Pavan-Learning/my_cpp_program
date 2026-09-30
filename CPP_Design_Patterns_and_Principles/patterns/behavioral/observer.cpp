#include "support/check.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

class Sensor {
public:
    using Token = std::size_t;
    Token subscribe(std::function<void(int)> callback) {
        if (!callback) { throw std::invalid_argument("Empty callback"); }
        const auto token = ++last_token_;
        observers_.emplace(token, std::move(callback));
        return token;
    }
    void unsubscribe(Token token) { observers_.erase(token); }
    void publish(int value) {
        std::vector<Token> snapshot;
        for (const auto& entry : observers_) { snapshot.push_back(entry.first); }
        for (const auto token : snapshot) {
            const auto found = observers_.find(token);
            if (found != observers_.end()) {
                const auto callback = found->second;
                callback(value);
            }
        }
    }

private:
    Token last_token_ = 0;
    std::map<Token, std::function<void(int)>> observers_;
};

int main() {
    Sensor sensor;
    std::vector<int> readings;
    const auto display = sensor.subscribe([&readings](int value) { readings.push_back(value); });
    Sensor::Token once = 0;
    int once_calls = 0;
    once = sensor.subscribe([&](int) { ++once_calls; sensor.unsubscribe(once); });
    sensor.publish(20);
    sensor.publish(21);
    check(readings == std::vector<int>({20, 21}), "Observers receive published values");
    check(once_calls == 1, "Self-removal during notification is safe");
    sensor.unsubscribe(display);
    sensor.publish(22);
    check(readings.size() == 2, "Disconnected observer is not called");

    int late_calls = 0;
    bool added = false;
    sensor.subscribe([&](int) {
        if (!added) {
            sensor.subscribe([&](int) { ++late_calls; });
            added = true;
        }
    });
    sensor.publish(23);
    check(late_calls == 0, "New subscriptions wait until the next publication");
    sensor.publish(24);
    check(late_calls == 1, "New subscription receives later publications");
    std::cout << "Observer delivery and subscription changes verified\n";
}