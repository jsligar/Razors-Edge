# Improvements Guide - Razors Edge v1.1.0

This guide explains the new features added in version 1.1.0 and how to use them.

## Table of Contents
1. [Testing Infrastructure](#testing-infrastructure)
2. [Structured Logging](#structured-logging)
3. [Configuration Management](#configuration-management)
4. [OTA Updates](#ota-updates)
5. [Web Authentication](#web-authentication)
6. [Build Configurations](#build-configurations)
7. [Version Management](#version-management)
8. [Hardware Abstraction](#hardware-abstraction)

---

## Testing Infrastructure

### Running Tests

```bash
# Run all tests
pio test -e test

# Run specific test
pio test -e test -f test_safety_system

# Run tests with verbose output
pio test -e test --verbose
```

### Adding New Tests

Create a new test directory under `test/`:

```cpp
// test/test_my_module/test_my_module.cpp
#include <unity.h>
#include "../../src/MyModule.h"

void test_my_feature(void) {
    MyModule module;
    TEST_ASSERT_TRUE(module.init());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_my_feature);
    UNITY_END();
}

void loop() {}
```

---

## Structured Logging

### Basic Usage

Replace old Serial.print() statements:

```cpp
// Old way
Serial.println("Motor started");
Serial.printf("Speed: %.1f\n", speed);

// New way with logging
#include "Logger.h"

LOG_INFO("Motors", "Motor started");
LOG_DEBUG("Motors", "Speed: %.1f", speed);
LOG_ERROR("Safety", "Battery voltage critical: %.1fV", voltage);
```

### Log Levels

```cpp
LOG_ERROR("Tag", "Critical error");   // Always shown
LOG_WARN("Tag", "Warning message");   // Important warnings
LOG_INFO("Tag", "Informational");     // General info
LOG_DEBUG("Tag", "Debug details");    // Detailed debugging
LOG_TRACE("Tag", "Trace data");       // Very detailed
```

### Configuration

```cpp
void setup() {
    // Set log level
    Logger::getInstance().setLogLevel(LOG_LEVEL_DEBUG);

    // Enable timestamps
    Logger::getInstance().enableTimestamps(true);

    // Enable colors (if your terminal supports it)
    Logger::getInstance().enableColors(true);
}
```

### Output Format

```
[    125476] [INFO ] [Motors] Motor started
[    125480] [DEBUG] [Motors] Speed: 45.5
[    125490] [ERROR] [Safety] Battery voltage critical: 52.3V
```

---

## Configuration Management

### Initialization

```cpp
#include "ConfigManager.h"

void setup() {
    // Initialize configuration manager
    ConfigManager::getInstance().init();

    // Access configuration
    SystemConfig& config = ConfigManager::getInstance().getConfig();

    // Use configuration values
    Serial.printf("WiFi SSID: %s\n", config.wifiSSID);
    Serial.printf("Pedal Min: %d\n", config.pedalAdcMin);
}
```

### Calibration

```cpp
// Pedal calibration
void calibratePedal() {
    uint16_t minValue = readPedalMin();
    uint16_t maxValue = readPedalMax();

    ConfigManager::getInstance().setPedalCalibration(minValue, maxValue, 50);
    // Automatically saves to NVS
}

// Motor balance
void setMotorTrim(float leftTrim, float rightTrim) {
    ConfigManager::getInstance().setMotorBalance(leftTrim, rightTrim);
}

// WiFi settings
void updateWiFi(const char* ssid, const char* password) {
    ConfigManager::getInstance().setWiFiCredentials(ssid, password, true);
}
```

### Factory Reset

```cpp
void factoryReset() {
    ConfigManager::getInstance().resetToDefaults();
    ESP.restart();
}
```

### Web API Integration

```cpp
// In WebInterface.cpp
void handleCalibration() {
    if (server.hasArg("pedal_min") && server.hasArg("pedal_max")) {
        uint16_t min = server.arg("pedal_min").toInt();
        uint16_t max = server.arg("pedal_max").toInt();

        ConfigManager::getInstance().setPedalCalibration(min, max, 50);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    }
}
```

---

## OTA Updates

### Initialization

```cpp
#include "OTAManager.h"

void setup() {
    // Initialize WiFi first
    WiFi.begin(ssid, password);

    // Initialize OTA
    OTAManager::getInstance().init("razors-edge", "YourSecurePassword");

    // Set callbacks (optional)
    OTAManager::getInstance().setOnUpdateStart([]() {
        motors.emergencyStop();  // Stop motors during update
        LOG_INFO("OTA", "Update starting - motors stopped");
    });

    OTAManager::getInstance().setOnUpdateEnd([]() {
        LOG_INFO("OTA", "Update complete - rebooting");
    });

    // Enable OTA
    OTAManager::getInstance().enable();
}

void loop() {
    // Must be called regularly
    OTAManager::getInstance().handle();
}
```

### Safety Integration

```cpp
void updateMotors() {
    // Disable OTA during driving for safety
    if (speed > 0) {
        OTAManager::getInstance().disable();
    } else {
        OTAManager::getInstance().enable();
    }
}
```

### Upload Firmware

Using Arduino IDE:
1. Tools → Port → Network Ports → razors-edge
2. Tools → Upload

Using PlatformIO:
```bash
pio run -e release -t upload --upload-port razors-edge.local
```

---

## Web Authentication

### Initialization

```cpp
#include "WebAuth.h"

void setup() {
    // Initialize authentication
    WebAuth::getInstance().init();

    // Optional: Change default credentials
    WebAuth::getInstance().setUsername("admin");
    WebAuth::getInstance().setPassword("MySecurePassword123");
}
```

### Protecting Web Endpoints

```cpp
void handleProtectedEndpoint() {
    // Check authentication
    if (!WebAuth::getInstance().checkAuth(server)) {
        return;  // 401 sent automatically
    }

    // Proceed with authenticated request
    server.send(200, "application/json", "{\"data\":\"secret\"}");
}
```

### Login Endpoint

```cpp
void handleLogin() {
    String username = server.arg("username");
    String password = server.arg("password");
    IPAddress clientIP = server.client().remoteIP();

    if (WebAuth::getInstance().authenticate(username, password, clientIP)) {
        String token = WebAuth::getInstance().getSessionToken();

        // Send token to client
        server.send(200, "application/json",
                   "{\"status\":\"ok\",\"token\":\"" + token + "\"}");
    } else {
        server.send(401, "application/json",
                   "{\"error\":\"Invalid credentials\"}");
    }
}
```

### Client-Side Usage

```javascript
// Login
fetch('/api/login', {
    method: 'POST',
    body: JSON.stringify({username: 'admin', password: 'pass'}),
    headers: {'Content-Type': 'application/json'}
})
.then(r => r.json())
.then(data => {
    // Store token
    localStorage.setItem('auth_token', data.token);
});

// Use token for subsequent requests
fetch('/api/data', {
    headers: {'X-Auth-Token': localStorage.getItem('auth_token')}
})
.then(r => r.json())
.then(data => console.log(data));
```

### Disabling Authentication

```cpp
void setup() {
    WebAuth::getInstance().init();
    WebAuth::getInstance().setAuthEnabled(false);  // Disable for testing
}
```

---

## Build Configurations

### Available Environments

```bash
# Release build (optimized, minimal logging)
pio run -e release

# Debug build (full logging, symbols)
pio run -e debug

# Default development build
pio run -e esp32dev

# Testing build
pio test -e test

# Native testing (runs on PC)
pio test -e native
```

### Uploading Specific Build

```bash
# Upload release firmware
pio run -e release -t upload

# Upload and monitor debug build
pio run -e debug -t upload -t monitor
```

### Custom Build Flags

Add to `platformio.ini`:
```ini
[env:custom]
build_flags =
    -DCORE_DEBUG_LEVEL=4
    -DLOG_LEVEL=LOG_LEVEL_TRACE
    -DCUSTOM_FEATURE=1
```

---

## Version Management

### Display Version

```cpp
#include "Version.h"

void setup() {
    // Print full version information
    VersionInfo::printVersion();

    // Get individual components
    Serial.printf("Version: %s\n", VersionInfo::getVersion());
    Serial.printf("Build: %d\n", VersionInfo::getBuildNumber());
    Serial.printf("Commit: %s\n", VersionInfo::getGitCommitHash());
}
```

### Web API Version Endpoint

```cpp
void handleVersion() {
    server.send(200, "application/json", VersionInfo::getVersionJSON());
}

// Returns:
// {"version":"1.1.0","build":42,"commit":"abc123","timestamp":"Nov 7 2025 10:30:00"}
```

### Update Version

Edit `src/Version.h`:
```cpp
#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_PATCH 0
```

Build number and git hash are set automatically by the build system.

---

## Hardware Abstraction

### Using HAL in New Code

```cpp
#include "HardwareAbstraction.h"

class MyModule {
public:
    MyModule(IMotorDriver* driver) : motorDriver(driver) {}

    void init() {
        motorDriver->init();
    }

    void setSpeed(float speed) {
        motorDriver->setPWM(0, speed);
    }

private:
    IMotorDriver* motorDriver;
};

// Production code
MDD20ADriver realDriver(18, 19, 12, 23);
MyModule module(&realDriver);

// Test code
MockMotorDriver mockDriver;
MyModule testModule(&mockDriver);
```

### Mock Testing Example

```cpp
void test_motor_speed() {
    MockMotorDriver mock;
    MotorController controller(&mock);

    controller.setSpeed(50.0f);

    TEST_ASSERT_EQUAL_FLOAT(50.0f, mock.getLeftPWM());
    TEST_ASSERT_TRUE(mock.getLeftDirection());
}
```

---

## CI/CD Pipeline

### Automatic Builds

Every push to main/develop triggers:
- Build for all environments
- Run all tests
- Static code analysis
- Generate firmware artifacts

### Viewing Results

1. Go to GitHub repository
2. Click "Actions" tab
3. Select workflow run
4. Download firmware artifacts

### Local Testing Before Push

```bash
# Run what CI will run
pio run -e esp32dev
pio run -e debug
pio run -e release
pio test -e test

# Check for issues
grep -r "TODO\|FIXME" src/
```

---

## Migration From v1.0.0

### Step 1: Update includes

Add to main.cpp:
```cpp
#include "Logger.h"
#include "ConfigManager.h"
#include "Version.h"
```

### Step 2: Initialize new systems

In `setup()`:
```cpp
// Initialize configuration
ConfigManager::getInstance().init();

// Set up logging
Logger::getInstance().setLogLevel(LOG_LEVEL_INFO);

// Print version
VersionInfo::printVersion();
```

### Step 3: Gradually replace Serial.print

```cpp
// Old
Serial.println("System ready");

// New
LOG_INFO("System", "System ready");
```

### Step 4: Build and test

```bash
pio run -e debug
pio run -t upload
pio device monitor
```

---

## Best Practices

### Logging
- Use appropriate log levels
- Include relevant context in messages
- Use module-specific tags
- Disable DEBUG/TRACE in production builds

### Configuration
- Validate configuration values before use
- Provide sensible defaults
- Document configuration options
- Test factory reset functionality

### OTA Updates
- Always test firmware before OTA deployment
- Disable OTA during vehicle operation
- Use strong OTA passwords
- Monitor update progress

### Testing
- Write tests for critical functionality
- Use mocks for hardware dependencies
- Run tests before committing
- Keep tests fast and focused

---

For more information, see:
- [CHANGELOG.md](CHANGELOG.md) - Complete list of changes
- [README.md](README.md) - Project overview
- [TESTING_PROCEDURES.md](TESTING_PROCEDURES.md) - Hardware testing guide
