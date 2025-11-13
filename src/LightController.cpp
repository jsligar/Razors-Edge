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
    // Initialize zone brightness array (7 main lights)
    for (uint8_t i = 0; i < TOTAL_MAIN_LIGHTS; i++) {
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
    if (zone < TOTAL_MAIN_LIGHTS) {
        zoneBrightness[zone] = constrain(percent, 0, 100);
    }
}

void LightController::setAllBrightness(uint8_t percent) {
    percent = constrain(percent, 0, 100);
    for (uint8_t i = 0; i < TOTAL_MAIN_LIGHTS; i++) {
        zoneBrightness[i] = percent;
    }
}

void LightController::setBrakeLights(bool active) {
    if (active) {
        // Tail lights bright for braking
        setBrightness(LIGHT_TAIL_LEFT, 100);
        setBrightness(LIGHT_TAIL_RIGHT, 100);
        // Also use dedicated brake channel if available
        pwmController.setPWM(LIGHT_BRAKE, 0, percentToPWM(100));
    } else {
        // Tail lights dim for normal running
        setBrightness(LIGHT_TAIL_LEFT, 30);
        setBrightness(LIGHT_TAIL_RIGHT, 30);
        pwmController.setPWM(LIGHT_BRAKE, 0, 0);
    }
}

void LightController::setTurnSignal(uint8_t side, bool active) {
    // Use dedicated turn signal channels
    if (side == 0) {
        // Left turn signal
        pwmController.setPWM(LIGHT_TURN_LEFT, 0, active ? percentToPWM(100) : 0);
    } else {
        // Right turn signal
        pwmController.setPWM(LIGHT_TURN_RIGHT, 0, active ? percentToPWM(100) : 0);
    }
}

