# u-center Software Guide for GT-U7 GPS Module

## What is u-center?

u-center is a free GPS evaluation software from u-blox for Windows. It allows you to:
- **View** satellites, signal strength, position on map
- **Test** GPS module before wiring to ESP32
- **Configure** GPS settings (baud rate, update rate, NMEA sentences)
- **Update** GPS firmware (if available)
- **Save** configuration to GPS module's flash memory

---

## Step 1: Download and Install u-center

### Download u-center

1. Go to u-blox website: https://www.u-blox.com/en/product/u-center
2. Click **"Download u-center"** (requires free registration)
3. Or direct link: Search for "u-blox u-center download"
4. **Version**: Download latest version (u-center 2 or u-center 22.x)
5. **Size**: ~50-100 MB

### Install u-center

1. Run the installer (.exe file)
2. Follow installation wizard
3. Default installation path is fine: `C:\Program Files (x86)\u-blox\u-center`
4. No special settings needed during install

---

## Step 2: Connect GT-U7 to PC

### Physical Connection

1. **Plug GT-U7 into PC via USB cable**
2. **Wait for Windows to install drivers** (automatic)
3. **Check Device Manager**:
   - Press `Win + X` → Device Manager
   - Expand **Ports (COM & LPT)**
   - Look for **"USB Serial Port (COM3)"** or similar
   - **Note the COM port number** (e.g., COM3, COM4, COM5)

### If Driver Issues Occur

**Windows doesn't recognize GPS:**
1. GT-U7 uses **CH340 USB-to-serial chip**
2. Download CH340 driver from:
   - https://sparks.gogo.co.nz/ch340.html
   - Or search "CH340 driver download"
3. Install driver and reconnect GPS

---

## Step 3: Connect u-center to GPS

### Open u-center

1. Launch **u-center** from Start Menu
2. Main window appears with map view

### Connect to COM Port

1. Click **Receiver** menu → **Connection** → **COM3** (or your port number)
2. You should immediately see:
   - Green checkmark on toolbar (connected)
   - NMEA sentences scrolling in **Text Console** (bottom panel)
   - Data appearing in various windows

### If Connection Fails

**Check baud rate:**
1. Click **Receiver** → **Baudrate** → **9600** (GT-U7 default)
2. If still nothing, try other baud rates: 4800, 38400, 57600, 115200

**Verify COM port:**
1. Close u-center
2. Unplug GT-U7, check Device Manager - COM port disappears
3. Plug GT-U7 back in, check Device Manager - COM port reappears
4. Re-open u-center and connect to correct COM port

---

## Step 4: View GPS Data

### Main u-center Views

Once connected, you'll see several panels:

#### 1. **Satellite View** (Sky Plot)
- Shows satellites above horizon
- Green bars = good signal
- Yellow bars = weak signal
- No bars = satellite visible but no signal
- **Azimuth** = direction to satellite (0-360°)
- **Elevation** = angle above horizon (0-90°)

#### 2. **Signal Strength** (Bar Graph)
- Horizontal bars showing signal quality (SNR)
- **Green (>30 dBHz)** = Excellent
- **Yellow (20-30 dBHz)** = Good
- **Red (<20 dBHz)** = Poor
- **Gray** = Satellite tracked but not used for fix

#### 3. **Map View**
- Shows your position on world map
- **Red circle** = position uncertainty
- **Blue line** = GPS track (your movement)
- Right-click map for zoom options

#### 4. **Text Console** (Bottom)
- Raw NMEA sentences
- Shows exactly what ESP32 will receive
- Look for: `$GPGGA`, `$GPRMC`, `$GPGSA`, `$GPGSV`

#### 5. **Data View** (Right Side)
- **NAV-POSLLH** = Position (Lat/Lng/Alt)
- **NAV-SOL** = GPS fix status
- **NAV-VELNED** = Velocity (speed)
- **NAV-TIMEUTC** = GPS time

### Check GPS Fix Status

Look at bottom status bar:
- **"No Fix"** = Searching for satellites (red)
- **"2D Fix"** = Position fix but no altitude (yellow)
- **"3D Fix"** = Full position fix (green) ✅
- **"Time Only"** = GPS time but no position

**Wait 30-90 seconds for first fix** (cold start)

---

## Step 5: Configure GPS Settings (Optional)

### View Current Configuration

1. Click **View** menu → **Configuration View**
2. Shows all GPS settings
3. This opens **UBX-CFG** panels

### Useful Configurations

#### A. **Increase Update Rate** (Default: 1Hz → 5Hz or 10Hz)

