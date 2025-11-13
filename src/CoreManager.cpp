#include "CoreManager.h"
#include "PowerManager.h"
#include "MotorController.h"
#include "Navigation.h"
#include "LightController.h"
#include "SafetySystem.h"
#include "WebInterface.h"
#include "UserInterface.h"
#include "InputHandler.h"

// Global instance
CoreManager coreManager;

CoreManager::CoreManager() :
    power(nullptr),
    motors(nullptr),
    navigation(nullptr),
    lights(nullptr),
    safety(nullptr),
    webInterface(nullptr),
    ui(nullptr),
    inputs(nullptr),
    dataMutex(nullptr),
    emergencyQueue(nullptr),
    criticalTaskHandle(nullptr),
    motorTaskHandle(nullptr),
    navigationTaskHandle(nullptr),
    webTaskHandle(nullptr),
    uiTaskHandle(nullptr),
    lastStatsUpdate(0),
    loopCounter(0)
{
    memset(&sharedData, 0, sizeof(sharedData));
}

bool CoreManager::init() {
    Serial.println("Initializing Dual-Core Task Manager...");
    
    // Create synchronization primitives
    if (!createMutexes()) {
        Serial.println("⚠ Failed to create synchronization objects");
        return false;
    }
    
    // Initialize shared data
    sharedData.systemReady = false;
    sharedData.emergencyStop = false;
    sharedData.safetyFault = false;
    sharedData.uptime = 0;
    sharedData.loopCount = 0;
    
    Serial.println("✓ Core Manager initialized");
    return true;
}

void CoreManager::setModuleReferences(PowerManager* pwr, MotorController* mot,
                                    Navigation* nav, LightController* lgt,
                                    SafetySystem* saf, WebInterface* web,
                                    UserInterface* interface, InputHandler* inp) {
    power = pwr;
    motors = mot;
    navigation = nav;
    lights = lgt;
    safety = saf;
    webInterface = web;
    ui = interface;
    inputs = inp;
    
    Serial.println("Core manager module references set");
}

bool CoreManager::startAllTasks() {
    Serial.println("Starting dual-core tasks...");
    
    // Mark system as ready BEFORE starting tasks
    lockData();
    sharedData.systemReady = true;
    unlockData();
    Serial.println("✓ System marked as ready");
    
    // Task 1: Critical Safety (Core 1, Highest Priority)
    xTaskCreatePinnedToCore(
        criticalTask,
        "Critical",
        STACK_SIZE_SMALL,
        this,
        PRIORITY_CRITICAL,
        &criticalTaskHandle,
        CORE_1
    );
    
    // Task 2: Motor Control (Core 1, High Priority)
    xTaskCreatePinnedToCore(
        motorControlTask,
        "Motors",
        STACK_SIZE_MEDIUM,
        this,
        PRIORITY_HIGH,
        &motorTaskHandle,
        CORE_1
    );
    
    // Task 3: Navigation & Geofencing (Core 1, Medium Priority)
    xTaskCreatePinnedToCore(
        navigationTask,
        "Navigation",
        STACK_SIZE_MEDIUM,
        this,
        PRIORITY_MEDIUM,
        &navigationTaskHandle,
        CORE_1
    );
    
    // Task 4: Web Interface (Core 0, Low Priority)
    xTaskCreatePinnedToCore(
        webInterfaceTask,
        "WebInterface",
        STACK_SIZE_LARGE,
        this,
        PRIORITY_LOW,
        &webTaskHandle,
        CORE_0  // Use Core 0 for WiFi tasks
    );
    
    // Task 5: User Interface (Core 1, Low Priority)
    xTaskCreatePinnedToCore(
        userInterfaceTask,
        "UserInterface",
        STACK_SIZE_MEDIUM,
        this,
        PRIORITY_LOW,
        &uiTaskHandle,
        CORE_1
    );
    
    // Wait for all tasks to start
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    Serial.println("✓ All dual-core tasks started successfully");
    printTaskStats();
    
    return true;
}

