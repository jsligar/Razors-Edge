#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "config.h"
#include "Geofencing.h"

struct GPSData {
    bool isValid;
    double latitude;
    double longitude;
    float altitude;
    float speed;           // mph
    float course;          // degrees
    uint8_t satellites;
    float hdop;           // horizontal dilution of precision
};

struct TripData {
    float odometer;       // total miles
    float tripMeter;      // trip miles (resettable)
    float maxSpeed;       // max speed this trip
    unsigned long tripTime; // trip time in seconds
};

class Navigation {
public:
    Navigation();
    
    // Initialization and main update
    bool init();
    void update();
    
    // GPS data getters
    float getSpeed();              // Current speed in MPH
    float getSpeedKmh();          // Current speed in km/h
    double getLatitude();
    double getLongitude();
    float getAltitude();
    float getCourse();            // Heading in degrees
    uint8_t getSatellites();
    float getHDOP();
    bool isGPSFixed();
    
    // Trip data
    float getOdometer();          // Total distance in miles
    float getTripMeter();         // Trip distance in miles
    float getMaxSpeed();          // Max speed this trip
    unsigned long getTripTime();   // Trip time in seconds
    void resetTrip();
    
    // Position calculations
    float getDistanceTo(double lat, double lng); // Distance in miles
    float getBearingTo(double lat, double lng);  // Bearing in degrees
    
    // Speed validation (compare with motor speed)
    void setMotorSpeed(float motorSpeedMph);
    float getSpeedDifference();   // GPS speed - motor speed
    bool isSpeedReasonable();     // Check for wheel slip
    
    // Data age and validity
    unsigned long getLastUpdateAge(); // milliseconds since last valid data
    bool isDataFresh();           // Data less than 2 seconds old
    
    // Geofencing integration
    GeofenceStatus getGeofenceStatus();
    bool isInSafeZone();
    bool isInProhibitedZone();
    float getCurrentSpeedLimit();
    GeofenceAction getCurrentGeofenceAction();
    bool shouldStopForGeofence();
    bool shouldReturnHome();
    bool shouldLimitSpeedForGeofence();
    float getGeofenceSpeedFactor();
    void triggerGeofenceEmergency();
    
    // Geofence management (delegate to geofencing system)
    bool addGeofence(uint8_t id, GeofenceType type, GeofenceAction action,
                    double lat, double lng, float radius, float speedLimit = 0);
    bool removeGeofence(uint8_t id);
    void setHomePosition(double lat, double lng);
    float getDistanceToHome();
    float getBearingToHome();
    
private:
    // GPS hardware and parsing
    HardwareSerial* gpsSerial;
    TinyGPSPlus gps;
    
    // Current GPS data
    GPSData currentData;
    unsigned long lastValidUpdate;

    // Speed smoothing filter
    float filteredSpeed;
    float speedSmoothingFactor;  // 0.0-1.0, lower = more smoothing

    // Trip tracking
    TripData tripData;
    double lastValidLat;
    double lastValidLng;
    bool lastPositionValid;
    unsigned long tripStartTime;
    
    // Speed comparison
    float motorSpeed;
    unsigned long lastMotorSpeedUpdate;
    
    // Geofencing system
    Geofencing geofencing;
    
    // Internal methods
    void parseGPSData();
    void updateTripData();
    void updateOdometer();
    float calculateDistance(double lat1, double lng1, double lat2, double lng2);
    
    // NMEA sentence handling
    void processSentences();
    
    // Data validation
    bool isSpeedValid(float speed);
    bool isPositionValid(double lat, double lng);
};

#endif // NAVIGATION_H