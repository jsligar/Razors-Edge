#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include <Arduino.h>
#include "config.h"

// Simplified SafetyLimits for tractor version
struct SafetyLimits {
    float batteryVoltageMin;
    float batteryVoltageMax;
    float speedMax;
};

// Simplified FaultHistory for tractor version
struct FaultHistory {
    uint8_t faultCode;
    unsigned long timestamp;
    float batteryVoltage;
};

class SafetySystem {
public:
    SafetySystem();
    
    // Initialization and main update
    bool init();
    void update();
    
    // Watchdog functions
    void enableWatchdog();
    void feedWatchdog();
    void disableWatchdog();
    
    // Safety status
    bool isSystemSafe();
    uint8_t getFaultCode();
    const char* getFaultString();
    const char* getFaultString(uint8_t code);
    
    // Fault management
    void clearFaults();
    void clearFault(uint8_t faultCode);
    bool hasFault(uint8_t faultCode);
    void forceFault(uint8_t faultCode);
    
    // Safety limits configuration
    void setSafetyLimits(const SafetyLimits& limits);
    SafetyLimits getSafetyLimits();
    
    // Data inputs (called by main loop) - simplified for tractor
    void setBatteryData(float voltage, float current);  // current ignored
    void setKeyState(bool keyOn);

    // Individual safety checks - simplified for tractor
    bool checkBatteryVoltage() { return true; }  // No voltage sensing
    bool checkKeySwitch();

    // Stub methods for compatibility
    void setMotorData(float leftCurrent, float rightCurrent) {}
    void setSpeedData(float speed) {}
    void setMotorSpeed(float leftSpeed, float rightSpeed) {}
    bool checkBatteryCurrent() { return true; }
    bool checkMotorCurrents() { return true; }
    bool checkMotorStall() { return true; }
    bool checkMotorImbalance() { return true; }
    bool checkSpeedLimit() { return true; }
    
    // Fault history
    uint8_t getFaultHistoryCount();
    FaultHistory getFaultHistory(uint8_t index);
    void clearFaultHistory();
    
    // Emergency functions
    void emergencyShutdown();
    bool isEmergencyShutdown();
    
private:
    // Safety state
    uint16_t activeFaults;        // Bitmask of active faults
    SafetyLimits limits;
    bool emergencyState;
    
    // Watchdog
    unsigned long lastWatchdogFeed;
    bool watchdogEnabled;
    
    // Input data - simplified for tractor
    float batteryVoltage;
    bool keyState;

    // Fault timing
    unsigned long faultStartTimes[16]; // Time when each fault first occurred
    
    // Fault history
    static const uint8_t MAX_FAULT_HISTORY = 10;
    FaultHistory faultHistory[MAX_FAULT_HISTORY];
    uint8_t faultHistoryCount;
    uint8_t faultHistoryIndex;
    
    // Internal methods
    void setFault(uint8_t faultCode);
    void clearFaultBit(uint8_t faultCode);
    bool isFaultActive(uint8_t faultCode);
    void recordFault(uint8_t faultCode);
    void checkWatchdog();

    // Fault code to bit conversion
    uint16_t faultCodeToBit(uint8_t faultCode);
};

#endif // SAFETY_SYSTEM_H