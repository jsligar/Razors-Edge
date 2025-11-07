#include "WebInterface.h"
#include "PowerManager.h"
#include "MotorController.h"
#include "Navigation.h"
#include "LightController.h"
#include "SafetySystem.h"
#include "UserInterface.h"

WebInterface::WebInterface() :
    server(WEB_SERVER_PORT),
    power(nullptr),
    motorController(nullptr),
    navigation(nullptr),
    lightController(nullptr),
    safetySystem(nullptr),
    enabled(false),
    serverStarted(false),
    lastTelemetryUpdate(0),
    lastClientActivity(0)
{
    setDefaultConfig();
    memset(&telemetry, 0, sizeof(telemetry));
}

bool WebInterface::init() {
    Serial.println("Initializing Web Interface...");
    
    // Initialize SPIFFS for web files
    if (!SPIFFS.begin(true)) {
        Serial.println("⚠ SPIFFS initialization failed");
        return false;
    }
    
    // Load saved configuration
    loadConfig();
    
    // Start WiFi based on configuration
    if (config.mode == WEB_WIFI_AP || config.mode == WEB_WIFI_DUAL) {
        if (!startAccessPoint()) {
            Serial.println("⚠ Failed to start Access Point");
            return false;
        }
    }
    
    if (config.mode == WEB_WIFI_STA || config.mode == WEB_WIFI_DUAL) {
        if (strlen(config.stationSSID) > 0) {
            connectToNetwork(config.stationSSID, config.stationPassword);
        }
    }
    
    // Setup web server routes
    setupWebServer();
    
    // Start web server
    server.begin();
    serverStarted = true;
    enabled = true;
    
    Serial.println("✓ Web Interface initialized");
    printNetworkInfo();
    
    return true;
}

void WebInterface::update() {
    if (!enabled || !serverStarted) return;
    
    // Handle web server requests
    server.handleClient();
    
    // Update telemetry data periodically
    unsigned long currentTime = millis();
    if (currentTime - lastTelemetryUpdate >= WS_UPDATE_INTERVAL) {
        updateTelemetryData();
        lastTelemetryUpdate = currentTime;
    }
    
    // Monitor WiFi connection
    if (config.mode == WEB_WIFI_STA || config.mode == WEB_WIFI_DUAL) {
        if (WiFi.status() != WL_CONNECTED) {
            // Try to reconnect if disconnected
            static unsigned long lastReconnectAttempt = 0;
            if (currentTime - lastReconnectAttempt > 30000) { // Try every 30 seconds
                Serial.println("WiFi disconnected, attempting reconnect...");
                connectToNetwork(config.stationSSID, config.stationPassword);
                lastReconnectAttempt = currentTime;
            }
        }
    }
}

bool WebInterface::startAccessPoint() {
    Serial.printf("Starting Access Point: %s\n", config.apSSID);
    
    // Configure AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.apSSID, config.apPassword);
    
    // Set static IP for AP
    IPAddress apIP(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apIP, subnet);
    
    // Wait for AP to start
    delay(1000);
    
    IPAddress ip = WiFi.softAPIP();
    Serial.printf("✓ Access Point started - IP: %s\n", ip.toString().c_str());
    
    return true;
}

bool WebInterface::connectToNetwork(const char* ssid, const char* password) {
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    
    WiFi.begin(ssid, password);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT_MS) {
        delay(100);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n✓ Connected to %s - IP: %s\n", 
                     ssid, WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.printf("\n⚠ Failed to connect to %s\n", ssid);
        return false;
    }
}

