#include "PowerManager.h"

PowerManager::PowerManager() :
    lastUpdateTime(0),
    estimatedVoltage(12.0f)
{
}

bool PowerManager::init() {
    Serial.println("Initializing Power Manager (Tractor Version - No Current Sensing)...");

    // No hardware to initialize - just set initial voltage estimate
    estimatedVoltage = 12.0f;  // Default estimate
    lastUpdateTime = millis();

    Serial.println("✓ Power Manager initialized (Simplified)");
    return true;
}

void PowerManager::update() {
    // In tractor version, we just maintain a simple voltage estimate
    // You could add voltage divider ADC reading here if connected
    lastUpdateTime = millis();
}

float PowerManager::getBatteryVoltage() {
    // Simple voltage estimate - could be enhanced with voltage divider ADC
    // For now, return a fixed estimate
    // TODO: Connect battery voltage divider to an ADC pin and read actual voltage
    return estimatedVoltage;
}

float PowerManager::getBatterySOC() {
    float voltage = getBatteryVoltage();

    // Simple linear SOC estimate based on voltage
    // Adjust these values based on your battery type
    if (voltage >= BATTERY_VOLTAGE_MAX) return 100.0f;
    if (voltage <= BATTERY_VOLTAGE_MIN) return 0.0f;

    // Linear approximation
    float range = BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN;
    return ((voltage - BATTERY_VOLTAGE_MIN) / range) * 100.0f;
}

bool PowerManager::isLowVoltage() {
    float voltage = getBatteryVoltage();
    return voltage < BATTERY_VOLTAGE_MIN;
}
