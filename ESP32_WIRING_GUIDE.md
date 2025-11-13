# ESP32 Complete Wiring Guide for Razors Edge Controller

## Overview

This guide shows all ESP32 GPIO pin connections for the Razors Edge electric vehicle controller.

---

## 🚨 Critical: ESP32 ADC2 and WiFi Conflict

**IMPORTANT**: On ESP32, ADC2 pins (GPIO 0, 2, 4, 12-15, 25-27) **CANNOT** be read while WiFi is active. This is a hardware limitation, not a software bug.

### What This Means:
- **Throttle pedal MUST use ADC1** (GPIO 32-39) to work with WiFi enabled
- ADC1 pins work regardless of WiFi status
- This project uses GPIO 36 for throttle to ensure WiFi compatibility

### ADC1 vs ADC2 Pins:

**ADC1 Channels (Safe for WiFi):**
- GPIO 36 (ADC1_CH0) ✅ **THROTTLE PEDAL** - Input only
- GPIO 39 (ADC1_CH3) - Input only
- GPIO 34 (ADC1_CH6) - Input only
- GPIO 35 (ADC1_CH7) - Input only
- GPIO 32 (ADC1_CH4)
- GPIO 33 (ADC1_CH5)

**ADC2 Channels (WiFi CONFLICT):**
- GPIO 4 (ADC2_CH0) ❌ Used for motor direction
- GPIO 0 (ADC2_CH1) ❌ Boot pin
- GPIO 2 (ADC2_CH2) ❌ Boot pin
- GPIO 15 (ADC2_CH3) ✅ Used for key switch (digital input only)
- GPIO 13 (ADC2_CH4) ✅ Used for shift down button
- GPIO 12 (ADC2_CH5) ✅ Used for motor PWM
- GPIO 14 (ADC2_CH6) ✅ Used for shift up button
- GPIO 27 (ADC2_CH7) ✅ Used for encoder button
- GPIO 25 (ADC2_CH8) ✅ Used for encoder CLK
- GPIO 26 (ADC2_CH9) ✅ Used for encoder DT

**Note**: Digital inputs (buttons, switches) on ADC2 pins work fine with WiFi. Only analog reads fail.

---

## ESP32 DevKit V1 Full Pinout

```
                    ESP32 DevKit V1
                    ===============
                         USB
                       ┌─────┐
         Left Side     │     │     Right Side
         =========     └─────┘     ==========

    3V3  ●  ○ 1 ───────────────  1 ○  ● GND
    EN   ●  ○ 2                  2 ○  ● GPIO 23  (Motor DIR Right)
 GPIO 36 ●  ○ 3 ← THROTTLE      3 ○  ● GPIO 22  (I2C SCL)
 GPIO 39 ●  ○ 4                  4 ○  ● GPIO 1  (TX0 - USB)
 GPIO 34 ●  ○ 5                  5 ○  ● GPIO 3  (RX0 - USB)
 GPIO 35 ●  ○ 6                  6 ○  ● GPIO 21 (I2C SDA)
 GPIO 32 ●  ○ 7                  7 ○  ● GND
 GPIO 33 ●  ○ 8                  8 ○  ● GPIO 19 (Motor DIR Left)
 GPIO 25 ●  ○ 9 ← ENC CLK        9 ○  ● GPIO 18 (Motor PWM Left)
 GPIO 26 ●  ○ 10 ← ENC DT       10 ○  ● GPIO 5
 GPIO 27 ●  ○ 11 ← ENC BTN      11 ○  ● GPIO 17 (GPS RX from module)
 GPIO 14 ●  ○ 12 ← KEY0 (UP)    12 ○  ● GPIO 16 (GPS TX to module)
 GPIO 12 ●  ○ 13 ← MOTOR PWM R  13 ○  ● GPIO 4  (Motor DIR Right)
    GND  ●  ○ 14                14 ○  ● GPIO 0
 GPIO 13 ●  ○ 15 ← KEY1 (DOWN)  15 ○  ● GPIO 2

```

---

## Pin Assignment Summary

### Power
- **3.3V (Pin 1 left)** → GT-U7 GPS VCC, pull-ups, 3.3V devices
- **GND (Pins 1 right, 7 right, 14 left)** → All ground connections

