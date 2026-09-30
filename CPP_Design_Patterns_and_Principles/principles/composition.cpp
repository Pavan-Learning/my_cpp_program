#include "support/check.hpp"

#include <iostream>
#include <string>

class Motor {
public:
    std::string move() const { return "moving"; }
};

class Camera {
public:
    std::string capture() const { return "photo"; }
};

class InspectionRobot {
public:
    std::string inspect() const { return motor_.move() + ": " + camera_.capture(); }

private:
    Motor motor_;
    Camera camera_;
};

int main() {
    const InspectionRobot robot;
    check(robot.inspect() == "moving: photo", "Robot combines capabilities without inheriting devices");
    std::cout << robot.inspect() << '\n';
}