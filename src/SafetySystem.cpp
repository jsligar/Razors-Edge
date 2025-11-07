#include "SafetySystem.h"
#include <esp_task_wdt.h>

SafetySystem::SafetySystem() :
    activeFaults(0),
    emergencyState(false),
    lastWatchdogFeed(0),
    watchdogEnabled(false),
    batteryVoltage(0.0f),
    batteryCurrent(0.0f),
    motorLeftCurrent(0.0f),
    motorRightCurrent(0.0f),
    vehicleSpeed(0.0f),
    keyState(false),
    motorLeftSpeed(0.0f),
    motorRightSpeed(0.0f),
    faultHistoryCount(0),
    faultHistoryIndex(0)
{
    // Initialize safety limits to defaults
    limits.batteryVoltageMin = BATTERY_VOLTAGE_MIN;
    limits.batteryVoltageMax = BATTERY_VOLTAGE_MAX;
    limits.batteryCurrentMax = BATTERY_CURRENT_MAX;
    limits.motorCurrentMax = MOTOR_CURRENT_MAX;
    limits.motorCurrentStall = MOTOR_CURRENT_STALL;
    limits.motorImbalanceWarn = MOTOR_IMBALANCE_WARN;
    limits.motorImbalanceFault = MOTOR_IMBALANCE_FAULT;
    limits.speedMax = SPEED_MAX_MPH;
    
    // Initialize fault timing array
    for (uint8_t i = 0; i < 16; i++) {
        faultStartTimes[i] = 0;
    }
    
    // Initialize stall detection
    stallDetectionStart[0] = 0;
    stallDetectionStart[1] = 0;
    lastMotorSpeeds[0] = 0.0f;
    lastMotorSpeeds[1] = 0.0f;
}

bool SafetySystem::init() {
    Serial.println("Initializing Safety System...");
    
    // Clear any existing faults
    activeFaults = 0;
    emergencyState = false;
    
    // Initialize watchdog
    lastWatchdogFeed = millis();
    
    Serial.println("✓ Safety System initialized");
    return true;
}

void SafetySystem::update() {
    // Run all safety checks
    checkBatteryVoltage();
    checkBatteryCurrent();
    checkMotorCurrents();
    checkMotorStall();
    checkMotorImbalance();
    checkKeySwitch();
    checkSpeedLimit();
    
    // Check watchdog if enabled
    if (watchdogEnabled) {
        checkWatchdog();
    }
}

void SafetySystem::enableWatchdog() {
    esp_task_wdt_init(WATCHDOG_TIMEOUT_MS / 1000, true);
    esp_task_wdt_add(NULL);
    watchdogEnabled = true;
    lastWatchdogFeed = millis();
    Serial.println("Watchdog timer enabled");
}

void SafetySystem::feedWatchdog() {
    if (watchdogEnabled) {
        esp_task_wdt_reset();
        lastWatchdogFeed = millis();
    }
}

void SafetySystem::disableWatchdog() {
    if (watchdogEnabled) {
        esp_task_wdt_delete(NULL);
        esp_task_wdt_deinit();
        watchdogEnabled = false;
        Serial.println("Watchdog timer disabled");
    }
}

bool SafetySystem::isSystemSafe() {
    return (activeFaults == 0) && !emergencyState;
}

uint8_t SafetySystem::getFaultCode() {
    // Return the lowest numbered active fault
    for (uint8_t i = 1; i < 16; i++) {
        if (activeFaults & (1 << i)) {
            return i;
        }
    }
    return NO_FAULT;
}

const char* SafetySystem::getFaultString() {
    return getFaultString(getFaultCode());
}

const char* SafetySystem::getFaultString(uint8_t code) {
    switch (code) {
        case NO_FAULT: return "No Fault";
        case LOW_BATTERY: return "Low Battery";
        case BATTERY_OVERCURRENT: return "Battery Overcurrent";
        case MOTOR_LEFT_OVERCURRENT: return "Motor L Overcurrent";
        case MOTOR_RIGHT_OVERCURRENT: return "Motor R Overcurrent";
        case MOTOR_LEFT_STALL: return "Motor L Stall";
        case MOTOR_RIGHT_STALL: return "Motor R Stall";
        case MOTOR_IMBALANCE: return "Motor Imbalance";
        case KEY_SWITCH_OFF: return "Key Switch Off";
        case WATCHDOG_TIMEOUT: return "Watchdog Timeout";
        default: return "Unknown Fault";
    }
}

