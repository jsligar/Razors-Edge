#include "LightController.h"

LightController::LightController() :
    pwmReady(false),
    currentMode(LIGHT_OFF),
    animationActive(false),
    animationStartTime(0),
    animationPattern(0),
    lastMovingState(false),
    lastBrakingState(false)
{
    // Initialize zone brightness array
    for (uint8_t i = 0; i < 4; i++) {
        zoneBrightness[i] = 0;
    }
}

bool LightController::init() {
    Serial.println("Initializing Light Controller...");
    
    // Initialize PCA9685 PWM controller
    pwmController = Adafruit_PWMServoDriver(PCA9685_ADDR);
    
    pwmController.begin();
    pwmReady = true;  // Assume success for now
    
    // Set PWM frequency for LED dimming (1kHz is good for automotive LEDs)
    pwmController.setPWMFreq(1000);
    
    // Turn off all lights initially
    setAllBrightness(0);
    applyBrightness();
    
    pwmReady = true;
    
    Serial.println("✓ Light Controller initialized");
    return true;
}

void LightController::update() {
    if (!pwmReady) return;
    
    updateAnimations();
    applyBrightness();
}

void LightController::setMode(LightMode mode) {
    LightMode oldMode = currentMode;
    currentMode = mode;
    
    Serial.printf("Light mode: %d → %d\n", oldMode, currentMode);
    
    // Apply immediate mode changes
    switch (currentMode) {
        case LIGHT_OFF:
            setAllBrightness(0);
            break;
            
        case LIGHT_ALWAYS_ON:
            setAllBrightness(100);
            break;
            
        case LIGHT_DIM:
            setAllBrightness(30);
            break;
            
        case LIGHT_AUTO:
            // Auto mode handled in updateAutoMode()
            break;
    }
}

LightMode LightController::getMode() {
    return currentMode;
}

void LightController::setBrightness(uint8_t zone, uint8_t percent) {
    if (zone < 4) {
        zoneBrightness[zone] = constrain(percent, 0, 100);
    }
}

void LightController::setAllBrightness(uint8_t percent) {
    percent = constrain(percent, 0, 100);
    for (uint8_t i = 0; i < 4; i++) {
        zoneBrightness[i] = percent;
    }
}

void LightController::setBrakeLights(bool active) {
    if (active) {
        // Rear lights bright for braking
        setBrightness(LIGHT_REAR_LEFT, 100);
        setBrightness(LIGHT_REAR_RIGHT, 100);
    } else {
        // Rear lights dim for normal running
        setBrightness(LIGHT_REAR_LEFT, 30);
        setBrightness(LIGHT_REAR_RIGHT, 30);
    }
}

void LightController::setTurnSignal(uint8_t side, bool active) {
    // Future implementation for turn signals
    // side: 0 = left, 1 = right
    if (side == 0) {
        // Left turn signal
        setBrightness(LIGHT_FRONT_LEFT, active ? 100 : 0);
        setBrightness(LIGHT_REAR_LEFT, active ? 100 : 0);
    } else {
        // Right turn signal
        setBrightness(LIGHT_FRONT_RIGHT, active ? 100 : 0);
        setBrightness(LIGHT_REAR_RIGHT, active ? 100 : 0);
    }
}

void LightController::updateAutoMode(bool isMoving, bool isBraking) {
    if (currentMode != LIGHT_AUTO) return;
    
    // State change detection
    bool movingChanged = (isMoving != lastMovingState);
    bool brakingChanged = (isBraking != lastBrakingState);
    
    if (movingChanged || brakingChanged) {
        if (isMoving) {
            // Vehicle is moving - turn on headlights
            setBrightness(LIGHT_FRONT_LEFT, 80);
            setBrightness(LIGHT_FRONT_RIGHT, 80);
            
            if (isBraking) {
                // Braking - bright tail lights
                setBrightness(LIGHT_REAR_LEFT, 100);
                setBrightness(LIGHT_REAR_RIGHT, 100);
            } else {
                // Not braking - dim tail lights
                setBrightness(LIGHT_REAR_LEFT, 30);
                setBrightness(LIGHT_REAR_RIGHT, 30);
            }
        } else {
            // Vehicle stopped - turn off headlights, keep dim tail lights
            setBrightness(LIGHT_FRONT_LEFT, 0);
            setBrightness(LIGHT_FRONT_RIGHT, 0);
            setBrightness(LIGHT_REAR_LEFT, 10);
            setBrightness(LIGHT_REAR_RIGHT, 10);
        }
        
        lastMovingState = isMoving;
        lastBrakingState = isBraking;
    }
}

