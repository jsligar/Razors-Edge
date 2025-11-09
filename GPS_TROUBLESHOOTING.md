# GPS Troubleshooting Guide

## Current GPS Configuration

### Hardware Setup
- **GPS Module**: Expected on UART2 (Serial2)
- **RX Pin**: GPIO 16 (receives data FROM GPS module TX)
- **TX Pin**: GPIO 17 (sends data TO GPS module RX)
- **Baud Rate**: 9600
- **Data Format**: NMEA sentences (via TinyGPSPlus library)

### GPS Connection Diagram
```
ESP32 GPIO 16 (RX) ← TX GPS Module (data from GPS)
ESP32 GPIO 17 (TX) → RX GPS Module (commands to GPS)
GPS Module VCC     → 3.3V or 5V (check module spec)
GPS Module GND     → GND
```

**IMPORTANT**: RX/TX are crossed - ESP32 RX connects to GPS TX, ESP32 TX connects to GPS RX.

---

## GPS Debug Output

The system prints GPS status every 5 seconds in the serial monitor:

```
GPS: Sats=8 Fix=1 Lat=39.123456 Lng=-94.654321 Speed=15.3 mph HDOP=1.2
```

**Fields Explained:**
- **Sats**: Number of satellites in view (need 4+ for fix)
- **Fix**: 0=No fix, 1=GPS fixed
- **Lat/Lng**: Current position in decimal degrees
- **Speed**: Current speed in MPH from GPS
- **HDOP**: Horizontal Dilution of Precision (lower is better, <3.0 is good)

---

## Common GPS Issues

### Issue 1: No Satellite Fix (Sats=0 Fix=0)

**Symptoms:**
```
GPS: Sats=0 Fix=0 Lat=0.000000 Lng=0.000000 Speed=0.0 mph HDOP=99.9
```

**Possible Causes:**

1. **GPS Module Not Connected**
   - Check physical wiring
   - Verify 3.3V/5V power to GPS module
   - Check continuity of TX/RX wires

2. **Wrong Baud Rate**
   - Most GPS modules use 9600 baud (configured)
   - Some use 4800 or 38400
   - Check GPS module datasheet

3. **RX/TX Crossed Incorrectly**
   - Verify: ESP32 GPIO 16 (RX) connects to GPS module TX pin
   - Verify: ESP32 GPIO 17 (TX) connects to GPS module RX pin

4. **Indoor Location**
   - GPS needs clear view of sky
   - Won't work indoors or under metal roof
   - Move outside for testing

5. **Cold Start Time**
   - GPS modules take 30-90 seconds for first fix
   - Need to download satellite almanac
   - Be patient on first power-up

**Solutions:**
```cpp
// Check if GPS is receiving data (add to Navigation task temporarily)
if (gpsSerial->available()) {
    Serial.printf("GPS RX: %d bytes available\n", gpsSerial->available());
    while (gpsSerial->available()) {
        char c = gpsSerial->read();
        Serial.write(c);  // Print raw NMEA sentences
    }
}
```

---

### Issue 2: Satellites Visible But No Fix (Sats>0 Fix=0)

**Symptoms:**
```
GPS: Sats=3 Fix=0 Lat=0.000000 Lng=0.000000 Speed=0.0 mph HDOP=5.5
```

**Possible Causes:**

1. **Not Enough Satellites**
   - Need at least 4 satellites for 3D fix
   - 3 satellites only gives 2D fix (no altitude)
   - Wait for more satellites to rise above horizon

2. **Poor HDOP** (Horizontal Dilution of Precision)
   - HDOP > 3.0 indicates poor satellite geometry
   - Satellites are clustered instead of spread out
   - System requires HDOP < 3.0 for fix
   - Wait for better satellite positions

3. **Weak Signals**
   - Obstructed view (trees, buildings)
   - Poor antenna placement
   - Move to open area

**Fix Criteria in Code** (`Navigation.cpp:95`):
```cpp
bool Navigation::isGPSFixed() {
    return currentData.isValid &&
           (currentData.satellites >= 4) &&
           (currentData.hdop < 3.0);
}
```

---

### Issue 3: GPS Fix But Wrong Location

