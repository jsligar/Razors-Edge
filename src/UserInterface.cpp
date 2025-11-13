#include "UserInterface.h"
#include "MGRSConverter.h"

UserInterface::UserInterface() :
    displayReady(false),
    currentScreen(SCREEN_MAIN_DRIVE),
    warningActive(false),
    warningStartTime(0),
    batteryVoltage(0.0f),
    batteryCurrent(0.0f),
    batteryPower(0.0f),
    vehicleSpeed(0.0f),
    batterySOC(0.0f),
    currentGear(GEAR_PARK),
    motorLeftCurrent(0.0f),
    motorRightCurrent(0.0f),
    motorImbalance(0.0f),
    gpsSatellites(0),
    gpsFixed(false),
    gpsLatitude(0.0),
    gpsLongitude(0.0),
    bearingToHome(0.0f),
    distanceToHome(0.0f),
    currentCourse(0.0f),
    homeLat(0.0),
    homeLon(0.0),
    homePositionSet(false),
    displayBrightness(100),
    lightMode(LIGHT_AUTO),
    gearChangeAnimation(false),
    gearChangeStartTime(0),
    animationGear(GEAR_PARK),
    previousScreen(SCREEN_MAIN_DRIVE),
    screenTransitionStartTime(0),
    screenTransitioning(false),
    encoderPressCount(0),
    lastEncoderPress(0),
    calibrationUnlocked(false),
    selectedSettingIndex(0),
    settingAdjustmentValue(0)
{
}

bool UserInterface::init() {
    Serial.println("Initializing User Interface...");
    
    // Initialize OLED display
    display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDR)) {
        Serial.println("⚠ SSD1306 OLED not found");
        displayReady = false;
        return false;
    }
    
    // Clear display and set text parameters
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    displayReady = true;
    
    Serial.println("✓ User Interface initialized");
    return true;
}

void UserInterface::update() {
    if (!displayReady) return;

    updateWarningAnimation();
    updateScreenTransition();

    // Draw current screen
    display.clearDisplay();

    switch (currentScreen) {
        case SCREEN_MAIN_DRIVE:
            drawMainDriveScreen();
            break;
        case SCREEN_DETAILED_METRICS:
            drawDetailedMetricsScreen();
            break;
        case SCREEN_NAVIGATION:
            drawNavigationScreen();
            break;
        case SCREEN_SETTINGS:
            drawSettingsScreen();
            break;
        case SCREEN_WARNING:
            drawWarningScreen();
            break;
        case SCREEN_CALIBRATION:
            drawCalibrationScreen();
            break;
    }

    // Draw gear change animation overlay (on top of everything)
    updateGearChangeAnimation();

    display.display();
}

void UserInterface::setScreen(ScreenType type) {
    currentScreen = type;
}

void UserInterface::cycleScreen() {
    // Check for calibration sequence first
    checkCalibrationSequence();

    // Start screen transition
    previousScreen = currentScreen;
    screenTransitioning = true;
    screenTransitionStartTime = millis();

    switch (currentScreen) {
        case SCREEN_MAIN_DRIVE:
            currentScreen = SCREEN_DETAILED_METRICS;
            break;
        case SCREEN_DETAILED_METRICS:
            currentScreen = SCREEN_NAVIGATION;
            break;
        case SCREEN_NAVIGATION:
            currentScreen = SCREEN_SETTINGS;
            break;
        case SCREEN_SETTINGS:
            if (calibrationUnlocked) {
                currentScreen = SCREEN_CALIBRATION;
                calibrationUnlocked = false; // Reset after use
            } else {
                currentScreen = SCREEN_MAIN_DRIVE;
            }
            break;
        case SCREEN_CALIBRATION:
            currentScreen = SCREEN_MAIN_DRIVE;
            break;
        case SCREEN_WARNING:
            // Don't cycle from warning screen
            screenTransitioning = false;
            break;
    }
}

