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
    dirSwitch = {DIR_NEUTRAL, DIR_NEUTRAL, false, 0};
}

bool InputHandler::init() {
    Serial.println("Initializing Input Handler (Tractor Version)...");

    // Configure input pins
    pinMode(GPIO_KEY_SWITCH, INPUT);        // External 10k pulldown required!
    pinMode(GPIO_DIR_FORWARD, INPUT_PULLUP); // Direction switch forward
    pinMode(GPIO_DIR_REVERSE, INPUT_PULLUP); // Direction switch reverse
    pinMode(GPIO_PEDAL_ADC, INPUT);         // No pullup on ADC

    // Configure ADC
    analogReadResolution(12);               // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db);         // 0-3.3V range

    Serial.println("✓ Input Handler initialized (Tractor Version)");
    return true;
}

void InputHandler::update() {
    updatePedal();
    updateKeySwitch();
    updateDirectionSwitch();
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

DirectionMode InputHandler::getDirection() {
    return dirSwitch.current;
}

bool InputHandler::directionChanged() {
    bool changed = dirSwitch.changed;
    if (changed) {
        dirSwitch.changed = false;
    }
    return changed;
}

void InputHandler::calibratePedal() {
    Serial.println("Starting pedal calibration...");
    Serial.println("Release pedal completely (waiting 3 seconds)...");

    delay(3000);

    // Record minimum value
    uint16_t minSum = 0;
    for (int i = 0; i < 100; i++) {
        minSum += analogRead(GPIO_PEDAL_ADC);
        delay(10);
    }
    pedalMinADC = minSum / 100;

    Serial.printf("Min value recorded: %d\n", pedalMinADC);
    Serial.println("Now press pedal fully (waiting 5 seconds)...");

    delay(5000);

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

void InputHandler::updateDirectionSwitch() {
    DirectionMode newDirection = readDirectionSwitch();
    unsigned long currentTime = millis();

    // Check for state change
    if (newDirection != dirSwitch.current) {
        // State changed, check if enough time has passed (debounce)
        if (currentTime - dirSwitch.lastChangeTime > SWITCH_DEBOUNCE_MS) {
            dirSwitch.previous = dirSwitch.current;
            dirSwitch.current = newDirection;
            dirSwitch.changed = true;
            dirSwitch.lastChangeTime = currentTime;

            Serial.printf("Direction changed: %s -> %s\n",
                         DIR_CONFIGS[dirSwitch.previous].name,
                         DIR_CONFIGS[dirSwitch.current].name);
        }
    }
}

DirectionMode InputHandler::readDirectionSwitch() {
    // Read both switch positions (active low with pullup)
    bool forwardActive = !digitalRead(GPIO_DIR_FORWARD);
    bool reverseActive = !digitalRead(GPIO_DIR_REVERSE);

    // Determine direction based on switch state
    if (forwardActive && !reverseActive) {
        return DIR_FORWARD;
    } else if (!forwardActive && reverseActive) {
        return DIR_REVERSE;
    } else {
        // Both low or both high = neutral
        return DIR_NEUTRAL;
    }
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

