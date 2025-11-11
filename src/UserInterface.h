#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

class UserInterface {
public:
    UserInterface();
    
    // Initialization and main update
    bool init();
    void update();
    
    // Screen management
    void setScreen(ScreenType type);
    void cycleScreen();
    void checkCalibrationSequence();
    ScreenType getCurrentScreen();
    void handleEncoderRotation(int16_t delta);
    
    // Special displays
    void showStartupMessage();
    void showCalibrationScreen();
    void showGearChange(GearMode gear);
    void showWarning(const char* message);
    void showFault(uint8_t faultCode);
    void clearWarning();
    
    // Data sources (set by main loop)
    void setVoltage(float voltage);
    void setCurrent(float current);
    void setPower(float power);
    void setSpeed(float speed);
    void setGear(GearMode gear);
    void setBatterySOC(float percent);
    void setMotorData(float leftCurrent, float rightCurrent, float imbalance);
    void setGPSData(uint8_t satellites, bool fixed);
    void setGPSCoordinates(double latitude, double longitude);
    void setNavigationData(float bearing, float distance, float course);
    void setHomePosition(double lat, double lon);

    // Button handlers (A = brightness, B = light mode)
    void handleButtonA();  // Brightness control
    void handleButtonB();  // Light mode control
    uint8_t getBrightness();
    LightMode getLightMode();
    
private:
    // Display hardware
    Adafruit_SSD1306 display;
    bool displayReady;
    
    // Current state
    ScreenType currentScreen;
    bool warningActive;
    unsigned long warningStartTime;
    
    // Display data
    float batteryVoltage;
    float batteryCurrent;
    float batteryPower;
    float vehicleSpeed;
    float batterySOC;
    GearMode currentGear;
    float motorLeftCurrent;
    float motorRightCurrent;
    float motorImbalance;
    uint8_t gpsSatellites;
    bool gpsFixed;
    double gpsLatitude;
    double gpsLongitude;

    // Navigation data
    float bearingToHome;     // Degrees to home
    float distanceToHome;    // Miles to home
    float currentCourse;     // Current heading in degrees
    double homeLat;
    double homeLon;
    bool homePositionSet;

    // User adjustable settings
    uint8_t displayBrightness;  // 0-100%
    LightMode lightMode;
    
    // Animation state
    bool gearChangeAnimation;
    unsigned long gearChangeStartTime;
    GearMode animationGear;
    
    // Calibration access
    uint8_t encoderPressCount;
    unsigned long lastEncoderPress;
    bool calibrationUnlocked;
    
    // Settings adjustment
    uint8_t selectedSettingIndex;
    int16_t settingAdjustmentValue;
    
    // Screen drawing methods
    void drawMainDriveScreen();
    void drawDetailedMetricsScreen();
    void drawNavigationScreen();
    void drawSettingsScreen();
    void drawWarningScreen();
    void drawCalibrationScreen();

    // Helper drawing methods
    void drawHeader();
    void drawGearIndicator(int16_t x, int16_t y, GearMode gear, bool large = false);
    void drawBatteryGauge(int16_t x, int16_t y, float percent);
    void drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, float percent);
    void drawValue(int16_t x, int16_t y, float value, const char* unit, uint8_t decimals = 1);
    void drawCompassNeedle(int16_t centerX, int16_t centerY, int16_t radius, float angle);

    // Animation methods
    void updateGearChangeAnimation();
    void updateWarningAnimation();
    
    // Utility methods
    const char* getFaultString(uint8_t faultCode);
    uint16_t getGearColor(GearMode gear);
};

#endif // USER_INTERFACE_H