### Throttle Pedal (CRITICAL - ADC1 Required)
- **GPIO 36 (LEFT Pin 3)** → Throttle pedal analog input
  - **ADC1_CH0** - Works with WiFi ✅
  - Input-only pin (safe, cannot be configured as output)
  - Connect via voltage divider: 0-3.3V range
  - Previously GPIO 4 (ADC2) - moved for WiFi compatibility

### GPS Module (GT-U7)
- **GPIO 16 (RIGHT Pin 12)** → GPS TX (ESP32 receives data)
- **GPIO 17 (RIGHT Pin 11)** → GPS RX (ESP32 sends data)
- **3.3V (LEFT Pin 1)** → GPS VCC
- **GND** → GPS GND

### Motor Control
- **GPIO 18 (RIGHT Pin 9)** → Left motor PWM
- **GPIO 12 (LEFT Pin 13)** → Right motor PWM
- **GPIO 19 (RIGHT Pin 8)** → Left motor direction
- **GPIO 23 (RIGHT Pin 2)** → Right motor direction (also used for ADC2_CH3, but only as digital output)

### I2C Bus (Sensors, Display, PWM Driver)
- **GPIO 21 (RIGHT Pin 6)** → I2C SDA
- **GPIO 22 (RIGHT Pin 3)** → I2C SCL
  - OLED Display (SSD1306)
  - Battery Monitor (INA228)
  - Motor Current Monitors (INA228 x2)
  - PWM Driver (PCA9685) for lights

### User Interface
- **GPIO 25 (LEFT Pin 9)** → Rotary encoder CLK (track A)
- **GPIO 26 (LEFT Pin 10)** → Rotary encoder DT (track B)
- **GPIO 27 (LEFT Pin 11)** → Rotary encoder button
- **GPIO 14 (LEFT Pin 12)** → KEY0 - Confirm/Shift Up button
- **GPIO 13 (LEFT Pin 15)** → KEY1 - Back/Shift Down button

### Key Switch
- **GPIO 15 (special - not on all boards)** → Key switch input
  - May need to wire to an alternate pin if GPIO 15 unavailable

---

## Detailed Connection Diagrams

### Throttle Pedal Wiring (GPIO 36)

```
Throttle Pedal Potentiometer:
┌────────────────────┐
│   5V / 3.3V        │ Pin 1 (VCC) ─────→ 5V or 3.3V (depends on pedal)
│   Signal           │ Pin 2 ──┬─────→ GPIO 36 (ESP32 LEFT Pin 3)
│   GND              │         │
└────────────────────┘         │
                               │
                              [ ] 10kΩ to GND (optional pull-down)
                               │
                              GND

If pedal outputs 0-5V, use voltage divider:

Pedal Signal ──[ 10kΩ ]──┬──→ GPIO 36
                          │
                      [ 10kΩ ]
                          │
                         GND

This divides voltage by 2: 0-5V becomes 0-2.5V (safe for ESP32)
```

**Important Notes:**
- ESP32 ADC input max: **3.3V** (do NOT exceed or damage will occur)
- GPIO 36 is **input-only** (cannot accidentally set as output)
- ADC1 channel - works with WiFi active
- Calibration range in `config.h`:
  ```cpp
  #define PEDAL_ADC_MIN    100    // Pedal released
  #define PEDAL_ADC_MAX    4000   // Pedal fully pressed
  ```

### GPS Module Wiring (GT-U7)

```
GT-U7 GPS Module (5-pin header):
┌─────────────────────────┐
│  Pin 1: VCC   (red)     │ ──────→ ESP32 3.3V (LEFT Pin 1)
│  Pin 2: RX    (yellow)  │ ──────→ ESP32 GPIO 17 (RIGHT Pin 11) TX from ESP32
│  Pin 3: TX    (green)   │ ──────→ ESP32 GPIO 16 (RIGHT Pin 12) RX to ESP32
│  Pin 4: GND   (black)   │ ──────→ ESP32 GND
│  Pin 5: PPS   (unused)  │ ──────  Not connected
└─────────────────────────┘
```

