/**
 * Nerdbilly Fab Dashboard Test
 * Tests OLED, Buttons, and EC11 Rotary Encoder
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========================================
// Pin Definitions (from your dashboard)
// ========================================
#define OLED_SDA            21
#define OLED_SCL            22
#define BUTTON_CONFIRM      14  // Button A (confirm)
#define BUTTON_BACK         13  // Button B (back)
#define ENCODER_CLK         25  // Encoder track A
#define ENCODER_DT          26  // Encoder track B
#define ENCODER_BTN         27  // Encoder push button

// ========================================
// Display Setup
// ========================================
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_RESET          -1
#define OLED_ADDRESS        0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========================================
// State Variables
// ========================================
int encoderCounter = 0;
int lastEncoderCLK = HIGH;
bool buttonConfirmPressed = false;
bool buttonBackPressed = false;
bool encoderBtnPressed = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

bool showSplash = true;
unsigned long splashStartTime = 0;
const unsigned long splashDuration = 3000; // 3 seconds

// ========================================
// Setup
// ========================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=================================");
    Serial.println("  NERDBILLY FAB DASHBOARD TEST");
    Serial.println("=================================\n");

    // Initialize I2C
    Wire.begin(OLED_SDA, OLED_SCL);

    // Initialize Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("SSD1306 allocation failed!");
        while (1) {
            delay(100);
        }
    }

    Serial.println("✓ OLED Display initialized");

    // Initialize Buttons (INPUT_PULLUP - buttons connect to GND)
    pinMode(BUTTON_CONFIRM, INPUT_PULLUP);
    pinMode(BUTTON_BACK, INPUT_PULLUP);
    pinMode(ENCODER_BTN, INPUT_PULLUP);

    // Initialize Encoder
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);

    lastEncoderCLK = digitalRead(ENCODER_CLK);

    Serial.println("✓ Buttons and Encoder initialized");
    Serial.println("\n=== Hardware Test Ready ===");
    Serial.println("- Turn encoder to change counter");
    Serial.println("- Press encoder button");
    Serial.println("- Press CONFIRM button (Button A)");
    Serial.println("- Press BACK button (Button B)\n");

    // Show splash screen
    showSplashScreen();
    splashStartTime = millis();
}

// ========================================
// Splash Screen
// ========================================
void showSplashScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Large Title
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println("NERDBILLY");

    display.setCursor(30, 30);
    display.println("FAB");

    // Subtitle
    display.setTextSize(1);
    display.setCursor(15, 50);
    display.println("Dashboard Test");

    display.display();
}

// ========================================
// Main Display
// ========================================
void updateDisplay() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("NERDBILLY FAB");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // Encoder Counter (large)
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print("Count: ");
    display.println(encoderCounter);

    // Button Status
    display.setTextSize(1);
    display.setCursor(0, 35);

    // Confirm button
    display.print("CONFIRM: ");
    if (buttonConfirmPressed) {
        display.println("PRESSED");
    } else {
        display.println("---");
    }

    // Back button
    display.setCursor(0, 45);
    display.print("BACK:    ");
    if (buttonBackPressed) {
        display.println("PRESSED");
    } else {
        display.println("---");
    }

    // Encoder button
    display.setCursor(0, 55);
    display.print("ENC BTN: ");
    if (encoderBtnPressed) {
        display.println("PRESSED");
    } else {
        display.println("---");
    }

    display.display();
}

// ========================================
// Read Encoder
// ========================================
void readEncoder() {
    int currentCLK = digitalRead(ENCODER_CLK);

    // Check if encoder has rotated
    if (currentCLK != lastEncoderCLK && currentCLK == LOW) {
        int dtValue = digitalRead(ENCODER_DT);

        if (dtValue != currentCLK) {
            // Clockwise rotation
            encoderCounter++;
            Serial.print("Encoder CW: ");
            Serial.println(encoderCounter);
        } else {
            // Counter-clockwise rotation
            encoderCounter--;
            Serial.print("Encoder CCW: ");
            Serial.println(encoderCounter);
        }
    }

    lastEncoderCLK = currentCLK;
}

// ========================================
// Read Buttons
// ========================================
void readButtons() {
    unsigned long currentTime = millis();

    // Read all buttons (LOW = pressed with INPUT_PULLUP)
    bool confirmNow = (digitalRead(BUTTON_CONFIRM) == LOW);
    bool backNow = (digitalRead(BUTTON_BACK) == LOW);
    bool encoderBtnNow = (digitalRead(ENCODER_BTN) == LOW);

    // Debounce and detect changes
    if (currentTime - lastDebounceTime > debounceDelay) {

        // Confirm button
        if (confirmNow && !buttonConfirmPressed) {
            buttonConfirmPressed = true;
            Serial.println("✓ CONFIRM button pressed!");
            lastDebounceTime = currentTime;
        } else if (!confirmNow && buttonConfirmPressed) {
            buttonConfirmPressed = false;
            Serial.println("  CONFIRM button released");
            lastDebounceTime = currentTime;
        }

        // Back button
        if (backNow && !buttonBackPressed) {
            buttonBackPressed = true;
            Serial.println("✓ BACK button pressed!");
            lastDebounceTime = currentTime;
        } else if (!backNow && buttonBackPressed) {
            buttonBackPressed = false;
            Serial.println("  BACK button released");
            lastDebounceTime = currentTime;
        }

        // Encoder button
        if (encoderBtnNow && !encoderBtnPressed) {
            encoderBtnPressed = true;
            encoderCounter = 0; // Reset counter when encoder button pressed
            Serial.println("✓ ENCODER button pressed! Counter reset.");
            lastDebounceTime = currentTime;
        } else if (!encoderBtnNow && encoderBtnPressed) {
            encoderBtnPressed = false;
            Serial.println("  ENCODER button released");
            lastDebounceTime = currentTime;
        }
    }
}

// ========================================
// Main Loop
// ========================================
void loop() {
    // Show splash screen for 3 seconds
    if (showSplash) {
        if (millis() - splashStartTime >= splashDuration) {
            showSplash = false;
            Serial.println("\n=== Test Active - Interact with hardware ===\n");
        }
        return;
    }

    // Read inputs
    readEncoder();
    readButtons();

    // Update display
    updateDisplay();

    // Small delay for stability
    delay(10);
}
