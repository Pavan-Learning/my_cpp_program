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

void demonstrate_drawback() {
    const LegacyThermometer boiling(212.0);
    const TemperatureAdapter correct(boiling);

    // Drawback: matching function names is not enough; the translated VALUE must
    // be correct too. This deliberately wrong conversion uses integer division:
    // 5 / 9 is zero in C++, even though the final variable is a double.
    const double incorrect = (boiling.read_fahrenheit() - 32.0) * (5 / 9);
    check(incorrect == 0.0, "Wrong adapter arithmetic silently gives zero");
    check(correct.celsius() == 100.0, "Floating-point conversion preserves the meaning");
    std::cout << "Drawback: a faulty adapter says " << incorrect
              << " C for boiling water; the correct conversion says " << correct.celsius() << " C.\n";
}

int main() {
    const LegacyThermometer freezing(32.0);
    const LegacyThermometer boiling(212.0);
    const TemperatureAdapter cold(freezing);
    const TemperatureAdapter hot(boiling);
    const TemperatureSensor& sensor = hot;
    check(cold.celsius() == 0.0 && sensor.celsius() == 100.0, "Adapter must convert units");
    std::cout << sensor.celsius() << " C\n";
    demonstrate_drawback();
}