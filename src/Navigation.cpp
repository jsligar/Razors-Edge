#include "Navigation.h"

Navigation::Navigation() :
    gpsSerial(nullptr),
    lastValidUpdate(0),
    lastValidLat(0.0),
    lastValidLng(0.0),
    lastPositionValid(false),
    tripStartTime(0),
    motorSpeed(0.0f),
    lastMotorSpeedUpdate(0)
{
    // Initialize GPS data structure
    currentData.isValid = false;
    currentData.latitude = 0.0;
    currentData.longitude = 0.0;
    currentData.altitude = 0.0;
    currentData.speed = 0.0;
    currentData.course = 0.0;
    currentData.satellites = 0;
    currentData.hdop = 99.9;
    
    // Initialize trip data
    tripData.odometer = 0.0f;
    tripData.tripMeter = 0.0f;
    tripData.maxSpeed = 0.0f;
    tripData.tripTime = 0;
}

bool Navigation::init() {
    Serial.println("Initializing Navigation (GPS)...");
    
    // Initialize UART2 for GPS communication
    gpsSerial = &Serial2;
    gpsSerial->begin(GPS_BAUD_RATE, SERIAL_8N1, GPIO_GPS_RX, GPIO_GPS_TX);
    
    // Initialize geofencing system
    if (!geofencing.init()) {
        Serial.println("⚠ Geofencing initialization failed");
        return false;
    }
    
    tripStartTime = millis();
    
    Serial.println("✓ GPS UART and geofencing initialized, waiting for satellites...");
    return true;
}

void Navigation::update() {
    // Parse incoming GPS data
    parseGPSData();
    
    // Update trip calculations if we have valid position data
    if (currentData.isValid) {
        updateTripData();
        
        // Update geofencing system with current position
        geofencing.update(currentData.latitude, currentData.longitude, isGPSFixed());
    }
}

float Navigation::getSpeed() {
    if (!isDataFresh()) return 0.0f;
    return currentData.speed;
}

float Navigation::getSpeedKmh() {
    return getSpeed() * 1.60934f; // Convert MPH to km/h
}

double Navigation::getLatitude() {
    return currentData.latitude;
}

double Navigation::getLongitude() {
    return currentData.longitude;
}

float Navigation::getAltitude() {
    return currentData.altitude;
}

float Navigation::getCourse() {
    return currentData.course;
}

uint8_t Navigation::getSatellites() {
    return currentData.satellites;
}

float Navigation::getHDOP() {
    return currentData.hdop;
}

bool Navigation::isGPSFixed() {
    return currentData.isValid && (currentData.satellites >= 4) && (currentData.hdop < 3.0);
}

float Navigation::getOdometer() {
    return tripData.odometer;
}

float Navigation::getTripMeter() {
    return tripData.tripMeter;
}

float Navigation::getMaxSpeed() {
    return tripData.maxSpeed;
}

unsigned long Navigation::getTripTime() {
    return tripData.tripTime;
}

void Navigation::resetTrip() {
    tripData.tripMeter = 0.0f;
    tripData.maxSpeed = 0.0f;
    tripData.tripTime = 0;
    tripStartTime = millis();
    
    Serial.println("Trip meter reset");
}

float Navigation::getDistanceTo(double lat, double lng) {
    if (!currentData.isValid) return 0.0f;
    
    return calculateDistance(currentData.latitude, currentData.longitude, lat, lng);
}

float Navigation::getBearingTo(double lat, double lng) {
    if (!currentData.isValid) return 0.0f;
    
    double lat1 = radians(currentData.latitude);
    double lat2 = radians(lat);
    double deltaLng = radians(lng - currentData.longitude);
    
    double y = sin(deltaLng) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(deltaLng);
    
    double bearing = atan2(y, x);
    return fmod(degrees(bearing) + 360.0, 360.0); // Normalize to 0-360°
}

void Navigation::setMotorSpeed(float motorSpeedMph) {
    motorSpeed = motorSpeedMph;
    lastMotorSpeedUpdate = millis();
}

float Navigation::getSpeedDifference() {
    if (!isDataFresh() || (millis() - lastMotorSpeedUpdate > 1000)) {
        return 0.0f; // No valid comparison available
    }
    
    return getSpeed() - motorSpeed;
}

bool Navigation::isSpeedReasonable() {
    float difference = abs(getSpeedDifference());
    
    // Allow up to 3 MPH difference (accounting for wheel slip, GPS lag, etc.)
    return difference < 3.0f;
}

unsigned long Navigation::getLastUpdateAge() {
    return millis() - lastValidUpdate;
}

bool Navigation::isDataFresh() {
    return (getLastUpdateAge() < 2000) && currentData.isValid;
}

