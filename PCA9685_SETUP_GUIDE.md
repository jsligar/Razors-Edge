# PCA9685 Light Controller Setup Guide

## Overview

The PCA9685 is a 16-channel PWM driver used to control the vehicle's lighting system. It communicates with the ESP32 via I2C and provides independent brightness control for front and rear lights.

---

## Hardware Configuration

### PCA9685 Module Pins (6-pin version)

Your PCA9685 module has these 6 pins:

| Pin | Purpose | Connection |
|-----|---------|------------|
| VCC | Logic power (3.3V) | ESP32 3.3V |
| GND | Ground | ESP32 GND |
| SDA | I2C data line | ESP32 GPIO 21 |
| SCL | I2C clock line | ESP32 GPIO 22 |
| OE  | Output Enable (optional) | Connect to GND or leave open |
| V+  | External LED power (optional) | 5V or 12V (if needed) |

---

## Wiring Diagram

### Basic Connection (4 wires minimum):

```
PCA9685 Module          ESP32 DevKit V1
==============          ===============

VCC              ────→  3.3V (LEFT Pin 1) 🔴
GND              ────→  GND (LEFT Pin 14) ⚫
SDA              ────→  GPIO 21 (RIGHT Pin 6) 🔵
SCL              ────→  GPIO 22 (RIGHT Pin 3) 🔵
```

### Optional Connections:

**OE (Output Enable):**
- Connect to **GND** to permanently enable all outputs (recommended)
- Or leave disconnected (outputs usually enabled by default)

**V+ (External Power for LEDs):**
- Leave disconnected if using low-power 3.3V LEDs
- Connect to **5V or 12V** if your LEDs require more power
- **CRITICAL:** Must share common GND with ESP32!

---

## I2C Address Configuration

### Current Configuration:

**Address: 0x40** (default, no jumpers needed)

This is the factory default address for PCA9685 modules.

### Confirming the Address:

Most PCA9685 boards have solder pads labeled A0-A5 on the back:
- **All pads open (not bridged) = 0x40** ← Your current config
- If you need to change the address, bridge pads to modify

**Address calculation:**
```
Base address: 0x40
Final address = 0x40 + (A0×1 + A1×2 + A2×4 + A3×8 + A4×16 + A5×32)

Examples:
All open:      0x40 (current)
A0 bridged:    0x41
A5+A4 bridged: 0x70 (previous config)
```

---

## I2C Bus Shared Devices

The PCA9685 shares the I2C bus (GPIO 21/22) with these devices:

| Address | Device | Purpose |
|---------|--------|---------|
| 0x3C | SSD1306 | OLED display |
| 0x40 | **PCA9685** | **16-channel PWM driver (lights)** |
| 0x41 | INA228 | Left motor current monitor |
| 0x44 | INA228 | Right motor current monitor |
| 0x45 | INA228 | Battery monitor (MATEKSYS INA-BM) |

**All devices connect to the same SDA/SCL lines!**

---

## Light Channel Assignments

The PCA9685 has 16 channels (0-15). Your system uses channels **12, 13, 14, 15** for lighting:

| Channel | Function | Config Name |
|---------|----------|-------------|
| 12 | Front Left Light | `LIGHT_FRONT_LEFT` |
| 13 | Front Right Light | `LIGHT_FRONT_RIGHT` |
| 14 | Rear Left Light | `LIGHT_REAR_LEFT` |
| 15 | Rear Right Light | `LIGHT_REAR_RIGHT` |
| 0-11 | Available for expansion | - |

### Why Channels 12-15?

This allows channels 0-11 to be used for other PWM devices (servos, additional lights, fans, etc.) while keeping the main lighting on dedicated channels.

---

## LED Wiring to PCA9685

### PWM Output Connections:

```
PCA9685 Outputs              LED Connections
===============              ===============

Channel 12 (PWM12) ────→ Front Left LED (+) or MOSFET gate
Channel 13 (PWM13) ────→ Front Right LED (+) or MOSFET gate
Channel 14 (PWM14) ────→ Rear Left LED (+) or MOSFET gate
Channel 15 (PWM15) ────→ Rear Right LED (+) or MOSFET gate

All LED cathodes (-) ────→ GND (common ground)
```

