#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// Configuration structure for persistent settings
struct SystemConfig {
    // Calibration data
    uint16_t pedalAdcMin;
    uint16_t pedalAdcMax;
    uint16_t pedalDeadband;

    // Safety limits
    float batteryVoltageMin;
    float batteryVoltageMax;
    float batteryCurrentMax;
    float motorCurrentMax;

    // WiFi settings
    char wifiSSID[32];
    char wifiPassword[64];
    bool wifiAPMode;

    // Display settings
    uint8_t displayBrightness;
    bool displayAutoOff;
    uint16_t displayTimeout;

    // Motor tuning
    float accelRate;
    float decelRate;
    float motorBalanceLeft;  // Trim for motor imbalance
    float motorBalanceRight;

    // System settings
    LogLevel logLevel;
    bool debugMode;

    // Default constructor with sane defaults
    SystemConfig() :
        pedalAdcMin(PEDAL_ADC_MIN),
        pedalAdcMax(PEDAL_ADC_MAX),
        pedalDeadband(PEDAL_DEADBAND),
        batteryVoltageMin(BATTERY_VOLTAGE_MIN),
        batteryVoltageMax(BATTERY_VOLTAGE_MAX),
        batteryCurrentMax(BATTERY_CURRENT_MAX),
        motorCurrentMax(MOTOR_CURRENT_MAX),
        wifiAPMode(true),
        displayBrightness(255),
        displayAutoOff(false),
        displayTimeout(30000),
        accelRate(50.0f),
        decelRate(100.0f),
        motorBalanceLeft(1.0f),
        motorBalanceRight(1.0f),
        logLevel(LOG_LEVEL_INFO),
        debugMode(false)
    {
        strcpy(wifiSSID, "RazorsEdge-Controller");
        strcpy(wifiPassword, "RazorEdge2025");
    }
};

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // Initialize NVS and load configuration
    bool init() {
        if (!preferences.begin("razors-edge", false)) {
            Serial.println("Failed to initialize NVS storage");
            return false;
        }

        loadConfig();
        return true;
    }

    // Load configuration from NVS
    void loadConfig() {
        config.pedalAdcMin = preferences.getUShort("pedal_min", PEDAL_ADC_MIN);
        config.pedalAdcMax = preferences.getUShort("pedal_max", PEDAL_ADC_MAX);
        config.pedalDeadband = preferences.getUShort("pedal_dead", PEDAL_DEADBAND);

        config.batteryVoltageMin = preferences.getFloat("batt_v_min", BATTERY_VOLTAGE_MIN);
        config.batteryVoltageMax = preferences.getFloat("batt_v_max", BATTERY_VOLTAGE_MAX);
        config.batteryCurrentMax = preferences.getFloat("batt_a_max", BATTERY_CURRENT_MAX);
        config.motorCurrentMax = preferences.getFloat("motor_a_max", MOTOR_CURRENT_MAX);

        preferences.getString("wifi_ssid", config.wifiSSID, sizeof(config.wifiSSID));
        preferences.getString("wifi_pass", config.wifiPassword, sizeof(config.wifiPassword));
        config.wifiAPMode = preferences.getBool("wifi_ap", true);

        config.displayBrightness = preferences.getUChar("disp_bright", 255);
        config.displayAutoOff = preferences.getBool("disp_auto", false);
        config.displayTimeout = preferences.getUShort("disp_timeout", 30000);

        config.accelRate = preferences.getFloat("accel_rate", 50.0f);
        config.decelRate = preferences.getFloat("decel_rate", 100.0f);
        config.motorBalanceLeft = preferences.getFloat("motor_bal_l", 1.0f);
        config.motorBalanceRight = preferences.getFloat("motor_bal_r", 1.0f);

        config.logLevel = (LogLevel)preferences.getUChar("log_level", LOG_LEVEL_INFO);
        config.debugMode = preferences.getBool("debug_mode", false);

        // If SSID is empty, use defaults
        if (strlen(config.wifiSSID) == 0) {
            strcpy(config.wifiSSID, "RazorsEdge-Controller");
            strcpy(config.wifiPassword, "RazorEdge2025");
        }
    }

    // Save configuration to NVS
    bool saveConfig() {
        preferences.putUShort("pedal_min", config.pedalAdcMin);
        preferences.putUShort("pedal_max", config.pedalAdcMax);
        preferences.putUShort("pedal_dead", config.pedalDeadband);

        preferences.putFloat("batt_v_min", config.batteryVoltageMin);
        preferences.putFloat("batt_v_max", config.batteryVoltageMax);
        preferences.putFloat("batt_a_max", config.batteryCurrentMax);
        preferences.putFloat("motor_a_max", config.motorCurrentMax);

        preferences.putString("wifi_ssid", config.wifiSSID);
        preferences.putString("wifi_pass", config.wifiPassword);
        preferences.putBool("wifi_ap", config.wifiAPMode);

        preferences.putUChar("disp_bright", config.displayBrightness);
        preferences.putBool("disp_auto", config.displayAutoOff);
        preferences.putUShort("disp_timeout", config.displayTimeout);

        preferences.putFloat("accel_rate", config.accelRate);
        preferences.putFloat("decel_rate", config.decelRate);
        preferences.putFloat("motor_bal_l", config.motorBalanceLeft);
        preferences.putFloat("motor_bal_r", config.motorBalanceRight);

        preferences.putUChar("log_level", (uint8_t)config.logLevel);
        preferences.putBool("debug_mode", config.debugMode);

        return true;
    }

    // Reset to factory defaults
    void resetToDefaults() {
        preferences.clear();
        config = SystemConfig(); // Reset to defaults
        saveConfig();
    }

    // Get configuration
    SystemConfig& getConfig() {
        return config;
    }

    // Calibration helpers
    void setPedalCalibration(uint16_t min, uint16_t max, uint16_t deadband) {
        config.pedalAdcMin = min;
        config.pedalAdcMax = max;
        config.pedalDeadband = deadband;
        saveConfig();
    }

    void setMotorBalance(float left, float right) {
        config.motorBalanceLeft = left;
        config.motorBalanceRight = right;
        saveConfig();
    }

    void setWiFiCredentials(const char* ssid, const char* password, bool apMode) {
        strncpy(config.wifiSSID, ssid, sizeof(config.wifiSSID) - 1);
        strncpy(config.wifiPassword, password, sizeof(config.wifiPassword) - 1);
        config.wifiAPMode = apMode;
        saveConfig();
    }

    void setLogLevel(LogLevel level) {
        config.logLevel = level;
        saveConfig();
    }

private:
    ConfigManager() {}
    ~ConfigManager() {
        preferences.end();
    }

    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    Preferences preferences;
    SystemConfig config;
};

// Global convenience reference
#define Config ConfigManager::getInstance().getConfig()

#endif // CONFIG_MANAGER_H
