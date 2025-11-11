#include "MotorController.h"
#include "Navigation.h"

MotorController::MotorController() :
    currentGear(GEAR_PARK),
    controlMode(MOTOR_SYNCHRONIZED),
    isEnabled(false),
    direction(true),
    navigation(nullptr),
    geofenceStopActive(false),
    geofenceSpeedFactor(1.0f),
    currentSpeedLeft(0.0f),
    currentSpeedRight(0.0f),
    targetSpeedLeft(0.0f),
    targetSpeedRight(0.0f),
    accelRate(50.0f),  // 50% per second
    decelRate(100.0f), // 100% per second (normal decel)
    emergencyDecelRate(150.0f), // 150% per second (emergency)
    backEMFProtectionRate(BACK_EMF_DECEL_RATE), // Use configured rate
    lastUpdateTime(0),
    backEMFProtectionActive(false),
    previousSpeedLeft(0.0f),
    previousSpeedRight(0.0f),
    backEMFStartTime(0),
    emergencyStopLogged(false)
{
}

bool MotorController::init() {
    Serial.println("Initializing Motor Controller...");
    
    // Configure PWM channels for motors
    ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(GPIO_MOTOR_PWM_LEFT, PWM_CHANNEL_LEFT);
    
    ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(GPIO_MOTOR_PWM_RIGHT, PWM_CHANNEL_RIGHT);
    
    // Configure direction pins
    pinMode(GPIO_MOTOR_DIR_LEFT, OUTPUT);
    pinMode(GPIO_MOTOR_DIR_RIGHT, OUTPUT);
    
    // Set initial safe state
    stop();
    setDirectionPins();
    
    lastUpdateTime = millis();
    
    Serial.println("✓ Motor Controller initialized");
    return true;
}

void MotorController::update() {
    // Check geofencing constraints first
    applyGeofenceSpeedLimit();
    
    updateBackEMFProtection();
    updateRamping();
    applyPWM();
    
    if (controlMode == MOTOR_BALANCED) {
        updateMotorBalance();
    }
    
    // Store previous speeds for back-EMF detection
    previousSpeedLeft = currentSpeedLeft;
    previousSpeedRight = currentSpeedRight;
}

void MotorController::setGear(GearMode gear) {
    if (gear >= 0 && gear < GEAR_COUNT) {
        GearMode oldGear = currentGear;
        currentGear = gear;

        Serial.printf("Gear change: %s → %s\n",
                     GEAR_CONFIGS[oldGear].name,
                     GEAR_CONFIGS[currentGear].name);

        // Handle special gear logic
        if (gear == GEAR_PARK) {
            stop();
            isEnabled = false;
        } else if (gear == GEAR_REVERSE) {
            // Set reverse direction
            direction = false;  // false = reverse
            setDirectionPins();
            isEnabled = true;
            Serial.println("  → Direction: REVERSE");
        } else {
            // Set forward direction for all forward gears
            direction = true;  // true = forward
            setDirectionPins();
            isEnabled = true;
            if (oldGear == GEAR_REVERSE) {
                Serial.println("  → Direction: FORWARD");
            }
        }
    }
}

void MotorController::setThrottleCommand(float pedalPercent) {
    if (!isEnabled || currentGear == GEAR_PARK) {
        setSpeed(0.0f);
        return;
    }
    
    // Apply gear-specific throttle curve
    float curvedThrottle = applyThrottleCurve(pedalPercent, currentGear);
    
    // Set target speeds based on control mode
    switch (controlMode) {
        case MOTOR_SYNCHRONIZED:
            setSpeed(curvedThrottle);
            break;
            
        case MOTOR_DIFFERENTIAL:
            // Differential steering not implemented yet
            setSpeed(curvedThrottle);
            break;
            
        case MOTOR_BALANCED:
            setSpeed(curvedThrottle);
            break;
    }
}

void MotorController::setSpeed(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    targetSpeedLeft = percent;
    targetSpeedRight = percent;
}

void MotorController::setSpeedLeft(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    targetSpeedLeft = percent;
}

void MotorController::setSpeedRight(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    targetSpeedRight = percent;
}

void MotorController::setControlMode(MotorControlMode mode) {
    controlMode = mode;
    Serial.printf("Motor control mode: %d\n", mode);
}

void MotorController::stop() {
    targetSpeedLeft = 0.0f;
    targetSpeedRight = 0.0f;
}

void MotorController::emergencyStop() {
    targetSpeedLeft = 0.0f;
    targetSpeedRight = 0.0f;
    
    // Force immediate back-EMF protection mode
    backEMFProtectionActive = true;
    backEMFStartTime = millis();
    
    // Don't immediately zero current speeds - let ramping handle it
    // This prevents sudden motor driver stress from back-EMF
    
    // Only log once to prevent spam
    if (!emergencyStopLogged) {
        Serial.println("EMERGENCY STOP activated - Back-EMF protection engaged");
        emergencyStopLogged = true;
    }
}

