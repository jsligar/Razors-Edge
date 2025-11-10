# GT-U7 GPS Diagnostics - Step-by-Step Testing

## Current Status
✅ **LED is blinking** - GPS module has power
❌ **No data visible** - Need to verify serial communication

---

## Diagnostic Test 1: Check USB Connection

### Windows - Check Device Manager

1. **Open Device Manager**:
   - Press `Win + X` → Device Manager
   - Or search "Device Manager" in Start menu

2. **Check for COM Port**:
   - Expand **"Ports (COM & LPT)"**
   - Look for one of these:
     - "USB Serial Port (COM3)" or similar
     - "USB-SERIAL CH340 (COM3)"
     - "USB Serial Device"
   - **Note the COM port number** (COM3, COM4, etc.)

### If NO COM Port Appears:

**Problem**: Missing CH340 USB driver

**Solution**:
1. Download CH340 driver:
   - https://sparks.gogo.co.nz/ch340.html
   - Or search "CH340 driver download"
2. Install the driver
3. Unplug GT-U7 and plug back in
4. Check Device Manager again - COM port should appear

### If COM Port Appears with Yellow Warning (!) :

**Problem**: Driver issue

**Solution**:
1. Right-click the COM port → **Update Driver**
2. Choose "Browse my computer for drivers"
3. Choose "Let me pick from a list"
4. Select "USB Serial Device" or "USB Serial CH340"
5. Click Next

---

## Diagnostic Test 2: Test with Serial Terminal

### Using Arduino IDE Serial Monitor

**Step 1: Open Arduino IDE**
- Don't need to upload anything yet
- Just use the serial monitor tool

**Step 2: Configure Serial Monitor**
1. Plug GT-U7 into PC via USB
2. Arduino IDE → Tools → Port → Select your COM port (COM3, etc.)
3. Tools → Serial Monitor (or Ctrl+Shift+M)
4. Set baud rate dropdown: **9600**
5. Set line ending: **"Both NL & CR"** or **"No line ending"**

**Step 3: Check for Data**

**What you should see:**
```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
```

**If you see this** ✅ - GPS is working! Move to Test 3

**If you see nothing** ❌ - Continue below

**If you see gibberish/random characters** ⚠️ - Wrong baud rate, try Test 4

---

## Diagnostic Test 3: Try Different Baud Rates

Sometimes GPS modules are configured to different baud rates.

**In Arduino Serial Monitor, try these baud rates:**
1. **9600** (default)
2. **4800**
3. **38400**
4. **57600**
5. **115200**

For each baud rate:
- Change dropdown at bottom of serial monitor
- Wait 5 seconds
- Look for NMEA sentences

**If one works**: Remember that baud rate! You'll need to update ESP32 code to match.

---

## Diagnostic Test 4: Try Different Serial Terminal

### Option A: PuTTY (Windows)

1. Download PuTTY: https://www.putty.org/
2. Run PuTTY
3. Select **"Serial"** connection type
4. Serial line: **COM3** (your port)
5. Speed: **9600**
6. Click **"Open"**
7. Look for NMEA sentences

### Option B: Tera Term (Windows)

1. Download Tera Term
2. File → New Connection
3. Select **Serial**
4. Port: **COM3**
5. Setup → Serial Port → Speed: **9600**
6. Look for NMEA sentences

### Option C: screen (Mac/Linux)

```bash
screen /dev/ttyUSB0 9600
```

Or:
```bash
screen /dev/cu.usbserial-* 9600
```

---

## Diagnostic Test 5: Simple Arduino GPS Test Sketch

Upload this minimal test sketch to verify GPS is transmitting:

### GPS Test Sketch

```cpp
// Simple GT-U7 GPS Test - Upload to Arduino/ESP32
// This will read USB GPS and print to Serial Monitor

#define GPS_BAUD 9600

void setup() {
  // Serial to PC for viewing data
  Serial.begin(115200);
  Serial.println("GT-U7 GPS Test Starting...");
  Serial.println("Waiting for GPS data...");
  Serial.println("");

  // Note: GT-U7 USB appears as Serial on PC
  // We're just reading from PC serial and displaying it
}

void loop() {
  // Read any available data from PC serial
  if (Serial.available()) {
    char c = Serial.read();
    Serial.write(c);  // Echo it back
  }

  delay(1);
}
```

