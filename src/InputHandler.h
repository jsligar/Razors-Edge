#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "config.h"

struct EncoderState {
    int16_t position;
    int16_t lastPosition;
    bool buttonPressed;
    bool buttonChanged;
};

struct ButtonState {
    bool current;
    bool previous;
    bool pressed;
    bool released;
    unsigned long lastChangeTime;
};

class InputHandler {
public:
    InputHandler();
    
    // Initialization and main update
    bool init();
    void update();
    
    // Pedal input
    float getPedalPosition();        // Returns 0-100% (calibrated)
    uint16_t getRawPedalADC();      // Returns raw ADC value
    
    // Key switch
    bool isKeyOn();
    bool keyStateChanged();
    
    // Rotary encoder
    int16_t getEncoderPosition();
    int16_t getEncoderDelta();
    bool isEncoderPressed();
    bool encoderButtonPressed();     // Edge detection
    bool encoderButtonReleased();    // Edge detection
    
    // Gear shift buttons
    bool isShiftUpPressed();         // Edge detection
    bool isShiftDownPressed();       // Edge detection
    bool isShiftUpHeld();           // Level detection
    bool isShiftDownHeld();         // Level detection
    
    // Calibration
    void calibratePedal();
    void setPedalCalibration(uint16_t minADC, uint16_t maxADC);
    void getPedalCalibration(uint16_t& minADC, uint16_t& maxADC);
    
private:
    // Pedal state
    uint16_t pedalMinADC;
    uint16_t pedalMaxADC;
    uint16_t pedalDeadband;
    float lastPedalPercent;
    
    // Key switch state
    ButtonState keySwitch;
    
    // Encoder state
    EncoderState encoder;
    int16_t lastEncoderA;
    int16_t lastEncoderB;
    
    // Button states
    ButtonState shiftUpButton;
    ButtonState shiftDownButton;
    
    // Input filtering
    static const uint8_t FILTER_SIZE = 4;
    uint16_t pedalFilter[FILTER_SIZE];
    uint8_t filterIndex;
    
    // Internal methods
    void updatePedal();
    void updateKeySwitch();
    void updateEncoder();
    void updateButtons();
    void updateButton(ButtonState& button, bool currentState);
    uint16_t getFilteredPedalADC();
    
    // Encoder decoding
    void handleEncoderChange();
};

#endif // INPUT_HANDLER_H