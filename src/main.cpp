#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "config.h"
#include "PowerManager.h"
#include "MotorController.h"
#include "LightController.h"
#include "UserInterface.h"
#include "InputHandler.h"
#include "Navigation.h"
#include "SafetySystem.h"
#include "StateMachine.h"
#include "WebInterface.h"
#include "CoreManager.h"

// ========================================
// GLOBAL MODULE INSTANCES
// ========================================
PowerManager power;
MotorController motors;
LightController lights;
UserInterface ui;
InputHandler inputs;
Navigation gps;
SafetySystem safety;
StateMachine stateMachine;
WebInterface webInterface;

// ========================================
// TIMING VARIABLES
// ========================================
unsigned long lastUIUpdate = 0;
unsigned long lastSafetyUpdate = 0;
unsigned long lastPowerUpdate = 0;
unsigned long lastWebUpdate = 0;

void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    Serial.println();
    Serial.println("=================================");
    Serial.println("    RAZOR 60V CONTROLLER v1.0   ");
    Serial.println("    ESP32 Electric Vehicle       ");
    Serial.println("=================================");
    
    // Disable WiFi initially (will be enabled by web interface if needed)
    WiFi.mode(WIFI_OFF);
    
    // Initialize I2C Bus
    Wire.begin(GPIO_SDA, GPIO_SCL);
    Wire.setClock(400000); // 400kHz I2C speed
    Serial.println("I2C Bus initialized");
    
    // Initialize all modules in dependency order
    Serial.println("Initializing modules...");
    
    // Core modules first
    inputs.init();
    Serial.println("✓ Input Handler initialized");
    
    power.init();
    Serial.println("✓ Power Manager initialized");
    
    safety.init();
    Serial.println("✓ Safety System initialized");
    
    // Control modules
    motors.init();
    Serial.println("✓ Motor Controller initialized");
    
    lights.init();
    Serial.println("✓ Light Controller initialized");
    
    // User interface
    ui.init();
    Serial.println("✓ User Interface initialized");
    
    // GPS (optional, may fail if not connected)
    gps.init();
    Serial.println("✓ Navigation initialized");
    
    // Web interface (optional WiFi functionality)
    webInterface.init();
    Serial.println("✓ Web Interface initialized");
    
    // State machine (coordinates everything)
    stateMachine.init();
    
    // Link modules for cross-communication
    motors.setNavigationPtr(&gps);  // Enable geofencing integration
    webInterface.setModuleReferences(&power, &motors, &gps, &lights, &safety, &ui);  // Enable web monitoring
    coreManager.setModuleReferences(&power, &motors, &gps, &lights, &safety, &webInterface, &ui, &inputs);
    
    // Initialize dual-core task manager
    coreManager.init();
    
    // Set module references for state machine
    stateMachine.setModuleReferences(&power, &motors, &lights, &ui, &inputs, &gps, &safety);
    Serial.println("✓ State Machine initialized");
    
    // Self-test sequence
    Serial.println("Running self-test...");
    
    // Check key switch
    if (!inputs.isKeyOn()) {
        Serial.println("⚠ Key switch is OFF - system in standby");
    } else {
        Serial.println("✓ Key switch is ON");
    }
    
    // Check battery voltage
    float batteryVoltage = power.getBatteryVoltage();
    if (batteryVoltage < BATTERY_VOLTAGE_MIN) {
        Serial.printf("⚠ Low battery voltage: %.1fV (min: %.1fV)\n", 
                     batteryVoltage, BATTERY_VOLTAGE_MIN);
    } else {
        Serial.printf("✓ Battery voltage: %.1fV\n", batteryVoltage);
    }
    
    // Display startup message on OLED
    ui.showStartupMessage();
    
    // Enable watchdog
    safety.enableWatchdog();
    
    Serial.println("=================================");
    Serial.println("System ready! Starting dual-core tasks...");
    Serial.println("=================================");
    
    // Start dual-core task system
    if (coreManager.startAllTasks()) {
        Serial.println("✓ Dual-core task system active");
        Serial.println("  → Core 0: WiFi, Web Interface");
        Serial.println("  → Core 1: Safety, Motors, Navigation, UI");
    } else {
        Serial.println("⚠ Failed to start dual-core tasks, falling back to single-core mode");
    }
    
    delay(2000); // Show startup message
}

void loop() {
    // ========================================
    // DUAL-CORE TASK MONITORING LOOP
    // ========================================
    
    // Most work is now handled by dedicated tasks on specific cores
    // This main loop just handles coordination and fallback operations
    
    // Handle gear shifting input events (time-critical)
    if (inputs.isShiftUpPressed()) {
        stateMachine.handleShiftUp();
        Serial.println("Shift UP requested");
    }
    
    if (inputs.isShiftDownPressed()) {
        stateMachine.handleShiftDown();
        Serial.println("Shift DOWN requested");
    }
    
    // Update state machine coordination
    stateMachine.update();
    
    // Update shared data for task communication
    coreManager.updateSharedData();
    
    // Monitor system health
    SharedSystemData sysData = coreManager.getSharedData();
    if (!sysData.systemReady) {
        Serial.println("⚠ Task system not ready, attempting restart...");
        coreManager.startAllTasks();
    }
    
    // Check for emergency conditions in main loop as backup
    if (sysData.emergencyStop || sysData.safetyFault) {
        motors.emergencyStop();
    }
    
    // Sport+ timeout check
    stateMachine.checkSportPlusTimeout();
    
    // Light main loop delay for task scheduling
    delay(10);  // Reduced delay since tasks handle most work
    
    // Performance monitoring every 10 seconds
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 10000) {
        Serial.printf("System Status - Tasks: %s, Heap: %d, Battery: %.1fV, Gear: %s\n",
                     sysData.systemReady ? "ACTIVE" : "STOPPED",
                     ESP.getFreeHeap(),
                     sysData.batteryVoltage,
                     GEAR_CONFIGS[sysData.currentGear].name);
        lastDebug = millis();
    }
}