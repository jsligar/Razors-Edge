# Dual-Core Architecture Documentation

## Overview
The Razors Edge controller leverages the ESP32's dual-core architecture to ensure optimal performance and real-time responsiveness. By carefully assigning tasks to specific cores, we achieve deterministic timing for critical safety functions while maintaining responsive user interfaces and WiFi connectivity.

## Core Assignment Strategy

### **Core 1 (Application CPU) - Real-Time Operations**
Handles time-critical vehicle control and safety functions with deterministic timing.

**Tasks Running on Core 1:**
- **Critical Safety Task** (100Hz, Priority 5)
  - Key switch monitoring
  - Emergency stop detection
  - Safety system updates
  - Fault detection and response

- **Motor Control Task** (50Hz, Priority 4)
  - Power monitoring (INA228 sensors)
  - Motor PWM control
  - Throttle processing
  - LED lighting updates

- **Navigation Task** (10Hz, Priority 3)
  - GPS data parsing
  - Geofencing calculations
  - Position tracking
  - Speed calculations

- **User Interface Task** (2Hz, Priority 2)
  - OLED display updates
  - Button press handling
  - Menu navigation
  - Status displays

### **Core 0 (Protocol CPU) - Communication & Background**
Handles network communications and non-critical background tasks.

**Tasks Running on Core 0:**
- **Web Interface Task** (1Hz, Priority 2)
  - WiFi management
  - Web server handling
  - HTTP request processing
  - JSON data serialization

**System Tasks (Automatic):**
- WiFi stack management
- TCP/IP processing
- FreeRTOS idle task
- Watchdog timers

## Task Priority System

### Priority Levels (1-5, higher = more urgent)
```
Priority 5 (CRITICAL):    Safety monitoring, emergency stops
Priority 4 (HIGH):        Motor control, power monitoring  
Priority 3 (MEDIUM):      Navigation, geofencing
Priority 2 (LOW):         User interface, web interface
Priority 1 (BACKGROUND):  Logging, diagnostics
```

### Task Timing & Performance
```
Task Name          Core  Priority  Frequency  Stack   Function
─────────────────────────────────────────────────────────────────
Critical Safety    1     5         100Hz     2KB     Emergency monitoring
Motor Control      1     4         50Hz      4KB     PWM & power control
Navigation         1     3         10Hz      4KB     GPS & geofencing
User Interface     1     2         2Hz       4KB     OLED & buttons
Web Interface      0     2         1Hz       8KB     WiFi & HTTP server
```

## Thread-Safe Data Sharing

### Shared Data Structure
```cpp
struct SharedSystemData {
    // Critical safety (atomic access)
    volatile bool emergencyStop;
    volatile bool safetyFault;
    volatile bool keySwitch;
    
    // Power monitoring
    volatile float batteryVoltage;
    volatile float batteryCurrent;
    volatile float batterySOC;
    
    // Motor control
    volatile float currentSpeedLeft;
    volatile float currentSpeedRight;
    volatile uint8_t currentGear;
    
    // GPS & geofencing
    volatile bool gpsFixed;
    volatile double latitude;
    volatile double longitude;
    volatile bool inSafeZone;
    volatile bool speedLimited;
};
```

### Synchronization Mechanisms
- **Mutex Locks:** Protect shared data during updates
- **Message Queues:** Emergency commands between tasks
- **Atomic Variables:** Simple boolean flags
- **Memory Barriers:** Ensure data consistency

## Performance Optimization

### Real-Time Guarantees
- **Safety Tasks:** Maximum 10ms response time
- **Motor Control:** Guaranteed 20ms update cycles  
- **Emergency Stop:** <5ms from trigger to motor shutdown
- **Geofencing:** Position checked every 100ms

### Memory Management
```
Component              RAM Usage    Flash Usage
─────────────────────────────────────────────────
Task Stacks            ~24KB        -
Shared Data            ~200 bytes   -
FreeRTOS Kernel        ~8KB         ~32KB
WiFi Stack             ~32KB        ~1MB
Application Code       ~16KB        ~1.2MB
Total Estimated        ~80KB        ~2.2MB
```

### CPU Load Distribution
```
Core 0 (Protocol):
├── WiFi Stack         ~40% (when active)
├── Web Interface      ~10%
├── TCP/IP             ~20%
└── Idle              ~30%

Core 1 (Application):
├── Critical Safety    ~20%
├── Motor Control      ~30%
├── Navigation         ~15%
├── User Interface     ~10%
└── Main Loop         ~25%
```

## Task Communication

### Inter-Task Data Flow
```
Critical Safety ──┐
                  ├──→ Shared Data ──→ Web Interface
Motor Control ────┤                 ┌→ User Interface
                  │                 │
Navigation ───────┼─────────────────┘
                  │
Input Handler ────┘
```

### Emergency Communication
```cpp
// Emergency stop propagation
1. Input Detected       → Critical Task (Core 1)
2. Emergency Queue      → All Motor Tasks  
3. Shared Data Flag     → Web Interface (Core 0)
4. Physical Motor Stop  → Hardware Layer
```

