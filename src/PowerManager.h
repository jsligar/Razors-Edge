#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Simplified PowerManager for tractor - no current sensing
class PowerManager {
public:
    PowerManager();

    // Initialization and main update
    bool init();
    void update();

    // Stub methods for compatibility (no voltage sensing)
    float getBatteryVoltage() { return 12.0f; }  // Fixed estimate
    float getBatterySOC() { return 100.0f; }      // Always show full
    float getBatteryCurrent() { return 0.0f; }
    float getBatteryPower() { return 0.0f; }
    float getMotorLeftVoltage() { return 0.0f; }
    float getMotorLeftCurrent() { return 0.0f; }
    float getMotorLeftPower() { return 0.0f; }
    float getMotorRightVoltage() { return 0.0f; }
    float getMotorRightCurrent() { return 0.0f; }
    float getMotorRightPower() { return 0.0f; }
    float getTotalMotorCurrent() { return 0.0f; }
    float getMotorImbalance() { return 0.0f; }
    float getSystemEfficiency() { return 0.0f; }
    float getEnergyConsumed() { return 0.0f; }
    void resetEnergyCounters() {}

    // Safety checks (all disabled)
    bool isLowVoltage() { return false; }
    bool isBatteryOvercurrent() { return false; }
    bool isMotorOvercurrent(uint8_t motor) { return false; }
    bool areMotorsBalanced() { return true; }

private:
    unsigned long lastUpdateTime;
};

#endif // POWER_MANAGER_H