**Why?** Faster position updates for your vehicle

1. **View** → **Messages View** → **UBX** → **CFG (Config)**
2. Expand **CFG** → Select **RATE (Rates)**
3. In right panel:
   - **Measurement Period**: Change from `1000 ms` to `200 ms` (5Hz) or `100 ms` (10Hz)
   - Click **Send** button (bottom left)
4. GPS now updates 5 or 10 times per second

**Note**: Your ESP32 code currently expects 1Hz. You'd need to update firmware to handle faster rates.

#### B. **Change Baud Rate** (Default: 9600)

**Why?** Higher baud rate for faster data transmission

1. **View** → **Messages View** → **UBX** → **CFG (Config)**
2. Expand **CFG** → Select **PRT (Ports)**
3. In right panel:
   - Find **UART1** section
   - **Baudrate**: Change from `9600` to `38400` or `115200`
   - Click **Send**
4. **Important**: Also change u-center baud rate: **Receiver** → **Baudrate** → Select new rate
5. **Also update ESP32 code** in `config.h`: Change `GPS_BAUD_RATE` to match

#### C. **Select NMEA Sentences**

**Why?** Reduce unnecessary data, save bandwidth

1. **View** → **Messages View** → **UBX** → **CFG (Config)**
2. Expand **CFG** → Select **MSG (Messages)**
3. Enable/disable specific NMEA sentences:
   - **GGA** - Position, altitude, satellites ✅ (keep enabled)
   - **RMC** - Position, speed, time ✅ (keep enabled)
   - **GSA** - DOP and active satellites ✅ (keep enabled)
   - **GSV** - Satellite details (can disable to reduce data)
   - **GLL** - Position only (can disable, redundant with GGA)
   - **VTG** - Course and speed (can disable if not needed)

#### D. **Enable SBAS/WAAS** (Better accuracy)

**Why?** Improves GPS accuracy from ~10m to ~3m

1. **View** → **Messages View** → **UBX** → **CFG (Config)**
2. Expand **CFG** → Select **SBAS**
3. Check boxes:
   - ✅ **Enable SBAS**
   - ✅ **Enable WAAS** (USA)
   - ✅ **Test Mode** (optional, for testing)
4. Click **Send**

---

## Step 6: Save Configuration to GPS Module

**CRITICAL**: After making changes, save to GPS flash memory!

### Save Configuration Permanently

1. **View** → **Messages View** → **UBX** → **CFG (Config)**
2. Select **CFG (Configuration)**
3. In right panel:
   - Check **Save current configuration**
   - Devices: Select **BBR (Battery Backed RAM)** and **FLASH**
   - Click **Send**
4. GPS module will save settings and keep them after power cycle

**Without saving**: Changes are lost when GPS powers off!

---

## Step 7: Update GPS Firmware (Advanced)

### Check Current Firmware Version

1. **View** → **Packet Console**
2. Look for **MON-VER** message
3. Shows firmware version (e.g., `7.03` or `1.00`)

### Update Firmware (if available)

**Warning**: Firmware updates can brick GPS if interrupted!

1. Download firmware from u-blox website:
   - Search for your chip: **NEO-6M** or **NEO-7M**
   - Download `.bin` firmware file
2. In u-center:
   - **Tools** → **Firmware Update**
   - Select firmware `.bin` file
   - Click **GO**
3. **DO NOT DISCONNECT** GPS during update (5-10 minutes)
4. Wait for "Update successful" message

**Recommendation**: Only update if you have a specific reason (bug fix, new feature)

---

## Step 8: Test GPS Performance

### Record GPS Track

1. Click **File** → **Database Logging** → **Start**
2. Choose filename and location
3. Walk/drive around with GPS
4. Click **Stop** when done
5. **File** → **Database Replay** → Open saved file
6. View your recorded track on map

### Check GPS Accuracy

**Position accuracy:**
1. **View** → **Messages View** → **UBX** → **NAV (Navigation)**
2. Select **NAV-SOL**
3. Look at **Position Accuracy**:
   - **<5m** = Excellent
   - **5-10m** = Good
   - **10-20m** = Fair
   - **>20m** = Poor (bad satellite geometry or interference)

**HDOP (Horizontal Dilution of Precision):**
1. Lower is better
2. **<2.0** = Excellent satellite geometry
3. **2.0-5.0** = Good
4. **>5.0** = Poor (wait for better satellite positions)

---

## Common u-center Tasks

### Task 1: Verify GPS Works Before Wiring to ESP32