void SafetySystem::clearFaults() {
    uint16_t oldFaults = activeFaults;
    activeFaults = 0;
    emergencyState = false;
    
    // Reset all fault timing
    for (uint8_t i = 0; i < 16; i++) {
        faultStartTimes[i] = 0;
    }
    
    if (oldFaults != 0) {
        Serial.printf("All faults cleared (was: 0x%04X)\n", oldFaults);
    }
}

void SafetySystem::clearFault(uint8_t faultCode) {
    clearFaultBit(faultCode);
    faultStartTimes[faultCode] = 0;
    Serial.printf("Fault %d cleared\n", faultCode);
}

bool SafetySystem::hasFault(uint8_t faultCode) {
    return isFaultActive(faultCode);
}

void SafetySystem::forceFault(uint8_t faultCode) {
    setFault(faultCode);
    Serial.printf("Fault %d forced\n", faultCode);
}

void SafetySystem::setSafetyLimits(const SafetyLimits& newLimits) {
    limits = newLimits;
    Serial.println("Safety limits updated");
}

SafetyLimits SafetySystem::getSafetyLimits() {
    return limits;
}

// Data input methods
void SafetySystem::setBatteryData(float voltage, float current) {
    batteryVoltage = voltage;
    batteryCurrent = current;
}

void SafetySystem::setMotorData(float leftCurrent, float rightCurrent) {
    motorLeftCurrent = leftCurrent;
    motorRightCurrent = rightCurrent;
}

void SafetySystem::setSpeedData(float speed) {
    vehicleSpeed = speed;
}

void SafetySystem::setKeyState(bool keyOn) {
    keyState = keyOn;
}

void SafetySystem::setMotorSpeed(float leftSpeed, float rightSpeed) {
    motorLeftSpeed = leftSpeed;
    motorRightSpeed = rightSpeed;
}

bool SafetySystem::checkBatteryVoltage() {
    if (batteryVoltage < limits.batteryVoltageMin) {
        setFault(LOW_BATTERY);
        return false;
    }
    return true;
}

bool SafetySystem::checkBatteryCurrent() {
    if (batteryCurrent > limits.batteryCurrentMax) {
        if (!isFaultActive(BATTERY_OVERCURRENT)) {
            faultStartTimes[BATTERY_OVERCURRENT] = millis();
        }
        
        // Allow brief overcurrent (1 second)
        if (millis() - faultStartTimes[BATTERY_OVERCURRENT] > 1000) {
            setFault(BATTERY_OVERCURRENT);
            return false;
        }
    } else {
        clearFaultBit(BATTERY_OVERCURRENT);
        faultStartTimes[BATTERY_OVERCURRENT] = 0;
    }
    return true;
}

bool SafetySystem::checkMotorCurrents() {
    bool leftOK = true, rightOK = true;
    
    if (motorLeftCurrent > limits.motorCurrentMax) {
        setFault(MOTOR_LEFT_OVERCURRENT);
        leftOK = false;
    }
    
    if (motorRightCurrent > limits.motorCurrentMax) {
        setFault(MOTOR_RIGHT_OVERCURRENT);
        rightOK = false;
    }
    
    return leftOK && rightOK;
}

bool SafetySystem::checkMotorStall() {
    bool leftOK = checkStallCondition(0);  // Left motor
    bool rightOK = checkStallCondition(1); // Right motor
    
    return leftOK && rightOK;
}

bool SafetySystem::checkMotorImbalance() {
    float imbalance = calculateMotorImbalance();
    
    if (imbalance > limits.motorImbalanceFault) {
        setFault(MOTOR_IMBALANCE);
        return false;
    }
    
    return true;
}

bool SafetySystem::checkKeySwitch() {
    if (!keyState) {
        setFault(KEY_SWITCH_OFF);
        return false;
    }
    return true;
}

bool SafetySystem::checkSpeedLimit() {
    if (vehicleSpeed > limits.speedMax) {
        // Speed limit check - could implement speed limiting here
        return false;
    }
    return true;
}