## Code Examples

### Task Creation
```cpp
// Create high-priority motor control task on Core 1
xTaskCreatePinnedToCore(
    motorControlTask,     // Task function
    "Motors",            // Task name
    4096,               // Stack size (words)
    this,               // Parameters
    PRIORITY_HIGH,      // Priority level
    &motorTaskHandle,   // Task handle
    CORE_1             // Pin to Core 1
);
```

### Thread-Safe Data Access
```cpp
// Reading shared data
SharedSystemData data = coreManager.getSharedData();
float voltage = data.batteryVoltage;

// Writing shared data (inside task)
coreManager.lockData();
sharedData.batteryVoltage = newVoltage;
coreManager.unlockData();
```

### Emergency Communication
```cpp
// Trigger emergency from any task
coreManager.triggerEmergencyStop();

// Handle emergency in critical task  
void runCriticalTask() {
    if (sharedData.emergencyStop) {
        motors->emergencyStop();
        // Emergency handled in <5ms
    }
}
```

## Debugging & Monitoring

### Task Performance Monitoring
```cpp
// Print runtime statistics
coreManager.printTaskStats();

// Output:
Task Runtime Stats:
Critical    12.5%   Core 1
Motors      18.7%   Core 1  
Navigation   8.2%   Core 1
WebInterface 5.1%   Core 0
UserInterface 3.4%  Core 1
```

### Real-Time Debugging
```cpp
// Task execution time monitoring
TASK_MONITOR_START();
// ... task work ...
TASK_MONITOR_END("TaskName");  // Warns if >100ms
```

### Memory Monitoring
```cpp
Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("Task Stack High Water: %d\n", 
              uxTaskGetStackHighWaterMark(taskHandle));
```

## Safety Features

### Fault Tolerance
- **Task Watchdogs:** Each task monitored for responsiveness
- **Memory Protection:** Stack overflow detection
- **Priority Inheritance:** Prevents priority inversion
- **Graceful Degradation:** Fallback to single-core if needed

### Emergency Protocols
```
Emergency Stop Sequence:
1. Signal detected (hardware/software)
2. Emergency flag set (atomic operation)  
3. Critical task responds (<5ms)
4. Motors disabled immediately
5. All tasks notified
6. System enters safe state
```

### Error Recovery
- **Task Restart:** Automatic restart of failed tasks
- **Heap Monitoring:** Prevent memory exhaustion
- **Stack Guards:** Detect stack overflow
- **Deadlock Detection:** Prevent mutex deadlocks

## Configuration Options

### Task Priorities (config.h)
```cpp
#define PRIORITY_CRITICAL    5  // Safety, emergency
#define PRIORITY_HIGH        4  // Motors, power
#define PRIORITY_MEDIUM      3  // Navigation
#define PRIORITY_LOW         2  // UI, web
```

### Update Intervals
```cpp
#define INTERVAL_CRITICAL    10   // 100Hz safety
#define INTERVAL_FAST        20   // 50Hz motor control
#define INTERVAL_MEDIUM      100  // 10Hz navigation
#define INTERVAL_SLOW        500  // 2Hz UI updates
```

### Stack Sizes
```cpp
#define STACK_SIZE_LARGE     8192  // Web interface
#define STACK_SIZE_MEDIUM    4096  // Navigation, motors
#define STACK_SIZE_SMALL     2048  // Safety, UI
```

## Best Practices

### Task Design
1. **Keep Critical Tasks Simple:** Minimize processing in high-priority tasks
2. **Avoid Blocking Operations:** Use timeouts and non-blocking calls
3. **Minimize Shared Data:** Reduce mutex contention
4. **Monitor Performance:** Regular timing analysis

### Memory Management
1. **Static Allocation:** Prefer static over dynamic allocation
2. **Stack Monitoring:** Check high water marks regularly
3. **Heap Fragmentation:** Minimize malloc/free usage
4. **Buffer Reuse:** Reuse buffers when possible

### Debugging Tips
1. **Serial Logging:** Use task-specific log prefixes
2. **Performance Monitoring:** Regular task timing checks
3. **Memory Profiling:** Monitor heap and stack usage
4. **Core Affinity:** Verify tasks run on intended cores

## Future Enhancements

### Potential Optimizations
- **DMA Transfers:** Offload I2C/SPI to DMA
- **Interrupt Handling:** Move time-critical code to ISRs
- **Cache Optimization:** Pin frequently-used code/data
- **Power Management:** Dynamic frequency scaling

### Scalability
- **Additional Tasks:** Framework supports easy expansion
- **Load Balancing:** Automatic task distribution
- **Priority Adaptation:** Dynamic priority adjustment
- **Resource Monitoring:** Automatic resource management

---

*This dual-core architecture ensures deterministic real-time performance for safety-critical vehicle control while maintaining responsive user interfaces and network connectivity.*