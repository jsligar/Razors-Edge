#include "Geofencing.h"
#include <math.h>

// Earth radius in meters for distance calculations
#define EARTH_RADIUS_M 6371000.0

// Convert degrees to radians
#define DEG_TO_RAD(x) ((x) * PI / 180.0)
#define RAD_TO_DEG(x) ((x) * 180.0 / PI)

// Distance thresholds
#define MIN_MOVEMENT_DISTANCE 1.0  // meters
#define MAX_VALID_SPEED 50.0       // mph

Geofencing::Geofencing() :
    numFences(0),
    currentLat(0.0),
    currentLng(0.0),
    positionValid(false),
    lastUpdateTime(0),
    homePositionSet(false),
    globalSpeedLimit(25.0f),    // 25 mph default
    warningDistance(50.0f),     // 50 meter warning zone
    emergencyModeActive(false),
    totalViolations(0),
    lastViolationTime(0)
{
    // Initialize status
    memset(&status, 0, sizeof(status));
    status.currentSpeedLimit = globalSpeedLimit;
    
    // Initialize fences
    memset(fences, 0, sizeof(fences));
    
    // Initialize home position
    memset(&homePosition, 0, sizeof(homePosition));
}

bool Geofencing::init() {
    Serial.println("Initializing Geofencing System...");
    
    // Load default safety fences
    loadDefaultFences();
    
    Serial.printf("✓ Geofencing initialized with %d default fences\n", numFences);
    return true;
}

void Geofencing::loadDefaultFences() {
    // Clear existing fences
    clearAllFences();
    
    // Example default fences - customize for your area
    
    // Fence 1: Home safe zone (100m radius)
    // Replace with your actual home coordinates
    addCircularFence(1, FENCE_SAFE_ZONE, ACTION_NONE,
                    40.7128, -74.0060, 100.0f);  // Example: NYC coordinates
    
    // Fence 2: Neighborhood speed limit (500m radius, 15 mph)
    addCircularFence(2, FENCE_SPEED_LIMIT, ACTION_LIMIT_SPEED,
                    40.7128, -74.0060, 500.0f, 15.0f);
    
    // Fence 3: Prohibited area - busy road (rectangle)
    addRectangularFence(3, FENCE_PROHIBITED, ACTION_STOP,
                       40.7120, -74.0070, 40.7140, -74.0050);
    
    // Fence 4: Emergency boundary (1km radius)
    addCircularFence(4, FENCE_EMERGENCY_RTB, ACTION_RETURN_HOME,
                    40.7128, -74.0060, 1000.0f);
    
    Serial.println("  Default geofences loaded (update coordinates in code)");
}

void Geofencing::update(double latitude, double longitude, bool gpsValid) {
    currentLat = latitude;
    currentLng = longitude;
    positionValid = gpsValid && isValidPosition(latitude, longitude);
    lastUpdateTime = millis();
    
    if (!positionValid) {
        // No valid position - maintain last status but don't update fences
        return;
    }
    
    // Reset status for this update
    status.anyFenceActive = false;
    status.inSafeZone = false;
    status.inProhibitedZone = false;
    status.speedLimited = false;
    status.currentSpeedLimit = globalSpeedLimit;
    status.currentAction = ACTION_NONE;
    status.activeFenceId = 0;
    status.distanceToNearestFence = 9999.0f;
    
    // Check each fence
    for (uint8_t i = 0; i < numFences; i++) {
        if (fences[i].enabled) {
            updateFenceStatus(fences[i]);
        }
    }
    
    // Update global status
    updateGlobalStatus();
    
    // Update home distance/bearing if home is set
    if (homePositionSet) {
        status.distanceToSafeZone = getDistanceToHome();
        status.bearingToSafeZone = getBearingToHome();
    }
}