void WebInterface::setupWebServer() {
    // Serve static files
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/js/dashboard.js", HTTP_GET, [this]() { handleJS(); });
    
    // API endpoints
    server.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });
    server.on("/api/telemetry", HTTP_GET, [this]() { handleGetTelemetry(); });
    server.on("/api/config", HTTP_GET, [this]() { handleGetConfig(); });
    server.on("/api/config", HTTP_POST, [this]() { handleSetConfig(); });
    
    // Geofencing API
    server.on("/api/geofences", HTTP_GET, [this]() { handleGetGeofences(); });
    server.on("/api/geofences", HTTP_POST, [this]() { handleAddGeofence(); });
    server.on("/api/geofences", HTTP_DELETE, [this]() { handleRemoveGeofence(); });
    
    // Control API
    server.on("/api/emergency-stop", HTTP_POST, [this]() { handleEmergencyStop(); });
    server.on("/api/set-gear", HTTP_POST, [this]() { handleSetGear(); });
    server.on("/api/set-speed", HTTP_POST, [this]() { handleSetSpeed(); });
    server.on("/api/reset-trip", HTTP_POST, [this]() { handleResetTrip(); });
    server.on("/api/set-home", HTTP_POST, [this]() { handleSetHome(); });
    server.on("/api/calibration", HTTP_POST, [this]() { handleCalibration(); });
    
    // Handle file requests
    server.onNotFound([this]() { 
        if (!handleFileRead(server.uri())) {
            handleNotFound();
        }
    });
}

void WebInterface::handleRoot() {
    sendCORSHeaders();
    
    String html = "";
    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Razors Edge Controller</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #1a1a1a; color: #fff; }";
    html += ".dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }";
    html += ".card { background: #2d2d2d; border-radius: 10px; padding: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }";
    html += ".card h3 { margin-top: 0; color: #4CAF50; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }";
    html += ".status-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }";
    html += ".status-item { background: #3a3a3a; padding: 10px; border-radius: 5px; text-align: center; }";
    html += ".status-label { font-size: 0.9em; color: #ccc; margin-bottom: 5px; }";
    html += ".status-value { font-size: 1.2em; font-weight: bold; }";
    html += ".status-good { color: #4CAF50; }";
    html += ".status-warning { color: #FF9800; }";
    html += ".status-danger { color: #F44336; }";
    html += ".control-btn { background: #4CAF50; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; margin: 5px; }";
    html += ".control-btn:hover { background: #45a049; }";
    html += ".emergency-btn { background: #F44336; }";
    html += ".emergency-btn:hover { background: #da190b; }";
    html += "#map { height: 300px; background: #3a3a3a; border-radius: 5px; display: flex; align-items: center; justify-content: center; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>Razors Edge Controller Dashboard</h1>";
    html += "<div class='dashboard'>";
    
    // Power Status Card
    html += "<div class='card'>";
    html += "<h3>Power System</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div class='status-label'>Battery Voltage</div><div class='status-value' id='battery-voltage'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Battery Current</div><div class='status-value' id='battery-current'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Power Draw</div><div class='status-value' id='power-draw'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>State of Charge</div><div class='status-value' id='battery-soc'>--</div></div>";
    html += "</div></div>";
    
    // Motor Status Card
    html += "<div class='card'>";
    html += "<h3>Motor Control</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div class='status-label'>Current Gear</div><div class='status-value' id='current-gear'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Motor Status</div><div class='status-value' id='motor-status'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Left Motor</div><div class='status-value' id='left-motor'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Right Motor</div><div class='status-value' id='right-motor'>--</div></div>";
    html += "</div></div>";
    
    // GPS/Navigation Card  
    html += "<div class='card'>";
    html += "<h3>GPS & Navigation</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div class='status-label'>GPS Status</div><div class='status-value' id='gps-status'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Speed</div><div class='status-value' id='gps-speed'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Satellites</div><div class='status-value' id='satellites'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Odometer</div><div class='status-value' id='odometer'>--</div></div>";
    html += "</div>";
    html += "<div id='map'>GPS Map Loading...</div>";
    html += "</div>";
    
    // Geofencing Card
    html += "<div class='card'>";
    html += "<h3>Geofencing</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div class='status-label'>Safe Zone</div><div class='status-value' id='safe-zone'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Speed Limit</div><div class='status-value' id='speed-limit'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Distance to Home</div><div class='status-value' id='distance-home'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Violations</div><div class='status-value' id='violations'>--</div></div>";
    html += "</div></div>";
    
    // Controls Card
    html += "<div class='card'>";
    html += "<h3>Controls</h3>";
    html += "<button class='control-btn emergency-btn' onclick='emergencyStop()'>EMERGENCY STOP</button>";
    html += "<button class='control-btn' onclick='setGear(\"park\")'>Park</button>";
    html += "<button class='control-btn' onclick='setGear(\"eco\")'>Eco</button>";
    html += "<button class='control-btn' onclick='resetTrip()'>Reset Trip</button>";
    html += "<button class='control-btn' onclick='setHome()'>Set Home</button>";
    html += "<button class='control-btn' onclick='showCalibration()'>📺 OLED Test</button>";
    html += "</div>";
    
    // System Status Card
    html += "<div class='card'>";
    html += "<h3>System Status</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div class='status-label'>Key Switch</div><div class='status-value' id='key-switch'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Safety Status</div><div class='status-value' id='safety-status'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Uptime</div><div class='status-value' id='uptime'>--</div></div>";
    html += "<div class='status-item'><div class='status-label'>Connection</div><div class='status-value status-good' id='connection'>Connected</div></div>";
    html += "</div></div>";
    
    html += "</div>"; // Close dashboard
    html += "<script src='/js/dashboard.js'></script>";
    html += "</body>";
    html += "</html>";
    
    server.send(200, "text/html", html);
}