void CoreManager::stopAllTasks() {
    Serial.println("Stopping all tasks...");
    
    lockData();
    sharedData.systemReady = false;
    unlockData();
    
    if (criticalTaskHandle) vTaskDelete(criticalTaskHandle);
    if (motorTaskHandle) vTaskDelete(motorTaskHandle);
    if (navigationTaskHandle) vTaskDelete(navigationTaskHandle);
    if (webTaskHandle) vTaskDelete(webTaskHandle);
    if (uiTaskHandle) vTaskDelete(uiTaskHandle);
    
    Serial.println("All tasks stopped");
}

// Static task wrapper functions
void CoreManager::criticalTask(void* parameter) {
    CoreManager* manager = static_cast<CoreManager*>(parameter);
    manager->runCriticalTask();
}

void CoreManager::motorControlTask(void* parameter) {
    CoreManager* manager = static_cast<CoreManager*>(parameter);
    manager->runMotorControlTask();
}

void CoreManager::navigationTask(void* parameter) {
    CoreManager* manager = static_cast<CoreManager*>(parameter);
    manager->runNavigationTask();
}

void CoreManager::webInterfaceTask(void* parameter) {
    CoreManager* manager = static_cast<CoreManager*>(parameter);
    manager->runWebInterfaceTask();
}

void CoreManager::userInterfaceTask(void* parameter) {
    CoreManager* manager = static_cast<CoreManager*>(parameter);
    manager->runUserInterfaceTask();
}

// Task implementations
void CoreManager::runCriticalTask() {
    Serial.println("Critical safety task started on Core 1");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (sharedData.systemReady) {
        TASK_MONITOR_START();
        
        if (inputs) {
            // Read key switch (critical for safety)
            bool keyState = inputs->isKeyOn();
            lockData();
            sharedData.keySwitch = keyState;
            unlockData();
        }
        
        if (safety) {
            // Update safety system (highest priority)
            safety->update();
            
            lockData();
            sharedData.safetyFault = !safety->isSystemSafe();
            unlockData();
            
            // Handle emergency conditions
            if (sharedData.emergencyStop || sharedData.safetyFault) {
                if (motors) {
                    motors->emergencyStop();
                }
            }
        }
        
        // Handle emergency queue
        handleEmergencyQueue();
        
        TASK_MONITOR_END("Critical");
        
        // Run at 100Hz (every 10ms)
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(INTERVAL_CRITICAL));
    }
    
    Serial.println("Critical task ended");
    vTaskDelete(nullptr);
}

void CoreManager::runMotorControlTask() {
    Serial.println("Motor control task started on Core 1");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (sharedData.systemReady) {
        TASK_MONITOR_START();
        
        if (power) {
            // Update power monitoring (high frequency for precision)
            power->update();
            
            lockData();
            sharedData.batteryVoltage = power->getBatteryVoltage();
            sharedData.batteryCurrent = power->getBatteryCurrent();
            sharedData.batteryPower = power->getBatteryPower();
            sharedData.batterySOC = power->getBatterySOC();
            sharedData.motorCurrentLeft = power->getMotorLeftCurrent();
            sharedData.motorCurrentRight = power->getMotorRightCurrent();
            unlockData();
        }
        
        if (motors && inputs) {
            // Update regenerative braking parameters
            lockData();
            motors->setBatteryVoltage(sharedData.batteryVoltage);
            motors->setCurrentSpeed(sharedData.gpsSpeed);
            unlockData();

            // Update motor control (high priority for responsiveness)
            float pedalPercent = inputs->getPedalPosition();
            motors->setThrottleCommand(pedalPercent);
            motors->update();

            lockData();
            sharedData.currentSpeedLeft = motors->getCurrentSpeedLeft();
            sharedData.currentSpeedRight = motors->getCurrentSpeedRight();
            sharedData.currentGear = (uint8_t)motors->getCurrentGear();
            sharedData.motorEnabled = !motors->shouldStopForGeofence();
            unlockData();
        }
        
        if (lights) {
            // Update lighting (can be done with motor control)
            lights->update();
        }
        
        TASK_MONITOR_END("Motors");
        
        // Run at 50Hz (every 20ms)
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(INTERVAL_FAST));
    }
    
    Serial.println("Motor control task ended");
    vTaskDelete(nullptr);
}

