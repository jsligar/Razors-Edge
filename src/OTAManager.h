#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "Logger.h"

class OTAManager {
public:
    static OTAManager& getInstance() {
        static OTAManager instance;
        return instance;
    }

    // Initialize OTA with security
    bool init(const char* hostname = "razors-edge", const char* password = "RazorOTA2025") {
        if (!WiFi.isConnected()) {
            LOG_ERROR("OTA", "WiFi not connected, OTA init failed");
            return false;
        }

        ArduinoOTA.setHostname(hostname);
        ArduinoOTA.setPassword(password);

        // Configure OTA callbacks
        ArduinoOTA.onStart([this]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type = "sketch";
            } else {
                type = "filesystem";
            }

            LOG_INFO("OTA", "Starting OTA update: %s", type.c_str());
            updateInProgress = true;

            // Stop motors for safety during update
            if (onUpdateStart) {
                onUpdateStart();
            }
        });

        ArduinoOTA.onEnd([this]() {
            LOG_INFO("OTA", "OTA update completed");
            updateInProgress = false;

            if (onUpdateEnd) {
                onUpdateEnd();
            }
        });

        ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
            updateProgress = (progress / (total / 100));

            // Log every 10%
            if (updateProgress % 10 == 0 && updateProgress != lastLoggedProgress) {
                LOG_INFO("OTA", "Progress: %u%%", updateProgress);
                lastLoggedProgress = updateProgress;
            }

            if (onUpdateProgress) {
                onUpdateProgress(progress, total);
            }
        });

        ArduinoOTA.onError([this](ota_error_t error) {
            const char* errorStr = getErrorString(error);
            LOG_ERROR("OTA", "Update failed: %s", errorStr);

            updateInProgress = false;

            if (onUpdateError) {
                onUpdateError(error);
            }
        });

        ArduinoOTA.begin();
        LOG_INFO("OTA", "OTA initialized - hostname: %s", hostname);

        return true;
    }

    // Must be called in main loop
    void handle() {
        if (enabled) {
            ArduinoOTA.handle();
        }
    }

    // Enable/disable OTA (for safety - disable during driving)
    void enable() {
        enabled = true;
        LOG_INFO("OTA", "OTA updates enabled");
    }

    void disable() {
        enabled = false;
        LOG_INFO("OTA", "OTA updates disabled");
    }

    bool isEnabled() const {
        return enabled;
    }

    bool isUpdateInProgress() const {
        return updateInProgress;
    }

    uint8_t getUpdateProgress() const {
        return updateProgress;
    }

    // Set callbacks for application-specific actions
    void setOnUpdateStart(void (*callback)()) {
        onUpdateStart = callback;
    }

    void setOnUpdateEnd(void (*callback)()) {
        onUpdateEnd = callback;
    }

    void setOnUpdateProgress(void (*callback)(unsigned int, unsigned int)) {
        onUpdateProgress = callback;
    }

    void setOnUpdateError(void (*callback)(ota_error_t)) {
        onUpdateError = callback;
    }

private:
    OTAManager() :
        enabled(false),
        updateInProgress(false),
        updateProgress(0),
        lastLoggedProgress(255),
        onUpdateStart(nullptr),
        onUpdateEnd(nullptr),
        onUpdateProgress(nullptr),
        onUpdateError(nullptr)
    {}

    ~OTAManager() {}

    // Prevent copying
    OTAManager(const OTAManager&) = delete;
    OTAManager& operator=(const OTAManager&) = delete;

    bool enabled;
    bool updateInProgress;
    uint8_t updateProgress;
    uint8_t lastLoggedProgress;

    // Callbacks
    void (*onUpdateStart)();
    void (*onUpdateEnd)();
    void (*onUpdateProgress)(unsigned int, unsigned int);
    void (*onUpdateError)(ota_error_t);

    const char* getErrorString(ota_error_t error) {
        switch (error) {
            case OTA_AUTH_ERROR:    return "Authentication Failed";
            case OTA_BEGIN_ERROR:   return "Begin Failed";
            case OTA_CONNECT_ERROR: return "Connect Failed";
            case OTA_RECEIVE_ERROR: return "Receive Failed";
            case OTA_END_ERROR:     return "End Failed";
            default:                return "Unknown Error";
        }
    }
};

#endif // OTA_MANAGER_H