void WebInterface::handleGetTelemetry() {
    sendCORSHeaders();
    updateTelemetryData();
    server.send(200, "application/json", getTelemetryJSON());
}

void WebInterface::handleGetStatus() {
    sendCORSHeaders();
    server.send(200, "application/json", getStatusJSON());
}

void WebInterface::handleEmergencyStop() {
    sendCORSHeaders();
    
    if (motorController) {
        motorController->emergencyStop();
        Serial.println("🚨 Emergency stop triggered via web interface");
        server.send(200, "application/json", "{\"status\":\"emergency_stop_activated\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"motor_controller_not_available\"}");
    }
}

void WebInterface::handleSetGear() {
    sendCORSHeaders();
    
    if (!motorController) {
        server.send(500, "application/json", "{\"error\":\"motor_controller_not_available\"}");
        return;
    }
    
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, body);
    
    String gear = doc["gear"];
    GearMode gearMode = GEAR_PARK;
    
    if (gear == "park") gearMode = GEAR_PARK;
    else if (gear == "1st") gearMode = GEAR_1ST;
    else if (gear == "2nd") gearMode = GEAR_2ND;
    else if (gear == "3rd") gearMode = GEAR_3RD;
    else if (gear == "eco") gearMode = GEAR_ECO;
    else if (gear == "sport") gearMode = GEAR_SPORT_PLUS;
    
    motorController->setGear(gearMode);
    Serial.printf("Gear changed to %s via web interface\n", gear.c_str());
    
    server.send(200, "application/json", "{\"status\":\"gear_changed\"}");
}

void WebInterface::handleResetTrip() {
    sendCORSHeaders();
    
    if (navigation) {
        navigation->resetTrip();
        Serial.println("Trip meter reset via web interface");
        server.send(200, "application/json", "{\"status\":\"trip_reset\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"navigation_not_available\"}");
    }
}

void WebInterface::handleSetHome() {
    sendCORSHeaders();
    
    if (navigation && navigation->isGPSFixed()) {
        navigation->setHomePosition(navigation->getLatitude(), navigation->getLongitude());
        Serial.println("Home position set via web interface");
        server.send(200, "application/json", "{\"status\":\"home_set\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"gps_not_fixed\"}");
    }
}

void WebInterface::handleCalibration() {
    sendCORSHeaders();
    
    // Activate calibration screen on OLED
    if (ui) {
        ui->showCalibrationScreen();
        Serial.println("📺 Calibration screen activated via web interface");
        server.send(200, "application/json", "{\"status\":\"calibration_activated\",\"message\":\"OLED now showing calibration pattern. Please take a photo!\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"ui_not_available\"}");
    }
}

