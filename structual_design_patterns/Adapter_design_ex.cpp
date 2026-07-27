// Online C++ compiler to run C++ program online
#include <iostream>
#include <stdexcept>
#include <limits>
using namespace std;

class LegacyTemperatureSensor
{
public:
    double ReadFahrenheit() const
    {
        if(rand() % 2 == 0)
            throw std::runtime_error("Sensor disconnected");
        return 77.0;
    }
};

class AdvancedTemperatureSensor
{
public:
    double GetTemperatureInKelvin() const
    {
        return 298.15;
    }
};

class ITemperatureSensor
{
public:
    virtual double ReadCelsius() const = 0;

    virtual ~ITemperatureSensor() = default;
};

class TemperatureSensorAdapter : public ITemperatureSensor
{
    private: 
        LegacyTemperatureSensor LTS;
    public:
    TemperatureSensorAdapter(const LegacyTemperatureSensor& Temp) : LTS(Temp){}
    
    double ReadCelsius() const
    {
        try{
            auto celsius = LTS.ReadFahrenheit();
            return ((celsius - 32) * 5.0) / 9.0;
        }
        catch(const std::runtime_error& e)
        {
            // double nan_value = std::numeric_limits<double>::quiet_NaN();
            std::cerr << "Error: " << e.what() << '\n';
            return -1; // Return an error value or handle it as needed
        }
    }
};

class AdvancedTemperatureAdapter  : public ITemperatureSensor
{
    private:
        const AdvancedTemperatureSensor& sensor;
    public:
        AdvancedTemperatureAdapter (const AdvancedTemperatureSensor& sensor) : sensor(sensor){}
        double ReadCelsius() const
    {
        auto kelvin = sensor.GetTemperatureInKelvin();
        return kelvin-273.15;
    }
        
};
void DisplayTemperature(const ITemperatureSensor& sensor)
{
    std::cout << "Temperature : "
              << sensor.ReadCelsius()
              << " C\n";
}

int main() {
    LegacyTemperatureSensor sensor;
    TemperatureSensorAdapter oldadapter(sensor);
    DisplayTemperature(oldadapter);

    AdvancedTemperatureSensor modern;
    AdvancedTemperatureAdapter adapter(modern);
    DisplayTemperature(adapter);
    return 0;
}