# GT-U7 GPS Quick Wiring Guide

## GT-U7 with USB Port

Your GT-U7 has a built-in USB-to-serial adapter! This makes testing much easier.

### Two Connection Options:

**Option 1: USB Connection (Testing/Standalone)**
- Plug USB cable directly into GT-U7
- Powers the module and provides serial communication
- Great for testing GPS before wiring to ESP32
- Can view GPS data on PC using serial terminal

**Option 2: Direct Wiring to ESP32 (Final Installation)**
- Use the 5-pin header for ESP32 connection
- This is what you'll use in the vehicle
- Connects to GPIO 16/17 for serial communication

---

## Testing GPS Module First (Recommended!)

### Step 1: Test with USB Connection

Before wiring to ESP32, verify your GPS module works:

1. **Connect GT-U7 to your PC via USB**
2. **Find the COM port**:
   - Windows: Device Manager → Ports (COM & LPT)
   - Look for "USB Serial Port (COM3)" or similar
3. **Open serial terminal**:
   - Use Arduino Serial Monitor, PuTTY, or Tera Term
   - Settings: 9600 baud, 8 data bits, no parity, 1 stop bit
4. **Check for NMEA sentences**:
   ```
   $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
   ```
5. **Wait for GPS fix** (30-90 seconds outdoors)

✅ If you see NMEA sentences, your GPS module works!
❌ If you see nothing, check USB cable and try different COM port

### Step 2: Advanced Testing (Optional)

**Use u-center Software** (Windows only):
1. Download from u-blox website (free)
2. Connect GT-U7 via USB
3. Select COM port in u-center
4. Baud: 9600
5. See satellites, signal strength, position on map

**Alternative GPS Viewers**:
- **GPS Visualizer** - Web-based, simple
- **GPSInfo** - Android app (can connect via Bluetooth adapter)
- **Arduino Serial Plotter** - View satellite count over time

**What to Look For**:
- Satellite count should increase to 4+ (outdoors)
- LED on GT-U7 changes from slow blink to fast blink
- Latitude/longitude values appear in NMEA sentences
- HDOP value decreases (lower is better)

---

## Pin Connections (After USB Test)

```
GT-U7 Pin       →    ESP32 Pin
---------            ---------
VCC             →    3.3V (3V3)
RX              →    GPIO 17
TX              →    GPIO 16
GND             →    GND
PPS             →    (not connected)
```

## Visual Diagram

```
GT-U7 Module with USB:
┌─────────────────────┐
│   [GPS ANTENNA]     │
│                     │
│  VCC RX TX GND PPS  │ ← 5-pin header for ESP32
│                     │
│     [USB PORT]      │ ← USB for testing/power
└─────────────────────┘

ESP32 DevKit:
                3.3V ← VCC (GT-U7 Pin 1)

          GPIO 17/TX ← RX  (GT-U7 Pin 2)
          GPIO 16/RX ← TX  (GT-U7 Pin 3)

                 GND ← GND (GT-U7 Pin 4)

                     ← PPS (GT-U7 Pin 5) - leave disconnected
```

## Important Notes

1. **USB vs Pin Header - Use ONE at a Time**
   - ⚠️ **NEVER connect USB and ESP32 pins simultaneously!**
   - USB is for testing only (before final installation)
   - Once you confirm GPS works via USB, disconnect USB
   - Then wire the 5-pin header to ESP32 for final use
   - Using both could damage the GPS module or ESP32

2. **RX/TX are CROSSED**
   - GT-U7 RX → ESP32 GPIO 17 (ESP32 TX)
   - GT-U7 TX → ESP32 GPIO 16 (ESP32 RX)

3. **Voltage**: Use 3.3V from ESP32 (GT-U7 accepts 3.3V-5V)

4. **PPS Pin**: Leave disconnected - not needed for basic operation

5. **LED Indicator on GT-U7**:
   - Blinking slowly = Searching for satellites
   - Blinking fast = GPS fix acquired
   - No light = No power or module fault

## Testing Steps (Recommended Order)

### Phase 1: Test GPS Module Alone (USB)