void WebInterface::updateTelemetryData() {
    // Update power data
    if (power) {
        telemetry.batteryVoltage = power->getBatteryVoltage();
        telemetry.batteryCurrent = power->getBatteryCurrent();
        telemetry.batteryPower = power->getBatteryPower();
        telemetry.batterySOC = power->getBatterySOC();
        telemetry.motorCurrentLeft = power->getMotorLeftCurrent();
        telemetry.motorCurrentRight = power->getMotorRightCurrent();
    }
    
    // Update motor data
    if (motorController) {
        telemetry.currentSpeedLeft = motorController->getCurrentSpeedLeft();
        telemetry.currentSpeedRight = motorController->getCurrentSpeedRight();
        telemetry.targetSpeedLeft = motorController->getTargetSpeedLeft();
        telemetry.targetSpeedRight = motorController->getTargetSpeedRight();
        telemetry.currentGear = (uint8_t)motorController->getCurrentGear();
        telemetry.motorEnabled = !motorController->shouldStopForGeofence();
    }
    
    // Update GPS data
    if (navigation) {
        telemetry.gpsFixed = navigation->isGPSFixed();
        telemetry.latitude = navigation->getLatitude();
        telemetry.longitude = navigation->getLongitude();
        telemetry.gpsSpeed = navigation->getSpeed();
        telemetry.course = navigation->getCourse();
        telemetry.satellites = navigation->getSatellites();
        telemetry.hdop = navigation->getHDOP();
        telemetry.odometer = navigation->getOdometer();
        telemetry.tripMeter = navigation->getTripMeter();
        
        // Update geofencing data
        telemetry.inSafeZone = navigation->isInSafeZone();
        telemetry.inProhibitedZone = navigation->isInProhibitedZone();
        telemetry.speedLimited = navigation->shouldLimitSpeedForGeofence();
        telemetry.currentSpeedLimit = navigation->getCurrentSpeedLimit();
        telemetry.distanceToHome = navigation->getDistanceToHome();
        
        GeofenceStatus geoStatus = navigation->getGeofenceStatus();
        telemetry.activeFenceId = geoStatus.activeFenceId;
        // Note: totalViolations would need to be added to Navigation API
        telemetry.totalViolations = 0; // Placeholder
    }
    
    // Update system data
    telemetry.uptime = millis();
}

String WebInterface::getTelemetryJSON() {
    DynamicJsonDocument doc(2048);
    
    doc["batteryVoltage"] = telemetry.batteryVoltage;
    doc["batteryCurrent"] = telemetry.batteryCurrent;
    doc["batteryPower"] = telemetry.batteryPower;
    doc["batterySOC"] = telemetry.batterySOC;
    doc["motorCurrentLeft"] = telemetry.motorCurrentLeft;
    doc["motorCurrentRight"] = telemetry.motorCurrentRight;
    
    doc["currentSpeedLeft"] = telemetry.currentSpeedLeft;
    doc["currentSpeedRight"] = telemetry.currentSpeedRight;
    doc["targetSpeedLeft"] = telemetry.targetSpeedLeft;
    doc["targetSpeedRight"] = telemetry.targetSpeedRight;
    doc["currentGear"] = telemetry.currentGear;
    doc["motorEnabled"] = telemetry.motorEnabled;
    
    doc["gpsFixed"] = telemetry.gpsFixed;
    doc["latitude"] = telemetry.latitude;
    doc["longitude"] = telemetry.longitude;
    doc["gpsSpeed"] = telemetry.gpsSpeed;
    doc["course"] = telemetry.course;
    doc["satellites"] = telemetry.satellites;
    doc["hdop"] = telemetry.hdop;
    doc["odometer"] = telemetry.odometer;
    doc["tripMeter"] = telemetry.tripMeter;
    
    doc["inSafeZone"] = telemetry.inSafeZone;
    doc["inProhibitedZone"] = telemetry.inProhibitedZone;
    doc["speedLimited"] = telemetry.speedLimited;
    doc["currentSpeedLimit"] = telemetry.currentSpeedLimit;
    doc["distanceToHome"] = telemetry.distanceToHome;
    doc["activeFenceId"] = telemetry.activeFenceId;
    doc["totalViolations"] = telemetry.totalViolations;
    
    doc["keySwitch"] = telemetry.keySwitch;
    doc["safetyFault"] = telemetry.safetyFault;
    doc["systemTemp"] = telemetry.systemTemp;
    doc["uptime"] = telemetry.uptime;
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WebInterface::getStatusJSON() {
    DynamicJsonDocument doc(1024);
    
    doc["wifi_mode"] = (int)config.mode;
    doc["wifi_ssid"] = getSSID();
    doc["local_ip"] = getLocalIP().toString();
    doc["clients_connected"] = WiFi.softAPgetStationNum();
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["chip_id"] = ESP.getEfuseMac();
    
    String output;
    serializeJson(doc, output);
    return output;
}

void WebInterface::sendCORSHeaders() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

void WebInterface::handleNotFound() {
    sendCORSHeaders();
    server.send(404, "application/json", "{\"error\":\"Not Found\"}");
}

bool WebInterface::handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    
    String contentType = getContentType(path);
    
    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    
    return false;
}

