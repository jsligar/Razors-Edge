#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "config.h"

class LightController {
public:
    LightController();
    
    // Initialization and main update
    bool init();
    void update();
    
    // Light mode control
    void setMode(LightMode mode);
    LightMode getMode();
    
    // Zone control (0-100%)
    void setBrightness(uint8_t zone, uint8_t percent);
    void setAllBrightness(uint8_t percent);
    
    // Special functions
    void setBrakeLights(bool active);
    void setTurnSignal(uint8_t side, bool active); // 0=left, 1=right
    void updateAutoMode(bool isMoving, bool isBraking);
    
    // Animation controls
    void startGearShiftAnimation(GearMode gear);
    void startWarningFlash();
    void startStartupSequence();  // Cool startup flicker
    void stopAllAnimations();
    
private:
    // PCA9685 PWM controller
    Adafruit_PWMServoDriver pwmController;
    bool pwmReady;
    
    // Current state
    LightMode currentMode;
    uint8_t zoneBrightness[7]; // 4 headlights, 1 center, 2 tails
    
    // Animation state
    bool animationActive;
    unsigned long animationStartTime;
    uint16_t animationPattern;
    
    // Auto mode state
    bool lastMovingState;
    bool lastBrakingState;
    
    // Internal methods
    void applyBrightness();
    void updateAnimations();
    uint16_t percentToPWM(uint8_t percent);
    
    // Animation patterns
    void runGearShiftPattern(GearMode gear);
    void runWarningPattern();
    void runStartupPattern();
};

#endif // LIGHT_CONTROLLER_H