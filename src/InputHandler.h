#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "config.h"

struct ButtonState {
    bool current;
    bool previous;
    bool pressed;
    bool released;
    unsigned long lastChangeTime;
};

struct DirectionSwitchState {
    DirectionMode current;
    DirectionMode previous;
    bool changed;
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

    // Direction switch (3-position: Forward/Neutral/Reverse)
    DirectionMode getDirection();
    bool directionChanged();

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

    // Direction switch state
    DirectionSwitchState dirSwitch;

    // Input filtering
    static const uint8_t FILTER_SIZE = 4;
    uint16_t pedalFilter[FILTER_SIZE];
    uint8_t filterIndex;

    // Internal methods
    void updatePedal();
    void updateKeySwitch();
    void updateDirectionSwitch();
    void updateButton(ButtonState& button, bool currentState);
    uint16_t getFilteredPedalADC();
    DirectionMode readDirectionSwitch();
};

#endif // INPUT_HANDLER_H