bool Geofencing::addCircularFence(uint8_t id, GeofenceType type, GeofenceAction action,
                                 double centerLat, double centerLng, float radiusMeters,
                                 float speedLimit) {
    if (numFences >= MAX_GEOFENCES) {
        Serial.println("⚠ Maximum geofences exceeded");
        return false;
    }
    
    if (!isValidPosition(centerLat, centerLng)) {
        Serial.println("⚠ Invalid fence coordinates");
        return false;
    }
    
    // Find slot (replace existing ID or add new)
    uint8_t slot = numFences;
    for (uint8_t i = 0; i < numFences; i++) {
        if (fences[i].id == id) {
            slot = i;
            break;
        }
    }
    
    // Configure fence
    GeofenceZone& fence = fences[slot];
    fence.enabled = true;
    fence.id = id;
    fence.type = type;
    fence.shape = SHAPE_CIRCLE;
    fence.action = action;
    fence.speedLimit = speedLimit;
    
    fence.fence.circle.center.latitude = centerLat;
    fence.fence.circle.center.longitude = centerLng;
    fence.fence.circle.radius = radiusMeters;
    
    fence.currentlyInside = false;
    fence.previouslyInside = false;
    fence.breachStartTime = 0;
    fence.lastBreachTime = 0;
    fence.breachCount = 0;
    
    if (slot == numFences) {
        numFences++;
    }
    
    Serial.printf("  Added circular fence %d: %.6f,%.6f radius=%.0fm\n", 
                  id, centerLat, centerLng, radiusMeters);
    
    // Set as home position if it's a safe zone and no home set
    if (type == FENCE_SAFE_ZONE && !homePositionSet) {
        setHomePosition(centerLat, centerLng);
    }
    
    return true;
}

bool Geofencing::addRectangularFence(uint8_t id, GeofenceType type, GeofenceAction action,
                                    double lat1, double lng1, double lat2, double lng2,
                                    float speedLimit) {
    if (numFences >= MAX_GEOFENCES) {
        Serial.println("⚠ Maximum geofences exceeded");
        return false;
    }
    
    if (!isValidPosition(lat1, lng1) || !isValidPosition(lat2, lng2)) {
        Serial.println("⚠ Invalid fence coordinates");
        return false;
    }
    
    // Find slot (replace existing ID or add new)
    uint8_t slot = numFences;
    for (uint8_t i = 0; i < numFences; i++) {
        if (fences[i].id == id) {
            slot = i;
            break;
        }
    }
    
    // Configure fence
    GeofenceZone& fence = fences[slot];
    fence.enabled = true;
    fence.id = id;
    fence.type = type;
    fence.shape = SHAPE_RECTANGLE;
    fence.action = action;
    fence.speedLimit = speedLimit;
    
    fence.fence.rectangle.corner1.latitude = lat1;
    fence.fence.rectangle.corner1.longitude = lng1;
    fence.fence.rectangle.corner2.latitude = lat2;
    fence.fence.rectangle.corner2.longitude = lng2;
    
    fence.currentlyInside = false;
    fence.previouslyInside = false;
    fence.breachStartTime = 0;
    fence.lastBreachTime = 0;
    fence.breachCount = 0;
    
    if (slot == numFences) {
        numFences++;
    }
    
    Serial.printf("  Added rectangular fence %d: %.6f,%.6f to %.6f,%.6f\n", 
                  id, lat1, lng1, lat2, lng2);
    
    return true;
}

bool Geofencing::addPolygonFence(uint8_t id, GeofenceType type, GeofenceAction action,
                                GeoPoint* points, uint8_t numPoints, float speedLimit) {
    if (numFences >= MAX_GEOFENCES || numPoints > MAX_FENCE_POINTS || numPoints < 3) {
        Serial.println("⚠ Invalid polygon fence parameters");
        return false;
    }
    
    // Find slot (replace existing ID or add new)
    uint8_t slot = numFences;
    for (uint8_t i = 0; i < numFences; i++) {
        if (fences[i].id == id) {
            slot = i;
            break;
        }
    }
    
    // Configure fence
    GeofenceZone& fence = fences[slot];
    fence.enabled = true;
    fence.id = id;
    fence.type = type;
    fence.shape = SHAPE_POLYGON;
    fence.action = action;
    fence.speedLimit = speedLimit;
    
    fence.fence.polygon.numPoints = numPoints;
    for (uint8_t i = 0; i < numPoints; i++) {
        fence.fence.polygon.points[i] = points[i];
    }
    
    fence.currentlyInside = false;
    fence.previouslyInside = false;
    fence.breachStartTime = 0;
    fence.lastBreachTime = 0;
    fence.breachCount = 0;
    
    if (slot == numFences) {
        numFences++;
    }
    
    Serial.printf("  Added polygon fence %d with %d points\n", id, numPoints);
    
    return true;
}

