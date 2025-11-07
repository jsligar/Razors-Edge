#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "config.h"

// Forward declarations to avoid circular includes
class PowerManager;
class MotorController;
class Navigation;
class LightController;
class SafetySystem;
class UserInterface;

// WiFi Configuration
#define WIFI_AP_SSID "RazorsEdge-Controller"
#define WIFI_AP_PASSWORD "RazorEdge2025"
#define WIFI_TIMEOUT_MS 10000
#define WEB_SERVER_PORT 80
#define WS_UPDATE_INTERVAL 1000  // WebSocket update interval in ms

// Web interface modes
enum WiFiMode {
    WEB_WIFI_OFF,
    WEB_WIFI_AP,       // Access Point (default)
    WEB_WIFI_STA,      // Connect to existing network
    WEB_WIFI_DUAL      // Both AP and Station
};

struct WiFiConfig {
    WiFiMode mode;
    char stationSSID[32];
    char stationPassword[64];
    char apSSID[32];
    char apPassword[64];
    IPAddress staticIP;
    IPAddress gateway;
    IPAddress subnet;
    bool useStaticIP;
};

struct WebTelemetry {
    // Power data
    float batteryVoltage;
    float batteryCurrent;
    float batteryPower;
    float batterySOC;
    float motorCurrentLeft;
    float motorCurrentRight;
    
    // Motor data
    float currentSpeedLeft;
    float currentSpeedRight;
    float targetSpeedLeft;
    float targetSpeedRight;
    uint8_t currentGear;
    bool motorEnabled;
    
    // GPS data
    bool gpsFixed;
    double latitude;
    double longitude;
    float gpsSpeed;
    float course;
    uint8_t satellites;
    float hdop;
    float odometer;
    float tripMeter;
    
    // Geofencing data
    bool inSafeZone;
    bool inProhibitedZone;
    bool speedLimited;
    float currentSpeedLimit;
    float distanceToHome;
    uint8_t activeFenceId;
    uint16_t totalViolations;
    
    // Safety data
    bool keySwitch;
    bool safetyFault;
    float systemTemp;
    unsigned long uptime;
};

class WebInterface {
public:
    WebInterface();
    
    // Initialization and control
    bool init();
    void update();
    void enable(bool enabled);
    bool isEnabled();
    
    // WiFi management
    bool startAccessPoint();
    bool connectToNetwork(const char* ssid, const char* password);
    void disconnect();
    WiFiMode getCurrentMode();
    IPAddress getLocalIP();
    String getSSID();
    
    // Configuration
    void setWiFiConfig(const WiFiConfig& config);
    WiFiConfig getWiFiConfig();
    void saveConfig();
    bool loadConfig();
    
    // Module references (set by main.cpp)
    void setModuleReferences(PowerManager* pwr, MotorController* motors, 
                           Navigation* nav, LightController* lights, 
                           SafetySystem* safety, UserInterface* interface);
    
    // Web API
    String getStatusJSON();
    String getTelemetryJSON();
    String getConfigJSON();
    bool processConfigUpdate(const String& json);
    
    // Geofencing API
    bool addGeofenceFromJSON(const String& json);
    bool removeGeofenceFromJSON(const String& json);
    String getGeofencesJSON();
    
private:
    // Module references
    PowerManager* power;
    MotorController* motorController;
    Navigation* navigation;
    LightController* lightController;
    SafetySystem* safetySystem;
    UserInterface* ui;
    
    // WiFi and web server
    WebServer server;
    WiFiConfig config;
    bool enabled;
    bool serverStarted;
    unsigned long lastTelemetryUpdate;
    unsigned long lastClientActivity;
    
    // Data
    WebTelemetry telemetry;
    
    // Web server handlers
    void handleRoot();
    void handleJS();
    void handleAPI();
    void handleTelemetry();
    void handleConfig();
    void handleGeofencing();
    void handleFileUpload();
    void handleNotFound();
    
    // API endpoints
    void handleGetStatus();
    void handleGetTelemetry();
    void handleGetConfig();
    void handleSetConfig();
    void handleGetGeofences();
    void handleAddGeofence();
    void handleRemoveGeofence();
    void handleEmergencyStop();
    void handleSetGear();
    void handleSetSpeed();
    void handleResetTrip();
    void handleSetHome();
    void handleCalibration();
    
    // Utility methods
    void updateTelemetryData();
    void setupWebServer();
    void sendCORSHeaders();
    String getContentType(String filename);
    bool handleFileRead(String path);
    
    // Configuration management
    void setDefaultConfig();
    String configToJSON();
    bool configFromJSON(const String& json);
    
    // Network utilities
    void printNetworkInfo();
    bool isValidIP(const String& ip);
    
    // Security
    bool authenticateRequest();
    void logWebAccess(const String& endpoint, const String& clientIP);
};

#endif // WEB_INTERFACE_H