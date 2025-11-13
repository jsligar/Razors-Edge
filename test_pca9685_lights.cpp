#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// I2C pins
#define GPIO_SDA 21
#define GPIO_SCL 22

// PCA9685 address
#define PCA9685_ADDR 0x40

// Light channels (12, 13, 14, 15)
#define LIGHT_FRONT_LEFT  12
#define LIGHT_FRONT_RIGHT 13
#define LIGHT_REAR_LEFT   14
#define LIGHT_REAR_RIGHT  15

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDR);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== PCA9685 Light Test ===");
    
    // Initialize I2C
    Wire.begin(GPIO_SDA, GPIO_SCL);
    
    // Initialize PCA9685
    pwm.begin();
    pwm.setPWMFreq(1000);  // 1kHz for LEDs
    
    Serial.println("PCA9685 initialized");
    Serial.println("Testing channels 12, 13, 14, 15");
    Serial.println("Cycling from low to high...\n");
}

void loop() {
    int channels[] = {LIGHT_FRONT_LEFT, LIGHT_FRONT_RIGHT, LIGHT_REAR_LEFT, LIGHT_REAR_RIGHT};
    const char* names[] = {"Front Left (12)", "Front Right (13)", "Rear Left (14)", "Rear Right (15)"};
    
    // Cycle through each channel
    for (int i = 0; i < 4; i++) {
        Serial.printf("Channel %d - %s: ", channels[i], names[i]);
        
        // Fade up
        Serial.print("Fade UP...");
        for (int brightness = 0; brightness <= 4095; brightness += 100) {
            pwm.setPWM(channels[i], 0, brightness);
            delay(10);
        }
        
        delay(500);  // Stay bright
        
        // Fade down
        Serial.print(" Fade DOWN...");
        for (int brightness = 4095; brightness >= 0; brightness -= 100) {
            pwm.setPWM(channels[i], 0, brightness);
            delay(10);
        }
        
        Serial.println(" OFF");
        delay(300);  // Pause before next channel
    }
    
    Serial.println("\n--- All channels tested! ---");
    Serial.println("Starting next cycle in 2 seconds...\n");
    delay(2000);
}
