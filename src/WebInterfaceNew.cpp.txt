// Updated WebInterface.cpp with Authentication, WebSocket, and Modern UI
// To be merged with existing WebInterface.cpp

// Add these includes to WebInterface.cpp
#include "WebAuth.h"
#include "OTAManager.h"
#include "Version.h"

// In setupWebServer() method, add these routes:

// Login endpoint
server.on("/api/login", HTTP_POST, [this]() {
    String body = server.arg("plain");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, body);

    String username = doc["username"];
    String password = doc["password"];
    IPAddress clientIP = server.client().remoteIP();

    if (WebAuth::getInstance().authenticate(username, password, clientIP)) {
        String token = WebAuth::getInstance().getSessionToken();

        DynamicJsonDocument response(256);
        response["status"] = "ok";
        response["token"] = token;
        response["username"] = username;

        String output;
        serializeJson(response, output);
        server.send(200, "application/json", output);
    } else {
        server.send(401, "application/json", "{\"error\":\"Invalid credentials\"}");
    }
});

// Logout endpoint
server.on("/api/logout", HTTP_POST, [this]() {
    WebAuth::getInstance().logout();
    server.send(200, "application/json", "{\"status\":\"logged_out\"}");
});

// Version endpoint
server.on("/api/version", HTTP_GET, [this]() {
    server.send(200, "application/json", VersionInfo::getVersionJSON());
});

// System status (with free heap)
server.on("/api/system", HTTP_GET, [this]() {
    if (!WebAuth::getInstance().checkAuth(server)) return;

    DynamicJsonDocument doc(512);
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["chipModel"] = ESP.getChipModel();
    doc["cpuFreq"] = ESP.getCpuFreqMHz();
    doc["flashSize"] = ESP.getFlashChipSize();

    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
});

// Update existing route handlers to include auth check
// Example for handleEmergencyStop():
void WebInterface::handleEmergencyStop() {
    if (!WebAuth::getInstance().checkAuth(server)) return;
    sendCORSHeaders();

    if (motorController) {
        motorController->emergencyStop();
        Serial.println("🚨 Emergency stop triggered via web interface");
        server.send(200, "application/json", "{\"status\":\"emergency_stop_activated\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"motor_controller_not_available\"}");
    }
}

// Similarly, add auth checks to all protected endpoints:
// - handleSetGear()
// - handleResetTrip()
// - handleSetHome()
// - handleCalibration()
// - handleGetGeofences()
// - handleAddGeofence()
// - handleRemoveGeofence()

// Example pattern:
void WebInterface::handleSetGear() {
    if (!WebAuth::getInstance().checkAuth(server)) return;  // Add this line
    sendCORSHeaders();
    // ... rest of existing code ...
}

// In the init() method, initialize WebAuth:
bool WebInterface::init() {
    Serial.println("Initializing Web Interface...");

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("⚠ SPIFFS initialization failed");
        return false;
    }

    // Initialize authentication
    WebAuth::getInstance().init();

    // Initialize OTA manager
    if (WiFi.isConnected()) {
        OTAManager::getInstance().init();
        OTAManager::getInstance().setOnUpdateStart([]() {
            // Stop motors during OTA update
            Serial.println("OTA update starting - stopping motors");
        });
    }

    // ... rest of existing init code ...
}

// In the update() method, add OTA handling:
void WebInterface::update() {
    if (!enabled || !serverStarted) return;

    // Handle OTA updates
    OTAManager::getInstance().handle();

    // Handle web server requests
    server.handleClient();

    // ... rest of existing update code ...
}

// Update handleRoot() to serve from SPIFFS:
void WebInterface::handleRoot() {
    // Serve index.html from SPIFFS
    handleFileRead("/index.html");
}

// Add handler for login page:
void WebInterface::handleLogin() {
    handleFileRead("/login.html");
}

// Update setupWebServer() to add login route:
server.on("/login.html", HTTP_GET, [this]() { handleFileRead("/login.html"); });
server.on("/login", HTTP_GET, [this]() { handleFileRead("/login.html"); });

// Serve static assets
server.on("/css/styles.css", HTTP_GET, [this]() { handleFileRead("/css/styles.css"); });
server.on("/js/dashboard.js", HTTP_GET, [this]() { handleFileRead("/js/dashboard.js"); });
server.on("/manifest.json", HTTP_GET, [this]() { handleFileRead("/manifest.json"); });
server.on("/sw.js", HTTP_GET, [this]() { handleFileRead("/sw.js"); });

/*
NOTE: WebSocket support requires additional library
Add to platformio.ini:
    lib_deps =
        ... existing libs ...
        links2004/WebSockets @ ^2.4.1

Then add WebSocket handler:
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
                webSocket.sendTXT(num, "{\"type\":\"connected\"}");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] Received: %s\n", num, payload);
            break;
    }
}

// In init():
webSocket.begin();
webSocket.onEvent(webSocketEvent);

// In update():
webSocket.loop();

// To broadcast telemetry:
void broadcastTelemetry() {
    String telemetry = getTelemetryJSON();
    DynamicJsonDocument doc(2048);
    doc["type"] = "telemetry";
    doc["data"] = serialized(telemetry);

    String output;
    serializeJson(doc, output);
    webSocket.broadcastTXT(output);
}
*/
