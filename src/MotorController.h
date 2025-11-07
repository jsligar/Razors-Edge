#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// Forward declaration to avoid circular includes
class Navigation;

enum MotorControlMode {
    MOTOR_SYNCHRONIZED,  // Both motors same speed
    MOTOR_DIFFERENTIAL,  // Independent speeds for steering
    MOTOR_BALANCED      // Compensate for motor differences
};

class MotorController {
public:
    MotorController();
    
    // Initialization and main update
    bool init();
    void update();
    
    // Gear and throttle control
    void setGear(GearMode gear);
    void setThrottleCommand(float pedalPercent);
    
    // Direct speed control
    void setSpeed(float percent);
    void setSpeedLeft(float percent);
    void setSpeedRight(float percent);
    
    // Geofencing integration
    void setNavigationPtr(Navigation* nav);
    void applyGeofenceSpeedLimit();
    bool shouldStopForGeofence();
    
    // Motor control modes
    void setControlMode(MotorControlMode mode);
    
    // Emergency controls
    void stop();
    void emergencyStop();
    void brake();
    
    // Status getters
    float getCurrentSpeedLeft();
    float getCurrentSpeedRight();
    float getTargetSpeedLeft();
    float getTargetSpeedRight();
    GearMode getCurrentGear();
    MotorControlMode getControlMode();
    
    // Direction control
    void setDirection(bool forward);
    bool isForward();
    
    // Ramping controls
    void setAccelRate(float ratePerSecond);
    void setDecelRate(float ratePerSecond);
    void setEmergencyDecelRate(float ratePerSecond);
    void setBackEMFProtectionRate(float ratePerSecond);
    
    // Back-EMF protection
    bool isBackEMFProtectionActive();
    void forceGentleRampDown();
    
private:
    // Current state
    GearMode currentGear;
    MotorControlMode controlMode;
    bool isEnabled;
    bool direction; // true = forward, false = reverse
    
    // Geofencing integration
    Navigation* navigation;
    bool geofenceStopActive;
    float geofenceSpeedFactor;
    
    // Speed values (0-100%)
    float currentSpeedLeft;
    float currentSpeedRight;
    float targetSpeedLeft;
    float targetSpeedRight;
    
    // Ramping parameters
    float accelRate;
    float decelRate;
    float emergencyDecelRate;
    float backEMFProtectionRate;
    unsigned long lastUpdateTime;
    
    // Back-EMF protection
    bool backEMFProtectionActive;
    float previousSpeedLeft;
    float previousSpeedRight;
    unsigned long backEMFStartTime;
    
    // Internal methods
    void updateRamping();
    void applyPWM();
    float applyThrottleCurve(float input, GearMode gear);
    void updateMotorBalance();
    void updateBackEMFProtection();
    bool detectExcessiveDeceleration(float currentSpeed, float previousSpeed, float deltaTime);
    
    // PWM control
    void setPWMLeft(float percent);
    void setPWMRight(float percent);
    void setDirectionPins();
};

#endif // MOTOR_CONTROLLER_H