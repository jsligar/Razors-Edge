#include "UserInterface.h"

UserInterface::UserInterface() :
    batteryVoltage(0.0f),
    batterySOC(0.0f),
    currentDirection(DIR_NEUTRAL)
{
}

bool UserInterface::init() {
    Serial.println("Initializing User Interface (Tractor Version - No OLED)...");
    Serial.println("✓ User Interface initialized (Serial output only)");
    return true;
}

void UserInterface::update() {
    // No display to update in tractor version
    // All output goes to Serial
}

void UserInterface::showStartupMessage() {
    Serial.println("=================================");
    Serial.println("    TRACTOR CONTROLLER v1.0     ");
    Serial.println("    ESP32 Tractor Control        ");
    Serial.println("=================================");
}

void UserInterface::showGearChange(DirectionMode dir) {
    Serial.printf("Direction changed to: %s\n", DIR_CONFIGS[dir].name);
}

void UserInterface::showWarning(const char* message) {
    Serial.printf("⚠ WARNING: %s\n", message);
}

void UserInterface::showFault(uint8_t faultCode) {
    Serial.printf("⚠ FAULT: %s\n", getFaultString(faultCode));
}

const char* UserInterface::getFaultString(uint8_t faultCode) {
    switch (faultCode) {
        case NO_FAULT:          return "No Fault";
        case LOW_BATTERY:       return "Low Battery";
        case KEY_SWITCH_OFF:    return "Key Switch Off";
        case WATCHDOG_TIMEOUT:  return "Watchdog Timeout";
        default:                return "Unknown Fault";
    }
}