void UserInterface::handleEncoderRotation(int16_t delta) {
    // Different behavior depending on current screen
    switch (currentScreen) {
        case SCREEN_SETTINGS:
            // On settings screen, adjust brightness or light mode based on rotation
            if (delta > 0) {
                // Clockwise: Increase brightness or advance light mode
                if (displayBrightness < 100) {
                    displayBrightness += 25;
                    if (displayBrightness > 100) displayBrightness = 100;
                    Serial.printf("Display brightness: %d%%\n", displayBrightness);
                }
            } else if (delta < 0) {
                // Counter-clockwise: Decrease brightness
                if (displayBrightness > 25) {
                    displayBrightness -= 25;
                    if (displayBrightness < 25) displayBrightness = 25;
                    Serial.printf("Display brightness: %d%%\n", displayBrightness);
                }
            }
            break;

        case SCREEN_MAIN_DRIVE:
        case SCREEN_DETAILED_METRICS:
            // Could scroll through info or adjust display settings
            // For now, just log it
            break;

        case SCREEN_CALIBRATION:
            // Adjust calibration values
            settingAdjustmentValue += delta * 10; // Larger steps for calibration
            break;

        default:
            break;
    }
}

void UserInterface::checkCalibrationSequence() {
    unsigned long now = millis();
    
    // Reset count if too much time has passed
    if (now - lastEncoderPress > 3000) {
        encoderPressCount = 0;
    }
    
    encoderPressCount++;
    lastEncoderPress = now;
    
    if (encoderPressCount >= 5) {
        calibrationUnlocked = true;
        encoderPressCount = 0;
        Serial.println("🎯 Calibration mode unlocked! Navigate to Settings and press encoder again.");
    }
}

ScreenType UserInterface::getCurrentScreen() {
    return currentScreen;
}

void UserInterface::showStartupMessage() {
    if (!displayReady) return;

    // Just clear and prepare - animation will be handled by updateStartupAnimation
    display.clearDisplay();
    display.display();
}

void UserInterface::updateStartupAnimation(unsigned long elapsedMs) {
    if (!displayReady) return;

    display.clearDisplay();

    // Stage 1 (0-800ms): Character-by-character reveal of "NerdBillyFab"
    const char* brandName = "NerdBillyFab";
    int brandLen = strlen(brandName);

    if (elapsedMs < 800) {
        // Reveal characters progressively (100ms per character)
        int charsToShow = (elapsedMs / 65);  // ~65ms per character = 12 chars in 800ms
        if (charsToShow > brandLen) charsToShow = brandLen;

        display.setTextSize(1);
        display.setCursor(22, 12);  // Centered-ish

        for (int i = 0; i < charsToShow; i++) {
            display.print(brandName[i]);
        }

        // Add a blinking cursor at the end
        if ((elapsedMs / 150) % 2 == 0 && charsToShow < brandLen) {
            display.print("_");
        }
    }
    // Stage 2 (800-1200ms): Wipe-in effect for "RAZOR"
    else if (elapsedMs < 1200) {
        // Show full brand name
        display.setTextSize(1);
        display.setCursor(22, 12);
        display.println("NerdBillyFab");

        // Wipe in RAZOR from left to right
        unsigned long stageTime = elapsedMs - 800;
        int wipeProgress = (stageTime * 128) / 400;  // 0-128 pixels in 400ms

        display.setTextSize(2);
        display.setCursor(20, 28);
        display.println("RAZOR");

        // Draw a wipe mask (reveal from left)
        if (wipeProgress < 128) {
            display.fillRect(wipeProgress, 28, 128 - wipeProgress, 16, SSD1306_BLACK);
        }
    }
    // Stage 3 (1200ms+): Animated "Initializing..." with moving dots
    else {
        // Show everything
        display.setTextSize(1);
        display.setCursor(22, 12);
        display.println("NerdBillyFab");

        display.setTextSize(2);
        display.setCursor(20, 28);
        display.println("RAZOR");

        // Animated dots
        display.setTextSize(1);
        display.setCursor(22, 50);
        display.print("Initializing");

        int dotCount = ((elapsedMs - 1200) / 250) % 4;  // 0-3 dots, cycling
        for (int i = 0; i < dotCount; i++) {
            display.print(".");
        }
    }

    display.display();
}

