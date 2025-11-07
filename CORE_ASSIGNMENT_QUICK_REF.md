# Quick Reference: Dual-Core Task Assignment

## 🔧 **Core Assignment Summary**

### **Core 1 (Application CPU) - Real-Time Vehicle Control**
```
Task                Frequency    Priority    Function
─────────────────────────────────────────────────────────────
Critical Safety     100Hz        5          Emergency monitoring
Motor Control       50Hz         4          PWM, power, throttle  
Navigation          10Hz         3          GPS, geofencing
User Interface      2Hz          2          OLED, buttons
Main Loop           Variable     1          Coordination
```

### **Core 0 (Protocol CPU) - Communications**
```
Task                Frequency    Priority    Function
─────────────────────────────────────────────────────────────
Web Interface       1Hz          2          WiFi, HTTP server
WiFi Stack          Continuous   System     Network management
FreeRTOS Idle       Continuous   0          Power management
```

## ⚡ **Performance Guarantees**

| Function | Maximum Response Time | Update Rate |
|----------|----------------------|-------------|
| Emergency Stop | **<5ms** | 100Hz |
| Motor Control | **20ms** | 50Hz |
| Geofence Check | **100ms** | 10Hz |
| Web Response | **1000ms** | 1Hz |

## 🛡️ **Safety Features**

- **Hardware Emergency Stop:** Physical controls always work
- **Task Watchdogs:** Automatic restart of failed tasks  
- **Memory Protection:** Stack overflow detection
- **Fault Isolation:** Core failures don't affect safety
- **Graceful Degradation:** Fallback to single-core mode

## 📊 **Memory Usage**

| Component | RAM | Flash |
|-----------|-----|-------|
| Task Stacks | ~24KB | - |
| WiFi Stack | ~32KB | ~1MB |
| Application | ~16KB | ~1.2MB |
| **Total** | **~80KB** | **~2.2MB** |

## 🔍 **Monitoring Commands**

```cpp
// Check task performance
coreManager.printTaskStats();

// Monitor memory usage  
Serial.printf("Free Heap: %d\n", ESP.getFreeHeap());

// Emergency stop from code
coreManager.triggerEmergencyStop();

// Get real-time data
SharedSystemData data = coreManager.getSharedData();
```

## ⚙️ **Configuration**

Key settings in `CoreManager.h`:
```cpp
#define PRIORITY_CRITICAL    5    // Safety tasks
#define PRIORITY_HIGH        4    // Motor control  
#define PRIORITY_MEDIUM      3    // Navigation
#define PRIORITY_LOW         2    // UI/Web

#define INTERVAL_CRITICAL    10   // 100Hz (10ms)
#define INTERVAL_FAST        20   // 50Hz (20ms)
#define INTERVAL_MEDIUM      100  // 10Hz (100ms)
```

## 🚨 **Emergency Protocol**

1. **Detection** → Any task can trigger emergency
2. **Propagation** → Message queue to critical task  
3. **Response** → Motors disabled within 5ms
4. **Recovery** → Manual key cycle required

---

*This dual-core architecture ensures your electric vehicle controller maintains real-time safety performance while providing advanced features like WiFi connectivity and geofencing.*