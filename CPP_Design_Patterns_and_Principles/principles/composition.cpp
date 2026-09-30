#include "support/check.hpp"

#include <iostream>
#include <stdexcept>
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

class FailingCamera {
public:
    std::string capture() const { throw std::runtime_error("Camera unavailable"); }
};

void demonstrate_drawback() {
    const Motor motor;
    const FailingCamera camera;
    std::string completed_step;
    bool failed = false;
    try {
        completed_step = motor.move();
        static_cast<void>(camera.capture());
    } catch (const std::runtime_error&) { failed = true; }
    check(failed && completed_step == "moving", "The first helper finished before the second failed");

    // Drawback: replaceable helpers need agreed failure rules, not just matching
    // method names. A composed workflow must decide what to do if a later helper
    // fails. This records a step as TEXT only; no physical motor is being moved.
    // For real hardware, retrying the whole workflow might repeat an earlier move.
    std::cout << "Drawback: combining helpers does not undo completed steps; "
                 "the movement result remains after camera failure.\n";
}

int main() {
    const InspectionRobot robot;
    check(robot.inspect() == "moving: photo", "Robot combines capabilities without inheriting devices");
    std::cout << robot.inspect() << '\n';
    demonstrate_drawback();
}