void UserInterface::showGearChange(GearMode gear) {
    gearChangeAnimation = true;
    gearChangeStartTime = millis();
    animationGear = gear;
}

void UserInterface::showWarning(const char* message) {
    warningActive = true;
    warningStartTime = millis();
    currentScreen = SCREEN_WARNING;
}

void UserInterface::showFault(uint8_t faultCode) {
    warningActive = true;
    warningStartTime = millis();
    currentScreen = SCREEN_WARNING;
}

void UserInterface::clearWarning() {
    warningActive = false;
    if (currentScreen == SCREEN_WARNING) {
        currentScreen = SCREEN_MAIN_DRIVE;
    }
}

// Data setter methods
void UserInterface::setVoltage(float voltage) { batteryVoltage = voltage; }
void UserInterface::setCurrent(float current) { batteryCurrent = current; }
void UserInterface::setPower(float power) { batteryPower = power; }
void UserInterface::setSpeed(float speed) { vehicleSpeed = speed; }
void UserInterface::setGear(GearMode gear) { currentGear = gear; }
void UserInterface::setBatterySOC(float percent) { batterySOC = percent; }
void UserInterface::setMotorData(float leftCurrent, float rightCurrent, float imbalance) {
    motorLeftCurrent = leftCurrent;
    motorRightCurrent = rightCurrent;
    motorImbalance = imbalance;
}
void UserInterface::setGPSData(uint8_t satellites, bool fixed) {
    gpsSatellites = satellites;
    gpsFixed = fixed;
}

void UserInterface::setGPSCoordinates(double latitude, double longitude) {
    gpsLatitude = latitude;
    gpsLongitude = longitude;
}

void UserInterface::setNavigationData(float bearing, float distance, float course) {
    bearingToHome = bearing;
    distanceToHome = distance;
    currentCourse = course;
}

void UserInterface::setHomePosition(double lat, double lon) {
    homeLat = lat;
    homeLon = lon;
    homePositionSet = true;
    Serial.printf("Home position set: %.6f, %.6f\n", lat, lon);
}

void UserInterface::handleButtonA() {
    // Button A: Cycle brightness levels (25%, 50%, 75%, 100%)
    if (displayBrightness == 100) {
        displayBrightness = 25;
    } else if (displayBrightness == 25) {
        displayBrightness = 50;
    } else if (displayBrightness == 50) {
        displayBrightness = 75;
    } else {
        displayBrightness = 100;
    }

    Serial.printf("Display brightness: %d%%\n", displayBrightness);
}

void UserInterface::handleButtonB() {
    // Button B: Cycle light modes (OFF → AUTO → ALWAYS_ON → DIM → OFF)
    switch (lightMode) {
        case LIGHT_OFF:
            lightMode = LIGHT_AUTO;
            Serial.println("Light mode: AUTO");
            break;
        case LIGHT_AUTO:
            lightMode = LIGHT_ALWAYS_ON;
            Serial.println("Light mode: ALWAYS ON");
            break;
        case LIGHT_ALWAYS_ON:
            lightMode = LIGHT_DIM;
            Serial.println("Light mode: DIM");
            break;
        case LIGHT_DIM:
            lightMode = LIGHT_OFF;
            Serial.println("Light mode: OFF");
            break;
    }
}

uint8_t UserInterface::getBrightness() {
    return displayBrightness;
}

LightMode UserInterface::getLightMode() {
    return lightMode;
}

