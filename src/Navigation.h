#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include "config.h"

// Simplified Navigation for tractor - no GPS
class Navigation {
public:
    Navigation();

    // Initialization and main update
    bool init();
    void update();

    // Stub methods for compatibility
    float getSpeed() { return 0.0f; }
    float getSpeedKmh() { return 0.0f; }
    double getLatitude() { return 0.0; }
    double getLongitude() { return 0.0; }
    float getAltitude() { return 0.0f; }
    float getCourse() { return 0.0f; }
    uint8_t getSatellites() { return 0; }
    float getHDOP() { return 0.0f; }
    bool isGPSFixed() { return false; }
    float getOdometer() { return 0.0f; }
    float getTripMeter() { return 0.0f; }
    float getMaxSpeed() { return 0.0f; }
    unsigned long getTripTime() { return 0; }
    void resetTrip() {}
    float getDistanceTo(double lat, double lng) { return 0.0f; }
    float getBearingTo(double lat, double lng) { return 0.0f; }
    void setMotorSpeed(float motorSpeedMph) {}
    float getSpeedDifference() { return 0.0f; }
    bool isSpeedReasonable() { return true; }
    unsigned long getLastUpdateAge() { return 0; }
    bool isDataFresh() { return false; }

    // Geofencing stubs (all disabled)
    bool isInSafeZone() { return true; }
    bool isInProhibitedZone() { return false; }
    float getCurrentSpeedLimit() { return 100.0f; }
    bool shouldStopForGeofence() { return false; }
    bool shouldReturnHome() { return false; }
    bool shouldLimitSpeedForGeofence() { return false; }
    float getGeofenceSpeedFactor() { return 1.0f; }
    void triggerGeofenceEmergency() {}
    bool addGeofence(uint8_t id, uint8_t type, uint8_t action,
                    double lat, double lng, float radius, float speedLimit = 0) { return false; }
    bool removeGeofence(uint8_t id) { return false; }
    void setHomePosition(double lat, double lng) {}
    float getDistanceToHome() { return 0.0f; }
    float getBearingToHome() { return 0.0f; }
};

#endif // NAVIGATION_H