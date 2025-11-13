# Dashboard Hardware Test Guide

## Quick Start

This test program verifies your Nerdbilly Fab dashboard hardware:
- ✅ OLED Display (SSD1306)
- ✅ Confirm Button (Button A - GPIO 14)
- ✅ Back Button (Button B - GPIO 13)
- ✅ EC11 Rotary Encoder with button

## What the Test Does

### 1. Splash Screen (3 seconds)
Shows "NERDBILLY FAB" splash screen to verify OLED is working.

### 2. Interactive Test Screen
- **Encoder Counter**: Turn encoder clockwise/counter-clockwise to change number
- **CONFIRM Button**: Shows "PRESSED" when Button A is pressed
- **BACK Button**: Shows "PRESSED" when Button B is pressed
- **ENC BTN**: Press encoder button to reset counter to 0

### 3. Serial Monitor Output
All button presses and encoder turns are logged to Serial Monitor (115200 baud).

## How to Run the Test

### Option 1: Temporary Test (Recommended)

1. **Backup your main.cpp:**
   ```bash
   mv src/main.cpp src/main.cpp.backup
   ```

2. **Use test file as main.cpp:**
   ```bash
   cp test_dashboard_hardware.cpp src/main.cpp
   ```

3. **Upload and test:**
   ```bash
   pio run -t upload
   pio device monitor
   ```

4. **Restore when done:**
   ```bash
   mv src/main.cpp.backup src/main.cpp
   ```

### Option 2: Create Test Environment

Add this to `platformio.ini`:

```ini
[env:dashboard_test]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.5
build_src_filter =
    +<../test_dashboard_hardware.cpp>
    -<*>
```

Then run:
```bash
pio run -e dashboard_test -t upload
pio device monitor -e dashboard_test
```

## Expected Behavior

### Power On
```
=================================
  NERDBILLY FAB DASHBOARD TEST
=================================

✓ OLED Display initialized
✓ Buttons and Encoder initialized

=== Hardware Test Ready ===
- Turn encoder to change counter
- Press encoder button
- Press CONFIRM button (Button A)
- Press BACK button (Button B)
```

### OLED Display Shows
```
┌──────────────────────────┐
│ NERDBILLY FAB            │  ← Splash (3 sec)
│ ──────────────────────   │
│ Count: 0                 │  ← Large counter
│                          │
│ CONFIRM: ---             │  ← Button status
│ BACK:    ---             │
│ ENC BTN: ---             │
└──────────────────────────┘
```

### When You Interact

**Turn Encoder Clockwise:**
- Counter increases: 0 → 1 → 2 → 3...
- Serial: "Encoder CW: 1"

**Turn Encoder Counter-Clockwise:**
- Counter decreases: 3 → 2 → 1 → 0 → -1...
- Serial: "Encoder CCW: -1"

**Press Confirm Button:**
- Display shows: "CONFIRM: PRESSED"
- Serial: "✓ CONFIRM button pressed!"

**Press Back Button:**
- Display shows: "BACK: PRESSED"
- Serial: "✓ BACK button pressed!"

**Press Encoder Button:**
- Counter resets to 0
- Display shows: "ENC BTN: PRESSED"
- Serial: "✓ ENCODER button pressed! Counter reset."

## Troubleshooting

### OLED Shows Nothing
- ✓ Check I2C wiring (SDA=GPIO21, SCL=GPIO22)
- ✓ Verify 3.3V power connected
- ✓ Try I2C scanner to verify address 0x3C
- ✓ Check if display needs contrast adjustment

### OLED Shows "SSD1306 allocation failed!"
- ✓ I2C not connected properly
- ✓ Wrong I2C address (try 0x3D)
- ✓ Power issue

### Buttons Don't Work
- ✓ Check GPIO 14 and GPIO 13 connections
- ✓ Verify buttons connect pin to GND when pressed
- ✓ Check serial monitor for button events
- ✓ Ensure 3.3V power to dashboard module

### Encoder Doesn't Count
- ✓ Check GPIO 25 and GPIO 26 for encoder tracks
- ✓ Verify encoder common/ground connected
- ✓ Try swapping TRA/TRB if counting backwards
- ✓ Check if encoder is mechanical vs optical

### Encoder Counts Wrong Direction
- Swap the encoder pins in code:
  ```cpp
  #define ENCODER_CLK         26  // Swap these
  #define ENCODER_DT          25  // two lines
  ```

### Encoder Button Doesn't Work
- ✓ Check GPIO 27 connection
- ✓ Verify encoder button has separate pins from rotation
- ✓ Check if button is normally-open

## Pin Reference

| Dashboard Pin | Function | ESP32 GPIO | Type |
|--------------|----------|------------|------|
| Pin 1 | Confirm Button | GPIO 14 | INPUT_PULLUP |
| Pin 2 | OLED SDA | GPIO 21 | I2C Data |
| Pin 3 | OLED SCL | GPIO 22 | I2C Clock |
| Pin 4 | Encoder Button | GPIO 27 | INPUT_PULLUP |
| Pin 5 | Encoder TRA | GPIO 25 | INPUT_PULLUP |
| Pin 6 | Encoder TRB | GPIO 26 | INPUT_PULLUP |
| Pin 7 | Back Button | GPIO 13 | INPUT_PULLUP |
| Pin 8 | GND | GND | Ground |
| Pin 9 | 3V3 | 3.3V | Power |

## Hardware Checklist

Before testing:
- [ ] All wires connected per DASHBOARD_WIRING.md
- [ ] 3.3V and GND connected to dashboard
- [ ] I2C (SDA/SCL) connected correctly
- [ ] All 3 buttons connected (confirm, back, encoder)
- [ ] Encoder tracks (TRA, TRB) connected
- [ ] No shorts on breadboard
- [ ] ESP32 powered via USB

## Serial Monitor Commands

While test is running, you can type in Serial Monitor:

- **'r'** - Reset encoder counter to 0
- **'i'** - Show pin information
- **'s'** - Show current state

(Note: These would need to be added to the code if desired)

## Success Criteria

✅ **PASS**: All of these work:
1. Splash screen appears for 3 seconds
2. Counter changes when encoder turned
3. Both buttons show "PRESSED" when pressed
4. Encoder button resets counter
5. No errors in serial monitor

❌ **FAIL**: If any don't work, check wiring first!

## Next Steps

Once test passes:
1. Document which direction is "clockwise" for your encoder
2. Note if any pins need swapping
3. Restore main.cpp: `mv src/main.cpp.backup src/main.cpp`
4. Your dashboard hardware is verified! ✅

## Notes

- **Debouncing**: 50ms debounce on all buttons prevents false triggers
- **Encoder Logic**: Uses simple state change detection
- **Power**: All I/O is 3.3V - never exceed this voltage!
- **Serial Baud**: 115200 for clear monitoring

---

**Test Program**: `test_dashboard_hardware.cpp`
**Created for**: Nerdbilly Fab Dashboard
**Hardware**: ESP32 + SSD1306 OLED + 2 Buttons + EC11 Encoder
