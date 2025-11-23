#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Arduino.h>
#include "config.h"

// Simplified UserInterface for tractor - no OLED display
class UserInterface {
public:
    UserInterface();

    // Initialization and main update
    bool init();
    void update();

    // Stub methods for compatibility
    void showStartupMessage();
    void showCalibrationScreen() {}
    void showGearChange(DirectionMode dir);
    void showWarning(const char* message);
    void showFault(uint8_t faultCode);
    void clearWarning() {}

    // Data setters (stubs for compatibility)
    void setVoltage(float voltage) { batteryVoltage = voltage; }
    void setCurrent(float current) {}
    void setPower(float power) {}
    void setSpeed(float speed) {}
    void setGear(DirectionMode dir) { currentDirection = dir; }
    void setBatterySOC(float percent) { batterySOC = percent; }
    void setMotorData(float leftCurrent, float rightCurrent, float imbalance) {}
    void setGPSData(uint8_t satellites, bool fixed) {}
    void checkCalibrationSequence() {}

private:
    // Display data (for serial logging)
    float batteryVoltage;
    float batterySOC;
    DirectionMode currentDirection;

    // Utility methods
    const char* getFaultString(uint8_t faultCode);
};

#endif // USER_INTERFACE_H