### For Low-Power LEDs (< 25mA):

Connect directly:
```
PCA9685 PWM12 ──[ 330Ω resistor ]──→ LED (+)
LED (-) ──→ GND
```

### For High-Power LEDs (> 25mA):

Use MOSFET driver:
```
PCA9685 PWM12 ──→ MOSFET Gate (e.g., IRLZ44N)
LED (+) ──→ V+ (5V or 12V)
LED (-) ──→ MOSFET Drain
MOSFET Source ──→ GND
```

---

## Software Configuration

### In `src/config.h`:

```cpp
// I2C pins
#define GPIO_SDA                21
#define GPIO_SCL                22

// PCA9685 I2C address
#define PCA9685_ADDR            0x40    // Default address

// Light channel assignments
#define LIGHT_FRONT_LEFT        12
#define LIGHT_FRONT_RIGHT       13
#define LIGHT_REAR_LEFT         14
#define LIGHT_REAR_RIGHT        15
```

### PWM Frequency:

The PCA9685 is configured for **1000 Hz** (1 kHz) for LED control:
```cpp
pwm.setPWMFreq(1000);  // 1kHz for LEDs
```

- Lower frequencies (100-200 Hz) may cause visible flickering
- Higher frequencies (1-1.6 kHz) are recommended for LEDs

### Brightness Control:

PCA9685 uses 12-bit PWM (0-4095):
```cpp
pwm.setPWM(channel, 0, brightness);

// Examples:
pwm.setPWM(12, 0, 0);      // Front left OFF
pwm.setPWM(12, 0, 2048);   // Front left 50%
pwm.setPWM(12, 0, 4095);   // Front left 100%
```

---

## Testing the PCA9685

### Quick I2C Scanner Test:

Upload and run the I2C scanner to confirm the PCA9685 is detected:

```bash
pio run -e i2c_scanner -t upload -t monitor
```

**Expected output:**
```
I2C Scanner
Scanning...
Found device at 0x3C (SSD1306 display)
Found device at 0x40 (PCA9685 PWM driver) ← Should see this!
Found device at 0x41 (INA228 motor left)
Found device at 0x44 (INA228 motor right)
Found device at 0x45 (INA228 battery)
```

### PCA9685 Light Test:

A dedicated test environment has been created to verify the PCA9685 and light channels:

```bash
pio run -e light_test -t upload -t monitor
```

**What the test does:**
1. Initializes the PCA9685 at address 0x40
2. Sets PWM frequency to 1 kHz
3. Cycles through channels 12, 13, 14, 15
4. Fades each light from 0% → 100% → 0%
5. Displays status in serial monitor

**Expected serial output:**
```
=== PCA9685 Light Test ===
PCA9685 initialized
Testing channels 12, 13, 14, 15
Cycling from low to high...

Channel 12 - Front Left (12): Fade UP... Fade DOWN... OFF
Channel 13 - Front Right (13): Fade UP... Fade DOWN... OFF
Channel 14 - Rear Left (14): Fade UP... Fade DOWN... OFF
Channel 15 - Rear Right (15): Fade UP... Fade DOWN... OFF

--- All channels tested! ---
Starting next cycle in 2 seconds...
```

**What to observe:**
- Each LED should smoothly fade on, then fade off
- No flickering during the fade
- All 4 channels should work independently

---

## Troubleshooting

### Problem: PCA9685 Not Detected (I2C Scanner shows nothing at 0x40)

**Possible causes:**
1. **Wiring error** - Check VCC, GND, SDA, SCL connections
2. **Wrong I2C pins** - Verify SDA=GPIO21, SCL=GPIO22
3. **Bad solder joints** - Inspect header pins on PCA9685 module
4. **Wrong address** - Try scanning all addresses (0x40-0x7F)

**Solutions:**
- Re-check all 4 wire connections
- Verify 3.3V is present on VCC pin (use multimeter)
- Try a different PCA9685 module (could be defective)

### Problem: PCA9685 Detected, But No LED Output

**Possible causes:**
1. **OE pin not enabled** - Output Enable may be floating
2. **LEDs not connected** - No load on PWM channels
3. **Wrong channels** - Using 0-3 instead of 12-15
4. **Insufficient V+ power** - High-power LEDs need external power

