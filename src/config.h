#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// GPIO PIN ASSIGNMENTS
// ========================================
// I2C Bus
#define GPIO_SDA                21
#define GPIO_SCL                22

// Motor Control
#define GPIO_MOTOR_PWM_LEFT     18  // Changed from 32
#define GPIO_MOTOR_PWM_RIGHT    12
#define GPIO_MOTOR_DIR_LEFT     19  // Changed from 14
#define GPIO_MOTOR_DIR_RIGHT    23  // Changed from 4

// Sensors
#define GPIO_PEDAL_ADC          4   // Changed: Use GPIO4 with voltage divider (0-3.3V)
#define GPIO_KEY_SWITCH         15

// User Interface
#define GPIO_ENCODER_CLK        25
#define GPIO_ENCODER_DT         26
#define GPIO_ENCODER_BTN        27
#define GPIO_KEY0               5   // Changed from 33
#define GPIO_KEY1               13  // Shift Down

// GPS
#define GPIO_GPS_RX             16
#define GPIO_GPS_TX             17

// ========================================
// I2C DEVICE ADDRESSES
// ========================================
#define INA228_BATTERY_ADDR     0x45    // MATEKSYS INA-BM (decimal 69)
#define INA228_MOTOR_LEFT_ADDR  0x40    // Left motor current monitor (changed from 0x41)
#define INA228_MOTOR_RIGHT_ADDR 0x42    // Right motor current monitor (changed from 0x44)
#define PCA9685_ADDR            0x70    // 16-channel PWM driver
#define SSD1306_ADDR            0x3C    // OLED display

// ========================================
// HARDWARE LIMITS
// ========================================
// Voltage Limits (Volts)
#define BATTERY_VOLTAGE_MIN     54.0f
#define BATTERY_VOLTAGE_MAX     63.0f
#define MOTOR_VOLTAGE_NOMINAL   30.0f

// Current Limits (Amps)
#define BATTERY_CURRENT_MAX     50.0f   // Increased for MATEKSYS INA-BM (204.8A capable)
#define MOTOR_CURRENT_MAX       20.0f
#define MOTOR_CURRENT_STALL     15.0f

// Speed Limits
#define SPEED_MAX_MPH           25.0f
#define MOTOR_RPM_MAX           25000

// ========================================
// PWM CONFIGURATION
// ========================================
#define PWM_FREQUENCY           10000   // 10kHz
#define PWM_RESOLUTION          8       // 8-bit (0-255)
#define PWM_CHANNEL_LEFT        0
#define PWM_CHANNEL_RIGHT       1

// ========================================
// SAFETY THRESHOLDS
// ========================================
#define MOTOR_IMBALANCE_WARN    15.0f   // % difference warning
#define MOTOR_IMBALANCE_FAULT   30.0f   // % difference fault
#define WATCHDOG_TIMEOUT_MS     1000    // Watchdog timeout
#define SPORT_PLUS_TIMEOUT_MS   10000   // 10 seconds max in Sport+

// Back-EMF Protection Thresholds
#define BACK_EMF_DECEL_RATE     25.0f   // Safe deceleration rate (%/sec)
#define DANGEROUS_DECEL_RATE    200.0f  // Threshold for excessive deceleration
#define BACK_EMF_MIN_TIME_MS    1000    // Minimum protection time
#define BACK_EMF_MAX_TIME_MS    10000   // Maximum protection time

// ========================================
// GEAR SYSTEM
// ========================================
enum GearMode {
    GEAR_PARK = 0,
    GEAR_1ST,
    GEAR_2ND,
    GEAR_3RD,
    GEAR_ECO,
    GEAR_SPORT_PLUS,
    GEAR_COUNT
};

// Gear Configuration Structure
struct GearConfig {
    float maxThrottle;      // Maximum throttle % for this gear
    float curveExponent;    // Throttle curve exponent
    const char* name;       // Display name
    uint32_t color;         // LED color (if applicable)
};

// Gear Configurations
static const GearConfig GEAR_CONFIGS[GEAR_COUNT] = {
    {0.0f,  0.0f, "P", 0xFF0000},      // Park - Red
    {40.0f, 0.5f, "1", 0x00FF00},      // 1st - Green  
    {70.0f, 1.0f, "2", 0x0000FF},      // 2nd - Blue
    {100.0f, 1.5f, "3", 0xFFFF00},     // 3rd - Yellow
    {60.0f, 0.7f, "E", 0x00FFFF},      // Eco - Cyan
    {110.0f, 2.0f, "S+", 0xFF00FF}     // Sport+ - Magenta
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
    BATTERY_OVERCURRENT = 2,
    MOTOR_LEFT_OVERCURRENT = 3,
    MOTOR_RIGHT_OVERCURRENT = 4,
    MOTOR_LEFT_STALL = 5,
    MOTOR_RIGHT_STALL = 6,
    MOTOR_IMBALANCE = 7,
    KEY_SWITCH_OFF = 8,
    WATCHDOG_TIMEOUT = 9
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

// ========================================
// DISPLAY CONFIGURATION
// ========================================
enum ScreenType {
    SCREEN_MAIN_DRIVE,
    SCREEN_DETAILED_METRICS,
    SCREEN_SETTINGS,
    SCREEN_WARNING,
    SCREEN_CALIBRATION
};

#define DISPLAY_WIDTH           128
#define DISPLAY_HEIGHT          64
#define DISPLAY_UPDATE_HZ       10

// ========================================
// GPS CONFIGURATION
// ========================================
#define GPS_BAUD_RATE           9600
#define GPS_UPDATE_RATE         1       // Hz

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
// DEBOUNCE TIMES
// ========================================
#define BUTTON_DEBOUNCE_MS      50
#define ENCODER_DEBOUNCE_MS     20

#endif // CONFIG_H