**Symptoms:**
```
GPS: Sats=8 Fix=1 Lat=0.000000 Lng=0.000000 Speed=0.0 mph HDOP=1.2
GPS: Sats=8 Fix=1 Lat=39.999999 Lng=-180.000000 Speed=250.0 mph HDOP=1.2
```

**Possible Causes:**

1. **Invalid NMEA Data**
   - Corrupted sentences from GPS
   - Electrical noise on serial lines
   - Add decoupling capacitors near GPS module

2. **Data Age Issues**
   - TinyGPSPlus checks data.age() < 1000ms
   - Data might be stale
   - Check GPS update rate (should be 1Hz)

3. **Coordinate Validation**
   - System checks valid ranges in code
   - Lat: -90 to +90
   - Lng: -180 to +180
   - Speed: 0 to 50 MPH

**Validation Code** (`Navigation.cpp:265`):
```cpp
bool Navigation::isSpeedValid(float speed) {
    return (speed >= 0.0f && speed <= 50.0f);
}

bool Navigation::isPositionValid(double lat, double lng) {
    return (lat >= -90.0 && lat <= 90.0 &&
            lng >= -180.0 && lng <= 180.0);
}
```

---

### Issue 4: GPS Data Not Updating

**Symptoms:**
- Same GPS values printed repeatedly
- No change in position even when moving
- Timestamp doesn't update

**Possible Causes:**

1. **Navigation Task Not Running**
   - Check serial output for "Navigation task started on Core 1"
   - Verify `sharedData.systemReady == true`

2. **Serial Buffer Overflow**
   - GPS sends ~5-10 NMEA sentences per second
   - If not read frequently enough, buffer fills
   - Navigation task runs at 10Hz (100ms interval)

3. **TinyGPSPlus Not Parsing**
   - Add debug to see if `gps.encode()` returns true
   - Check if NMEA sentences are valid format

**Debug Code** (add to `Navigation::parseGPSData()`):
```cpp
void Navigation::parseGPSData() {
    int bytesRead = 0;
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        bytesRead++;
        if (gps.encode(c)) {
            Serial.printf("GPS parsed sentence (read %d bytes)\n", bytesRead);
            processSentences();
            bytesRead = 0;
        }
    }
    if (bytesRead > 0) {
        Serial.printf("GPS partial data: %d bytes (no complete sentence)\n", bytesRead);
    }
}
```

---

### Issue 5: GPS Position Jumps/Jitters

**Symptoms:**
- Position jumps around wildly
- Speed spikes to unrealistic values
- Odometer increases rapidly while stationary

**Possible Causes:**

1. **Poor Satellite Geometry**
   - High HDOP value (>3.0)
   - Wait for better satellite positions
   - Consider using WAAS/EGNOS if available

2. **Multipath Interference**
   - GPS signals bouncing off buildings/metal
   - Use GPS module with good antenna
   - Move to open area

3. **Distance Filter Too Loose**
   - Current filter: 0.001 to 0.1 miles per update
   - Adjust in `Navigation::updateTripData()` line 237

**Trip Data Filtering** (`Navigation.cpp:237`):
```cpp
// Only add distance if it's reasonable (not a GPS jump)
if (distance < 0.1f && distance > 0.001f) {  // Between 5 feet and 528 feet
    tripData.odometer += distance;
    tripData.tripMeter += distance;
}
```

---

## Testing GPS Module

### Step 1: Verify GPS Module Powers Up

**Check LED on GPS module:**
- Usually blinks when searching for satellites
- Solid or fast blink when fixed
- No light = power issue

**Measure voltage:**
- Check VCC pin has 3.3V or 5V (per module spec)
- Check GND connection

### Step 2: Monitor Raw NMEA Sentences

**Add to `Navigation::parseGPSData()` temporarily:**
```cpp
void Navigation::parseGPSData() {
    // Print raw NMEA data for debugging
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        Serial.write(c);  // Print raw character
        gps.encode(c);
    }
}
```

**Expected Output:**
```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
$GPGSA,A,3,04,05,09,12,24,25,27,29,,,,,2.5,1.3,2.1*39
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
```

If you see gibberish or nothing, check wiring and baud rate.

### Step 3: Test Different Baud Rates