**Wait, that won't work for USB GPS!** Let me give you the right test:

---

## Diagnostic Test 6: Raw USB Test (No Arduino Needed)

### Using Python (if installed)

Create a file called `gps_test.py`:

```python
import serial
import time

# Change COM3 to your port
port = 'COM3'  # Windows
# port = '/dev/ttyUSB0'  # Linux
# port = '/dev/cu.usbserial-1234'  # Mac

baud = 9600

print(f"Opening {port} at {baud} baud...")

try:
    ser = serial.Serial(port, baud, timeout=1)
    print("Connected! Waiting for GPS data...")
    print("=" * 50)

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('ascii', errors='ignore')
            print(line, end='')
        time.sleep(0.01)

except serial.SerialException as e:
    print(f"Error: {e}")
    print("Check COM port number in Device Manager")
except KeyboardInterrupt:
    print("\nTest stopped by user")
    ser.close()
```

Run with: `python gps_test.py`

---

## Diagnostic Test 7: Test GT-U7 with ESP32 (Without Main Firmware)

Upload this simple sketch to ESP32 to test GPS via the 5-pin header:

```cpp
// ESP32 GPS Test - Tests GPIO 16/17 serial connection
// Upload this INSTEAD of main firmware for testing

void setup() {
  // Serial to PC for viewing data
  Serial.begin(115200);
  Serial.println("ESP32 GPS Test");
  Serial.println("Connecting to GPS on GPIO 16/17...");

  // GPS on Serial2 (GPIO 16 RX, GPIO 17 TX)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for GPS data...");
  Serial.println("Take GPS module outside!");
  Serial.println("");
}

void loop() {
  // Read from GPS and print to PC
  while (Serial2.available()) {
    char c = Serial2.read();
    Serial.write(c);  // Print raw GPS data
  }
}
```

**Upload this to ESP32, open Serial Monitor at 115200, you should see NMEA sentences.**

---

## What Each Blinking Pattern Means

### GT-U7 LED Behavior:

**Slow Blink (1 per second)**:
- GPS is powered
- Searching for satellites
- No GPS fix yet
- **This is normal indoors!**

**Fast Blink (multiple per second)**:
- GPS has lock
- Receiving satellite data
- Position fix acquired
- **Need to be OUTDOORS for this**

**Solid On**:
- Some modules do this when locked
- Variant behavior

**No Blink**:
- No power
- Check USB cable
- Check VCC/GND connections

---

## Common Issues and Solutions

### Issue 1: GPS Blinks But No Data in Serial Monitor

**Possible Causes**:
1. **Wrong COM port selected**
   - Solution: Check Device Manager, select correct port

2. **Wrong baud rate**
   - Solution: Try 9600, 4800, 38400, 57600, 115200

3. **Driver not installed**
   - Solution: Install CH340 driver

4. **Serial port in use by another program**
   - Solution: Close Arduino IDE, u-center, PuTTY, etc. - only one program can use COM port at a time

### Issue 2: GPS Shows Data But No Satellite Fix

**Symptoms**: See NMEA sentences but all zeros
```
$GPGGA,123519,,,,0,00,99.9,,,,,,*48
```

**Solutions**:
1. **Go outside** - GPS needs clear view of sky
2. **Wait 30-90 seconds** - Cold start takes time
3. **Check antenna** - Should face up toward sky
4. **Move away from buildings** - Walls block GPS signals

### Issue 3: Gibberish Characters

**Symptoms**: Random characters like `â–'â–'â–'â–'` or `âœ"`

**Cause**: Wrong baud rate

**Solution**: Try different baud rates (see Test 3)

### Issue 4: "Port Already in Use" Error