**Critical:**
- GT-U7 **USB must be DISCONNECTED** when wiring to ESP32
- TX/RX are **crossed**: GPS TX → ESP32 RX (GPIO 16), GPS RX → ESP32 TX (GPIO 17)
- Use 3.3V power from ESP32 (GT-U7 accepts 3.3V-5V)

### Motor Driver Connections

```
Motor Controller Inputs:
┌──────────────────────┐
│  Left Motor:         │
│    PWM    ←──────────┼─── GPIO 18 (RIGHT Pin 9)
│    DIR    ←──────────┼─── GPIO 19 (RIGHT Pin 8)
│                      │
│  Right Motor:        │
│    PWM    ←──────────┼─── GPIO 12 (LEFT Pin 13)
│    DIR    ←──────────┼─── GPIO 23 (RIGHT Pin 2)
└──────────────────────┘
```

---

## Breadboard Wiring Example

```
ESP32 DevKit V1              Breadboard Connections
===============              ====================

LEFT SIDE:
  Pin 1  (3.3V)  ────────→ [Red rail +3.3V]
  Pin 3  (GPIO36)────────→ [Throttle signal wire]
  Pin 9  (GPIO25)────────→ [Encoder CLK]
  Pin 10 (GPIO26)────────→ [Encoder DT]
  Pin 11 (GPIO27)────────→ [Encoder BTN]
  Pin 12 (GPIO14)────────→ [Button: Shift UP]
  Pin 13 (GPIO12)────────→ [Motor R PWM]
  Pin 14 (GND)   ────────→ [Black rail GND]
  Pin 15 (GPIO13)────────→ [Button: Shift DOWN]

RIGHT SIDE:
  Pin 1  (GND)   ────────→ [Black rail GND]
  Pin 2  (GPIO23)────────→ [Motor R DIR]
  Pin 3  (GPIO22)────────→ [I2C SCL - all I2C devices]
  Pin 6  (GPIO21)────────→ [I2C SDA - all I2C devices]
  Pin 7  (GND)   ────────→ [Black rail GND]
  Pin 8  (GPIO19)────────→ [Motor L DIR]
  Pin 9  (GPIO18)────────→ [Motor L PWM]
  Pin 11 (GPIO17)────────→ [GPS RX wire (yellow)]
  Pin 12 (GPIO16)────────→ [GPS TX wire (green)]
```

---

## Wire Color Convention (Suggested)

| Color | Purpose |
|-------|---------|
| 🔴 Red | 3.3V power |
| ⚫ Black | Ground (GND) |
| 🟡 Yellow | Signal wires (GPS RX, encoder, buttons) |
| 🟢 Green | Signal wires (GPS TX, motor control) |
| 🔵 Blue | I2C (SDA/SCL) |
| 🟠 Orange | PWM signals |
| 🟣 Purple | Analog inputs (throttle) |

---

## Common Mistakes & How to Avoid Them

### ❌ Mistake 1: Using ADC2 for Throttle
**Problem:** Throttle pedal on GPIO 4 (ADC2) doesn't work with WiFi enabled.

**Error message:**
```
[E][esp32-hal-adc.c:170] __analogReadRaw(): GPIO4: ESP_ERR_TIMEOUT: ADC2 is in use by Wi-Fi
```

**Solution:** Use GPIO 36 (ADC1_CH0) as configured in latest firmware.

### ❌ Mistake 2: GPS TX/RX Not Crossed
**Problem:** GPS connected TX-to-TX, RX-to-RX.

**Symptom:** No GPS data in serial monitor, `Sats=0` always.

**Solution:**
- GT-U7 TX → ESP32 GPIO 16 (RX)
- GT-U7 RX → ESP32 GPIO 17 (TX)

### ❌ Mistake 3: Using GPIO 1/3 for GPS
**Problem:** GPS connected to GPIO 1 (TX0) / GPIO 3 (RX0).

**Symptom:** GPS data appears in serial monitor mixed with debug output, system unstable.

**Solution:** Use GPIO 16/17 (UART2), NOT GPIO 1/3 (UART0 - used for USB).

### ❌ Mistake 4: Exceeding 3.3V on ADC
**Problem:** 5V throttle pedal connected directly to GPIO 36.