void LightController::startGearShiftAnimation(GearMode gear) {
    if (!pwmReady) return;
    
    animationActive = true;
    animationStartTime = millis();
    animationPattern = gear; // Use gear as pattern selector
    
    Serial.printf("Starting gear shift light animation for gear %d\n", gear);
}

void LightController::startWarningFlash() {
    if (!pwmReady) return;
    
    animationActive = true;
    animationStartTime = millis();
    animationPattern = 0xFF; // Special pattern for warnings
    
    Serial.println("Starting warning flash animation");
}

void LightController::stopAllAnimations() {
    animationActive = false;
    animationPattern = 0;
}

void LightController::applyBrightness() {
    if (!pwmReady) return;
    
    // Convert brightness percentages to PWM values and set PCA9685 outputs
    for (uint8_t i = 0; i < 4; i++) {
        uint16_t pwmValue = percentToPWM(zoneBrightness[i]);
        pwmController.setPWM(i, 0, pwmValue);
    }
}

void LightController::updateAnimations() {
    if (!animationActive) return;
    
    unsigned long elapsed = millis() - animationStartTime;
    
    if (animationPattern == 0xFF) {
        // Warning flash pattern
        runWarningPattern();
        
        // Stop warning after 3 seconds
        if (elapsed > 3000) {
            stopAllAnimations();
        }
    } else if (animationPattern < GEAR_COUNT) {
        // Gear shift pattern
        runGearShiftPattern((GearMode)animationPattern);
        
        // Stop gear animation after 1 second
        if (elapsed > 1000) {
            stopAllAnimations();
        }
    }
}

uint16_t LightController::percentToPWM(uint8_t percent) {
    // Convert 0-100% to 0-4095 PWM value (12-bit)
    percent = constrain(percent, 0, 100);
    return (percent * 4095) / 100;
}

void LightController::runGearShiftPattern(GearMode gear) {
    unsigned long elapsed = millis() - animationStartTime;
    
    // Flash pattern based on gear
    bool flashOn = (elapsed / 200) % 2 == 0; // 200ms flash period
    
    if (flashOn) {
        // Light up different zones based on gear
        switch (gear) {
            case GEAR_1ST:
                setBrightness(LIGHT_FRONT_LEFT, 100);
                setBrightness(LIGHT_FRONT_RIGHT, 0);
                break;
                
            case GEAR_2ND:
                setBrightness(LIGHT_FRONT_LEFT, 100);
                setBrightness(LIGHT_FRONT_RIGHT, 100);
                break;
                
            case GEAR_3RD:
                setAllBrightness(100);
                break;
                
            case GEAR_ECO:
                setBrightness(LIGHT_FRONT_LEFT, 50);
                setBrightness(LIGHT_FRONT_RIGHT, 50);
                setBrightness(LIGHT_REAR_LEFT, 20);
                setBrightness(LIGHT_REAR_RIGHT, 20);
                break;
                
            case GEAR_SPORT_PLUS:
                setAllBrightness(100);
                break;
                
            default:
                setAllBrightness(0);
                break;
        }
    } else {
        setAllBrightness(0);
    }
}

void LightController::runWarningPattern() {
    unsigned long elapsed = millis() - animationStartTime;
    
    // Fast warning flash - all lights
    bool flashOn = (elapsed / 100) % 2 == 0; // 100ms flash period
    
    if (flashOn) {
        setAllBrightness(100);
    } else {
        setAllBrightness(0);
    }
}