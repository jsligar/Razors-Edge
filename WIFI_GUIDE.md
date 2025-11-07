# WiFi & Web Interface Documentation

## Overview
The Razors Edge controller includes a comprehensive WiFi-enabled web interface that allows you to monitor and configure your electric vehicle remotely from any smartphone, tablet, or computer.

## Quick Start

### 1. Connect to the Controller
When powered on, the controller automatically creates a WiFi Access Point:

- **Network Name (SSID):** `RazorsEdge-Controller`  
- **Password:** `RazorEdge2025`  
- **Web Interface:** Open browser and go to `http://192.168.4.1`

### 2. Mobile Dashboard Access
1. Connect your phone to the `RazorsEdge-Controller` WiFi network
2. Open any web browser on your phone
3. Navigate to `http://192.168.4.1`
4. Bookmark the page for quick access

## Web Dashboard Features

### 📊 Real-Time Monitoring
- **Power System:** Battery voltage, current, power draw, state of charge
- **Motor Control:** Current gear, motor speeds, motor currents
- **GPS Navigation:** Speed, position, satellite count, trip data
- **Geofencing:** Safe zone status, speed limits, violations, distance to home
- **System Status:** Key switch, safety faults, uptime, connection status

### 🎮 Remote Controls
- **Emergency Stop:** Immediate motor shutdown
- **Gear Changes:** Park, 1st, 2nd, 3rd, Eco, Sport+
- **Trip Reset:** Clear trip meter and max speed
- **Set Home Position:** Mark current GPS location as home base

### 🛡️ Safety Features
- **Real-Time Alerts:** Immediate notification of faults or violations
- **Geofence Status:** Visual indicators for zone boundaries
- **Emergency Controls:** One-tap emergency stop from anywhere
- **Connection Monitoring:** Shows if controller is responsive

## Advanced Configuration

### WiFi Modes

#### Access Point Mode (Default)
- **Pros:** Works anywhere, no existing WiFi needed
- **Cons:** Limited range, one network at a time
- **Best for:** Direct control, initial setup, isolated areas

#### Station Mode (Connect to Home WiFi)
- **Pros:** Internet access, longer range, network integration
- **Cons:** Requires existing WiFi network
- **Best for:** Home/garage use, remote monitoring

#### Dual Mode
- **Pros:** Best of both worlds
- **Cons:** Higher power consumption
- **Best for:** Advanced users, permanent installations

### Network Configuration
```cpp
// Example: Connect to home WiFi
webInterface.connectToNetwork("YourWiFiName", "YourPassword");

// Example: Change Access Point settings
WiFiConfig config;
config.mode = WIFI_AP_MODE;
strcpy(config.apSSID, "MyRazor");
strcpy(config.apPassword, "MyPassword123");
webInterface.setWiFiConfig(config);
```

## API Reference

### REST API Endpoints

#### Get Real-Time Data
```
GET /api/telemetry
Returns: JSON with all sensor data, updated every second
```

#### System Status
```
GET /api/status
Returns: WiFi info, system stats, connection count
```

#### Control Commands
```
POST /api/emergency-stop
Body: {} (empty)
Action: Immediate emergency stop

POST /api/set-gear
Body: {"gear": "park|1st|2nd|3rd|eco|sport"}
Action: Change transmission gear

POST /api/reset-trip
Body: {} (empty)
Action: Reset trip meter and max speed

POST /api/set-home
Body: {} (empty)
Action: Set current GPS position as home
```

#### Geofencing API
```
GET /api/geofences
Returns: List of all configured geofences

POST /api/geofences
Body: {
  "id": 1,
  "type": "safe_zone|speed_limit|prohibited",
  "action": "none|warn|limit_speed|stop|return_home",
  "latitude": 40.7128,
  "longitude": -74.0060,
  "radius": 100.0,
  "speedLimit": 15.0
}
Action: Add new circular geofence

DELETE /api/geofences
Body: {"id": 1}
Action: Remove geofence by ID
```

### Example JSON Response (Telemetry)
```json
{
  "batteryVoltage": 58.4,
  "batteryCurrent": 8.2,
  "batteryPower": 479,
  "batterySOC": 78,
  "motorCurrentLeft": 4.1,
  "motorCurrentRight": 4.3,
  "currentSpeedLeft": 45.0,
  "currentSpeedRight": 47.0,
  "currentGear": 2,
  "motorEnabled": true,
  "gpsFixed": true,
  "latitude": 40.712800,
  "longitude": -74.006000,
  "gpsSpeed": 12.5,
  "satellites": 8,
  "odometer": 15.7,
  "inSafeZone": true,
  "speedLimited": false,
  "currentSpeedLimit": 25.0,
  "distanceToHome": 45.2,
  "totalViolations": 0,
  "keySwitch": true,
  "safetyFault": false,
  "uptime": 3600000
}
```