**Cause**: Another program has the COM port open

**Solution**:
1. Close ALL serial programs (Arduino, u-center, PuTTY)
2. Unplug GPS
3. Plug GPS back in
4. Open only ONE serial program

---

## Diagnostic Results Interpretation

### Result A: No COM Port in Device Manager
→ **Driver issue** - Install CH340 driver

### Result B: COM Port Exists, But No Data
→ **Connection issue** - Try different USB cable or USB port

### Result C: Gibberish Characters
→ **Baud rate mismatch** - Try different baud rates

### Result D: See NMEA Sentences (Even with Zeros)
→ **GPS module works!** ✅ Just needs outdoor satellite fix

### Result E: Nothing at All
→ **Hardware issue** - GPS module may be defective

---

## Quick Diagnostic Checklist

Run through this checklist:

- [ ] USB cable plugged into GT-U7 and PC
- [ ] LED on GT-U7 is blinking (any speed)
- [ ] COM port appears in Device Manager
- [ ] COM port has NO yellow warning icon
- [ ] Opened serial terminal (Arduino Serial Monitor)
- [ ] Selected correct COM port
- [ ] Set baud rate to 9600
- [ ] Waited 10 seconds
- [ ] Tried different baud rates (4800, 38400, 115200)
- [ ] Closed all other programs that might use COM port
- [ ] Tried different USB port on PC
- [ ] Tried different USB cable
- [ ] Installed CH340 driver (if needed)

---

## Next Steps Based on Results

### If You See NMEA Data (Success!):

1. **Note your baud rate** (probably 9600)
2. **Test outdoors** - Take GPS outside for 1-2 minutes
3. Watch for satellite count to increase:
   ```
   $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*48
                                              ^^
                                              Satellite count (08 = 8 satellites)
   ```
4. **GPS confirmed working!** ✅
5. Now wire to ESP32 using 5-pin header (see GT-U7_WIRING.md)

### If Still No Data:

1. **Try different USB cable** - Cable might be power-only (no data lines)
2. **Try different PC USB port** - Some ports have issues
3. **Test on different computer** - Rule out PC issue
4. **Contact seller** - Module might be defective

---

## Hardware Test (Multimeter)

If you have a multimeter:

### Test 1: Power
- USB plugged in
- Measure between VCC pin and GND pin
- Should read: **3.3V to 5V**
- If 0V: Module not getting power

### Test 2: TX Pin
- USB plugged in, GPS sending data
- Multimeter in AC voltage mode
- Measure TX pin (relative to GND)
- Should see: **Small voltage fluctuations** (data signal)
- If steady 0V or 3.3V: GPS not transmitting

---

## Final Test: Test with Known-Good GPS Viewer

### u-center Software (Windows):

See UCENTER_GUIDE.md for full instructions, but quick test:

1. Download u-center from u-blox
2. Install and run
3. Receiver → Connection → COM3 (your port)
4. Receiver → Baudrate → 9600
5. Should see satellites appear

If u-center shows satellites but Arduino Serial Monitor doesn't, it's a serial monitor configuration issue.

---

## Get Help

If none of these diagnostics work:

**Post these details:**
1. What COM port appears in Device Manager?
2. Any yellow warnings on COM port?
3. What happens in Arduino Serial Monitor at 9600 baud?
4. Does LED blink?
5. Tried different baud rates?
6. Tried different USB cable?
7. Installed CH340 driver?

**Most Common Cause**: Wrong COM port or missing CH340 driver

---

## Expected Timeline

**Immediate** (0-10 seconds):
- LED starts blinking when USB plugged in
- COM port appears in Device Manager
- NMEA sentences start appearing in serial monitor

**After 30-90 seconds outdoors**:
- Satellite count increases from 0 to 4+
- GPS fix acquired
- Latitude/longitude values appear

**If nothing after 2 minutes**:
- Hardware or driver issue (not normal)

---

Good luck! Start with Diagnostic Test 1 (Device Manager) and work your way through. Let me know what you see! 🛰️