**GPS modules might use different baud rates:**
```cpp
// In Navigation::init(), try each:
gpsSerial->begin(4800, SERIAL_8N1, GPIO_GPS_RX, GPIO_GPS_TX);  // Try 4800
gpsSerial->begin(9600, SERIAL_8N1, GPIO_GPS_RX, GPIO_GPS_TX);  // Try 9600
gpsSerial->begin(38400, SERIAL_8N1, GPIO_GPS_RX, GPIO_GPS_TX); // Try 38400
```

### Step 4: Configure GPS Module

**Some modules need configuration for 1Hz update rate:**
```cpp
// Send UBX or NMEA command to set update rate
gpsSerial->println("$PMTK220,1000*1F");  // Set 1Hz update (for MTK modules)
gpsSerial->println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"); // Set output
```

---

## Recommended GPS Modules

### Good Options:
1. **Beitian BN-880** - Built-in compass, good antenna
2. **u-blox NEO-6M** - Common, cheap, good performance
3. **u-blox NEO-M8N** - Better than NEO-6M
4. **Beitian BN-180/220** - Compact, good performance

### Connection:
- **3.3V Logic Level**: Connect directly to ESP32
- **5V Logic Level**: Use logic level shifter or check if 3.3V tolerant

---

## Performance Monitoring

**GPS Task Performance:**
- **Task Frequency**: 10Hz (every 100ms)
- **Core Assignment**: Core 1
- **Priority**: Medium (2)
- **Stack Size**: 4096 bytes

**Monitor with:**
```
System Status - Tasks: ACTIVE, Heap: 45232, Battery: 12.6V, Gear: P
GPS: Sats=8 Fix=1 Lat=39.123456 Lng=-94.654321 Speed=0.0 mph HDOP=1.2
```

---

## Quick Diagnostic Checklist

- [ ] GPS module has power (LED blinking or solid)
- [ ] Wiring: ESP32 RX (16) ← GPS TX, ESP32 TX (17) → GPS RX
- [ ] Baud rate correct (9600)
- [ ] Outside with clear view of sky
- [ ] Wait 30-90 seconds for cold start
- [ ] Check serial monitor for "GPS: Sats=" messages every 5 seconds
- [ ] Verify satellites >= 4 for fix
- [ ] Verify HDOP < 3.0 for fix
- [ ] Check raw NMEA sentences are being received

---

## Code Locations

### GPS Configuration:
- `src/config.h` lines 30-32, 173-174

### GPS Initialization:
- `src/Navigation.cpp` lines 30-47

### GPS Parsing:
- `src/Navigation.cpp` lines 172-223

### GPS Fix Check:
- `src/Navigation.cpp` lines 95-97

### GPS Debug Output:
- `src/CoreManager.cpp` lines 308-318

### GPS Task:
- `src/CoreManager.cpp` lines 282-324

---

## Advanced Troubleshooting

### Enable Detailed TinyGPSPlus Debug

**Add to `Navigation::processSentences()`:**
```cpp
void Navigation::processSentences() {
    Serial.printf("GPS Stats - Chars=%lu Good=%lu Failed=%lu\n",
        gps.charsProcessed(),
        gps.sentencesWithFix(),
        gps.failedChecksum());

    // Rest of existing code...
}
```

### Check Serial Buffer Health

**Add to `Navigation::parseGPSData()`:**
```cpp
int available = gpsSerial->available();
if (available > 50) {
    Serial.printf("⚠ GPS buffer filling up: %d bytes\n", available);
}
```

### Test GPS Module Alone

**Create simple test sketch:**
```cpp
void setup() {
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.println("GPS Test");
}

void loop() {
    while (Serial2.available()) {
        Serial.write(Serial2.read());
    }
}
```

Upload this to verify GPS module is working before testing full system.

---

## Expected Performance

**Good GPS Performance:**
- Sats: 6-12 visible
- Fix: 1 (true)
- HDOP: 0.8-2.0
- Lat/Lng: Correct location
- Speed: Accurate to ±1 MPH
- Update: Every second

**Cold Start:**
- Time to first fix: 30-90 seconds
- Warm start: 5-15 seconds
- Hot start: 1-5 seconds

---

## Contact

If GPS still not working:
1. Verify GPS module is known-good (test with simple sketch)
2. Check ESP32 UART2 is working (loopback test GPIO 16→17)
3. Try different GPS module if available
4. Check for electromagnetic interference from motors
5. Verify power supply is stable (motors can cause brownouts)
