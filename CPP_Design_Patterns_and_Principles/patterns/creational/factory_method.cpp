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

// Drawback: one new delivery choice needs TWO new classes in this design.
// Plane is the vehicle; AirLogistics only explains how to create that vehicle.
// This is extra code to maintain when the creation step is very small.
class Plane final : public Transport {
public:
    std::string deliver() const override { return "air"; }
};

class AirLogistics final : public Logistics {
protected:
    std::unique_ptr<Transport> create_transport() const override {
        return std::make_unique<Plane>();
    }
};

void demonstrate_drawback() {
    // The pattern does not choose the delivery mode for us. Setup still chooses
    // AirLogistics here; only the common fulfill() steps remain unchanged.
    const AirLogistics air;
    check(air.fulfill() == "Deliver by air", "New choice reuses the shared steps");

    // With no shared workflow to reuse, a direct value can do the smaller job.
    // This is an alternative for a simpler requirement, not Factory Method.
    const Plane direct_vehicle;
    check(direct_vehicle.deliver() == "air", "Simple case needs no creator class");
    std::cout << "Drawback: air delivery adds Plane AND AirLogistics; "
                 "a direct Plane is enough when we only need deliver().\n";
}

int main() {
    const RoadLogistics road;
    const SeaLogistics sea;
    check(road.fulfill() == "Deliver by road", "Road creator must select Truck");
    check(sea.fulfill() == "Deliver by sea", "Sea creator must select Ship");
    std::cout << road.fulfill() << '\n' << sea.fulfill() << '\n';
    demonstrate_drawback();
}