#include "PowerManager.h"
#include <Wire.h>

PowerManager::PowerManager() : 
    batteryMonitorReady(false),
    motorLeftMonitorReady(false),
    motorRightMonitorReady(false),
    lastUpdateTime(0),
    energyConsumed_Wh(0.0f)
{
}

bool PowerManager::init() {
    Serial.println("Initializing Power Manager...");
    
    // Initialize all INA228 sensors (including MATEKSYS INA-BM)
    bool allSensorsOK = true;
    
    allSensorsOK &= initSensor(batteryMonitor, INA228_BATTERY_ADDR, "MATEKSYS INA-BM");
    allSensorsOK &= initSensor(motorLeftMonitor, INA228_MOTOR_LEFT_ADDR, "Motor Left");
    allSensorsOK &= initSensor(motorRightMonitor, INA228_MOTOR_RIGHT_ADDR, "Motor Right");
    
    if (!allSensorsOK) {
        Serial.println("⚠ Warning: Some power monitors failed to initialize");
        return false;
    }
    
    // Motor current measurement is configured in INA228 initialization
    
    lastUpdateTime = millis();
    
    Serial.println("✓ Power Manager initialized successfully");
    return true;
}

bool PowerManager::initSensor(Adafruit_INA228& sensor, uint8_t address, const char* name) {
    Serial.printf("  Initializing %s INA228 @ 0x%02X... ", name, address);
    
    if (!sensor.begin(address)) {
        Serial.println("FAILED");
        return false;
    }
    
    // Configure ADC averaging and conversion times for accuracy
    // Note: Using default settings for the INA228 library
    // sensor.setADCRange(...); // Method may not be available in this library version
    // sensor.setMode(...);     // Method may not be available in this library version
    
    Serial.println("OK");
    
    // Mark appropriate sensor as ready
    if (address == INA228_BATTERY_ADDR) batteryMonitorReady = true;
    else if (address == INA228_MOTOR_LEFT_ADDR) motorLeftMonitorReady = true;
    else if (address == INA228_MOTOR_RIGHT_ADDR) motorRightMonitorReady = true;
    
    return true;
}

void PowerManager::update() {
    unsigned long currentTime = millis();
    
    // Update energy tracking
    updateEnergyTracking();
    
    lastUpdateTime = currentTime;
}

float PowerManager::getBatteryVoltage() {
    if (!batteryMonitorReady) return 0.0f;
    
    float voltage = batteryMonitor.readBusVoltage();
    
    // Sanity check - should be 54-63V for 15S Li-ion
    if (voltage < 40.0f || voltage > 85.0f) {
        Serial.printf("⚠ Battery voltage out of range: %.2fV\n", voltage);
        return 0.0f;
    }
    
    return voltage;
}

float PowerManager::getBatteryCurrent() {
    if (!batteryMonitorReady) return 0.0f;
    
    float current = batteryMonitor.readCurrent();
    
    // Sanity check - MATEKSYS INA-BM supports up to 204.8A
    if (abs(current) > 100.0f) {
        Serial.printf("⚠ Battery current out of range: %.2fA\n", current);
        return 0.0f;
    }
    
    return current;
}

float PowerManager::getBatteryPower() {
    return getBatteryVoltage() * getBatteryCurrent();
}

float PowerManager::getBatterySOC() {
    float voltage = getBatteryVoltage();
    
    // Li-ion voltage curve approximation for 15S pack
    // 63.0V = 100%, 60.0V = 50%, 54.0V = 0%
    if (voltage >= 63.0f) return 100.0f;
    if (voltage <= 54.0f) return 0.0f;
    
    // Linear approximation between key points
    if (voltage >= 60.0f) {
        // 60-63V range (50-100%)
        return 50.0f + ((voltage - 60.0f) / 3.0f) * 50.0f;
    } else {
        // 54-60V range (0-50%)
        return ((voltage - 54.0f) / 6.0f) * 50.0f;
    }
}

float PowerManager::getMotorLeftVoltage() {
    if (!motorLeftMonitorReady) return 0.0f;
    return motorLeftMonitor.readBusVoltage();
}

float PowerManager::getMotorLeftCurrent() {
    if (!motorLeftMonitorReady) return 0.0f;
    return motorLeftMonitor.readCurrent() / 1000.0f; // Convert mA to A
}

float PowerManager::getMotorLeftPower() {
    return getMotorLeftVoltage() * getMotorLeftCurrent();
}

float PowerManager::getMotorRightVoltage() {
    if (!motorRightMonitorReady) return 0.0f;
    return motorRightMonitor.readBusVoltage();
}

float PowerManager::getMotorRightCurrent() {
    if (!motorRightMonitorReady) return 0.0f;
    return motorRightMonitor.readCurrent() / 1000.0f; // Convert mA to A
}

float PowerManager::getMotorRightPower() {
    return getMotorRightVoltage() * getMotorRightCurrent();
}

float PowerManager::getTotalMotorCurrent() {
    return getMotorLeftCurrent() + getMotorRightCurrent();
}

float PowerManager::getMotorImbalance() {
    float leftCurrent = getMotorLeftCurrent();
    float rightCurrent = getMotorRightCurrent();
    float avgCurrent = (leftCurrent + rightCurrent) / 2.0f;
    
    if (avgCurrent < 0.5f) return 0.0f; // No meaningful imbalance at low currents
    
    float difference = abs(leftCurrent - rightCurrent);
    return (difference / avgCurrent) * 100.0f; // Return as percentage
}

float PowerManager::getSystemEfficiency() {
    float batteryPower = getBatteryPower();
    float motorPower = getMotorLeftPower() + getMotorRightPower();
    
    if (batteryPower < 1.0f) return 0.0f; // Avoid division by zero
    
    return (motorPower / batteryPower) * 100.0f; // Return as percentage
}

float PowerManager::getEnergyConsumed() {
    return energyConsumed_Wh;
}

void PowerManager::resetEnergyCounters() {
    energyConsumed_Wh = 0.0f;
    lastUpdateTime = millis();
    Serial.println("Energy counters reset");
}

bool PowerManager::isLowVoltage() {
    float voltage = getBatteryVoltage();
    return voltage < BATTERY_VOLTAGE_MIN;
}

bool PowerManager::isBatteryOvercurrent() {
    float current = getBatteryCurrent();
    return current > BATTERY_CURRENT_MAX;
}

bool PowerManager::isMotorOvercurrent(uint8_t motor) {
    if (motor == 0) {
        return getMotorLeftCurrent() > MOTOR_CURRENT_MAX;
    } else {
        return getMotorRightCurrent() > MOTOR_CURRENT_MAX;
    }
}

bool PowerManager::areMotorsBalanced() {
    float imbalance = getMotorImbalance();
    return imbalance < MOTOR_IMBALANCE_WARN;
}

void PowerManager::updateEnergyTracking() {
    unsigned long currentTime = millis();
    
    if (lastUpdateTime == 0) {
        lastUpdateTime = currentTime;
        return;
    }
    
    float deltaTime_hours = (currentTime - lastUpdateTime) / 3600000.0f; // Convert ms to hours
    float currentPower_watts = getBatteryPower();
    
    if (currentPower_watts > 0) {
        energyConsumed_Wh += currentPower_watts * deltaTime_hours;
    }
}