void UserInterface::drawMainDriveScreen() {
    // Header
    drawHeader();

    // Speedometer arc (centered on left side)
    int16_t speedArcX = 32;
    int16_t speedArcY = 38;
    int16_t speedArcRadius = 22;
    drawSpeedometerArc(speedArcX, speedArcY, speedArcRadius, vehicleSpeed, SPEED_MAX_MPH);

    // Large speed number inside arc
    display.setTextSize(2);
    display.setCursor(speedArcX - 12, speedArcY - 5);
    display.printf("%.0f", vehicleSpeed);

    // MPH label below
    display.setTextSize(1);
    display.setCursor(speedArcX - 8, speedArcY + 10);
    display.println("MPH");

    // Gear indicator (right side, large)
    drawGearIndicator(90, 20, currentGear, true);

    // Battery gauge with icon (bottom)
    drawIcon(0, 55, "battery");
    drawBatteryGauge(10, 55, batterySOC);

    // GPS status with icon
    display.setCursor(90, 55);
    if (gpsFixed) {
        drawIcon(85, 55, "gps");
        display.printf("%d", gpsSatellites);
    } else {
        display.println("NO GPS");
    }
}

void UserInterface::drawDetailedMetricsScreen() {
    drawHeader();

    // GPS Coordinates in MGRS format
    if (gpsFixed && gpsLatitude != 0.0 && gpsLongitude != 0.0) {
        String mgrs = MGRSConverter::latLonToMGRS(gpsLatitude, gpsLongitude, 3);
        display.setCursor(0, 15);
        display.printf("GPS: %s", mgrs.c_str());
    } else {
        display.setCursor(0, 15);
        display.println("GPS: NO FIX");
    }

    // Motor currents
    display.setCursor(0, 25);
    display.printf("L:%.1fA R:%.1fA", motorLeftCurrent, motorRightCurrent);

    // Motor imbalance
    display.setCursor(0, 35);
    display.printf("Balance: %.1f%%", motorImbalance);

    // Power
    display.setCursor(0, 45);
    display.printf("Power: %.0fW", batteryPower);

    // Battery SOC
    display.setCursor(0, 55);
    display.printf("SOC: %.0f%%", batterySOC);
}

void UserInterface::drawSettingsScreen() {
    drawHeader();

    display.setCursor(0, 15);
    display.println("Settings");
    display.setCursor(0, 23);
    display.println("--------------");

    // Display current settings
    display.setCursor(0, 33);
    display.printf("Bright: %d%%", displayBrightness);

    display.setCursor(0, 43);
    display.print("Lights: ");
    switch (lightMode) {
        case LIGHT_OFF:      display.print("OFF"); break;
        case LIGHT_AUTO:     display.print("AUTO"); break;
        case LIGHT_ALWAYS_ON: display.print("ON"); break;
        case LIGHT_DIM:      display.print("DIM"); break;
    }

    display.setCursor(0, 53);
    display.println("Encoder: Bright");

    if (calibrationUnlocked) {
        display.setCursor(0, 25);
        display.println(">>> CALIBRATION <<<");
        display.setCursor(0, 35);
        display.println("Press encoder to");
        display.setCursor(0, 45);
        display.println("enter test mode");
    }
}

void UserInterface::drawNavigationScreen() {
    drawHeader();

    // Screen title
    display.setCursor(0, 15);
    display.println("Navigation");

    if (!homePositionSet) {
        // Show message to set home position
        display.setCursor(0, 30);
        display.println("No home position");
        display.setCursor(0, 45);
        display.println("Set in web UI");
        return;
    }

    if (!gpsFixed) {
        // Show waiting for GPS message
        display.setCursor(0, 30);
        display.println("Waiting for GPS");
        display.setCursor(0, 45);
        display.printf("Sats: %d", gpsSatellites);
        return;
    }

    // Draw compass circle (centered, larger)
    int16_t compassX = 64;
    int16_t compassY = 38;
    int16_t compassRadius = 20;

    // Draw compass circle
    display.drawCircle(compassX, compassY, compassRadius, SSD1306_WHITE);
    display.drawCircle(compassX, compassY, compassRadius - 1, SSD1306_WHITE);

    // Draw cardinal directions (N, E, S, W)
    display.setTextSize(1);
    display.setCursor(compassX - 3, compassY - compassRadius - 9);
    display.print("N");
    display.setCursor(compassX + compassRadius + 3, compassY - 3);
    display.print("E");
    display.setCursor(compassX - 3, compassY + compassRadius + 2);
    display.print("S");
    display.setCursor(compassX - compassRadius - 8, compassY - 3);
    display.print("W");

    // Calculate needle angle (bearing to home minus current course)
    // This gives us the relative bearing (where home is relative to our heading)
    float relativeAngle = bearingToHome - currentCourse;

    // Normalize angle to -180 to 180
    while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
    while (relativeAngle < -180.0f) relativeAngle += 360.0f;

    // Draw the needle pointing to home
    drawCompassNeedle(compassX, compassY, compassRadius - 3, relativeAngle);

    // Display distance to home
    display.setCursor(0, 56);
    if (distanceToHome < 1.0f) {
        display.printf("Home: %.0fft", distanceToHome * 5280.0f);  // Show in feet if < 1 mile
    } else {
        display.printf("Home: %.1fmi", distanceToHome);
    }
}

