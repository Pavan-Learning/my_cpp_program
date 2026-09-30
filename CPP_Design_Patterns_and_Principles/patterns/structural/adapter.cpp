#include "support/check.hpp"

#include <iostream>

class LegacyThermometer {
public:
    explicit LegacyThermometer(double fahrenheit) : fahrenheit_(fahrenheit) {}
    double read_fahrenheit() const { return fahrenheit_; }

private:
    double fahrenheit_;
};

struct TemperatureSensor {
    virtual ~TemperatureSensor() = default;
    virtual double celsius() const = 0;
};

class TemperatureAdapter final : public TemperatureSensor {
public:
    explicit TemperatureAdapter(const LegacyThermometer& legacy) : legacy_(legacy) {}
    double celsius() const override { return (legacy_.read_fahrenheit() - 32.0) * 5.0 / 9.0; }

private:
    const LegacyThermometer& legacy_;
};

int main() {
    const LegacyThermometer freezing(32.0);
    const LegacyThermometer boiling(212.0);
    const TemperatureAdapter cold(freezing);
    const TemperatureAdapter hot(boiling);
    const TemperatureSensor& sensor = hot;
    check(cold.celsius() == 0.0 && sensor.celsius() == 100.0, "Adapter must convert units");
    std::cout << sensor.celsius() << " C\n";
}