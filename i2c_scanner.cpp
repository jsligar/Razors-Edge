/**
 * I2C Scanner for ESP32
 * Scans I2C bus to find connected devices
 */

#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=================================");
    Serial.println("      I2C SCANNER - ESP32");
    Serial.println("=================================\n");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("I2C initialized on SDA=%d, SCL=%d\n\n", I2C_SDA, I2C_SCL);
    
    Serial.println("Scanning I2C bus...\n");
}

void loop() {
    byte error, address;
    int deviceCount = 0;
    
    Serial.println("Scanning...");
    Serial.println("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    
    for (address = 0; address <= 127; address++) {
        if (address % 16 == 0) {
            Serial.printf("%02X: ", address);
        }
        
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            // Device found
            Serial.printf("%02X ", address);
            deviceCount++;
        } else if (error == 4) {
            // Unknown error
            Serial.print("?? ");
        } else {
            // No device
            Serial.print("-- ");
        }
        
        if ((address + 1) % 16 == 0) {
            Serial.println();
        }
        
        delay(10);
    }
    
    Serial.println("\n---------------------------------");
    if (deviceCount == 0) {
        Serial.println("❌ No I2C devices found!");
        Serial.println("\nTroubleshooting:");
        Serial.println("1. Check SDA/SCL wiring");
        Serial.println("2. Verify 3.3V power connected");
        Serial.println("3. Check for loose connections");
        Serial.println("4. Ensure GND is connected");
    } else {
        Serial.printf("✓ Found %d device(s)\n\n", deviceCount);
        Serial.println("Common I2C Addresses:");
        Serial.println("  0x3C or 0x3D = SSD1306 OLED Display");
        Serial.println("  0x40-0x45 = INA228 Current Sensors");
        Serial.println("  0x70 = PCA9685 PWM Driver");
    }
    Serial.println("=================================\n");
    
    delay(5000);  // Scan every 5 seconds
}