void CoreManager::runNavigationTask() {
    Serial.println("Navigation task started on Core 1");
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned long lastGPSDebug = 0;
    
    while (sharedData.systemReady) {
        TASK_MONITOR_START();
        
        if (navigation) {
            // Update GPS and geofencing
            navigation->update();
            
            lockData();
            sharedData.gpsFixed = navigation->isGPSFixed();
            sharedData.latitude = navigation->getLatitude();
            sharedData.longitude = navigation->getLongitude();
            sharedData.gpsSpeed = navigation->getSpeed();
            sharedData.satellites = navigation->getSatellites();
            
            // Update geofencing status
            sharedData.inSafeZone = navigation->isInSafeZone();
            sharedData.inProhibitedZone = navigation->isInProhibitedZone();
            sharedData.speedLimited = navigation->shouldLimitSpeedForGeofence();
            sharedData.currentSpeedLimit = navigation->getCurrentSpeedLimit();
            unlockData();
            
            // GPS debug output every 5 seconds
            if (millis() - lastGPSDebug > 5000) {
                Serial.printf("GPS: Sats=%d Fix=%d Lat=%.6f Lng=%.6f Speed=%.1f mph HDOP=%.1f\n",
                    navigation->getSatellites(),
                    navigation->isGPSFixed() ? 1 : 0,
                    navigation->getLatitude(),
                    navigation->getLongitude(),
                    navigation->getSpeed(),
                    navigation->getHDOP());
                lastGPSDebug = millis();
            }
        }
        
        TASK_MONITOR_END("Navigation");
        
        // Run at 10Hz (every 100ms)
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(INTERVAL_MEDIUM));
    }
    
    Serial.println("Navigation task ended");
    vTaskDelete(nullptr);
}

void CoreManager::runWebInterfaceTask() {
    Serial.println("Web interface task started on Core 0");
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (sharedData.systemReady) {
        TASK_MONITOR_START();
        
        if (webInterface) {
            // Update web interface (WiFi operations on Core 0)
            webInterface->update();
        }
        
        TASK_MONITOR_END("WebInterface");
        
        // Run at 1Hz (every 1000ms) - web updates don't need high frequency
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(INTERVAL_WEB));
    }
    
    Serial.println("Web interface task ended");
    vTaskDelete(nullptr);
}

void CoreManager::runUserInterfaceTask() {
    Serial.println("User interface task started on Core 1");
    Serial.printf("DEBUG: systemReady = %d\n", sharedData.systemReady);
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (sharedData.systemReady) {
        TASK_MONITOR_START();

        if (ui && inputs) {
            // Update input handler (read all buttons and encoder) - Core 1
            inputs->update();

            // DEBUG: Log button states every cycle
            static int debugCounter = 0;
            if (debugCounter++ % 4 == 0) { // Every 2 seconds (4 * 500ms)
                Serial.printf("BTN DEBUG - Enc:%d Up:%d Down:%d | GPIO Enc:%d Up:%d Down:%d\n",
                    inputs->isEncoderPressed(),
                    inputs->isShiftUpHeld(),
                    inputs->isShiftDownHeld(),
                    digitalRead(GPIO_ENCODER_BTN),
                    digitalRead(GPIO_KEY0),
                    digitalRead(GPIO_KEY1));
            }

            // Handle encoder button (use edge detection, not state)
            if (inputs->encoderButtonPressed()) {
                ui->cycleScreen();
                Serial.println("✓ Encoder pressed - cycling screen");
            }

            // Handle encoder rotation
            int16_t encoderDelta = inputs->getEncoderDelta();
            if (encoderDelta != 0) {
                ui->handleEncoderRotation(encoderDelta);
                Serial.printf("✓ Encoder rotated: %d\n", encoderDelta);
            }

            // Check shift buttons and store in shared data for main loop
            bool shiftUp = inputs->isShiftUpPressed();
            bool shiftDown = inputs->isShiftDownPressed();
            
            if (shiftUp) Serial.println("✓ Shift UP button pressed");
            if (shiftDown) Serial.println("✓ Shift DOWN button pressed");
            
            lockData();
            sharedData.shiftUpPressed = shiftUp;
            sharedData.shiftDownPressed = shiftDown;
            unlockData();

            // Update display with shared data
            lockData();
            ui->setVoltage(sharedData.batteryVoltage);
            ui->setCurrent(sharedData.batteryCurrent);
            ui->setPower(sharedData.batteryPower);
            ui->setBatterySOC(sharedData.batterySOC);
            ui->setSpeed(sharedData.gpsSpeed);
            ui->setMotorData(sharedData.motorCurrentLeft, sharedData.motorCurrentRight, 0);
            ui->setGPSData(sharedData.satellites, sharedData.gpsFixed);
            ui->setGPSCoordinates(sharedData.latitude, sharedData.longitude);
            unlockData();

            // Update navigation data (call directly from navigation, not shared data)
            if (navigation) {
                float bearing = navigation->getBearingToHome();
                float distance = navigation->getDistanceToHome();
                float course = navigation->getCourse();
                ui->setNavigationData(bearing, distance, course);
            }

            ui->update();
        }
        
        // Update performance stats
        updatePerformanceStats();
        
        TASK_MONITOR_END("UserInterface");
        
        // Run at 2Hz (every 500ms) - UI updates don't need high frequency
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(INTERVAL_SLOW));
    }
    
    Serial.println("User interface task ended");
    vTaskDelete(nullptr);
}