void UserInterface::drawWarningScreen() {
    drawHeader();

    // Pulsing border animation
    unsigned long elapsed = millis() - warningStartTime;
    bool flashOn = (elapsed / 500) % 2 == 0;

    // Draw pulsing double border
    if (flashOn) {
        display.drawRect(0, 10, DISPLAY_WIDTH, DISPLAY_HEIGHT - 10, SSD1306_WHITE);
        display.drawRect(2, 12, DISPLAY_WIDTH - 4, DISPLAY_HEIGHT - 14, SSD1306_WHITE);
    }

    // Warning icon (triangle with !)
    if (flashOn) {
        int16_t iconX = 64;
        int16_t iconY = 25;
        // Triangle
        display.drawLine(iconX, iconY - 8, iconX - 8, iconY + 8, SSD1306_WHITE);
        display.drawLine(iconX, iconY - 8, iconX + 8, iconY + 8, SSD1306_WHITE);
        display.drawLine(iconX - 8, iconY + 8, iconX + 8, iconY + 8, SSD1306_WHITE);
        // Exclamation mark
        display.drawLine(iconX, iconY - 4, iconX, iconY + 2, SSD1306_WHITE);
        display.drawPixel(iconX, iconY + 5, SSD1306_WHITE);
    }

    // Flash warning message
    if (flashOn) {
        display.setTextSize(1);
        display.setCursor(25, 45);
        display.println("System Fault");
        display.setCursor(22, 55);
        display.println("Check Status");
    }
}

void UserInterface::drawHeader() {
    // Gear with icon
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("G:%s", GEAR_CONFIGS[currentGear].name);

    // Battery voltage with icon
    drawIcon(40, 0, "battery");
    display.setCursor(48, 0);
    display.printf("%.1fV", batteryVoltage);

    // GPS status with icon
    if (gpsFixed) {
        drawIcon(90, 0, "gps");
        display.setCursor(98, 0);
        display.printf("%d", gpsSatellites);
    } else {
        display.setCursor(95, 0);
        display.println("---");
    }

    // Horizontal line with slight padding
    display.drawLine(0, 8, DISPLAY_WIDTH, 8, SSD1306_WHITE);
}

void UserInterface::drawGearIndicator(int16_t x, int16_t y, GearMode gear, bool large) {
    if (large) {
        display.setTextSize(2);
    } else {
        display.setTextSize(1);
    }
    
    display.setCursor(x, y);
    display.println(GEAR_CONFIGS[gear].name);
    
    display.setTextSize(1); // Reset to normal size
}