1. Connect GT-U7 via USB
2. Open u-center, connect to COM port
3. Wait for 3D fix (30-90 seconds)
4. Check satellite count (need 4+)
5. Check position on map is correct
6. ✅ GPS confirmed working

### Task 2: Increase Update Rate to 5Hz

1. **CFG** → **RATE** → Measurement Period = `200 ms`
2. Click **Send**
3. **CFG** → **CFG** → Save configuration
4. Update ESP32 code: Change `GPS_UPDATE_RATE` in `config.h` to `5`

### Task 3: View Raw NMEA Data

1. Bottom panel: **Text Console**
2. See exactly what ESP32 receives:
   ```
   $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
   ```

### Task 4: Export GPS Data

1. **File** → **Database Logging** → Start
2. Records all GPS data to `.ubx` file
3. Can replay later: **File** → **Database Replay**
4. Or export to KML for Google Earth

---

## Troubleshooting u-center

### Problem: u-center won't connect

**Solution**:
1. Check COM port in Device Manager
2. Close other programs using COM port (Arduino IDE, PuTTY)
3. Try different baud rate (9600, 38400, 115200)
4. Restart u-center
5. Reconnect GPS

### Problem: No satellites visible

**Solution**:
1. Take GPS **outside** with clear view of sky
2. Wait 30-90 seconds for cold start
3. Check antenna is facing up
4. Avoid metal buildings, trees, thick walls

### Problem: Lots of satellites but no fix

**Solution**:
1. Check **HDOP** value (should be <5.0)
2. Enable SBAS/WAAS for better accuracy
3. Wait for better satellite geometry
4. Move to open area (away from buildings)

### Problem: GPS position is wrong

**Solution**:
1. Wait longer for fix (can take 2 minutes for first fix)
2. Check you're looking at correct map region
3. Enable SBAS/WAAS
4. Check GPS time is correct (if time is wrong, position will be wrong)

---

## After Using u-center

### Before Connecting to ESP32

1. **Disconnect GT-U7 from PC** (unplug USB)
2. **Important**: If you changed baud rate in u-center, update ESP32 code:
   - Edit `src/config.h`
   - Change `#define GPS_BAUD_RATE 9600` to new baud rate
   - Re-upload firmware to ESP32
3. Wire GT-U7 to ESP32 using 5-pin header (see GT-U7_WIRING.md)

### If You Changed Update Rate

**Changed to 5Hz or 10Hz?**

Update ESP32 code:
```cpp
// In src/config.h
#define GPS_UPDATE_RATE 5  // Changed from 1 to 5 Hz
```

Re-upload firmware.

---

## u-center Keyboard Shortcuts

| Key | Action |
|-----|--------|
| F3 | Toggle Text Console |
| F5 | Toggle Packet Console |
| F9 | Toggle Messages View |
| Ctrl+D | Start database logging |
| Ctrl+R | Database replay |
| Ctrl+L | Clear map track |

---

## Alternative to u-center (Non-Windows)

### For Mac/Linux:

**Option 1: Serial Terminal**
- Use any serial terminal (screen, minicom, CoolTerm)
- Connect at 9600 baud
- View raw NMEA sentences
- No pretty GUI but confirms GPS works

**Option 2: GPSD + GPS Test Tools**
- Install `gpsd` (GPS daemon)
- Use `cgps` or `xgps` for visual display
- More complex setup but works on Linux

**Option 3: Web-Based GPS Viewers**
- Some websites can decode NMEA data
- Paste NMEA sentences to visualize
- No software installation needed

---

## Quick Reference

**Basic u-center Workflow:**
1. Connect GPS via USB
2. Open u-center
3. **Receiver** → **Connection** → **COM3** (your port)
4. Wait for satellites and 3D fix
5. View data in various panels
6. Make changes if needed
7. **CFG** → **CFG** → Save configuration
8. Test and verify

**GT-U7 Defaults:**
- Baud: 9600
- Update Rate: 1Hz
- Protocol: NMEA + UBX
- Fix Mode: Auto (2D/3D)

---

## Need Help?

**u-center Documentation:**
- u-blox website has full user manual
- Search: "u-center user guide PDF"

**GT-U7 Specific:**
- See `GT-U7_WIRING.md` for pin connections
- See `GPS_TROUBLESHOOTING.md` for GPS issues

**Common Issue**: If GPS works in u-center but not with ESP32, check:
- Baud rate matches between GPS and ESP32 code
- RX/TX wires are crossed correctly
- ESP32 has 3.3V power to GPS
- No USB connected while ESP32 is powered (can damage both!)

---

Good luck configuring your GT-U7! 🛰️
