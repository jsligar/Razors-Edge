#include "InputHandler.h"

InputHandler::InputHandler() :
    pedalMinADC(PEDAL_ADC_MIN),
    pedalMaxADC(PEDAL_ADC_MAX),
    pedalDeadband(PEDAL_DEADBAND),
    lastPedalPercent(0.0f),
    filterIndex(0)
{
    // Initialize filter array
    for (uint8_t i = 0; i < FILTER_SIZE; i++) {
        pedalFilter[i] = 0;
    }
    
    // Initialize button states
    keySwitch = {false, false, false, false, 0};
    encoder = {0, 0, false, false, 0, 0};
    shiftUpButton = {false, false, false, false, 0};
    shiftDownButton = {false, false, false, false, 0};
    
    lastEncoderA = 0;
    lastEncoderB = 0;
}

bool InputHandler::init() {
    Serial.println("Initializing Input Handler...");
    
    // Configure input pins
    pinMode(GPIO_KEY_SWITCH, INPUT);        // External 10k pulldown required!
    pinMode(GPIO_ENCODER_CLK, INPUT_PULLUP);
    pinMode(GPIO_ENCODER_DT, INPUT_PULLUP);
    pinMode(GPIO_ENCODER_BTN, INPUT_PULLUP);
    pinMode(GPIO_KEY0, INPUT_PULLUP);       // Shift Up
    pinMode(GPIO_KEY1, INPUT_PULLUP);       // Shift Down
    pinMode(GPIO_PEDAL_ADC, INPUT);         // No pullup on ADC
    
    // Configure ADC
    analogReadResolution(12);               // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db);         // 0-3.3V range
    
    // Read initial encoder state
    lastEncoderA = digitalRead(GPIO_ENCODER_CLK);
    lastEncoderB = digitalRead(GPIO_ENCODER_DT);
    
    Serial.println("✓ Input Handler initialized");
    return true;
}

void InputHandler::update() {
    updatePedal();
    updateKeySwitch();
    updateEncoder();
    updateButtons();
}

float InputHandler::getPedalPosition() {
    return lastPedalPercent;
}

uint16_t InputHandler::getRawPedalADC() {
    return analogRead(GPIO_PEDAL_ADC);
}

bool InputHandler::isKeyOn() {
    return keySwitch.current;
}

bool InputHandler::keyStateChanged() {
    bool changed = keySwitch.pressed || keySwitch.released;
    if (changed) {
        keySwitch.pressed = false;
        keySwitch.released = false;
    }
    return changed;
}

int16_t InputHandler::getEncoderPosition() {
    return encoder.position;
}

int16_t InputHandler::getEncoderDelta() {
    int16_t delta = encoder.position - encoder.lastPosition;
    encoder.lastPosition = encoder.position;
    return delta;
}

bool InputHandler::isEncoderPressed() {
    return encoder.buttonPressed;
}

bool InputHandler::encoderButtonPressed() {
    bool pressed = encoder.buttonChanged && encoder.buttonPressed;
    if (pressed) {
        encoder.buttonChanged = false;
    }
    return pressed;
}

bool InputHandler::encoderButtonReleased() {
    bool released = encoder.buttonChanged && !encoder.buttonPressed;
    if (released) {
        encoder.buttonChanged = false;
    }
    return released;
}

bool InputHandler::isShiftUpPressed() {
    bool pressed = shiftUpButton.pressed;
    if (pressed) {
        shiftUpButton.pressed = false;
    }
    return pressed;
}

bool InputHandler::isShiftDownPressed() {
    bool pressed = shiftDownButton.pressed;
    if (pressed) {
        shiftDownButton.pressed = false;
    }
    return pressed;
}

bool InputHandler::isShiftUpHeld() {
    return shiftUpButton.current;
}

bool InputHandler::isShiftDownHeld() {
    return shiftDownButton.current;
}

void InputHandler::calibratePedal() {
    Serial.println("Starting pedal calibration...");
    Serial.println("Release pedal completely and press shift up button...");
    
    // Wait for shift up button press
    while (!isShiftUpPressed()) {
        update();
        delay(10);
    }
    
    // Record minimum value
    uint16_t minSum = 0;
    for (int i = 0; i < 100; i++) {
        minSum += analogRead(GPIO_PEDAL_ADC);
        delay(10);
    }
    pedalMinADC = minSum / 100;
    
    Serial.printf("Min value recorded: %d\n", pedalMinADC);
    Serial.println("Now press pedal fully and press shift down button...");
    
    // Wait for shift down button press
    while (!isShiftDownPressed()) {
        update();
        delay(10);
    }
    
    // Record maximum value
    uint16_t maxSum = 0;
    for (int i = 0; i < 100; i++) {
        maxSum += analogRead(GPIO_PEDAL_ADC);
        delay(10);
    }
    pedalMaxADC = maxSum / 100;
    
    Serial.printf("Max value recorded: %d\n", pedalMaxADC);
    Serial.println("Pedal calibration complete!");
}

