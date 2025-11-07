#ifndef CORE_MANAGER_H
#define CORE_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

// Forward declarations
class PowerManager;
class MotorController;
class Navigation;
class LightController;
class SafetySystem;
class WebInterface;
class UserInterface;
class InputHandler;

// Core assignments for ESP32 dual-core processing
#define CORE_0 0  // Protocol CPU (WiFi, Bluetooth, system tasks)
#define CORE_1 1  // Application CPU (main Arduino loop)

// Task priorities (higher number = higher priority)
#define PRIORITY_CRITICAL    5  // Safety, emergency stop
#define PRIORITY_HIGH        4  // Motor control, power monitoring
#define PRIORITY_MEDIUM      3  // Navigation, geofencing
#define PRIORITY_LOW         2  // Web interface, UI updates
#define PRIORITY_BACKGROUND  1  // Logging, diagnostics

// Task stack sizes (in words, not bytes)
#define STACK_SIZE_LARGE     8192   // Web interface, complex calculations
#define STACK_SIZE_MEDIUM    4096   // Navigation, geofencing
#define STACK_SIZE_SMALL     2048   // Safety checks, power monitoring
#define STACK_SIZE_MINIMAL   1024   // Simple tasks

// Update intervals (milliseconds)
#define INTERVAL_CRITICAL    10     // Safety monitoring
#define INTERVAL_FAST        20     // Motor control, power
#define INTERVAL_MEDIUM      100    // Navigation, geofencing
#define INTERVAL_SLOW        500    // UI updates
#define INTERVAL_WEB         1000   // Web interface updates

// Shared data structure for inter-core communication
struct SharedSystemData {
    // Critical safety data (atomic access)
    volatile bool emergencyStop;
    volatile bool safetyFault;
    volatile bool keySwitch;
    
    // Power data
    volatile float batteryVoltage;
    volatile float batteryCurrent;
    volatile float batteryPower;
    volatile float batterySOC;
    
    // Motor data
    volatile float currentSpeedLeft;
    volatile float currentSpeedRight;
    volatile float motorCurrentLeft;
    volatile float motorCurrentRight;
    volatile uint8_t currentGear;
    volatile bool motorEnabled;
    
    // GPS data
    volatile bool gpsFixed;
    volatile double latitude;
    volatile double longitude;
    volatile float gpsSpeed;
    volatile uint8_t satellites;
    
    // Geofencing data
    volatile bool inSafeZone;
    volatile bool inProhibitedZone;
    volatile bool speedLimited;
    volatile float currentSpeedLimit;
    
    // System status
    volatile unsigned long uptime;
    volatile uint32_t loopCount;
    volatile bool systemReady;
};

class CoreManager {
public:
    CoreManager();
    
    // Initialization
    bool init();
    void setModuleReferences(PowerManager* power, MotorController* motors,
                           Navigation* nav, LightController* lights,
                           SafetySystem* safety, WebInterface* web,
                           UserInterface* ui, InputHandler* inputs);
    
    // Task management
    bool startAllTasks();
    void stopAllTasks();
    
    // Data access (thread-safe)
    SharedSystemData getSharedData();
    void updateSharedData();
    
    // Performance monitoring
    void printTaskStats();
    uint32_t getLoopRate();
    uint32_t getFreeHeap();
    
    // Emergency controls
    void triggerEmergencyStop();
    void clearEmergencyStop();
    
private:
    // Module references
    PowerManager* power;
    MotorController* motors;
    Navigation* navigation;
    LightController* lights;
    SafetySystem* safety;
    WebInterface* webInterface;
    UserInterface* ui;
    InputHandler* inputs;
    
    // Shared data and synchronization
    SharedSystemData sharedData;
    SemaphoreHandle_t dataMutex;
    QueueHandle_t emergencyQueue;
    
    // Task handles
    TaskHandle_t criticalTaskHandle;
    TaskHandle_t motorTaskHandle;
    TaskHandle_t navigationTaskHandle;
    TaskHandle_t webTaskHandle;
    TaskHandle_t uiTaskHandle;
    
    // Performance monitoring
    unsigned long lastStatsUpdate;
    uint32_t loopCounter;
    
    // Static task functions (required for FreeRTOS)
    static void criticalTask(void* parameter);
    static void motorControlTask(void* parameter);
    static void navigationTask(void* parameter);
    static void webInterfaceTask(void* parameter);
    static void userInterfaceTask(void* parameter);
    
    // Internal task implementations
    void runCriticalTask();
    void runMotorControlTask();
    void runNavigationTask();
    void runWebInterfaceTask();
    void runUserInterfaceTask();
    
    // Utility functions
    bool createMutexes();
    void updatePerformanceStats();
    void handleEmergencyQueue();
    
    // Thread-safe data access
    void lockData();
    void unlockData();
    bool tryLockData(uint32_t timeoutMs);
};

// Global instance (defined in CoreManager.cpp)
extern CoreManager coreManager;

// Macros for thread-safe data access
#define LOCK_SHARED_DATA() coreManager.lockData()
#define UNLOCK_SHARED_DATA() coreManager.unlockData()
#define TRY_LOCK_SHARED_DATA(timeout) coreManager.tryLockData(timeout)

// Task monitoring macros
#define TASK_MONITOR_START() unsigned long taskStart = millis()
#define TASK_MONITOR_END(taskName) \
    do { \
        unsigned long taskTime = millis() - taskStart; \
        if (taskTime > 100) { \
            Serial.printf("⚠ Task %s took %lu ms\n", taskName, taskTime); \
        } \
    } while(0)

#endif // CORE_MANAGER_H