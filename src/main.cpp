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
    Serial.println("    TRACTOR CONTROLLER v1.0     ");
    Serial.println("    ESP32 Tractor Control        ");
    Serial.println("=================================");
    
    // Disable WiFi initially (will be enabled by web interface if needed)
    WiFi.mode(WIFI_OFF);
    
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
    
    // User interface (tractor version - serial only)
    ui.init();
    Serial.println("✓ User Interface initialized");

    // GPS (disabled in tractor version)
    gps.init();
    Serial.println("✓ Navigation initialized (disabled)");

    // Web interface (optional WiFi functionality)
    webInterface.init();
    Serial.println("✓ Web Interface initialized");

    // State machine (coordinates everything)
    stateMachine.init();

    // Link modules for cross-communication
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
    
    // No battery voltage monitoring in tractor version
    Serial.println("✓ Battery monitoring: DISABLED (tractor version)");
    
    // Display startup message (tractor version - serial only)
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
    
    // Handle direction switch changes (tractor version)
    if (inputs.directionChanged()) {
        DirectionMode newDir = inputs.getDirection();
        stateMachine.setDirection(newDir);
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
    
    // No Sport+ in tractor version
    
    // Light main loop delay for task scheduling
    delay(10);  // Reduced delay since tasks handle most work
    
    // Performance monitoring every 10 seconds
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 10000) {
        Serial.printf("System Status - Tasks: %s, Heap: %d, Battery: %.1fV, Direction: %s\n",
                     sysData.systemReady ? "ACTIVE" : "STOPPED",
                     ESP.getFreeHeap(),
                     sysData.batteryVoltage,
                     DIR_CONFIGS[stateMachine.getCurrentDirection()].name);
        lastDebug = millis();
    }
}