uint8_t SafetySystem::getFaultHistoryCount() {
    return faultHistoryCount;
}

FaultHistory SafetySystem::getFaultHistory(uint8_t index) {
    if (index < faultHistoryCount) {
        return faultHistory[index];
    }
    return {0, 0, 0, 0, 0, 0}; // Return empty fault history
}

void SafetySystem::clearFaultHistory() {
    faultHistoryCount = 0;
    faultHistoryIndex = 0;
    Serial.println("Fault history cleared");
}

void SafetySystem::emergencyShutdown() {
    emergencyState = true;
    activeFaults = 0xFFFF; // Set all fault bits
    Serial.println("EMERGENCY SHUTDOWN ACTIVATED");
}

bool SafetySystem::isEmergencyShutdown() {
    return emergencyState;
}

void SafetySystem::setFault(uint8_t faultCode) {
    uint16_t faultBit = faultCodeToBit(faultCode);
    
    if (!(activeFaults & faultBit)) {
        activeFaults |= faultBit;
        faultStartTimes[faultCode] = millis();
        recordFault(faultCode);
        
        Serial.printf("FAULT SET: %s (Code: %d)\n", getFaultString(faultCode), faultCode);
    }
}

void SafetySystem::clearFaultBit(uint8_t faultCode) {
    uint16_t faultBit = faultCodeToBit(faultCode);
    activeFaults &= ~faultBit;
}

bool SafetySystem::isFaultActive(uint8_t faultCode) {
    uint16_t faultBit = faultCodeToBit(faultCode);
    return (activeFaults & faultBit) != 0;
}

void SafetySystem::recordFault(uint8_t faultCode) {
    // Record fault in circular buffer
    faultHistory[faultHistoryIndex].faultCode = faultCode;
    faultHistory[faultHistoryIndex].timestamp = millis();
    faultHistory[faultHistoryIndex].batteryVoltage = batteryVoltage;
    faultHistory[faultHistoryIndex].batteryCurrent = batteryCurrent;
    faultHistory[faultHistoryIndex].motorLeftCurrent = motorLeftCurrent;
    faultHistory[faultHistoryIndex].motorRightCurrent = motorRightCurrent;
    
    faultHistoryIndex = (faultHistoryIndex + 1) % MAX_FAULT_HISTORY;
    if (faultHistoryCount < MAX_FAULT_HISTORY) {
        faultHistoryCount++;
    }
}

void SafetySystem::checkWatchdog() {
    unsigned long elapsed = millis() - lastWatchdogFeed;
    
    if (elapsed > WATCHDOG_TIMEOUT_MS) {
        setFault(WATCHDOG_TIMEOUT);
        Serial.println("WATCHDOG TIMEOUT - System will reset");
        // The ESP32 watchdog will reset the system
    }
}

bool SafetySystem::checkStallCondition(uint8_t motor) {
    float currentCurrent = (motor == 0) ? motorLeftCurrent : motorRightCurrent;
    float currentSpeed = (motor == 0) ? motorLeftSpeed : motorRightSpeed;
    
    // Check if motor is drawing high current but not moving
    if (currentCurrent > limits.motorCurrentStall && currentSpeed < 5.0f) {
        if (stallDetectionStart[motor] == 0) {
            stallDetectionStart[motor] = millis();
        } else if (millis() - stallDetectionStart[motor] > 500) {
            // Stall detected for 500ms
            if (motor == 0) {
                setFault(MOTOR_LEFT_STALL);
            } else {
                setFault(MOTOR_RIGHT_STALL);
            }
            return false;
        }
    } else {
        stallDetectionStart[motor] = 0; // Reset stall timer
    }
    
    return true;
}

float SafetySystem::calculateMotorImbalance() {
    float avgCurrent = (motorLeftCurrent + motorRightCurrent) / 2.0f;
    
    if (avgCurrent < 0.5f) return 0.0f; // No meaningful imbalance at low currents
    
    float difference = abs(motorLeftCurrent - motorRightCurrent);
    return (difference / avgCurrent) * 100.0f; // Return as percentage
}

uint16_t SafetySystem::faultCodeToBit(uint8_t faultCode) {
    if (faultCode == 0 || faultCode > 15) return 0;
    return (1 << faultCode);
}