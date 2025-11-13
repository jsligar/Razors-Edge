#ifndef VERSION_H
#define VERSION_H

#include <Arduino.h>

// Semantic versioning
#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_PATCH 0

// Build information (can be set by build script)
#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif

// Version string formatting
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define VERSION_STRING \
    TOSTRING(VERSION_MAJOR) "." \
    TOSTRING(VERSION_MINOR) "." \
    TOSTRING(VERSION_PATCH)

#define FULL_VERSION_STRING \
    VERSION_STRING " (build " TOSTRING(BUILD_NUMBER) ")"

class VersionInfo {
public:
    static const char* getVersion() {
        return VERSION_STRING;
    }

    static const char* getFullVersion() {
        return FULL_VERSION_STRING;
    }

    static const char* getBuildTimestamp() {
        return BUILD_TIMESTAMP;
    }

    static const char* getGitCommitHash() {
        return GIT_COMMIT_HASH;
    }

    static uint8_t getMajor() {
        return VERSION_MAJOR;
    }

    static uint8_t getMinor() {
        return VERSION_MINOR;
    }

    static uint8_t getPatch() {
        return VERSION_PATCH;
    }

    static uint16_t getBuildNumber() {
        return BUILD_NUMBER;
    }

    // Print version information to Serial
    static void printVersion() {
        Serial.println("=================================");
        Serial.printf("Firmware: Razors Edge Controller\n");
        Serial.printf("Version: %s\n", getFullVersion());
        Serial.printf("Built: %s\n", getBuildTimestamp());
        Serial.printf("Commit: %s\n", getGitCommitHash());
        Serial.printf("Platform: ESP32\n");
        Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
        Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.println("=================================");
    }

    // Get version as JSON string
    static String getVersionJSON() {
        char json[256];
        snprintf(json, sizeof(json),
                 "{\"version\":\"%s\",\"build\":%d,\"commit\":\"%s\",\"timestamp\":\"%s\"}",
                 VERSION_STRING, BUILD_NUMBER, GIT_COMMIT_HASH, BUILD_TIMESTAMP);
        return String(json);
    }
};

#endif // VERSION_H
