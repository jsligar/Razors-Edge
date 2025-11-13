#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// GPIO PIN ASSIGNMENTS - ESP32-S3-WROOM-1
// ========================================
// I2C Bus
#define GPIO_SDA                8   // ESP32-S3 Pin 13 (was GPIO21 on ESP32)
#define GPIO_SCL                9   // ESP32-S3 Pin 14 (was GPIO22 on ESP32)

// Motor Control
#define GPIO_MOTOR_PWM_LEFT     10  // ESP32-S3 Pin 15, LEDC Channel 0, 10kHz (was GPIO18)
#define GPIO_MOTOR_PWM_RIGHT    11  // ESP32-S3 Pin 16, LEDC Channel 1, 10kHz (was GPIO12)
#define GPIO_MOTOR_DIR_LEFT     12  // ESP32-S3 Pin 17 (was GPIO19)
#define GPIO_MOTOR_DIR_RIGHT    13  // ESP32-S3 Pin 18 (was GPIO23)

// Sensors
#define GPIO_PEDAL_ADC          1   // ESP32-S3 Pin 4 (ADC1_CH0) - Improved ADC accuracy vs ESP32
#define GPIO_KEY_SWITCH         16  // ESP32-S3 Pin 22 (was GPIO15)

// User Interface
#define GPIO_ENCODER_CLK        4   // ESP32-S3 Pin 7, Encoder track A (was GPIO25)
#define GPIO_ENCODER_DT         5   // ESP32-S3 Pin 8, Encoder track B (was GPIO26)
#define GPIO_ENCODER_BTN        6   // ESP32-S3 Pin 9, Encoder push button (was GPIO27)
#define GPIO_KEY0               7   // ESP32-S3 Pin 12, Shift Up button (was GPIO14)
#define GPIO_KEY1               15  // ESP32-S3 Pin 21, Shift Down button (was GPIO13)

// GPS
#define GPIO_GPS_RX             17  // ESP32-S3 Pin 23, GPS TX → ESP RX (was GPIO16)
#define GPIO_GPS_TX             18  // ESP32-S3 Pin 24, ESP TX → GPS RX (was GPIO17)

// ========================================
// I2C DEVICE ADDRESSES
// ========================================
#define INA228_BATTERY_ADDR     0x45    // MATEKSYS INA-BM (decimal 69)
#define INA228_MOTOR_LEFT_ADDR  0x41    // Left motor current monitor
#define INA228_MOTOR_RIGHT_ADDR 0x44    // Right motor current monitor
#define PCA9685_ADDR            0x40    // 16-channel PWM driver (default address)
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
    GEAR_REVERSE,
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
    {35.0f, 0.6f, "R", 0xFF8800},      // Reverse - Orange (cautious speed)
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

// ========================================
// REGENERATIVE BRAKING SYSTEM
// ========================================
enum RegenMode {
    REGEN_OFF,      // No regenerative braking
    REGEN_LOW,      // 15% regen strength
    REGEN_MEDIUM,   // 30% regen strength
    REGEN_HIGH      // 50% regen strength
};

// Regen configuration
#define REGEN_VOLTAGE_MAX       63.0f   // Don't regen if battery above this
#define REGEN_VOLTAGE_CUTOFF    62.5f   // Start tapering regen at this voltage
#define REGEN_MIN_SPEED         2.0f    // Minimum speed for regen (MPH)
#define REGEN_THROTTLE_DEADBAND 5.0f    // % deadband to detect decel

// ========================================
// LIGHTING SYSTEM
// ========================================
// Main Headlights (4 independent headlights)
#define LIGHT_HEAD_1            0       // Headlight zone 1
#define LIGHT_HEAD_2            1       // Headlight zone 2
#define LIGHT_HEAD_3            2       // Headlight zone 3
#define LIGHT_HEAD_4            3       // Headlight zone 4

// Center Light
#define LIGHT_CENTER            4       // Big center light

// Tail Lights
#define LIGHT_TAIL_LEFT         5       // Left tail light
#define LIGHT_TAIL_RIGHT        6       // Right tail light

// Precision Lighting (4 additional channels)
#define LIGHT_TURN_LEFT         8       // Left turn signal
#define LIGHT_TURN_RIGHT        9       // Right turn signal
#define LIGHT_BRAKE             10      // Brake lights (high intensity)
#define LIGHT_REVERSE           11      // Reverse/backup lights

// Total lights in use: channels 0-6 (main), 8-11 (auxiliary)
#define TOTAL_MAIN_LIGHTS       7       // For startup sequence

// ========================================
// DISPLAY CONFIGURATION
// ========================================
enum ScreenType {
    SCREEN_MAIN_DRIVE,
    SCREEN_DETAILED_METRICS,
    SCREEN_NAVIGATION,
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
#define BUTTON_DEBOUNCE_MS      150     // Increased from 50ms - prevents gear shift bounce
#define ENCODER_DEBOUNCE_MS     20

#endif // CONFIG_H