bool Geofencing::removeFence(uint8_t id) {
    for (uint8_t i = 0; i < numFences; i++) {
        if (fences[i].id == id) {
            // Shift remaining fences down
            for (uint8_t j = i; j < numFences - 1; j++) {
                fences[j] = fences[j + 1];
            }
            numFences--;
            Serial.printf("  Removed fence %d\n", id);
            return true;
        }
    }
    return false;
}

bool Geofencing::enableFence(uint8_t id, bool enabled) {
    for (uint8_t i = 0; i < numFences; i++) {
        if (fences[i].id == id) {
            fences[i].enabled = enabled;
            Serial.printf("  Fence %d %s\n", id, enabled ? "enabled" : "disabled");
            return true;
        }
    }
    return false;
}

void Geofencing::clearAllFences() {
    numFences = 0;
    memset(fences, 0, sizeof(fences));
    Serial.println("  All geofences cleared");
}

GeofenceStatus Geofencing::getStatus() {
    return status;
}

bool Geofencing::isInSafeZone() {
    return status.inSafeZone;
}

bool Geofencing::isInProhibitedZone() {
    return status.inProhibitedZone;
}

float Geofencing::getCurrentSpeedLimit() {
    return status.currentSpeedLimit;
}

GeofenceAction Geofencing::getCurrentAction() {
    return status.currentAction;
}

bool Geofencing::shouldStop() {
    return (status.currentAction == ACTION_STOP || 
            status.currentAction == ACTION_EMERGENCY_STOP);
}

bool Geofencing::shouldReturnHome() {
    return (status.currentAction == ACTION_RETURN_HOME);
}

bool Geofencing::shouldLimitSpeed() {
    return (status.currentAction == ACTION_LIMIT_SPEED || status.speedLimited);
}

float Geofencing::getSpeedLimitFactor() {
    if (!shouldLimitSpeed()) {
        return 1.0f;
    }
    
    // Return factor based on current speed limit vs global limit
    return status.currentSpeedLimit / globalSpeedLimit;
}

void Geofencing::triggerEmergencyMode() {
    emergencyModeActive = true;
    status.currentAction = ACTION_EMERGENCY_STOP;
    logFenceEvent(0, "EMERGENCY MODE ACTIVATED");
}

void Geofencing::setHomePosition(double latitude, double longitude) {
    if (isValidPosition(latitude, longitude)) {
        homePosition.latitude = latitude;
        homePosition.longitude = longitude;
        homePositionSet = true;
        Serial.printf("  Home position set: %.6f, %.6f\n", latitude, longitude);
    }
}

bool Geofencing::hasHomePosition() {
    return homePositionSet;
}

float Geofencing::getDistanceToHome() {
    if (!homePositionSet || !positionValid) {
        return -1.0f;
    }
    
    return calculateDistance(currentLat, currentLng, 
                           homePosition.latitude, homePosition.longitude);
}

float Geofencing::getBearingToHome() {
    if (!homePositionSet || !positionValid) {
        return -1.0f;
    }
    
    return calculateBearing(currentLat, currentLng,
                          homePosition.latitude, homePosition.longitude);
}

uint16_t Geofencing::getTotalViolations() {
    return totalViolations;
}

unsigned long Geofencing::getLastViolationTime() {
    return lastViolationTime;
}

