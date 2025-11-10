#include "MGRSConverter.h"
#include <math.h>

const char* MGRSConverter::LETTER_ARRAY_E = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // No I/O
const char* MGRSConverter::LETTER_ARRAY_N = "ABCDEFGHJKLMNPQRSTUV";     // No I/O

double MGRSConverter::toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

int MGRSConverter::getZoneNumber(double lon) {
    // Calculate UTM zone number from longitude
    if (lon >= -180.0 && lon <= 180.0) {
        return ((int)((lon + 180.0) / 6.0) + 1);
    }
    return 0; // Invalid
}

char MGRSConverter::getLatitudeBand(double lat) {
    // Get latitude band letter (C-X, excluding I and O)
    const char* bands = "CDEFGHJKLMNPQRSTUVWX";

    if (lat >= -80.0 && lat < 84.0) {
        int index = (int)((lat + 80.0) / 8.0);
        return bands[index];
    }
    return 'Z'; // Invalid/polar
}

void MGRSConverter::get100kID(double lat, double lon, int zoneNumber, char& letter1, char& letter2) {
    // Simplified 100km square identification
    // This is an approximation - full MGRS uses complex lookup tables

    double centralMeridian = (zoneNumber - 1) * 6 - 180 + 3;

    // Convert to UTM-like coordinates (simplified)
    double lonRad = toRadians(lon);
    double latRad = toRadians(lat);

    double N = 6378137.0; // WGS84 equatorial radius
    double e2 = 0.00669438; // WGS84 eccentricity squared

    double A = (lon - centralMeridian) * cos(latRad);
    double E = 500000 + (N * A); // Easting (simplified)

    // Get 100km square letters (simplified)
    int e100k = ((int)(E / 100000.0)) % 8;
    int n100k = ((int)(lat * 110574)) / 100000 % 20; // Rough latitude to meters

    letter1 = LETTER_ARRAY_E[e100k % 24];
    letter2 = LETTER_ARRAY_N[n100k % 20];
}

String MGRSConverter::getGridZone(double lat, double lon) {
    int zone = getZoneNumber(lon);
    char band = getLatitudeBand(lat);

    char buffer[4];
    snprintf(buffer, sizeof(buffer), "%d%c", zone, band);
    return String(buffer);
}

String MGRSConverter::latLonToMGRS(double lat, double lon, uint8_t precision) {
    if (precision > 5) precision = 5; // Max 1m precision

    // Get UTM zone and latitude band
    int zoneNumber = getZoneNumber(lon);
    char latBand = getLatitudeBand(lat);

    // Get 100km square ID
    char letter1, letter2;
    get100kID(lat, lon, zoneNumber, letter1, letter2);

    // Calculate easting and northing within 100km square (simplified)
    double centralMeridian = (zoneNumber - 1) * 6 - 180 + 3;
    double lonRad = toRadians(lon);
    double latRad = toRadians(lat);

    // Simplified UTM calculation
    double N = 6378137.0;
    double k0 = 0.9996;
    double e2 = 0.00669438;

    double A = (lon - centralMeridian) * cos(latRad);
    double E = 500000 + (k0 * N * A); // Easting
    double Northing = k0 * (lat * 110574.0); // Simplified northing
    if (Northing < 0) Northing += 10000000.0; // Southern hemisphere

    // Get digits within 100km square
    int easting = ((int)E) % 100000;
    int northing = ((int)Northing) % 100000;

    // Format based on precision
    char result[32];
    int divisor = 1;
    for (uint8_t i = 0; i < (5 - precision); i++) {
        divisor *= 10;
    }

    easting /= divisor;
    northing /= divisor;

    snprintf(result, sizeof(result), "%d%c%c%c%0*d%0*d",
             zoneNumber, latBand, letter1, letter2,
             precision, easting, precision, northing);

    return String(result);
}