void MotorController::brake() {
    // Activate back-EMF protection for controlled braking
    backEMFProtectionActive = true;
    backEMFStartTime = millis();
    stop();
    
    Serial.println("Braking with back-EMF protection");
}

float MotorController::getCurrentSpeedLeft() {
    return currentSpeedLeft;
}

float MotorController::getCurrentSpeedRight() {
    return currentSpeedRight;
}

float MotorController::getTargetSpeedLeft() {
    return targetSpeedLeft;
}

float MotorController::getTargetSpeedRight() {
    return targetSpeedRight;
}

GearMode MotorController::getCurrentGear() {
    return currentGear;
}

MotorControlMode MotorController::getControlMode() {
    return controlMode;
}

void MotorController::setDirection(bool forward) {
    direction = forward;
    setDirectionPins();
}

bool MotorController::isForward() {
    return direction;
}

void MotorController::setAccelRate(float ratePerSecond) {
    accelRate = constrain(ratePerSecond, 10.0f, 200.0f);
}

void MotorController::setDecelRate(float ratePerSecond) {
    decelRate = constrain(ratePerSecond, 10.0f, 500.0f);
}

void MotorController::setEmergencyDecelRate(float ratePerSecond) {
    emergencyDecelRate = constrain(ratePerSecond, 50.0f, 300.0f);
}

void MotorController::setBackEMFProtectionRate(float ratePerSecond) {
    backEMFProtectionRate = constrain(ratePerSecond, 5.0f, 50.0f);
}

bool MotorController::isBackEMFProtectionActive() {
    return backEMFProtectionActive;
}

void MotorController::forceGentleRampDown() {
    backEMFProtectionActive = true;
    backEMFStartTime = millis();
    targetSpeedLeft = 0.0f;
    targetSpeedRight = 0.0f;
    
    Serial.println("Forced gentle ramp-down activated");
}

void MotorController::updateRamping() {
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastUpdateTime) / 1000.0f; // Convert to seconds
    lastUpdateTime = currentTime;
    
    if (deltaTime > 0.1f) deltaTime = 0.1f; // Limit delta time to prevent jumps
    
    // Determine deceleration rate based on protection status
    float activeDecelRate = decelRate;
    if (backEMFProtectionActive) {
        activeDecelRate = backEMFProtectionRate;
    }
    
    // Update left motor with ramping
    float leftError = targetSpeedLeft - currentSpeedLeft;
    if (abs(leftError) > 0.1f) {
        float rate = (leftError > 0) ? accelRate : activeDecelRate;
        
        // Additional safety check for excessive deceleration
        if (leftError < 0 && detectExcessiveDeceleration(currentSpeedLeft, previousSpeedLeft, deltaTime)) {
            rate = BACK_EMF_DECEL_RATE; // Force gentle decel
            if (!backEMFProtectionActive) {
                Serial.println("Auto-engaging back-EMF protection (Left motor)");
                backEMFProtectionActive = true;
                backEMFStartTime = millis();
            }
        }
        
        float change = rate * deltaTime;
        
        if (abs(change) > abs(leftError)) {
            currentSpeedLeft = targetSpeedLeft;
        } else {
            currentSpeedLeft += (leftError > 0) ? change : -change;
        }
    }
    
    // Update right motor with ramping
    float rightError = targetSpeedRight - currentSpeedRight;
    if (abs(rightError) > 0.1f) {
        float rate = (rightError > 0) ? accelRate : activeDecelRate;
        
        // Additional safety check for excessive deceleration
        if (rightError < 0 && detectExcessiveDeceleration(currentSpeedRight, previousSpeedRight, deltaTime)) {
            rate = BACK_EMF_DECEL_RATE; // Force gentle decel
            if (!backEMFProtectionActive) {
                Serial.println("Auto-engaging back-EMF protection (Right motor)");
                backEMFProtectionActive = true;
                backEMFStartTime = millis();
            }
        }
        
        float change = rate * deltaTime;
        
        if (abs(change) > abs(rightError)) {
            currentSpeedRight = targetSpeedRight;
        } else {
            currentSpeedRight += (rightError > 0) ? change : -change;
        }
    }
}

void MotorController::applyPWM() {
    setPWMLeft(currentSpeedLeft);
    setPWMRight(currentSpeedRight);
}

float MotorController::applyThrottleCurve(float input, GearMode gear) {
    if (gear < 0 || gear >= GEAR_COUNT) return 0.0f;
    
    // Normalize input to 0-1 range
    float normalizedInput = constrain(input / 100.0f, 0.0f, 1.0f);
    
    const GearConfig& config = GEAR_CONFIGS[gear];
    
    // Apply exponential curve: output = input^exponent
    float curvedOutput = pow(normalizedInput, config.curveExponent);
    
    // Scale by gear's maximum throttle
    float finalOutput = curvedOutput * config.maxThrottle;
    
    return finalOutput;
}