void LightController::updateAutoMode(bool isMoving, bool isBraking) {
    if (currentMode != LIGHT_AUTO) return;

    // State change detection
    bool movingChanged = (isMoving != lastMovingState);
    bool brakingChanged = (isBraking != lastBrakingState);

    if (movingChanged || brakingChanged) {
        if (isMoving) {
            // Vehicle is moving - turn on all 4 headlights + center
            setBrightness(LIGHT_HEAD_1, 80);
            setBrightness(LIGHT_HEAD_2, 80);
            setBrightness(LIGHT_HEAD_3, 80);
            setBrightness(LIGHT_HEAD_4, 80);
            setBrightness(LIGHT_CENTER, 100); // Big center light full brightness

            if (isBraking) {
                // Braking - bright tail lights
                setBrightness(LIGHT_TAIL_LEFT, 100);
                setBrightness(LIGHT_TAIL_RIGHT, 100);
            } else {
                // Not braking - dim tail lights
                setBrightness(LIGHT_TAIL_LEFT, 30);
                setBrightness(LIGHT_TAIL_RIGHT, 30);
            }
        } else {
            // Vehicle stopped - turn off headlights, keep dim tail lights
            setBrightness(LIGHT_HEAD_1, 0);
            setBrightness(LIGHT_HEAD_2, 0);
            setBrightness(LIGHT_HEAD_3, 0);
            setBrightness(LIGHT_HEAD_4, 0);
            setBrightness(LIGHT_CENTER, 0);
            setBrightness(LIGHT_TAIL_LEFT, 10);
            setBrightness(LIGHT_TAIL_RIGHT, 10);
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

void LightController::startStartupSequence() {
    if (!pwmReady) return;

    animationActive = true;
    animationStartTime = millis();
    animationPattern = 0xFE; // Special pattern for startup

    Serial.println("Starting startup flicker sequence");
}

void LightController::stopAllAnimations() {
    animationActive = false;
    animationPattern = 0;
}

void LightController::applyBrightness() {
    if (!pwmReady) return;

    // Convert brightness percentages to PWM values and set PCA9685 outputs
    // Apply to all 7 main lights on their respective channels
    pwmController.setPWM(LIGHT_HEAD_1, 0, percentToPWM(zoneBrightness[0]));
    pwmController.setPWM(LIGHT_HEAD_2, 0, percentToPWM(zoneBrightness[1]));
    pwmController.setPWM(LIGHT_HEAD_3, 0, percentToPWM(zoneBrightness[2]));
    pwmController.setPWM(LIGHT_HEAD_4, 0, percentToPWM(zoneBrightness[3]));
    pwmController.setPWM(LIGHT_CENTER, 0, percentToPWM(zoneBrightness[4]));
    pwmController.setPWM(LIGHT_TAIL_LEFT, 0, percentToPWM(zoneBrightness[5]));
    pwmController.setPWM(LIGHT_TAIL_RIGHT, 0, percentToPWM(zoneBrightness[6]));
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
    } else if (animationPattern == 0xFE) {
        // Startup flicker pattern
        runStartupPattern();

        // Stop startup after 3 seconds
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
                // Single headlight
                setBrightness(LIGHT_HEAD_1, 100);
                break;

            case GEAR_2ND:
                // Two headlights
                setBrightness(LIGHT_HEAD_1, 100);
                setBrightness(LIGHT_HEAD_2, 100);
                break;

            case GEAR_3RD:
                // All headlights + center
                setBrightness(LIGHT_HEAD_1, 100);
                setBrightness(LIGHT_HEAD_2, 100);
                setBrightness(LIGHT_HEAD_3, 100);
                setBrightness(LIGHT_HEAD_4, 100);
                setBrightness(LIGHT_CENTER, 100);
                break;

            case GEAR_ECO:
                // Dim all lights for eco mode
                setAllBrightness(30);
                break;

            case GEAR_SPORT_PLUS:
                // Full brightness everything!
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

void LightController::runStartupPattern() {
    unsigned long elapsed = millis() - animationStartTime;

    // Cool startup sequence - sequential flicker with randomness
    // Stage 1 (0-800ms): Flicker headlights one by one
    // Stage 2 (800-1500ms): Center light flicker
    // Stage 3 (1500-2200ms): Tail lights flicker
    // Stage 4 (2200-3000ms): All lights stabilize

    if (elapsed < 800) {
        // Stage 1: Flicker headlights sequentially
        uint8_t flickerIntensity = random(40, 100);
        uint16_t step = elapsed / 200; // Each light gets 200ms

        setAllBrightness(0);
        if (step >= 0) setBrightness(LIGHT_HEAD_1, (elapsed % 100 < 50) ? flickerIntensity : 0);
        if (step >= 1) setBrightness(LIGHT_HEAD_2, (elapsed % 80 < 40) ? flickerIntensity : 0);
        if (step >= 2) setBrightness(LIGHT_HEAD_3, (elapsed % 90 < 45) ? flickerIntensity : 0);
        if (step >= 3) setBrightness(LIGHT_HEAD_4, (elapsed % 70 < 35) ? flickerIntensity : 0);

    } else if (elapsed < 1500) {
        // Stage 2: All headlights on, center flickers in
        uint8_t centerFlicker = random(50, 100);
        setBrightness(LIGHT_HEAD_1, 80);
        setBrightness(LIGHT_HEAD_2, 80);
        setBrightness(LIGHT_HEAD_3, 80);
        setBrightness(LIGHT_HEAD_4, 80);
        setBrightness(LIGHT_CENTER, (elapsed % 60 < 30) ? centerFlicker : 0);
        setBrightness(LIGHT_TAIL_LEFT, 0);
        setBrightness(LIGHT_TAIL_RIGHT, 0);

    } else if (elapsed < 2200) {
        // Stage 3: Headlights + center solid, tail lights flicker in
        uint8_t tailFlicker = random(20, 60);
        setBrightness(LIGHT_HEAD_1, 80);
        setBrightness(LIGHT_HEAD_2, 80);
        setBrightness(LIGHT_HEAD_3, 80);
        setBrightness(LIGHT_HEAD_4, 80);
        setBrightness(LIGHT_CENTER, 100);
        setBrightness(LIGHT_TAIL_LEFT, (elapsed % 50 < 25) ? tailFlicker : 0);
        setBrightness(LIGHT_TAIL_RIGHT, (elapsed % 50 < 25) ? tailFlicker : 0);

    } else {
        // Stage 4: All lights stabilize to default AUTO mode brightness
        setBrightness(LIGHT_HEAD_1, 80);
        setBrightness(LIGHT_HEAD_2, 80);
        setBrightness(LIGHT_HEAD_3, 80);
        setBrightness(LIGHT_HEAD_4, 80);
        setBrightness(LIGHT_CENTER, 100);
        setBrightness(LIGHT_TAIL_LEFT, 30);
        setBrightness(LIGHT_TAIL_RIGHT, 30);
    }
}