void UserInterface::drawBatteryGauge(int16_t x, int16_t y, float percent) {
    // Battery outline (no terminal on right side - cleaner look)
    display.drawRect(x, y, 50, 8, SSD1306_WHITE);

    // Fill based on percentage
    int16_t fillWidth = (percent / 100.0f) * 48;
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, 6, SSD1306_WHITE);
    }

    // Charging animation (moving chevron) if current is negative (regen/charging)
    if (batteryCurrent < -0.5f) {  // Charging/regen
        unsigned long animTime = millis() / 200;  // Animation speed
        int offset = (animTime % 10) * 2;  // Scrolling offset

        // Draw animated chevron pattern inside battery
        for (int i = 0; i < 3; i++) {
            int chevronX = x + 5 + (i * 10) - offset;
            if (chevronX >= x && chevronX < x + 48) {
                // Inverted chevron (shows as black on white background)
                display.drawLine(chevronX, y + 2, chevronX + 2, y + 4, SSD1306_BLACK);
                display.drawLine(chevronX + 2, y + 4, chevronX, y + 6, SSD1306_BLACK);
            }
        }
    }

    // Percentage text
    display.setCursor(x + 52, y);
    display.printf("%.0f%%", percent);
}

void UserInterface::drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, float percent) {
    // Bar outline
    display.drawRect(x, y, width, height, SSD1306_WHITE);
    
    // Fill
    int16_t fillWidth = (percent / 100.0f) * (width - 2);
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void UserInterface::drawValue(int16_t x, int16_t y, float value, const char* unit, uint8_t decimals) {
    display.setCursor(x, y);
    if (decimals == 0) {
        display.printf("%.0f%s", value, unit);
    } else if (decimals == 1) {
        display.printf("%.1f%s", value, unit);
    } else {
        display.printf("%.2f%s", value, unit);
    }
}

void UserInterface::drawCompassNeedle(int16_t centerX, int16_t centerY, int16_t radius, float angle) {
    // Convert angle to radians (0° = North, clockwise)
    // Screen coordinates: 0° = up, 90° = right, 180° = down, 270° = left
    float radians = (angle - 90.0f) * PI / 180.0f;

    // Calculate needle tip (pointing to home)
    int16_t tipX = centerX + radius * cos(radians);
    int16_t tipY = centerY + radius * sin(radians);

    // Calculate needle base (opposite direction, shorter)
    int16_t baseRadius = radius / 4;
    int16_t baseX = centerX - baseRadius * cos(radians);
    int16_t baseY = centerY - baseRadius * sin(radians);

    // Calculate arrow wings for tip
    float wingAngle1 = radians + 2.8f;  // ~160 degrees back
    float wingAngle2 = radians - 2.8f;
    int16_t wingRadius = radius / 3;
    int16_t wing1X = centerX + wingRadius * cos(wingAngle1);
    int16_t wing1Y = centerY + wingRadius * sin(wingAngle1);
    int16_t wing2X = centerX + wingRadius * cos(wingAngle2);
    int16_t wing2Y = centerY + wingRadius * sin(wingAngle2);

    // Draw arrow needle
    display.drawLine(baseX, baseY, tipX, tipY, SSD1306_WHITE);  // Main shaft
    display.drawLine(tipX, tipY, wing1X, wing1Y, SSD1306_WHITE);  // Arrow wing 1
    display.drawLine(tipX, tipY, wing2X, wing2Y, SSD1306_WHITE);  // Arrow wing 2

    // Draw center dot
    display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
}

void UserInterface::drawSpeedometerArc(int16_t centerX, int16_t centerY, int16_t radius, float speed, float maxSpeed) {
    // Draw arc from 225° to 315° (180° total, bottom semicircle)
    // 225° = lower left, 270° = bottom, 315° = lower right

    float startAngle = 225.0f;  // Start angle in degrees
    float endAngle = 315.0f;    // End angle in degrees
    float totalAngle = endAngle - startAngle;  // 90° total sweep

    // Draw outer arc (discrete points)
    for (int angle = startAngle; angle <= endAngle; angle += 3) {
        float radians = angle * PI / 180.0f;
        int16_t x = centerX + radius * cos(radians);
        int16_t y = centerY + radius * sin(radians);
        display.drawPixel(x, y, SSD1306_WHITE);
    }

    // Draw inner arc
    for (int angle = startAngle; angle <= endAngle; angle += 3) {
        float radians = angle * PI / 180.0f;
        int16_t x = centerX + (radius - 2) * cos(radians);
        int16_t y = centerY + (radius - 2) * sin(radians);
        display.drawPixel(x, y, SSD1306_WHITE);
    }

    // Calculate needle angle based on speed
    float speedRatio = constrain(speed / maxSpeed, 0.0f, 1.0f);
    float needleAngle = startAngle + (totalAngle * speedRatio);
    float needleRadians = needleAngle * PI / 180.0f;

    // Draw needle
    int16_t needleLength = radius - 4;
    int16_t needleX = centerX + needleLength * cos(needleRadians);
    int16_t needleY = centerY + needleLength * sin(needleRadians);

    display.drawLine(centerX, centerY, needleX, needleY, SSD1306_WHITE);
    display.drawLine(centerX + 1, centerY, needleX + 1, needleY, SSD1306_WHITE);  // Thicker needle

    // Draw tick marks at 0, 25%, 50%, 75%, 100%
    for (int tick = 0; tick <= 4; tick++) {
        float tickAngle = startAngle + (totalAngle * tick / 4.0f);
        float tickRadians = tickAngle * PI / 180.0f;
        int16_t tickStart = radius - 3;
        int16_t tickEnd = radius - 6;

        int16_t x1 = centerX + tickStart * cos(tickRadians);
        int16_t y1 = centerY + tickStart * sin(tickRadians);
        int16_t x2 = centerX + tickEnd * cos(tickRadians);
        int16_t y2 = centerY + tickEnd * sin(tickRadians);

        display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }

    // Center pivot point
    display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
}

void UserInterface::drawIcon(int16_t x, int16_t y, const char* iconType) {
    // Draw 5x7 pixel icons
    if (strcmp(iconType, "battery") == 0) {
        // Battery icon
        display.drawRect(x, y + 1, 5, 5, SSD1306_WHITE);  // Battery body
        display.drawPixel(x + 5, y + 2, SSD1306_WHITE);   // Battery terminal
        display.drawPixel(x + 5, y + 3, SSD1306_WHITE);
    } else if (strcmp(iconType, "gps") == 0) {
        // GPS satellite icon (simplified)
        display.drawCircle(x + 2, y + 3, 2, SSD1306_WHITE);  // Satellite
        display.drawPixel(x + 1, y + 1, SSD1306_WHITE);      // Signal
        display.drawPixel(x + 3, y + 1, SSD1306_WHITE);
    }
}

void UserInterface::updateScreenTransition() {
    if (!screenTransitioning) return;

    unsigned long elapsed = millis() - screenTransitionStartTime;
    if (elapsed > 200) {  // 200ms transition
        screenTransitioning = false;
    }

    // Could add slide/fade effects here in the future
    // For now, just a simple flag to control transition timing
}

void UserInterface::updateGearChangeAnimation() {
    if (!gearChangeAnimation) return;

    unsigned long elapsed = millis() - gearChangeStartTime;

    // Show gear change overlay for 800ms
    if (elapsed < 800) {
        // Slide in from right animation
        int slideOffset = 128 - (elapsed * 128 / 200);  // Slides in over 200ms
        if (slideOffset < 0) slideOffset = 0;

        // Draw semi-transparent box (using patterns)
        int boxX = 30 + slideOffset;
        int boxY = 25;
        int boxW = 68;
        int boxH = 25;

        // Only draw if visible
        if (boxX < 128) {
            // Outer border
            display.drawRect(boxX, boxY, boxW, boxH, SSD1306_WHITE);
            display.drawRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, SSD1306_WHITE);

            // Clear inside
            display.fillRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, SSD1306_BLACK);

            // Show new gear (large)
            display.setTextSize(2);
            display.setCursor(boxX + boxW/2 - 8, boxY + 8);
            display.print(GEAR_CONFIGS[animationGear].name);

            // Pulse effect (scale based on time)
            if (elapsed < 400 && (elapsed / 100) % 2 == 0) {
                // Draw extra border for pulse
                display.drawRect(boxX - 1, boxY - 1, boxW + 2, boxH + 2, SSD1306_WHITE);
            }

            display.setTextSize(1); // Reset
        }
    } else {
        gearChangeAnimation = false;
    }
}

