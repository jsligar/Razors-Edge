#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// GPIO PIN ASSIGNMENTS - TRACTOR VERSION
// ========================================
// Motor Control
#define GPIO_MOTOR_PWM_LEFT     18
#define GPIO_MOTOR_PWM_RIGHT    12
#define GPIO_MOTOR_DIR_LEFT     19
#define GPIO_MOTOR_DIR_RIGHT    23

// Sensors
#define GPIO_PEDAL_ADC          4   // Pedal position (0-3.3V)
#define GPIO_KEY_SWITCH         15

// Direction Switch (3-position: Forward/Neutral/Reverse)
#define GPIO_DIR_FORWARD        25  // Forward position
#define GPIO_DIR_REVERSE        26  // Reverse position
// Neutral = both LOW

// ========================================
// I2C DEVICE ADDRESSES - TRACTOR VERSION
// ========================================
#define PCA9685_ADDR            0x70    // 16-channel PWM driver (for lights if used)

// ========================================
// HARDWARE LIMITS - TRACTOR VERSION
// ========================================
// No voltage monitoring in tractor version

// Speed Limits
#define SPEED_MAX_MPH           5.0f    // Tractor speed limit
#define MOTOR_RPM_MAX           5000

// ========================================
// PWM CONFIGURATION
// ========================================
#define PWM_FREQUENCY           10000   // 10kHz
#define PWM_RESOLUTION          8       // 8-bit (0-255)
#define PWM_CHANNEL_LEFT        0
#define PWM_CHANNEL_RIGHT       1

// ========================================
// SAFETY THRESHOLDS - TRACTOR VERSION
// ========================================
#define WATCHDOG_TIMEOUT_MS     1000    // Watchdog timeout

// Back-EMF Protection Thresholds
#define BACK_EMF_DECEL_RATE     25.0f   // Safe deceleration rate (%/sec)
#define DANGEROUS_DECEL_RATE    200.0f  // Threshold for excessive deceleration
#define BACK_EMF_MIN_TIME_MS    1000    // Minimum protection time
#define BACK_EMF_MAX_TIME_MS    10000   // Maximum protection time

// ========================================
// DIRECTION SYSTEM - TRACTOR VERSION
// ========================================
enum DirectionMode {
    DIR_NEUTRAL = 0,
    DIR_FORWARD,
    DIR_REVERSE,
    DIR_COUNT
};

// Direction Configuration Structure
struct DirectionConfig {
    float maxThrottle;      // Maximum throttle % for this direction
    const char* name;       // Display name
    uint32_t color;         // LED color (if applicable)
};

// Direction Configurations
static const DirectionConfig DIR_CONFIGS[DIR_COUNT] = {
    {0.0f,   "N", 0xFF0000},    // Neutral - Red
    {100.0f, "F", 0x00FF00},    // Forward - Green
    {50.0f,  "R", 0xFFFF00}     // Reverse - Yellow (limited speed)
};

// ========================================
// SYSTEM STATES
// ========================================
enum SystemState {
    STATE_INIT,
    STATE_STANDBY,
    STATE_READY,
    STATE_FORWARD,
    STATE_NEUTRAL,
    STATE_REVERSE,
    STATE_FAULT
};

enum FaultCode {
    NO_FAULT = 0,
    LOW_BATTERY = 1,
    KEY_SWITCH_OFF = 2,
    WATCHDOG_TIMEOUT = 3
};

// ========================================
// LIGHTING SYSTEM
// ========================================
enum LightMode {
    LIGHT_OFF,
    LIGHT_AUTO,
    LIGHT_ALWAYS_ON,
    LIGHT_DIM
};

// Light Zone Assignments (PCA9685 channels)
#define LIGHT_FRONT_LEFT        0
#define LIGHT_FRONT_RIGHT       1
#define LIGHT_REAR_LEFT         2
#define LIGHT_REAR_RIGHT        3

// Note: No OLED display or GPS in tractor version

// ========================================
// CALIBRATION VALUES
// ========================================
#define PEDAL_ADC_MIN           100     // Minimum pedal ADC value
#define PEDAL_ADC_MAX           4000    // Maximum pedal ADC value
#define PEDAL_DEADBAND          50      // ADC units of deadband

// ========================================
// TIMING CONSTANTS
// ========================================
#define MAIN_LOOP_DELAY_MS      1       // 1ms = ~1kHz loop rate
#define UI_UPDATE_INTERVAL_MS   100     // 10Hz display update
#define SAFETY_UPDATE_INTERVAL_MS 100   // 10Hz safety checks
#define POWER_UPDATE_INTERVAL_MS  50    // 20Hz power monitoring

// ========================================
// DEBOUNCE TIMES - TRACTOR VERSION
// ========================================
#define SWITCH_DEBOUNCE_MS      50  // Direction switch debounce

#endif // CONFIG_H