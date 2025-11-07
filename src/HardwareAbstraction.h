#ifndef HARDWARE_ABSTRACTION_H
#define HARDWARE_ABSTRACTION_H

#include <Arduino.h>

// ========================================
// Motor Driver Interface
// ========================================
class IMotorDriver {
public:
    virtual ~IMotorDriver() {}

    virtual bool init() = 0;
    virtual void setPWM(uint8_t channel, float percent) = 0;
    virtual void setDirection(uint8_t channel, bool forward) = 0;
    virtual void emergencyStop() = 0;
};

// Real motor driver implementation (Cytron MDD20A)
class MDD20ADriver : public IMotorDriver {
public:
    MDD20ADriver(uint8_t pwmPinLeft, uint8_t dirPinLeft,
                 uint8_t pwmPinRight, uint8_t dirPinRight) :
        pwmLeft(pwmPinLeft), dirLeft(dirPinLeft),
        pwmRight(pwmPinRight), dirRight(dirPinRight)
    {}

    bool init() override {
        // Setup PWM channels
        ledcSetup(0, 10000, 8); // 10kHz, 8-bit
        ledcSetup(1, 10000, 8);

        ledcAttachPin(pwmLeft, 0);
        ledcAttachPin(pwmRight, 1);

        pinMode(dirLeft, OUTPUT);
        pinMode(dirRight, OUTPUT);

        emergencyStop();
        return true;
    }

    void setPWM(uint8_t channel, float percent) override {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;

        uint8_t pwmValue = (uint8_t)((percent / 100.0f) * 255.0f);

        if (channel == 0) {
            ledcWrite(0, pwmValue);
        } else {
            ledcWrite(1, pwmValue);
        }
    }

    void setDirection(uint8_t channel, bool forward) override {
        if (channel == 0) {
            digitalWrite(dirLeft, forward ? HIGH : LOW);
        } else {
            digitalWrite(dirRight, forward ? HIGH : LOW);
        }
    }

    void emergencyStop() override {
        ledcWrite(0, 0);
        ledcWrite(1, 0);
    }

private:
    uint8_t pwmLeft, dirLeft, pwmRight, dirRight;
};

// Mock motor driver for testing
class MockMotorDriver : public IMotorDriver {
public:
    bool init() override {
        initialized = true;
        return true;
    }

    void setPWM(uint8_t channel, float percent) override {
        if (channel == 0) {
            leftPWM = percent;
        } else {
            rightPWM = percent;
        }
    }

    void setDirection(uint8_t channel, bool forward) override {
        if (channel == 0) {
            leftDirection = forward;
        } else {
            rightDirection = forward;
        }
    }

    void emergencyStop() override {
        leftPWM = 0;
        rightPWM = 0;
        stopCalled = true;
    }

    // Test helpers
    float getLeftPWM() const { return leftPWM; }
    float getRightPWM() const { return rightPWM; }
    bool getLeftDirection() const { return leftDirection; }
    bool getRightDirection() const { return rightDirection; }
    bool wasStopCalled() const { return stopCalled; }
    bool isInitialized() const { return initialized; }

private:
    bool initialized = false;
    float leftPWM = 0;
    float rightPWM = 0;
    bool leftDirection = true;
    bool rightDirection = true;
    bool stopCalled = false;
};

// ========================================
// Current Sensor Interface
// ========================================
class ICurrentSensor {
public:
    virtual ~ICurrentSensor() {}

    virtual bool init(uint8_t address) = 0;
    virtual float readVoltage() = 0;
    virtual float readCurrent() = 0;
    virtual float readPower() = 0;
};

// Real INA228 sensor implementation
#ifdef UNIT_TEST
// Simplified version for testing
class INA228Sensor : public ICurrentSensor {
public:
    bool init(uint8_t address) override { return true; }
    float readVoltage() override { return 60.0f; }
    float readCurrent() override { return 0.0f; }
    float readPower() override { return 0.0f; }
};
#else
// Production implementation with actual Adafruit library
#include <Adafruit_INA228.h>

class INA228Sensor : public ICurrentSensor {
public:
    bool init(uint8_t address) override {
        addr = address;
        if (!sensor.begin(address)) {
            return false;
        }
        return true;
    }

    float readVoltage() override {
        return sensor.readBusVoltage();
    }

    float readCurrent() override {
        return sensor.readCurrent();
    }

    float readPower() override {
        return sensor.readPower();
    }

private:
    Adafruit_INA228 sensor;
    uint8_t addr;
};
#endif

// Mock current sensor for testing
class MockCurrentSensor : public ICurrentSensor {
public:
    bool init(uint8_t address) override {
        this->address = address;
        initialized = true;
        return true;
    }

    float readVoltage() override { return voltage; }
    float readCurrent() override { return current; }
    float readPower() override { return voltage * current; }

    // Test helpers
    void setVoltage(float v) { voltage = v; }
    void setCurrent(float i) { current = i; }
    bool isInitialized() const { return initialized; }
    uint8_t getAddress() const { return address; }

private:
    uint8_t address = 0;
    bool initialized = false;
    float voltage = 0.0f;
    float current = 0.0f;
};

// ========================================
// GPIO Interface
// ========================================
class IGPIO {
public:
    virtual ~IGPIO() {}

    virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
    virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;
    virtual int digitalRead(uint8_t pin) = 0;
    virtual int analogRead(uint8_t pin) = 0;
};

// Real GPIO implementation
class ArduinoGPIO : public IGPIO {
public:
    void pinMode(uint8_t pin, uint8_t mode) override {
        ::pinMode(pin, mode);
    }

    void digitalWrite(uint8_t pin, uint8_t value) override {
        ::digitalWrite(pin, value);
    }

    int digitalRead(uint8_t pin) override {
        return ::digitalRead(pin);
    }

    int analogRead(uint8_t pin) override {
        return ::analogRead(pin);
    }
};

// Mock GPIO for testing
class MockGPIO : public IGPIO {
public:
    void pinMode(uint8_t pin, uint8_t mode) override {
        pinModes[pin] = mode;
    }

    void digitalWrite(uint8_t pin, uint8_t value) override {
        pinStates[pin] = value;
    }

    int digitalRead(uint8_t pin) override {
        return pinStates[pin];
    }

    int analogRead(uint8_t pin) override {
        return analogValues[pin];
    }

    // Test helpers
    void setDigitalValue(uint8_t pin, uint8_t value) { pinStates[pin] = value; }
    void setAnalogValue(uint8_t pin, int value) { analogValues[pin] = value; }
    uint8_t getPinMode(uint8_t pin) const { return pinModes[pin]; }
    uint8_t getPinState(uint8_t pin) const { return pinStates[pin]; }

private:
    uint8_t pinModes[40] = {0};
    uint8_t pinStates[40] = {0};
    int analogValues[40] = {0};
};

#endif // HARDWARE_ABSTRACTION_H
