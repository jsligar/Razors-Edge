# Web Interface Guide - Razors Edge v1.1.0

## Overview

The Razors Edge web interface has been completely rebuilt with modern features:

- **Modern UI**: Responsive design with dark theme
- **Real-time Updates**: WebSocket support with polling fallback
- **Authentication**: Secure session-based login
- **GPS Mapping**: Interactive Leaflet.js map
- **Progressive Web App**: Installable on mobile devices
- **Toast Notifications**: User-friendly feedback
- **Offline Support**: Service worker caching

## Quick Start

### 1. Upload Web Files to ESP32

The web files are in the `data/` directory and need to be uploaded to SPIFFS/LittleFS:

```bash
# Upload filesystem
pio run --target uploadfs
```

### 2. Access the Dashboard

1. Connect to WiFi: **RazorsEdge-Controller**
2. Password: **RazorEdge2025**
3. Open browser: **http://192.168.4.1**
4. Login with:
   - Username: **admin**
   - Password: **RazorEdge2025** (default)

### 3. Change Default Password

For security, change the default password immediately:

```cpp
// Via WebAuth API
WebAuth::getInstance().setPassword("YourSecurePassword");
WebAuth::getInstance().setUsername("yourusername");
```

Or implement a settings page in the web interface.

## Features

### Authentication System

- **Session-based**: Secure token authentication
- **IP Validation**: Sessions tied to client IP
- **Timeout**: 30-minute default timeout
- **Persistent**: Uses NVS storage

**Login Flow:**
1. User enters credentials on `/login.html`
2. POST to `/api/login` with username/password
3. Server validates and returns token
4. Token stored in localStorage
5. Token sent in `X-Auth-Token` header for API calls

### Real-time Data Updates

The dashboard supports two modes:

**1. WebSocket (Preferred)**
- Real-time bidirectional communication
- Low latency updates
- Event notifications
- Connects to `ws://192.168.4.1/ws`

**2. Polling (Fallback)**
- Fetches `/api/telemetry` every 1 second
- Works when WebSocket unavailable
- Higher latency but more reliable

### GPS Mapping

**Features:**
- Interactive Leaflet.js map
- Vehicle position marker
- Auto-center on GPS fix
- OpenStreetMap tiles

**Setup:**
No configuration needed - map initializes automatically when GPS fix is acquired.

### Progressive Web App (PWA)

**Install on Mobile:**
1. Open dashboard in mobile browser
2. Tap "Add to Home Screen"
3. App installs like native app
4. Works offline (cached data)

**Features:**
- Standalone app experience
- Offline support via service worker
- App icon on home screen
- Fast loading from cache

### Toast Notifications

User-friendly notifications for all actions:

```javascript
// Types: success, error, warning, info
showToast('Trip meter reset', 'success');
showToast('GPS not fixed', 'warning');
showToast('Emergency stop activated', 'error');
```

## API Endpoints

### Authentication

#### POST `/api/login`
```json
{
  "username": "admin",
  "password": "RazorEdge2025"
}
```

**Response:**
```json
{
  "status": "ok",
  "token": "abc123...",
  "username": "admin"
}
```

#### POST `/api/logout`
Invalidates current session.

### Telemetry

#### GET `/api/telemetry`
Returns complete system telemetry (requires auth):

```json
{
  "batteryVoltage": 60.5,
  "batteryCurrent": 12.3,
  "batteryPower": 744.15,
  "batterySOC": 85,
  "motorCurrentLeft": 5.2,
  "motorCurrentRight": 5.4,
  "currentSpeedLeft": 45,
  "currentSpeedRight": 46,
  "currentGear": 2,
  "gpsFixed": true,
  "latitude": 40.7128,
  "longitude": -74.0060,
  "gpsSpeed": 12.5,
  "satellites": 8,
  "odometer": 125.8,
  "tripMeter": 12.5,
  "inSafeZone": true,
  "speedLimited": false,
  "currentSpeedLimit": 25,
  "keySwitch": true,
  "safetyFault": false,
  "uptime": 3600000
}
```

