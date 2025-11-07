# Geofencing System Documentation

## Overview
The **Razors Edge** electric vehicle controller includes a comprehensive GPS-based geofencing system for safety and operational control. The system monitors vehicle position in real-time and automatically enforces speed limits, prohibited zones, and emergency boundaries.

## Features

### Fence Types
- **SAFE_ZONE**: Designated safe areas (home, parking)
- **SPEED_LIMIT**: Areas with reduced speed restrictions
- **PROHIBITED**: No-entry zones (busy roads, private property)
- **WARNING**: Areas requiring driver attention
- **EMERGENCY_RTB**: Emergency return-to-base boundaries

### Fence Shapes
- **Circular**: Defined by center point and radius
- **Rectangular**: Defined by two corner coordinates
- **Polygon**: Defined by up to 8 coordinate points

### Automatic Actions
- **ACTION_NONE**: Monitoring only
- **ACTION_WARN**: Display warning message
- **ACTION_LIMIT_SPEED**: Reduce maximum speed
- **ACTION_STOP**: Immediate motor stop
- **ACTION_RETURN_HOME**: Navigate back to safe zone
- **ACTION_EMERGENCY_STOP**: Emergency shutdown

## API Reference

### Basic Setup
```cpp
#include "Navigation.h"

Navigation gps;

void setup() {
    gps.init();  // Initializes GPS and geofencing
    
    // Set home position (required for safety features)
    gps.setHomePosition(40.7128, -74.0060);  // NYC example
}

void loop() {
    gps.update();  // Updates GPS and processes geofences
    
    // Check geofencing status
    if (gps.shouldStopForGeofence()) {
        // Vehicle should stop immediately
        motors.emergencyStop();
    }
    
    if (gps.shouldLimitSpeedForGeofence()) {
        // Apply speed reduction
        float speedFactor = gps.getGeofenceSpeedFactor();
        motors.setSpeed(normalSpeed * speedFactor);
    }
}
```

### Adding Geofences
```cpp
// Add circular safe zone (100m radius)
gps.addGeofence(1, FENCE_SAFE_ZONE, ACTION_NONE,
               40.7128, -74.0060, 100.0f);

// Add speed limit zone (500m radius, 15 mph max)
gps.addGeofence(2, FENCE_SPEED_LIMIT, ACTION_LIMIT_SPEED,
               40.7130, -74.0065, 500.0f, 15.0f);

// Add prohibited zone
gps.addGeofence(3, FENCE_PROHIBITED, ACTION_STOP,
               40.7135, -74.0070, 50.0f);

// Emergency boundary (1km radius)
gps.addGeofence(4, FENCE_EMERGENCY_RTB, ACTION_RETURN_HOME,
               40.7128, -74.0060, 1000.0f);
```

### Status Monitoring
```cpp
GeofenceStatus status = gps.getGeofenceStatus();

Serial.printf("In Safe Zone: %s\n", 
              status.inSafeZone ? "YES" : "NO");
Serial.printf("Speed Limit: %.1f mph\n", 
              status.currentSpeedLimit);
Serial.printf("Distance to Home: %.0f meters\n", 
              status.distanceToSafeZone);
Serial.printf("Active Fence ID: %d\n", 
              status.activeFenceId);
```

## Configuration Examples

### Neighborhood Setup
```cpp
void setupNeighborhoodGeofences() {
    // Home base (safe zone)
    gps.addGeofence(1, FENCE_SAFE_ZONE, ACTION_NONE,
                   40.7128, -74.0060, 50.0f);
    
    // Neighborhood speed limit (15 mph)
    gps.addGeofence(2, FENCE_SPEED_LIMIT, ACTION_LIMIT_SPEED,
                   40.7128, -74.0060, 300.0f, 15.0f);
    
    // Main road prohibition
    gps.addGeofence(3, FENCE_PROHIBITED, ACTION_STOP,
                   40.7140, -74.0070, 25.0f);
    
    // Emergency boundary (don't go beyond 500m)
    gps.addGeofence(4, FENCE_EMERGENCY_RTB, ACTION_RETURN_HOME,
                   40.7128, -74.0060, 500.0f);
}
```