**Step 1: Test with USB (RECOMMENDED FIRST)**
1. Plug GT-U7 into PC via USB cable
2. Open serial terminal (9600 baud)
3. Take module outdoors
4. Wait 30-90 seconds
5. Confirm you see NMEA sentences with satellite data
6. ✅ GPS module is confirmed working!

**Step 2: Disconnect USB**
- Unplug USB cable from GT-U7
- Important: Never use USB and ESP32 pins together

### Phase 2: Wire to ESP32

**Step 3: Connect 5-Pin Header to ESP32**
Wire the pins (USB disconnected):
- GT-U7 VCC → ESP32 3.3V
- GT-U7 RX → ESP32 GPIO 17
- GT-U7 TX → ESP32 GPIO 16
- GT-U7 GND → ESP32 GND
- GT-U7 PPS → (not connected)

**Step 4: Upload Firmware**
Upload the Razors Edge firmware to ESP32

**Step 5: Monitor Serial Output**
Open serial monitor at 115200 baud and look for:
```
GPS: Sats=0 Fix=0 Lat=0.000000 Lng=0.000000 Speed=0.0 mph HDOP=99.9
```

This appears every 5 seconds once the system is running.

**Step 6: Wait for Fix**
- Take ESP32 + GPS outside
- Wait 30-90 seconds for first fix
- Watch serial monitor for satellites to appear:
```
GPS: Sats=4 Fix=1 Lat=39.123456 Lng=-94.654321 Speed=0.0 mph HDOP=1.8
```

## Expected Serial Output

**System Startup:**
```
=================================
    RAZOR 60V CONTROLLER v1.1
    ESP32 Electric Vehicle
=================================
I2C Bus initialized
Initializing modules...
✓ Input Handler initialized
✓ Power Manager initialized
✓ Safety System initialized
✓ Motor Controller initialized
✓ Light Controller initialized
✓ User Interface initialized
✓ Navigation initialized  ← GPS initialized here
✓ Web Interface initialized
✓ State Machine initialized
✓ Dual-core task system active
  → Core 0: WiFi, Web Interface
  → Core 1: Safety, Motors, Navigation, UI
```

**Every 5 seconds:**
```
GPS: Sats=6 Fix=1 Lat=39.123456 Lng=-94.654321 Speed=0.0 mph HDOP=1.2
```

## Troubleshooting Quick Checks

| Problem | Check | Solution |
|---------|-------|----------|
| No GPS messages in serial | Check wiring | Verify GPIO 16/17 connections |
| Sats=0 always | GPS not receiving data | Check RX/TX are crossed |
| Sats>0 but Fix=0 | Not enough satellites or poor HDOP | Move outside, wait longer |
| Fix jumps around | Poor satellite geometry | Wait for HDOP < 3.0 |

## Common Mistakes

❌ **Wrong**: GT-U7 TX → ESP32 GPIO 16 (ESP32 RX) ← ✅ **Correct**
❌ **Wrong**: GT-U7 RX → ESP32 GPIO 17 (ESP32 TX) ← ✅ **Correct**

❌ **Wrong**: GT-U7 TX → ESP32 GPIO 17
❌ **Wrong**: GT-U7 RX → ESP32 GPIO 16

Remember: **RX connects to TX, and TX connects to RX** (crossed connection)

## Breadboard Example

```
GT-U7 Module:          Breadboard:              ESP32:

VCC ────────────────── [red wire] ──────────── 3.3V
RX  ────────────────── [yellow] ────────────── GPIO 17
TX  ────────────────── [green] ─────────────── GPIO 16
GND ────────────────── [black] ─────────────── GND
PPS (not connected)
```

## For More Info

See `GPS_TROUBLESHOOTING.md` for detailed diagnostics and advanced troubleshooting.

## Quick Facts

- **Module**: GT-U7 (u-blox NEO-6M/7M based)
- **Baud Rate**: 9600 (configured in firmware)
- **Update Rate**: 1Hz (once per second)
- **Protocol**: NMEA 0183
- **Voltage**: 3.3V-5V (use 3.3V from ESP32)
- **Current**: ~50mA when searching, ~30mA when locked
- **Cold Start**: 27 seconds typical
- **Antenna**: Built-in ceramic patch

Good luck with your GPS setup! 🛰️
