#ifndef MGRS_CONVERTER_H
#define MGRS_CONVERTER_H

#include <Arduino.h>

// Simple MGRS converter for GPS coordinates
// MGRS (Military Grid Reference System) is a geocoordinate standard

class MGRSConverter {
public:
    // Convert latitude/longitude to MGRS string
    // Returns format like: "15SWC1234567890" (Grid Zone + 100km Square + Easting + Northing)
    static String latLonToMGRS(double lat, double lon, uint8_t precision = 5);

    // Helper to get just the grid zone designator (e.g., "15S")
    static String getGridZone(double lat, double lon);

private:
    static const char* LETTER_ARRAY_E;
    static const char* LETTER_ARRAY_N;

    static int getZoneNumber(double lon);
    static char getLatitudeBand(double lat);
    static void get100kID(double lat, double lon, int zoneNumber, char& letter1, char& letter2);
    static double toRadians(double degrees);
};

#endif // MGRS_CONVERTER_H