### Park/Recreation Area Setup
```cpp
void setupParkGeofences() {
    // Park boundary (polygon)
    GeoPoint parkBoundary[] = {
        {40.7120, -74.0080},
        {40.7130, -74.0080},
        {40.7130, -74.0040},
        {40.7120, -74.0040}
    };
    
    // Allowed riding area
    gps.addPolygonFence(1, FENCE_SAFE_ZONE, ACTION_NONE,
                       parkBoundary, 4);
    
    // Playground area (prohibited)
    gps.addGeofence(2, FENCE_PROHIBITED, ACTION_STOP,
                   40.7125, -74.0060, 30.0f);
    
    // Walking path (speed limit 5 mph)
    gps.addGeofence(3, FENCE_SPEED_LIMIT, ACTION_LIMIT_SPEED,
                   40.7122, -74.0070, 20.0f, 5.0f);
}
```

## Safety Features

### Automatic Integration
The geofencing system automatically integrates with:
- **Motor Controller**: Applies speed limits and stops
- **Safety System**: Triggers emergency protocols
- **User Interface**: Displays warnings and status
- **Data Logging**: Records all fence violations

### Emergency Protocols
1. **Prohibited Zone Entry**: Immediate motor stop
2. **Emergency Boundary**: Auto-return to home
3. **Speed Violations**: Gradual speed reduction
4. **GPS Loss**: Maintain last known restrictions

### Violation Tracking
```cpp
uint16_t violations = gps.getTotalViolations();
unsigned long lastViolation = gps.getLastViolationTime();

// Clear violation history
gps.clearViolationHistory();
```

## Hardware Requirements

### GPS Module
- **Minimum**: 4 satellite fix required
- **Accuracy**: HDOP < 3.0 for fence activation
- **Update Rate**: 1-5 Hz recommended
- **Cold Start**: May take 30-60 seconds

### Performance Specs
- **Position Accuracy**: ±3 meters typical
- **Response Time**: <1 second for fence detection
- **Memory Usage**: ~2KB RAM for 10 fences
- **Processing**: <1ms per fence check

## Troubleshooting

### Common Issues
1. **GPS Not Fixed**: Wait for satellite acquisition
2. **Fence Not Triggering**: Check coordinate accuracy
3. **Speed Limit Not Applied**: Verify gear selection
4. **False Violations**: Increase fence buffer distance

### Debug Commands
```cpp
// Check GPS status
Serial.printf("GPS Fixed: %s\n", gps.isGPSFixed() ? "YES" : "NO");
Serial.printf("Satellites: %d\n", gps.getSatellites());
Serial.printf("HDOP: %.1f\n", gps.getHDOP());

// Check current position
Serial.printf("Position: %.6f, %.6f\n", 
              gps.getLatitude(), gps.getLongitude());

// Check fence status
GeofenceStatus status = gps.getGeofenceStatus();
Serial.printf("Active Fences: %s\n", 
              status.anyFenceActive ? "YES" : "NO");
```

### Calibration
1. **Set Accurate Home Position**: Use smartphone GPS for reference
2. **Test Fence Boundaries**: Walk/drive perimeter to verify
3. **Adjust Buffer Distances**: Account for GPS accuracy
4. **Validate Speed Limits**: Test with known speeds

## Legal and Safety Notes

### Important Warnings
- ⚠️ **GPS is not 100% reliable** - Always maintain visual awareness
- ⚠️ **Geofencing is an aid, not a replacement** for safe operation
- ⚠️ **Test thoroughly** before relying on geofences
- ⚠️ **Update coordinates** when moving to new areas

### Recommended Practices
1. **Always set conservative boundaries** (larger buffer zones)
2. **Test all geofences** before first use
3. **Keep manual override** capability
4. **Regular system checks** and updates
5. **Respect local laws** and property rights

## Integration Examples

### With Motor Controller
```cpp
void loop() {
    // Normal motor control
    motors.setThrottleCommand(throttleInput);
    
    // Geofencing automatically applied in motors.update()
    motors.update();
    
    // Check if geofencing intervention occurred
    if (motors.shouldStopForGeofence()) {
        ui.displayWarning("GEOFENCE STOP ACTIVE");
    }
}
```

### With User Interface
```cpp
void updateDisplay() {
    GeofenceStatus status = gps.getGeofenceStatus();
    
    if (status.inSafeZone) {
        ui.setStatusLED(LED_GREEN);
    } else if (status.inProhibitedZone) {
        ui.setStatusLED(LED_RED);
        ui.displayWarning("PROHIBITED AREA");
    } else if (status.speedLimited) {
        ui.setStatusLED(LED_YELLOW);
        ui.displayMessage("SPEED LIMITED");
    }
}
```

---

*For technical support and updates, see the main README.md file.*