### Controls

#### POST `/api/emergency-stop`
Triggers immediate emergency stop.

#### POST `/api/set-gear`
```json
{
  "gear": "eco"  // Options: park, 1st, 2nd, 3rd, eco, sport
}
```

#### POST `/api/reset-trip`
Resets trip meter to zero.

#### POST `/api/set-home`
Sets current GPS location as home position.

#### POST `/api/calibration`
Activates OLED calibration test screen.

### System

#### GET `/api/version`
Returns firmware version information:

```json
{
  "version": "1.1.0",
  "build": 42,
  "commit": "abc123",
  "timestamp": "Nov 7 2025 10:30:00"
}
```

#### GET `/api/system`
Returns ESP32 system information:

```json
{
  "uptime": 3600000,
  "freeHeap": 150000,
  "chipModel": "ESP32-D0WD-V3",
  "cpuFreq": 240,
  "flashSize": 4194304
}
```

### Geofencing

#### GET `/api/geofences`
Returns list of configured geofences.

#### POST `/api/geofences`
Creates new geofence (implementation pending).

#### DELETE `/api/geofences`
Removes geofence (implementation pending).

## WebSocket Protocol

### Connection

```javascript
const ws = new WebSocket('ws://192.168.4.1/ws');
```

### Messages

**From Server:**

**Telemetry Update:**
```json
{
  "type": "telemetry",
  "data": { /* full telemetry object */ }
}
```

**Event Notification:**
```json
{
  "type": "event",
  "event": "emergency_stop",
  "data": { /* event-specific data */ }
}
```

**Events:**
- `emergency_stop`: Emergency stop activated
- `gear_change`: Gear changed
- `geofence_violation`: Geofence boundary crossed
- `low_battery`: Battery voltage warning

## File Structure

```
data/
├── index.html          # Main dashboard
├── login.html          # Login page
├── manifest.json       # PWA manifest
├── sw.js              # Service worker
├── css/
│   └── styles.css     # All styles
├── js/
│   └── dashboard.js   # Dashboard logic
└── images/            # App icons (add your own)
    ├── icon-192.png
    └── icon-512.png
```

## Customization

### Colors

Edit `data/css/styles.css`:

```css
:root {
    --primary: #4CAF50;      /* Main brand color */
    --danger: #F44336;       /* Emergency/errors */
    --warning: #FF9800;      /* Warnings */
    --success: #4CAF50;      /* Success states */
    --bg-dark: #1a1a1a;      /* Background */
    --bg-card: #2d2d2d;      /* Card background */
}
```

### Logo/Branding

Replace the "⚡ Razors Edge" text in:
- `data/index.html` (line 39)
- `data/login.html` (line 14)

### Add Custom Pages

1. Create HTML file in `data/` directory
2. Add route in `WebInterface.cpp`:

```cpp
server.on("/custom-page.html", HTTP_GET, [this]() {
    handleFileRead("/custom-page.html");
});
```

3. Upload filesystem: `pio run --target uploadfs`

## Integration with Existing Code

### In main.cpp

```cpp
#include "WebInterface.h"
#include "WebAuth.h"
#include "OTAManager.h"

void setup() {
    // ... existing setup ...

    // Initialize authentication
    WebAuth::getInstance().init();

    // Initialize web interface
    webInterface.init();

    // Enable OTA updates (optional)
    if (WiFi.isConnected()) {
        OTAManager::getInstance().init();
    }
}

void loop() {
    // ... existing loop ...

    // Update web interface
    webInterface.update();

    // Handle OTA
    OTAManager::getInstance().handle();
}
```

### Adding Authentication to Endpoints

```cpp
void WebInterface::handleYourEndpoint() {
    // Add this at the start of every protected endpoint
    if (!WebAuth::getInstance().checkAuth(server)) {
        return;  // 401 response sent automatically
    }

    // Your endpoint logic here...
}
```