## Mobile App Development

### Web App Features
The included web interface is designed as a Progressive Web App (PWA):
- **Responsive Design:** Optimized for all screen sizes
- **Offline Capable:** Core functions work without internet
- **App-Like Feel:** Full-screen mode, home screen installation
- **Touch Optimized:** Large buttons, gesture-friendly

### Custom App Integration
For developers wanting to create custom mobile apps:

#### React Native Example
```javascript
const API_BASE = 'http://192.168.4.1/api';

// Get telemetry data
const getTelemetry = async () => {
  const response = await fetch(`${API_BASE}/telemetry`);
  return await response.json();
};

// Emergency stop
const emergencyStop = async () => {
  await fetch(`${API_BASE}/emergency-stop`, { method: 'POST' });
};

// Set gear
const setGear = async (gear) => {
  await fetch(`${API_BASE}/set-gear`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ gear })
  });
};
```

#### Flutter Example
```dart
class RazorAPI {
  static const String baseUrl = 'http://192.168.4.1/api';
  
  static Future<Map<String, dynamic>> getTelemetry() async {
    final response = await http.get(Uri.parse('$baseUrl/telemetry'));
    return json.decode(response.body);
  }
  
  static Future<void> emergencyStop() async {
    await http.post(Uri.parse('$baseUrl/emergency-stop'));
  }
  
  static Future<void> setGear(String gear) async {
    await http.post(
      Uri.parse('$baseUrl/set-gear'),
      headers: {'Content-Type': 'application/json'},
      body: json.encode({'gear': gear}),
    );
  }
}
```

## Security Considerations

### Network Security
- **Password Protection:** Default AP password should be changed
- **Network Isolation:** AP mode isolates controller from internet
- **Local Access Only:** Web interface only accessible on local network

### Safety Interlocks
- **Emergency Override:** Physical controls always take precedence
- **Key Switch:** Web interface cannot override key switch requirement
- **Safety Limits:** Speed/power limits enforced regardless of source
- **Connection Monitoring:** Lost connection triggers safe mode

### Recommended Practices
1. **Change Default Password:** Use strong, unique password
2. **Monitor Connections:** Regularly check connected devices
3. **Physical Security:** Secure controller from unauthorized access
4. **Update Regularly:** Keep firmware updated for security patches

## Troubleshooting

### Connection Issues
- **Can't See Network:** Check controller power, wait 30 seconds after boot
- **Wrong Password:** Default is `RazorEdge2025` (case sensitive)
- **Slow Loading:** Move closer to controller, check for interference
- **Connection Drops:** Monitor battery voltage, low power affects WiFi

### Web Interface Issues
- **Page Won't Load:** Try `http://192.168.4.1`, clear browser cache
- **Data Not Updating:** Check network connection, refresh page
- **Controls Not Working:** Verify key switch is ON, check safety interlocks
- **Emergency Stop Unresponsive:** Use physical emergency stop if needed

### Performance Optimization
- **Range:** Stay within 30 feet for best performance
- **Battery:** WiFi uses power, monitor battery drain during long sessions
- **Interference:** Avoid areas with many WiFi networks
- **Update Rate:** Data updates every second, avoid rapid button presses

### Factory Reset
If network settings become corrupted:
1. Power off controller
2. Hold shift up + shift down buttons during power on
3. Release when LED blinks 3 times
4. Network settings reset to defaults

## Power Management

### WiFi Power Consumption
- **Access Point Mode:** ~150mA additional current
- **Station Mode:** ~100mA additional current
- **Sleep Mode:** WiFi disabled automatically after 10 minutes idle
- **Battery Saver:** Web interface disabled below 20% battery

### Auto-Disable Features
- **Low Battery:** WiFi disabled at 15% SOC to preserve power for driving
- **Emergency Mode:** WiFi disabled during safety faults
- **Key Off:** Web interface disabled when key switch is OFF
- **Thermal Protection:** WiFi reduced power if ESP32 overheats

---

*The web interface provides convenient remote access while maintaining all safety interlocks and emergency overrides. Physical controls always take precedence over web commands.*