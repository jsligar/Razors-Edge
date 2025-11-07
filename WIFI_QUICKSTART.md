# Quick Start: WiFi Web Interface

## Step-by-Step Setup (5 minutes)

### 1. Power On Controller
- Turn key switch ON
- Wait 30 seconds for full boot
- Look for solid status LED

### 2. Connect Phone to Controller
**On your smartphone:**
1. Open WiFi settings
2. Look for network: `RazorsEdge-Controller`
3. Connect using password: `RazorEdge2025`
4. Wait for connection confirmation

### 3. Open Web Dashboard
1. Open any web browser on your phone
2. Go to: `http://192.168.4.1`
3. Bookmark this page for quick access
4. Dashboard should load in 2-3 seconds

### 4. Test Remote Functions
**Try these features:**
- View real-time battery voltage
- Check GPS satellite count
- Change gear to "Park" 
- Test emergency stop (vehicle must be stopped first)
- Reset trip meter

## Mobile Dashboard Overview

### 📊 **Power System Card**
- Battery Voltage (should show 54-63V)
- Battery Current (shows power draw)
- Power Draw (watts consumed)
- State of Charge (percentage remaining)

### 🔧 **Motor Control Card**
- Current Gear (Park, 1st, 2nd, 3rd, Eco, Sport+)
- Motor Status (Active/Stopped)
- Left Motor Speed (percentage)
- Right Motor Speed (percentage)

### 🛰️ **GPS & Navigation Card**
- GPS Status (Fixed/Searching)
- Current Speed (MPH)
- Satellite Count (need 4+ for accuracy)
- Odometer (total miles driven)
- GPS Map (shows position if available)

### 🛡️ **Geofencing Card**
- Safe Zone Status (YES/NO)
- Current Speed Limit (MPH)
- Distance to Home (meters)
- Total Violations (count)

### 🎮 **Controls Card**
- **🛑 EMERGENCY STOP** - Immediate motor shutdown
- **🅿️ Park** - Set transmission to park
- **🌱 Eco** - Switch to eco mode
- **📊 Reset Trip** - Clear trip meter
- **🏠 Set Home** - Mark current GPS location as home

### ⚙️ **System Status Card**
- Key Switch (ON/OFF)
- Safety Status (OK/FAULT)
- Uptime (hours and minutes)
- Connection Status (Connected/Disconnected)

## Common Tasks

### View Real-Time Data
Data updates automatically every second. No need to refresh the page.

### Change Gears Remotely
1. Ensure vehicle is stopped
2. Tap desired gear button (Park, Eco, etc.)
3. Gear change happens immediately
4. Dashboard shows new gear within 1 second

### Emergency Stop
1. Tap the red "🛑 EMERGENCY STOP" button
2. Confirm action in popup dialog
3. Motors stop immediately
4. Must restart key switch to resume

### Set Home Position
1. Drive to desired home location
2. Wait for GPS fix (4+ satellites)
3. Tap "🏠 Set Home" button
4. Home position saved for geofencing

### Monitor Geofencing
- Green "Safe Zone: YES" = inside safe area
- Red "Safe Zone: NO" = outside safe boundaries
- Speed limit shows current maximum allowed
- Distance to home shows meters from home base

## Troubleshooting

### "Can't find RazorsEdge-Controller network"
- Wait 30 seconds after powering on
- Move closer to controller (30 feet max range)
- Check that controller key switch is ON

### "Page won't load at 192.168.4.1"
- Verify connected to correct WiFi network
- Try typing `http://192.168.4.1` exactly
- Clear browser cache and try again

### "Data shows all zeros"
- Check that controller is fully booted
- Verify key switch is ON
- Try refreshing the page

### "Controls don't work"
- Key switch must be ON for controls to work
- Emergency stop requires vehicle to be stopped
- Gear changes only work when pedal is released

### "WiFi keeps disconnecting"
- Check controller battery voltage (low battery affects WiFi)
- Stay within 30 feet of controller
- Avoid areas with many competing WiFi networks

## Advanced Features

### Custom WiFi Network
To connect controller to your home WiFi instead of using its own network:
1. Modify `config.h` file before uploading
2. Set your WiFi name and password
3. Controller will connect to your network
4. Find controller's IP address in your router settings

### API for Custom Apps
Developers can create custom mobile apps using the REST API:
- `/api/telemetry` - Get all sensor data
- `/api/emergency-stop` - Trigger emergency stop
- `/api/set-gear` - Change transmission gear
- See [WIFI_GUIDE.md](WIFI_GUIDE.md) for complete API documentation

### Security Notes
- Change default password in code before deployment
- Web interface only works on local network (no internet access)
- Physical controls always override web commands
- Emergency stops work even if WiFi fails

---

**Need Help?** See the complete [WIFI_GUIDE.md](WIFI_GUIDE.md) for detailed configuration options and troubleshooting.