bool CoreManager::createMutexes() {
    dataMutex = xSemaphoreCreateMutex();
    if (!dataMutex) {
        Serial.println("Failed to create data mutex");
        return false;
    }
    
    emergencyQueue = xQueueCreate(10, sizeof(bool));
    if (!emergencyQueue) {
        Serial.println("Failed to create emergency queue");
        return false;
    }
    
    return true;
}

void CoreManager::lockData() {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
}

void CoreManager::unlockData() {
    xSemaphoreGive(dataMutex);
}

bool CoreManager::tryLockData(uint32_t timeoutMs) {
    return xSemaphoreTake(dataMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

SharedSystemData CoreManager::getSharedData() {
    SharedSystemData data;
    lockData();
    data = sharedData;  // Safe copy now that atomics are removed
    unlockData();
    return data;
}

void CoreManager::updateSharedData() {
    lockData();
    sharedData.uptime = millis();
    sharedData.loopCount = loopCounter++;
    unlockData();
}

void CoreManager::clearShiftUpFlag() {
    lockData();
    sharedData.shiftUpPressed = false;
    unlockData();
}

void CoreManager::clearShiftDownFlag() {
    lockData();
    sharedData.shiftDownPressed = false;
    unlockData();
}

void CoreManager::triggerEmergencyStop() {
    bool emergency = true;
    xQueueSend(emergencyQueue, &emergency, 0);
    
    lockData();
    sharedData.emergencyStop = true;
    unlockData();
    
    Serial.println("🚨 EMERGENCY STOP TRIGGERED via Core Manager");
}

void CoreManager::clearEmergencyStop() {
    lockData();
    sharedData.emergencyStop = false;
    unlockData();
    
    Serial.println("✅ Emergency stop cleared");
}

void CoreManager::handleEmergencyQueue() {
    bool emergency;
    while (xQueueReceive(emergencyQueue, &emergency, 0) == pdTRUE) {
        if (emergency && motors) {
            motors->emergencyStop();
            Serial.println("Emergency stop executed from queue");
        }
    }
}

void CoreManager::updatePerformanceStats() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();
    
    if (now - lastUpdate >= 5000) {  // Every 5 seconds
        printTaskStats();
        lastUpdate = now;
    }
}

void CoreManager::printTaskStats() {
    Serial.println("=== Task Performance Stats ===");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Uptime: %lu ms\n", sharedData.uptime);
    Serial.printf("Loop Count: %lu\n", sharedData.loopCount);
    
    // Task runtime statistics - simplified version
    Serial.println("Task Runtime Stats:");
    Serial.println("Core 0 Tasks: WebInterface, UserInterface");
    Serial.println("Core 1 Tasks: Safety, Motor, Navigation");
    Serial.printf("Web Task Free Stack: %d\n", uxTaskGetStackHighWaterMark(webTaskHandle));
    Serial.printf("Critical Task Free Stack: %d\n", uxTaskGetStackHighWaterMark(criticalTaskHandle));
    
    Serial.println("===============================");
}

uint32_t CoreManager::getLoopRate() {
    return sharedData.loopCount;
}

uint32_t CoreManager::getFreeHeap() {
    return ESP.getFreeHeap();
}