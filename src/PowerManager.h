#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <Adafruit_INA228.h>
#include "config.h"

class PowerManager {
public:
    PowerManager();
    
    // Initialization and main update
    bool init();
    void update();
    
    // Battery monitoring
    float getBatteryVoltage();
    float getBatteryCurrent();
    float getBatteryPower();
    float getBatterySOC();
    
    // Motor monitoring
    float getMotorLeftVoltage();
    float getMotorLeftCurrent();
    float getMotorLeftPower();
    
    float getMotorRightVoltage();
    float getMotorRightCurrent();
    float getMotorRightPower();
    
    // Calculated values
    float getTotalMotorCurrent();
    float getMotorImbalance();
    float getSystemEfficiency();
    
    // Energy tracking
    float getEnergyConsumed();
    void resetEnergyCounters();
    
    // Safety checks
    bool isLowVoltage();
    bool isBatteryOvercurrent();
    bool isMotorOvercurrent(uint8_t motor);
    bool areMotorsBalanced();
    
private:
    // All monitors use INA228 (including MATEKSYS INA-BM)
    Adafruit_INA228 batteryMonitor;
    Adafruit_INA228 motorLeftMonitor;
    Adafruit_INA228 motorRightMonitor;
    
    // Sensor status
    bool batteryMonitorReady;
    bool motorLeftMonitorReady;
    bool motorRightMonitorReady;
    
    // Energy tracking
    unsigned long lastUpdateTime;
    float energyConsumed_Wh;
    
    // Helper methods
    bool initSensor(Adafruit_INA228& sensor, uint8_t address, const char* name);
    void updateEnergyTracking();
};

#endif // POWER_MANAGER_H