**Symptom:** ESP32 damage, erratic readings, system crashes.

**Solution:** Use voltage divider to reduce 5V to 0-2.5V or 0-3.3V max.

### ❌ Mistake 5: GPS USB Connected While Wired to ESP32
**Problem:** Both USB and 5-pin header connected simultaneously.

**Symptom:** Potential damage to GPS module or ESP32.

**Solution:** Use EITHER USB (testing) OR 5-pin header (final installation), never both.

---

## Verification Checklist

Before powering on, verify:

### Power Connections
- [ ] 3.3V from ESP32 to GPS VCC
- [ ] All GND connections secure
- [ ] No short circuits between 3.3V and GND

### Throttle Pedal
- [ ] Connected to **GPIO 36** (LEFT Pin 3)
- [ ] Voltage range is 0-3.3V max (use divider if needed)
- [ ] Pull-down resistor installed if needed

### GPS Module
- [ ] VCC → 3.3V
- [ ] TX → GPIO 16 (ESP32 RX)
- [ ] RX → GPIO 17 (ESP32 TX)
- [ ] GND → GND
- [ ] USB cable **DISCONNECTED**

### User Interface
- [ ] Encoder CLK → GPIO 25
- [ ] Encoder DT → GPIO 26
- [ ] Encoder BTN → GPIO 27
- [ ] KEY0 (Shift Up) → GPIO 14
- [ ] KEY1 (Shift Down) → GPIO 13

### Motors
- [ ] Left PWM → GPIO 18
- [ ] Left DIR → GPIO 19
- [ ] Right PWM → GPIO 12
- [ ] Right DIR → GPIO 23

### I2C Devices
- [ ] SDA → GPIO 21 (all devices)
- [ ] SCL → GPIO 22 (all devices)
- [ ] Each device has unique I2C address

---

## Testing After Wiring

### Test 1: Power-On Check
1. Connect ESP32 to USB
2. Open serial monitor (115200 baud)
3. Look for startup message:
   ```
   =================================
       RAZOR 60V CONTROLLER v1.1
       ESP32 Electric Vehicle
   =================================
   ```
4. Verify all modules initialize:
   ```
   ✓ Input Handler initialized
   ✓ Power Manager initialized
   ✓ Safety System initialized
   ✓ Motor Controller initialized
   ✓ Navigation initialized
   ```

### Test 2: Throttle Pedal
1. Open serial monitor
2. Press throttle pedal slowly
3. Look for throttle reading to increase:
   ```
   Throttle: 0% → 25% → 50% → 100%
   ```
4. Should work even with WiFi enabled (GPIO 36 is ADC1)

### Test 3: GPS Reception
1. Take ESP32 + GPS module outside
2. Wait 30-90 seconds for satellite fix
3. Serial monitor should show:
   ```
   GPS: Sats=0 Fix=0 ...  (searching)
   GPS: Sats=4 Fix=1 ...  (fix acquired)
   GPS: Sats=6 Fix=1 Lat=37.826744 Lng=-92.133801 Speed=0.0 mph HDOP=1.2
   ```

### Test 4: Buttons & Encoder
1. Press encoder button → should cycle OLED screens
2. Press KEY0 (GPIO 14) → shift up gear
3. Press KEY1 (GPIO 13) → shift down gear
4. Rotate encoder → adjust settings

### Test 5: WiFi & Web Interface
1. Connect to WiFi network: `Razors-Edge-XXXX`
2. Password: `razor123`
3. Open browser: `http://razors-edge.local` or `http://192.168.4.1`
4. Verify web interface loads
5. **Verify throttle still works while WiFi active** (GPIO 36 fix)

---

## Advanced: Pin Conflicts & Constraints

### Boot-Time Pin States
Some pins have constraints during ESP32 boot:

- **GPIO 0**: Must be HIGH during boot (pulled up) - Used for motor direction (OK)
- **GPIO 2**: Must be LOW during boot (for normal boot) - Currently unused
- **GPIO 5**: Strapping pin - Currently unused
- **GPIO 12**: Strapping pin, LOW during boot - Used for motor PWM (OK)
- **GPIO 15**: Strapping pin, HIGH during boot - Used for key switch (requires pull-up)