void MotorController::updateMotorBalance() {
    // This function could implement motor balance compensation
    // For now, it's a placeholder for future advanced motor balancing
}

void MotorController::setPWMLeft(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    
    // Convert percentage to PWM value (0-255 for 8-bit resolution)
    uint32_t pwmValue = (percent / 100.0f) * ((1 << PWM_RESOLUTION) - 1);
    
    ledcWrite(PWM_CHANNEL_LEFT, pwmValue);
}

void MotorController::setPWMRight(float percent) {
    percent = constrain(percent, 0.0f, 100.0f);
    
    // Convert percentage to PWM value (0-255 for 8-bit resolution)
    uint32_t pwmValue = (percent / 100.0f) * ((1 << PWM_RESOLUTION) - 1);
    
    ledcWrite(PWM_CHANNEL_RIGHT, pwmValue);
}

void MotorController::setDirectionPins() {
    // LOW = forward, HIGH = reverse for MDD20A
    digitalWrite(GPIO_MOTOR_DIR_LEFT, direction ? LOW : HIGH);
    digitalWrite(GPIO_MOTOR_DIR_RIGHT, direction ? LOW : HIGH);
}

void MotorController::updateBackEMFProtection() {
    if (!backEMFProtectionActive) return;
    
    // Check if we've reached target (stopped) and can disable protection
    if (currentSpeedLeft <= 0.1f && currentSpeedRight <= 0.1f && 
        targetSpeedLeft <= 0.1f && targetSpeedRight <= 0.1f) {
        
        unsigned long protectionTime = millis() - backEMFStartTime;
        if (protectionTime > BACK_EMF_MIN_TIME_MS) { // Keep protection active for at least 1 second
            backEMFProtectionActive = false;
            Serial.println("Back-EMF protection deactivated - motors stopped safely");
        }
    }
    
    // Auto-disable protection after 10 seconds regardless
    unsigned long protectionTime = millis() - backEMFStartTime;
    if (protectionTime > BACK_EMF_MAX_TIME_MS) {
        backEMFProtectionActive = false;
        Serial.println("Back-EMF protection auto-disabled after 10 seconds");
    }
}

bool MotorController::detectExcessiveDeceleration(float currentSpeed, float previousSpeed, float deltaTime) {
    if (deltaTime <= 0) return false;
    
    // Calculate actual deceleration rate (%/second)
    float actualDecelRate = (previousSpeed - currentSpeed) / deltaTime;
    
    // If decelerating faster than the configured threshold, it's potentially dangerous
    
    // Only trigger if we're actually decelerating significantly
    if (actualDecelRate > DANGEROUS_DECEL_RATE && currentSpeed > 10.0f) {
        return true;
    }
    
    return false;
}

// Geofencing integration methods

void MotorController::setNavigationPtr(Navigation* nav) {
    navigation = nav;
    Serial.println("Motor controller linked to navigation system");
}

void MotorController::applyGeofenceSpeedLimit() {
    if (!navigation) {
        geofenceStopActive = false;
        geofenceSpeedFactor = 1.0f;
        return;
    }
    
    // Check if geofencing requires stopping
    if (navigation->shouldStopForGeofence()) {
        if (!geofenceStopActive) {
            Serial.println("🚨 GEOFENCE STOP ACTIVATED");
            geofenceStopActive = true;
            emergencyStop();
        }
        return;
    }
    
    // Check if geofencing requires speed limiting
    if (navigation->shouldLimitSpeedForGeofence()) {
        float newSpeedFactor = navigation->getGeofenceSpeedFactor();
        if (abs(newSpeedFactor - geofenceSpeedFactor) > 0.05f) {
            geofenceSpeedFactor = newSpeedFactor;
            Serial.printf("🛑 Geofence speed limit: %.0f%% of normal\n", 
                         geofenceSpeedFactor * 100.0f);
            
            // Apply speed reduction to current targets
            targetSpeedLeft *= geofenceSpeedFactor;
            targetSpeedRight *= geofenceSpeedFactor;
        }
    } else {
        // Reset to normal speed if no longer limited
        if (geofenceSpeedFactor < 1.0f) {
            Serial.println("✅ Geofence speed limit cleared");
            geofenceSpeedFactor = 1.0f;
        }
        
        // Clear stop condition if it was active
        if (geofenceStopActive) {
            Serial.println("✅ Geofence stop cleared");
            geofenceStopActive = false;
        }
    }
}

bool MotorController::shouldStopForGeofence() {
    return geofenceStopActive;
}