**Solutions:**
- Connect OE pin to GND to enable outputs
- Verify LED wiring to channels 12, 13, 14, 15
- Check V+ voltage if using external LED power supply
- Test with a multimeter: should see 0-3.3V PWM on channel pins

### Problem: LEDs Flicker or Flash Erratically

**Possible causes:**
1. **Insufficient power** - V+ voltage drooping under load
2. **Ground loops** - Multiple ground paths
3. **Low PWM frequency** - < 100 Hz visible to human eye

**Solutions:**
- Add bulk capacitor (100-1000µF) across V+ and GND
- Ensure single, solid ground connection
- Verify PWM frequency is 1000 Hz: `pwm.setPWMFreq(1000);`

### Problem: Only Some Channels Work

**Possible causes:**
1. **LED wired to wrong channels** - Check channel 12-15 specifically
2. **Damaged PCA9685** - Some channels may be burned out
3. **Software targeting wrong channels** - Code using 0-3 instead of 12-15

**Solutions:**
- Verify physical wiring: channel 12 = pin labeled "12" on module
- Run light_test and observe which channels actually work
- Check `src/config.h` confirms channels 12, 13, 14, 15

### Problem: I2C Address Conflict

**Symptom:** Random behavior, some devices not working

**Cause:** Two devices with same address on I2C bus

**Solution:**
- Check all I2C device addresses:
  - 0x3C: Display
  - 0x40: PCA9685 (you might need to change this if conflict)
  - 0x41: Motor sensor left
  - 0x44: Motor sensor right
  - 0x45: Battery sensor
- If conflict, bridge address pads on PCA9685 to change (e.g., to 0x41)
- Update `config.h` with new address

---

## Advanced: Multiple PCA9685 Boards

If you need more than 16 channels, you can connect up to 62 PCA9685 boards on the same I2C bus by changing their addresses.

**Example configuration:**
- Board 1: Address 0x40 (A0-A5 all open) - Channels 0-15
- Board 2: Address 0x41 (A0 bridged) - Channels 0-15
- Board 3: Address 0x42 (A1 bridged) - Channels 0-15

Each board needs:
- Its own VCC, GND, SDA, SCL connections
- Unique I2C address (via solder jumpers)
- Optional separate V+ power if driving many LEDs

---

## Physical Installation Tips

### Mounting:
- Use standoffs to mount PCA9685 away from metal chassis
- Keep module dry (no exposure to rain/moisture)
- Ensure good ventilation if driving high-current loads

### Wire Gauge:
- **I2C wires (VCC, GND, SDA, SCL):** 22-26 AWG is fine
- **V+ and LED power:** Size based on total LED current
  - < 1A: 22 AWG
  - 1-3A: 18 AWG
  - > 3A: 16 AWG or larger

### Cable Length:
- **I2C maximum length:** ~1 meter (3 feet) for reliable communication
- If longer distances needed, use I2C extender/repeater
- Keep I2C wires twisted together to reduce noise

---

## Quick Reference Card

**Copy this for easy reference:**

```
┌─────────────────────────────────────────┐
│   PCA9685 Quick Reference               │
├─────────────────────────────────────────┤
│ I2C Address:     0x40                   │
│ I2C Pins:        SDA=GPIO21, SCL=GPIO22 │
│                                          │
│ Light Channels:                         │
│   12 = Front Left                       │
│   13 = Front Right                      │
│   14 = Rear Left                        │
│   15 = Rear Right                       │
│                                          │
│ PWM Frequency:   1000 Hz                │
│ PWM Range:       0-4095 (12-bit)        │
│                                          │
│ Test Command:                           │
│   pio run -e light_test -t upload       │
└─────────────────────────────────────────┘
```

---

## Related Documentation

- **Complete pin assignments:** `ESP32_WIRING_GUIDE.md`
- **I2C device overview:** `src/config.h` (lines 35-41)
- **Test code:** `test_pca9685_lights.cpp`
- **Build instructions:** `BUILD_INSTRUCTIONS.md`

---

**Happy wiring! 💡🚗⚡**
