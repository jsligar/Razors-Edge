#ifndef CORE_MANAGER_H
#define CORE_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <atomic>

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
// NOTE: Critical safety flags use std::atomic for lock-free thread safety.
// Other data is accessed via mutex-protected getters/setters.
struct SharedSystemData {
    // Critical safety data (atomic for lock-free access)
    std::atomic<bool> emergencyStop;
    std::atomic<bool> safetyFault;
    std::atomic<bool> keySwitch;
    std::atomic<bool> motorEnabled;
    std::atomic<bool> systemReady;

    // Power data (mutex-protected access required)
    float batteryVoltage;
    float batteryCurrent;
    float batteryPower;
    float batterySOC;

    // Motor data (mutex-protected access required)
    float currentSpeedLeft;
    float currentSpeedRight;
    float motorCurrentLeft;
    float motorCurrentRight;
    uint8_t currentGear;

    // GPS data (mutex-protected access required)
    bool gpsFixed;
    double latitude;
    double longitude;
    float gpsSpeed;
    uint8_t satellites;

    // Geofencing data (mutex-protected access required)
    bool inSafeZone;
    bool inProhibitedZone;
    bool speedLimited;
    float currentSpeedLimit;

    // System status (mutex-protected access required)
    unsigned long uptime;
    uint32_t loopCount;

    // Constructor to initialize atomics
    SharedSystemData() :
        emergencyStop(false),
        safetyFault(false),
        keySwitch(false),
        motorEnabled(false),
        systemReady(false),
        batteryVoltage(0.0f),
        batteryCurrent(0.0f),
        batteryPower(0.0f),
        batterySOC(0.0f),
        currentSpeedLeft(0.0f),
        currentSpeedRight(0.0f),
        motorCurrentLeft(0.0f),
        motorCurrentRight(0.0f),
        currentGear(0),
        gpsFixed(false),
        latitude(0.0),
        longitude(0.0),
        gpsSpeed(0.0f),
        satellites(0),
        inSafeZone(false),
        inProhibitedZone(false),
        speedLimited(false),
        currentSpeedLimit(0.0f),
        uptime(0),
        loopCount(0)
    {}
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