void Navigation::parseGPSData() {
    // Process all available GPS data
    while (gpsSerial->available() > 0) {
        if (gps.encode(gpsSerial->read())) {
            // New sentence was successfully parsed
            processSentences();
        }
    }
}

void Navigation::processSentences() {
    // Check if we have a valid location fix
    if (gps.location.isValid() && gps.location.age() < 1000) {
        currentData.isValid = true;
        currentData.latitude = gps.location.lat();
        currentData.longitude = gps.location.lng();
        lastValidUpdate = millis();
    }
    
    // Update speed (if valid)
    if (gps.speed.isValid() && gps.speed.age() < 1000) {
        float newSpeed = gps.speed.mph();
        if (isSpeedValid(newSpeed)) {
            currentData.speed = newSpeed;
            
            // Update max speed for trip
            if (newSpeed > tripData.maxSpeed) {
                tripData.maxSpeed = newSpeed;
            }
        }
    }
    
    // Update altitude
    if (gps.altitude.isValid() && gps.altitude.age() < 1000) {
        currentData.altitude = gps.altitude.feet();
    }
    
    // Update course
    if (gps.course.isValid() && gps.course.age() < 1000) {
        currentData.course = gps.course.deg();
    }
    
    // Update satellite count
    if (gps.satellites.isValid()) {
        currentData.satellites = gps.satellites.value();
    }
    
    // Update HDOP
    if (gps.hdop.isValid()) {
        currentData.hdop = gps.hdop.value() / 100.0f; // TinyGPS++ gives centimeters
    }
}

void Navigation::updateTripData() {
    if (!currentData.isValid) return;
    
    // Update trip time
    tripData.tripTime = (millis() - tripStartTime) / 1000;
    
    // Update odometer if we have a previous valid position
    if (lastPositionValid) {
        float distance = calculateDistance(lastValidLat, lastValidLng,
                                         currentData.latitude, currentData.longitude);
        
        // Only add distance if it's reasonable (not a GPS jump)
        if (distance < 0.1f && distance > 0.001f) { // Between 1 meter and 528 feet
            tripData.odometer += distance;
            tripData.tripMeter += distance;
        }
    }
    
    // Save current position for next calculation
    lastValidLat = currentData.latitude;
    lastValidLng = currentData.longitude;
    lastPositionValid = true;
}

float Navigation::calculateDistance(double lat1, double lng1, double lat2, double lng2) {
    // Haversine formula for great circle distance
    double R = 3959.0; // Earth radius in miles
    
    double dLat = radians(lat2 - lat1);
    double dLng = radians(lng2 - lng1);
    
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(radians(lat1)) * cos(radians(lat2)) *
               sin(dLng/2) * sin(dLng/2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return R * c; // Distance in miles
}

bool Navigation::isSpeedValid(float speed) {
    // Sanity check for GPS speed
    return (speed >= 0.0f && speed <= 50.0f); // 0-50 MPH is reasonable for this vehicle
}

bool Navigation::isPositionValid(double lat, double lng) {
    // Basic sanity check for coordinates
    return (lat >= -90.0 && lat <= 90.0 && lng >= -180.0 && lng <= 180.0);
}

// Geofencing integration methods

GeofenceStatus Navigation::getGeofenceStatus() {
    return geofencing.getStatus();
}

bool Navigation::isInSafeZone() {
    return geofencing.isInSafeZone();
}

bool Navigation::isInProhibitedZone() {
    return geofencing.isInProhibitedZone();
}

float Navigation::getCurrentSpeedLimit() {
    return geofencing.getCurrentSpeedLimit();
}

GeofenceAction Navigation::getCurrentGeofenceAction() {
    return geofencing.getCurrentAction();
}

bool Navigation::shouldStopForGeofence() {
    return geofencing.shouldStop();
}

bool Navigation::shouldReturnHome() {
    return geofencing.shouldReturnHome();
}

bool Navigation::shouldLimitSpeedForGeofence() {
    return geofencing.shouldLimitSpeed();
}

float Navigation::getGeofenceSpeedFactor() {
    return geofencing.getSpeedLimitFactor();
}

void Navigation::triggerGeofenceEmergency() {
    geofencing.triggerEmergencyMode();
}

bool Navigation::addGeofence(uint8_t id, GeofenceType type, GeofenceAction action,
                            double lat, double lng, float radius, float speedLimit) {
    return geofencing.addCircularFence(id, type, action, lat, lng, radius, speedLimit);
}

bool Navigation::removeGeofence(uint8_t id) {
    return geofencing.removeFence(id);
}

void Navigation::setHomePosition(double lat, double lng) {
    geofencing.setHomePosition(lat, lng);
}

float Navigation::getDistanceToHome() {
    return geofencing.getDistanceToHome();
}

float Navigation::getBearingToHome() {
    return geofencing.getBearingToHome();
}