void InputHandler::setPedalCalibration(uint16_t minADC, uint16_t maxADC) {
    pedalMinADC = minADC;
    pedalMaxADC = maxADC;
    Serial.printf("Pedal calibration set: %d - %d\n", pedalMinADC, pedalMaxADC);
}

void InputHandler::getPedalCalibration(uint16_t& minADC, uint16_t& maxADC) {
    minADC = pedalMinADC;
    maxADC = pedalMaxADC;
}

void InputHandler::updatePedal() {
    // Read raw ADC value
    uint16_t rawADC = analogRead(GPIO_PEDAL_ADC);
    
    // Add to rolling average filter
    pedalFilter[filterIndex] = rawADC;
    filterIndex = (filterIndex + 1) % FILTER_SIZE;
    
    // Get filtered value
    uint16_t filteredADC = getFilteredPedalADC();
    
    // Apply calibration and deadband
    if (filteredADC < (pedalMinADC + pedalDeadband)) {
        lastPedalPercent = 0.0f;
    } else if (filteredADC > pedalMaxADC) {
        lastPedalPercent = 100.0f;
    } else {
        // Map calibrated range to 0-100%
        float mapped = map(filteredADC, 
                          pedalMinADC + pedalDeadband, 
                          pedalMaxADC, 
                          0, 100);
        lastPedalPercent = constrain(mapped, 0.0f, 100.0f);
    }
    
    // Sanity check for fault detection
    if (rawADC < 50 || rawADC > 4000) {
        // Possible sensor fault
        lastPedalPercent = 0.0f;
    }
}

void InputHandler::updateKeySwitch() {
    bool currentState = digitalRead(GPIO_KEY_SWITCH);
    updateButton(keySwitch, currentState);
}

void InputHandler::updateEncoder() {
    unsigned long currentTime = millis();

    // Read encoder signals with debouncing
    int currentA = digitalRead(GPIO_ENCODER_CLK);
    int currentB = digitalRead(GPIO_ENCODER_DT);

    // Detect state changes with debounce (5ms minimum between steps)
    if ((currentA != lastEncoderA || currentB != lastEncoderB) &&
        (currentTime - encoder.lastRotationTime > ENCODER_DEBOUNCE_MS)) {
        handleEncoderChange();
        lastEncoderA = currentA;
        lastEncoderB = currentB;
        encoder.lastRotationTime = currentTime;
    }

    // Read encoder button with improved debouncing (50ms)
    bool buttonState = !digitalRead(GPIO_ENCODER_BTN); // Active low
    if (buttonState != encoder.buttonPressed &&
        (currentTime - encoder.lastButtonChangeTime > BUTTON_DEBOUNCE_MS)) {
        encoder.buttonPressed = buttonState;
        encoder.buttonChanged = true;
        encoder.lastButtonChangeTime = currentTime;
    }
}

void InputHandler::updateButtons() {
    // Shift up button (KEY0) - active low
    bool shiftUpState = !digitalRead(GPIO_KEY0);
    updateButton(shiftUpButton, shiftUpState);
    
    // Shift down button (KEY1) - active low
    bool shiftDownState = !digitalRead(GPIO_KEY1);
    updateButton(shiftDownButton, shiftDownState);
}

void InputHandler::updateButton(ButtonState& button, bool currentState) {
    unsigned long currentTime = millis();
    
    // Check for state change
    if (currentState != button.current) {
        // State changed, check if enough time has passed (debounce)
        if (currentTime - button.lastChangeTime > BUTTON_DEBOUNCE_MS) {
            button.previous = button.current;
            button.current = currentState;
            button.lastChangeTime = currentTime;
            
            // Set edge detection flags
            if (currentState && !button.previous) {
                button.pressed = true;
            } else if (!currentState && button.previous) {
                button.released = true;
            }
        }
    }
}

uint16_t InputHandler::getFilteredPedalADC() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < FILTER_SIZE; i++) {
        sum += pedalFilter[i];
    }
    return sum / FILTER_SIZE;
}

void InputHandler::handleEncoderChange() {
    // Simple quadrature decoding
    int currentA = digitalRead(GPIO_ENCODER_CLK);
    int currentB = digitalRead(GPIO_ENCODER_DT);
    
    // Determine direction based on the sequence
    if (lastEncoderA == 0 && currentA == 1) {
        if (currentB == 0) {
            encoder.position++;
        } else {
            encoder.position--;
        }
    }
}