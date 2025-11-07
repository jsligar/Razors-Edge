#ifndef GEOFENCING_H
#define GEOFENCING_H

#include <Arduino.h>
#include "config.h"

// Geofence types
enum GeofenceType {
    FENCE_SAFE_ZONE,        // Safe area (home, charging station)
    FENCE_SPEED_LIMIT,      // Speed restricted zone
    FENCE_PROHIBITED,       // No-go zone (roads, private property)
    FENCE_WARNING,          // Warning zone (approaching boundary)
    FENCE_EMERGENCY_RTB     // Emergency return-to-base zone
};

// Geofence actions
enum GeofenceAction {
    ACTION_NONE,            // No action required
    ACTION_WARN,            // Display warning
    ACTION_LIMIT_SPEED,     // Reduce maximum speed
    ACTION_STOP,            // Stop vehicle
    ACTION_RETURN_HOME,     // Force return to safe zone
    ACTION_EMERGENCY_STOP   // Immediate emergency stop
};

// Geofence shapes
enum GeofenceShape {
    SHAPE_CIRCLE,           // Circular fence (center + radius)
    SHAPE_RECTANGLE,        // Rectangular fence (corners)
    SHAPE_POLYGON           // Custom polygon (up to 8 points)
};

// Maximum fence points for polygon
#define MAX_FENCE_POINTS 8
#define MAX_GEOFENCES 10

struct GeoPoint {
    double latitude;
    double longitude;
};

struct GeofenceZone {
    bool enabled;
    uint8_t id;
    GeofenceType type;
    GeofenceShape shape;
    GeofenceAction action;
    float speedLimit;       // mph (for speed limit zones)
    
    // Fence definition
    union {
        struct {
            GeoPoint center;
            float radius;   // meters
        } circle;
        
        struct {
            GeoPoint corner1;
            GeoPoint corner2;
        } rectangle;
        
        struct {
            GeoPoint points[MAX_FENCE_POINTS];
            uint8_t numPoints;
        } polygon;
    } fence;
    
    // Breach tracking
    bool currentlyInside;
    bool previouslyInside;
    unsigned long breachStartTime;
    unsigned long lastBreachTime;
    uint16_t breachCount;
};

struct GeofenceStatus {
    bool anyFenceActive;
    bool inSafeZone;
    bool inProhibitedZone;
    bool speedLimited;
    float currentSpeedLimit;
    GeofenceAction currentAction;
    uint8_t activeFenceId;
    float distanceToNearestFence;
    float bearingToSafeZone;
    float distanceToSafeZone;
};

class Geofencing {
public:
    Geofencing();
    
    // Initialization
    bool init();
    void loadDefaultFences();
    
    // Main update (call with current GPS position)
    void update(double latitude, double longitude, bool gpsValid);
    
    // Fence management
    bool addCircularFence(uint8_t id, GeofenceType type, GeofenceAction action,
                         double centerLat, double centerLng, float radiusMeters,
                         float speedLimit = 0.0f);
    
    bool addRectangularFence(uint8_t id, GeofenceType type, GeofenceAction action,
                            double lat1, double lng1, double lat2, double lng2,
                            float speedLimit = 0.0f);
    
    bool addPolygonFence(uint8_t id, GeofenceType type, GeofenceAction action,
                        GeoPoint* points, uint8_t numPoints, float speedLimit = 0.0f);
    
    bool removeFence(uint8_t id);
    bool enableFence(uint8_t id, bool enabled);
    void clearAllFences();
    
    // Status getters
    GeofenceStatus getStatus();
    bool isInSafeZone();
    bool isInProhibitedZone();
    float getCurrentSpeedLimit();
    GeofenceAction getCurrentAction();
    
    // Safety functions
    bool shouldStop();
    bool shouldReturnHome();
    bool shouldLimitSpeed();
    float getSpeedLimitFactor(); // 0.0-1.0 multiplier
    
    // Emergency functions
    void triggerEmergencyMode();
    void setHomePosition(double latitude, double longitude);
    bool hasHomePosition();
    float getDistanceToHome();
    float getBearingToHome();
    
    // Fence violation tracking
    uint16_t getTotalViolations();
    unsigned long getLastViolationTime();
    void clearViolationHistory();
    
    // Configuration
    void setGlobalSpeedLimit(float maxSpeedMph);
    void setWarningDistance(float distanceMeters);
    void setEmergencyMode(bool enabled);
    
private:
    // Fence storage
    GeofenceZone fences[MAX_GEOFENCES];
    uint8_t numFences;
    
    // Current status
    GeofenceStatus status;
    double currentLat;
    double currentLng;
    bool positionValid;
    unsigned long lastUpdateTime;
    
    // Home/safe position
    GeoPoint homePosition;
    bool homePositionSet;
    
    // Configuration
    float globalSpeedLimit;     // Global maximum speed
    float warningDistance;      // Distance for early warnings (meters)
    bool emergencyModeActive;
    
    // Violation tracking
    uint16_t totalViolations;
    unsigned long lastViolationTime;
    
    // Internal calculation methods
    bool isPointInCircle(double lat, double lng, const GeofenceZone& fence);
    bool isPointInRectangle(double lat, double lng, const GeofenceZone& fence);
    bool isPointInPolygon(double lat, double lng, const GeofenceZone& fence);
    
    float calculateDistance(double lat1, double lng1, double lat2, double lng2);
    float calculateBearing(double lat1, double lng1, double lat2, double lng2);
    
    void updateFenceStatus(GeofenceZone& fence);
    void processFenceViolation(GeofenceZone& fence);
    void updateGlobalStatus();
    
    // Utility functions
    bool isValidPosition(double lat, double lng);
    float constrainSpeed(float speed);
    void logFenceEvent(uint8_t fenceId, const char* event);
};

#endif // GEOFENCING_H