### Input-Only Pins
Cannot be used as outputs:
- **GPIO 36** (Throttle - perfect for ADC input)
- **GPIO 39**
- **GPIO 34**
- **GPIO 35**

### Flash Pins (DO NOT USE)
Will crash ESP32:
- GPIO 6, 7, 8, 9, 10, 11 - Used for SPI flash

---

## Troubleshooting

### Problem: "ADC2 is in use by Wi-Fi" Error

**Symptom:**
```
[E][esp32-hal-adc.c:170] __analogReadRaw(): GPIO4: ESP_ERR_TIMEOUT: ADC2 is in use by Wi-Fi
```

**Cause:** Throttle pedal on GPIO 4 (ADC2 channel).

**Solution:**
1. Verify `src/config.h` shows:
   ```cpp
   #define GPIO_PEDAL_ADC  36  // GPIO36 (ADC1_CH0)
   ```
2. Move throttle wire from GPIO 4 to GPIO 36 (LEFT Pin 3)
3. Re-upload firmware

### Problem: No GPS Data

**Symptom:** `GPS: Sats=0 Fix=0` always, no change.

**Solutions:**
1. Check wiring: TX→16, RX→17 (crossed)
2. Disconnect GPS USB cable
3. Take module outside (GPS doesn't work indoors)
4. Wait 30-90 seconds for cold start
5. See `GT-U7_DIAGNOSTICS.md` for detailed troubleshooting

### Problem: Encoder Doesn't Respond

**Symptom:** Rotating encoder or pressing button does nothing.

**Solutions:**
1. Check wiring: CLK→25, DT→26, BTN→27
2. Verify pull-up resistors on encoder (usually built-in)
3. See `BUTTON_ENCODER_TROUBLESHOOTING.md`

### Problem: I2C Devices Not Detected

**Symptom:** Display blank, power readings show 0.

**Solutions:**
1. Check SDA/SCL connections (GPIO 21/22)
2. Verify each device has unique I2C address
3. Run I2C scanner sketch to detect addresses
4. Check 3.3V power to all I2C devices

---

## For More Information

See these additional guides:
- **GPS**: `GT-U7_WIRING.md`, `GT-U7_DIAGNOSTICS.md`, `GPS_TROUBLESHOOTING.md`
- **Physical Pins**: `ESP32_PIN_LOCATIONS.md`
- **Display & Buttons**: `DASHBOARD_WIRING.md`, `BUTTON_ENCODER_TROUBLESHOOTING.md`
- **Build & Upload**: `BUILD_INSTRUCTIONS.md`
- **WiFi Setup**: `WIFI_QUICKSTART.md`, `WIFI_GUIDE.md`

---

## Quick Reference: All GPIO Assignments

| GPIO | Function | Type | ADC | Notes |
|------|----------|------|-----|-------|
| 36 | **Throttle Pedal** | Analog In | **ADC1_CH0** | ✅ Works with WiFi |
| 16 | GPS TX | UART RX | - | Receives from GPS |
| 17 | GPS RX | UART TX | - | Sends to GPS |
| 18 | Motor L PWM | PWM Out | - | |
| 19 | Motor L DIR | Digital Out | - | |
| 12 | Motor R PWM | PWM Out | ADC2_CH5 | Digital only |
| 23 | Motor R DIR | Digital Out | - | |
| 21 | I2C SDA | I2C | - | All I2C devices |
| 22 | I2C SCL | I2C | - | All I2C devices |
| 25 | Encoder CLK | Digital In | ADC2_CH8 | Digital only |
| 26 | Encoder DT | Digital In | ADC2_CH9 | Digital only |
| 27 | Encoder BTN | Digital In | ADC2_CH7 | Digital only |
| 14 | KEY0 Shift Up | Digital In | ADC2_CH6 | Digital only |
| 13 | KEY1 Shift Down | Digital In | ADC2_CH4 | Digital only |
| 15 | Key Switch | Digital In | ADC2_CH3 | Strapping pin |

---

**Good luck with your wiring! 🔌🛰️⚡**
