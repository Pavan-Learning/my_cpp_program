#include "support/check.hpp"

#include <iostream>
#include <memory>
#include <string>

class Transport {
public:
    virtual ~Transport() = default;
    virtual std::string deliver() const = 0;
};

class Truck final : public Transport {
public:
    std::string deliver() const override { return "road"; }
};

class Ship final : public Transport {
public:
    std::string deliver() const override { return "sea"; }
};

class Logistics {
public:
    virtual ~Logistics() = default;

    std::string fulfill() const {
        const auto transport = create_transport();
        return "Deliver by " + transport->deliver();
    }

protected:
    virtual std::unique_ptr<Transport> create_transport() const = 0;
};

class RoadLogistics final : public Logistics {
protected:
    std::unique_ptr<Transport> create_transport() const override {
        return std::make_unique<Truck>();
    }
};

class SeaLogistics final : public Logistics {
protected:
    std::unique_ptr<Transport> create_transport() const override {
        return std::make_unique<Ship>();
    }
};

int main() {
    const RoadLogistics road;
    const SeaLogistics sea;
    check(road.fulfill() == "Deliver by road", "Road creator must select Truck");
    check(sea.fulfill() == "Deliver by sea", "Sea creator must select Ship");
    std::cout << road.fulfill() << '\n' << sea.fulfill() << '\n';
}