void Geofencing::clearViolationHistory() {
    totalViolations = 0;
    lastViolationTime = 0;
    for (uint8_t i = 0; i < numFences; i++) {
        fences[i].breachCount = 0;
        fences[i].lastBreachTime = 0;
    }
    Serial.println("  Violation history cleared");
}

void Geofencing::setGlobalSpeedLimit(float maxSpeedMph) {
    globalSpeedLimit = constrain(maxSpeedMph, 1.0f, MAX_VALID_SPEED);
    Serial.printf("  Global speed limit set to %.1f mph\n", globalSpeedLimit);
}

void Geofencing::setWarningDistance(float distanceMeters) {
    warningDistance = constrain(distanceMeters, 10.0f, 1000.0f);
    Serial.printf("  Warning distance set to %.0f meters\n", warningDistance);
}

void Geofencing::setEmergencyMode(bool enabled) {
    emergencyModeActive = enabled;
    Serial.printf("  Emergency mode %s\n", enabled ? "enabled" : "disabled");
}

// Private methods

bool Geofencing::isPointInCircle(double lat, double lng, const GeofenceZone& fence) {
    float distance = calculateDistance(lat, lng,
                                     fence.fence.circle.center.latitude,
                                     fence.fence.circle.center.longitude);
    return distance <= fence.fence.circle.radius;
}

bool Geofencing::isPointInRectangle(double lat, double lng, const GeofenceZone& fence) {
    double minLat = min(fence.fence.rectangle.corner1.latitude, 
                       fence.fence.rectangle.corner2.latitude);
    double maxLat = max(fence.fence.rectangle.corner1.latitude, 
                       fence.fence.rectangle.corner2.latitude);
    double minLng = min(fence.fence.rectangle.corner1.longitude, 
                       fence.fence.rectangle.corner2.longitude);
    double maxLng = max(fence.fence.rectangle.corner1.longitude, 
                       fence.fence.rectangle.corner2.longitude);
    
    return (lat >= minLat && lat <= maxLat && lng >= minLng && lng <= maxLng);
}

bool Geofencing::isPointInPolygon(double lat, double lng, const GeofenceZone& fence) {
    // Ray casting algorithm for point-in-polygon test
    bool inside = false;
    uint8_t numPoints = fence.fence.polygon.numPoints;
    
    for (uint8_t i = 0, j = numPoints - 1; i < numPoints; j = i++) {
        const GeoPoint& pi = fence.fence.polygon.points[i];
        const GeoPoint& pj = fence.fence.polygon.points[j];
        
        if (((pi.latitude > lat) != (pj.latitude > lat)) &&
            (lng < (pj.longitude - pi.longitude) * (lat - pi.latitude) / 
             (pj.latitude - pi.latitude) + pi.longitude)) {
            inside = !inside;
        }
    }
    
    return inside;
}