void UserInterface::updateWarningAnimation() {
    if (!warningActive) return;
    
    // Auto-clear warning after 5 seconds
    unsigned long elapsed = millis() - warningStartTime;
    if (elapsed > 5000) {
        clearWarning();
    }
}

const char* UserInterface::getFaultString(uint8_t faultCode) {
    switch (faultCode) {
        case 0: return "No Fault";
        case 1: return "Low Battery";
        case 2: return "Battery Overcurrent";
        case 3: return "Motor L Overcurrent";
        case 4: return "Motor R Overcurrent";
        case 5: return "Motor L Stall";
        case 6: return "Motor R Stall";
        case 7: return "Motor Imbalance";
        case 8: return "Key Switch Off";
        case 9: return "Watchdog Timeout";
        default: return "Unknown Fault";
    }
}

void UserInterface::showCalibrationScreen() {
    currentScreen = SCREEN_CALIBRATION;
    Serial.println("📺 Calibration screen activated - Please take a photo and share!");
}

void UserInterface::drawCalibrationScreen() {
    // Draw comprehensive calibration pattern for visual verification
    
    // Title
    display.setTextSize(1);
    display.setCursor(30, 0);
    display.println("CALIBRATION");
    
    // Draw border frame
    display.drawRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, SSD1306_WHITE);
    display.drawRect(1, 1, DISPLAY_WIDTH-2, DISPLAY_HEIGHT-2, SSD1306_WHITE);
    
    // Corner markers
    display.fillRect(4, 4, 3, 3, SSD1306_WHITE);    // Top-left
    display.fillRect(DISPLAY_WIDTH-7, 4, 3, 3, SSD1306_WHITE);  // Top-right
    display.fillRect(4, DISPLAY_HEIGHT-7, 3, 3, SSD1306_WHITE); // Bottom-left
    display.fillRect(DISPLAY_WIDTH-7, DISPLAY_HEIGHT-7, 3, 3, SSD1306_WHITE); // Bottom-right
    
    // Center crosshair
    display.drawLine(DISPLAY_WIDTH/2-5, DISPLAY_HEIGHT/2, DISPLAY_WIDTH/2+5, DISPLAY_HEIGHT/2, SSD1306_WHITE);
    display.drawLine(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2-5, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2+5, SSD1306_WHITE);
    
    // Test patterns
    // Horizontal lines
    display.drawLine(10, 15, 118, 15, SSD1306_WHITE);
    display.drawLine(10, 49, 118, 49, SSD1306_WHITE);
    
    // Vertical lines  
    display.drawLine(20, 10, 20, 54, SSD1306_WHITE);
    display.drawLine(108, 10, 108, 54, SSD1306_WHITE);
    
    // Text size test
    display.setTextSize(1);
    display.setCursor(25, 20);
    display.println("Size1");
    
    display.setTextSize(2);
    display.setCursor(65, 18);
    display.println("S2");
    
    // Pixel-level details
    display.setTextSize(1);
    display.setCursor(25, 35);
    display.printf("128x64");
    
    display.setCursor(75, 35);
    display.printf("OLED");
    
    // Progress bars test
    display.drawRect(10, 55, 50, 6, SSD1306_WHITE);
    display.fillRect(11, 56, 35, 4, SSD1306_WHITE); // 70% filled
    
    display.drawRect(68, 55, 50, 6, SSD1306_WHITE);
    display.fillRect(69, 56, 20, 4, SSD1306_WHITE); // 40% filled
    
    // Screen info at bottom
    display.setTextSize(1);
    display.setCursor(3, DISPLAY_HEIGHT-8);
    display.printf("Photo test - Share image!");
}

uint16_t UserInterface::getGearColor(GearMode gear) {
    if (gear >= 0 && gear < GEAR_COUNT) {
        return GEAR_CONFIGS[gear].color;
    }
    return 0x0000; // Black for invalid gear
}