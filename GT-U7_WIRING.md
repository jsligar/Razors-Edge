# GT-U7 GPS Quick Wiring Guide

## Pin Connections (Simple Version)

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
GT-U7 Module:
┌─────────────────────┐
│   [GPS ANTENNA]     │
│                     │
│  VCC RX TX GND PPS  │ ← pins
└─────────────────────┘

ESP32 DevKit:
                3.3V ← VCC (GT-U7 Pin 1)

          GPIO 17/TX ← RX  (GT-U7 Pin 2)
          GPIO 16/RX ← TX  (GT-U7 Pin 3)

                 GND ← GND (GT-U7 Pin 4)

                     ← PPS (GT-U7 Pin 5) - leave disconnected
```

## Important Notes

1. **RX/TX are CROSSED**
   - GT-U7 RX → ESP32 GPIO 17 (ESP32 TX)
   - GT-U7 TX → ESP32 GPIO 16 (ESP32 RX)

2. **Voltage**: Use 3.3V from ESP32 (GT-U7 accepts 3.3V-5V)

3. **PPS Pin**: Leave disconnected - not needed for basic operation

4. **LED Indicator on GT-U7**:
   - Blinking slowly = Searching for satellites
   - Blinking fast = GPS fix acquired
   - No light = No power or module fault

## Testing Steps

### Step 1: Connect Power First
Connect only VCC and GND, then check:
- LED on GT-U7 should blink
- If no LED, check voltage at VCC pin (should be 3.3V)

### Step 2: Add Serial Connections
Connect TX and RX pins:
- GT-U7 TX → ESP32 GPIO 16
- GT-U7 RX → ESP32 GPIO 17

### Step 3: Upload Firmware
Upload the Razors Edge firmware to ESP32

### Step 4: Monitor Serial Output
Open serial monitor at 115200 baud and look for:
```
GPS: Sats=0 Fix=0 Lat=0.000000 Lng=0.000000 Speed=0.0 mph HDOP=99.9
```

This appears every 5 seconds once the system is running.

### Step 5: Wait for Fix
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