float Geofencing::calculateDistance(double lat1, double lng1, double lat2, double lng2) {
    // Haversine formula for great circle distance
    double dLat = DEG_TO_RAD(lat2 - lat1);
    double dLng = DEG_TO_RAD(lng2 - lng1);
    
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(DEG_TO_RAD(lat1)) * cos(DEG_TO_RAD(lat2)) *
               sin(dLng/2) * sin(dLng/2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return EARTH_RADIUS_M * c; // Distance in meters
}

float Geofencing::calculateBearing(double lat1, double lng1, double lat2, double lng2) {
    double dLng = DEG_TO_RAD(lng2 - lng1);
    double lat1Rad = DEG_TO_RAD(lat1);
    double lat2Rad = DEG_TO_RAD(lat2);
    
    double y = sin(dLng) * cos(lat2Rad);
    double x = cos(lat1Rad) * sin(lat2Rad) - sin(lat1Rad) * cos(lat2Rad) * cos(dLng);
    
    double bearing = RAD_TO_DEG(atan2(y, x));
    
    // Normalize to 0-360
    return fmod(bearing + 360.0, 360.0);
}

void Geofencing::updateFenceStatus(GeofenceZone& fence) {
    // Store previous state
    fence.previouslyInside = fence.currentlyInside;
    
    // Check current position against fence
    switch (fence.shape) {
        case SHAPE_CIRCLE:
            fence.currentlyInside = isPointInCircle(currentLat, currentLng, fence);
            break;
        case SHAPE_RECTANGLE:
            fence.currentlyInside = isPointInRectangle(currentLat, currentLng, fence);
            break;
        case SHAPE_POLYGON:
            fence.currentlyInside = isPointInPolygon(currentLat, currentLng, fence);
            break;
    }
    
    // Detect fence transitions
    bool justEntered = fence.currentlyInside && !fence.previouslyInside;
    bool justExited = !fence.currentlyInside && fence.previouslyInside;
    
    if (justEntered || justExited) {
        processFenceViolation(fence);
    }
    
    // Update global status based on this fence
    if (fence.currentlyInside) {
        status.anyFenceActive = true;
        
        switch (fence.type) {
            case FENCE_SAFE_ZONE:
                status.inSafeZone = true;
                break;
            case FENCE_PROHIBITED:
                status.inProhibitedZone = true;
                status.currentAction = fence.action;
                status.activeFenceId = fence.id;
                break;
            case FENCE_SPEED_LIMIT:
                status.speedLimited = true;
                if (fence.speedLimit > 0 && fence.speedLimit < status.currentSpeedLimit) {
                    status.currentSpeedLimit = fence.speedLimit;
                    status.currentAction = fence.action;
                    status.activeFenceId = fence.id;
                }
                break;
            case FENCE_EMERGENCY_RTB:
                status.currentAction = fence.action;
                status.activeFenceId = fence.id;
                break;
        }
    }
}

void Geofencing::processFenceViolation(GeofenceZone& fence) {
    unsigned long currentTime = millis();
    
    if (fence.currentlyInside && !fence.previouslyInside) {
        // Entering fence
        fence.breachStartTime = currentTime;
        logFenceEvent(fence.id, "ENTERED");
        
        if (fence.type == FENCE_PROHIBITED) {
            fence.breachCount++;
            fence.lastBreachTime = currentTime;
            totalViolations++;
            lastViolationTime = currentTime;
        }
    } else if (!fence.currentlyInside && fence.previouslyInside) {
        // Exiting fence
        logFenceEvent(fence.id, "EXITED");
        
        if (fence.type == FENCE_SAFE_ZONE) {
            // Exiting safe zone could be a violation depending on configuration
            fence.breachCount++;
            fence.lastBreachTime = currentTime;
        }
    }
}

void Geofencing::updateGlobalStatus() {
    // Apply emergency mode override
    if (emergencyModeActive) {
        status.currentAction = ACTION_EMERGENCY_STOP;
        return;
    }
    
    // If in prohibited zone, that takes highest priority
    if (status.inProhibitedZone) {
        return; // Action already set in updateFenceStatus
    }
    
    // Check for emergency return-to-base conditions
    if (homePositionSet) {
        float distanceToHome = getDistanceToHome();
        if (distanceToHome > 2000.0f) { // 2km emergency boundary
            status.currentAction = ACTION_RETURN_HOME;
            logFenceEvent(0, "EMERGENCY RTB - TOO FAR FROM HOME");
        }
    }
}

bool Geofencing::isValidPosition(double lat, double lng) {
    return (lat >= -90.0 && lat <= 90.0 && lng >= -180.0 && lng <= 180.0 &&
            lat != 0.0 && lng != 0.0); // Exclude null island
}

float Geofencing::constrainSpeed(float speed) {
    return constrain(speed, 0.0f, MAX_VALID_SPEED);
}

void Geofencing::logFenceEvent(uint8_t fenceId, const char* event) {
    Serial.printf("[GEOFENCE] Fence %d: %s at %.6f,%.6f\n", 
                  fenceId, event, currentLat, currentLng);
}