String WebInterface::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".xml")) return "text/xml";
    else if (filename.endsWith(".pdf")) return "application/pdf";
    else if (filename.endsWith(".zip")) return "application/zip";
    return "text/plain";
}

void WebInterface::setModuleReferences(PowerManager* pwr, MotorController* motors, 
                                     Navigation* nav, LightController* lights, 
                                     SafetySystem* safety, UserInterface* interface) {
    power = pwr;
    motorController = motors;
    navigation = nav;
    lightController = lights;
    safetySystem = safety;
    ui = interface;
    
    Serial.println("Web interface module references set");
}

void WebInterface::setDefaultConfig() {
    config.mode = WEB_WIFI_AP;
    strcpy(config.apSSID, WIFI_AP_SSID);
    strcpy(config.apPassword, WIFI_AP_PASSWORD);
    strcpy(config.stationSSID, "");
    strcpy(config.stationPassword, "");
    config.useStaticIP = false;
    config.staticIP = IPAddress(0, 0, 0, 0);
    config.gateway = IPAddress(0, 0, 0, 0);
    config.subnet = IPAddress(255, 255, 255, 0);
}

WiFiMode WebInterface::getCurrentMode() {
    return config.mode;
}

IPAddress WebInterface::getLocalIP() {
    if (config.mode == WEB_WIFI_AP) {
        return WiFi.softAPIP();
    } else {
        return WiFi.localIP();
    }
}

String WebInterface::getSSID() {
    if (config.mode == WEB_WIFI_AP) {
        return String(config.apSSID);
    } else {
        return WiFi.SSID();
    }
}

void WebInterface::printNetworkInfo() {
    Serial.println("=== Network Information ===");
    Serial.printf("Mode: %s\n", 
                  config.mode == WEB_WIFI_AP ? "Access Point" : "Station");
    Serial.printf("SSID: %s\n", getSSID().c_str());
    Serial.printf("IP Address: %s\n", getLocalIP().toString().c_str());
    Serial.printf("Web Interface: http://%s\n", getLocalIP().toString().c_str());
    
    if (config.mode == WEB_WIFI_AP) {
        Serial.printf("Connected Clients: %d\n", WiFi.softAPgetStationNum());
    }
    Serial.println("========================");
}