### Broadcasting Events via WebSocket

```cpp
// When an event occurs
DynamicJsonDocument doc(256);
doc["type"] = "event";
doc["event"] = "emergency_stop";
doc["data"]["reason"] = "button_pressed";

String output;
serializeJson(doc, output);
webSocket.broadcastTXT(output);  // Send to all connected clients
```

## Troubleshooting

### Web Interface Not Loading

1. **Check SPIFFS:** Ensure files uploaded
   ```bash
   pio run --target uploadfs
   ```

2. **Check WiFi:** Connect to AP
   ```
   SSID: RazorsEdge-Controller
   Pass: RazorEdge2025
   ```

3. **Check Serial Output:**
   ```
   Web Interface: http://192.168.4.1
   ✓ Web Interface initialized
   ```

### Can't Login

1. **Check Default Credentials:**
   - Username: `admin`
   - Password: `RazorEdge2025`

2. **Clear Browser Data:** May have old session

3. **Check Serial:** Look for auth errors

### WebSocket Not Connecting

WebSocket support requires code integration (see `WebInterfaceNew.cpp`):
1. Add WebSocket library (already in platformio.ini)
2. Integrate WebSocket code
3. Rebuild and upload

**Fallback:** Dashboard automatically uses polling if WebSocket fails.

### Map Not Showing

1. **Check GPS Fix:** Satellites must be > 3
2. **Check Internet:** Leaflet.js loads from CDN
3. **Check Console:** Browser F12 → Console for errors

### Offline Mode Not Working

1. **HTTPS Required:** Service workers need HTTPS (or localhost)
2. **First Load:** Visit site once while online
3. **Check Registration:** Console should show "Service Worker installing..."

## Performance Tips

1. **Use WebSocket:** Faster than polling
2. **Limit Map Updates:** Only update when moved > 10m
3. **Debounce Inputs:** Don't spam API with rapid button clicks
4. **Cache Static Assets:** Service worker handles this

## Security Best Practices

1. **Change Default Password** immediately
2. **Use Strong Passwords**: Minimum 12 characters
3. **HTTPS**: Consider reverse proxy with SSL
4. **Session Timeout**: Adjust based on use case
5. **Update Regularly**: Keep firmware updated via OTA

## Mobile App Development

The web interface is PWA-ready and can be packaged as a native app:

### Using Capacitor (Ionic)

```bash
# Install Capacitor
npm install @capacitor/core @capacitor/cli

# Initialize
npx cap init

# Add platforms
npx cap add ios
npx cap add android

# Build
npm run build
npx cap sync
npx cap open ios
```

### Features for Native App

- Push notifications
- Background sync
- Native device access
- App store distribution

## Future Enhancements

Planned features:
- [ ] Settings page for configuration
- [ ] Historical data charts
- [ ] Export trip data
- [ ] Multiple user accounts
- [ ] Geofence drawing on map
- [ ] Video streaming (optional camera)
- [ ] Bluetooth integration
- [ ] Voice commands

## Support

For issues or questions:
1. Check console (F12 → Console)
2. Check serial output (115200 baud)
3. Review API responses
4. Check GitHub issues

---

**Version:** 1.1.0
**Last Updated:** November 2025
**License:** Educational Use

## Quick Reference

| Feature | URL | Auth Required |
|---------|-----|---------------|
| Login | `/login.html` | No |
| Dashboard | `/` | Yes |
| API Docs | This file | - |
| WebSocket | `ws://192.168.4.1/ws` | Yes |
| OTA Update | Network auto-discovery | Yes |

**Default Credentials:**
- Username: `admin`
- Password: `RazorEdge2025`

**WiFi AP:**
- SSID: `RazorsEdge-Controller`
- Password: `RazorEdge2025`
- IP: `192.168.4.1`