void WebInterface::handleJS() {
    sendCORSHeaders();
    
    String js = "";
    js += "function updateTelemetry() {";
    js += "fetch('/api/telemetry')";
    js += ".then(response => response.json())";
    js += ".then(data => {";
    js += "updateElement('battery-voltage', data.batteryVoltage.toFixed(1) + 'V');";
    js += "updateElement('battery-current', data.batteryCurrent.toFixed(1) + 'A');";
    js += "updateElement('power-draw', data.batteryPower.toFixed(0) + 'W');";
    js += "updateElement('battery-soc', data.batterySOC.toFixed(0) + '%');";
    js += "const gearNames = ['Park', '1st', '2nd', '3rd', 'Eco', 'Sport+'];";
    js += "updateElement('current-gear', gearNames[data.currentGear] || 'Unknown');";
    js += "updateElement('motor-status', data.motorEnabled ? 'Active' : 'Stopped');";
    js += "updateElement('left-motor', data.currentSpeedLeft.toFixed(0) + '%');";
    js += "updateElement('right-motor', data.currentSpeedRight.toFixed(0) + '%');";
    js += "updateElement('gps-status', data.gpsFixed ? 'Fixed' : 'Searching');";
    js += "updateElement('gps-speed', data.gpsSpeed.toFixed(1) + ' mph');";
    js += "updateElement('satellites', data.satellites);";
    js += "updateElement('odometer', data.odometer.toFixed(1) + ' mi');";
    js += "updateElement('safe-zone', data.inSafeZone ? 'YES' : 'NO');";
    js += "updateElement('speed-limit', data.currentSpeedLimit.toFixed(0) + ' mph');";
    js += "updateElement('distance-home', data.distanceToHome.toFixed(0) + 'm');";
    js += "updateElement('violations', data.totalViolations);";
    js += "updateElement('key-switch', data.keySwitch ? 'ON' : 'OFF');";
    js += "updateElement('safety-status', data.safetyFault ? 'FAULT' : 'OK');";
    js += "updateElement('uptime', formatUptime(data.uptime));";
    js += "updateElement('connection', 'Connected');";
    js += "document.getElementById('connection').className = 'status-value status-good';";
    js += "})";
    js += ".catch(error => {";
    js += "console.error('Error fetching telemetry:', error);";
    js += "updateElement('connection', 'Disconnected');";
    js += "document.getElementById('connection').className = 'status-value status-danger';";
    js += "});";
    js += "}";
    
    js += "function updateElement(id, text) {";
    js += "const element = document.getElementById(id);";
    js += "if (element) element.textContent = text;";
    js += "}";
    
    js += "function formatUptime(ms) {";
    js += "const seconds = Math.floor(ms / 1000);";
    js += "const minutes = Math.floor(seconds / 60);";
    js += "const hours = Math.floor(minutes / 60);";
    js += "return hours + 'h ' + (minutes % 60) + 'm';";
    js += "}";
    
    js += "function emergencyStop() {";
    js += "if (confirm('Are you sure you want to trigger an emergency stop?')) {";
    js += "fetch('/api/emergency-stop', { method: 'POST' });";
    js += "}";
    js += "}";
    
    js += "function setGear(gear) {";
    js += "fetch('/api/set-gear', {";
    js += "method: 'POST',";
    js += "headers: { 'Content-Type': 'application/json' },";
    js += "body: JSON.stringify({ gear: gear })";
    js += "});";
    js += "}";
    
    js += "function resetTrip() {";
    js += "if (confirm('Reset trip meter?')) {";
    js += "fetch('/api/reset-trip', { method: 'POST' });";
    js += "}";
    js += "}";
    
    js += "function setHome() {";
    js += "if (confirm('Set current location as home position?')) {";
    js += "fetch('/api/set-home', { method: 'POST' });";
    js += "}";
    js += "}";
    
    js += "function showCalibration() {";
    js += "if (confirm('Activate OLED calibration test screen?')) {";
    js += "fetch('/api/calibration', { method: 'POST' })";
    js += ".then(response => response.json())";
    js += ".then(data => {";
    js += "alert('OLED calibration screen activated! Check the display and take a photo to share.');";
    js += "});";
    js += "}";
    js += "}";
    
    js += "document.addEventListener('DOMContentLoaded', function() {";
    js += "updateTelemetry();";
    js += "setInterval(updateTelemetry, 1000);";
    js += "});";
    
    server.send(200, "application/javascript", js);
}

// Placeholder implementations for other methods
void WebInterface::handleGetConfig() { sendCORSHeaders(); server.send(200, "application/json", "{}"); }
void WebInterface::handleSetConfig() { sendCORSHeaders(); server.send(200, "application/json", "{}"); }
void WebInterface::handleGetGeofences() { sendCORSHeaders(); server.send(200, "application/json", "[]"); }
void WebInterface::handleAddGeofence() { sendCORSHeaders(); server.send(200, "application/json", "{}"); }
void WebInterface::handleRemoveGeofence() { sendCORSHeaders(); server.send(200, "application/json", "{}"); }
void WebInterface::handleSetSpeed() { sendCORSHeaders(); server.send(501, "application/json", "{\"error\":\"not_implemented\"}"); }

void WebInterface::enable(bool enabled) { this->enabled = enabled; }
bool WebInterface::isEnabled() { return enabled; }
void WebInterface::disconnect() { WiFi.disconnect(); }
void WebInterface::setWiFiConfig(const WiFiConfig& config) { this->config = config; }
WiFiConfig WebInterface::getWiFiConfig() { return config; }
void WebInterface::saveConfig() { /* Implement SPIFFS save */ }
bool WebInterface::loadConfig() { return true; /* Implement SPIFFS load */ }