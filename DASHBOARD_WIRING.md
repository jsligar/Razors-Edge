# Dashboard Breadboard Wiring Guide

## Your Dashboard Pinout (from your module)
```
Pin 1: confirm (button A)
Pin 2: oled_sda
Pin 3: oled_scl
Pin 4: encoder_push
Pin 5: encoder_tra
Pin 6: encoder_trb
Pin 7: back (button B)
Pin 8: gnd
Pin 9: 3v3
```

## Correct ESP32 Wiring

### Power (Wire First!)
```
Dashboard Pin 9 (3v3)  →  ESP32 3.3V
Dashboard Pin 8 (gnd)  →  ESP32 GND
```

### OLED Display (I2C)
```
Dashboard Pin 2 (oled_sda)  →  ESP32 GPIO 21 (SDA)
Dashboard Pin 3 (oled_scl)  →  ESP32 GPIO 22 (SCL)
```

### Rotary Encoder
```
Dashboard Pin 5 (encoder_tra)  →  ESP32 GPIO 25 (CLK/A)
Dashboard Pin 6 (encoder_trb)  →  ESP32 GPIO 26 (DT/B)
Dashboard Pin 4 (encoder_push) →  ESP32 GPIO 27 (Button)
```

### Buttons
```
Dashboard Pin 1 (confirm - Button A)  →  ESP32 GPIO 14 ✅ SAFE PIN
Dashboard Pin 7 (back - Button B)     →  ESP32 GPIO 13
```

## Complete Pin Mapping Table

| Dashboard Pin | Function | → | ESP32 GPIO | Notes |
|--------------|----------|---|------------|-------|
| Pin 1 | confirm (Button A) | → | **GPIO 14** | Shift Up / Confirm |
| Pin 2 | oled_sda | → | GPIO 21 | I2C Data (SDA) |
| Pin 3 | oled_scl | → | GPIO 22 | I2C Clock (SCL) |
| Pin 4 | encoder_push | → | GPIO 27 | Encoder button |
| Pin 5 | encoder_tra | → | GPIO 25 | Encoder track A |
| Pin 6 | encoder_trb | → | GPIO 26 | Encoder track B |
| Pin 7 | back (Button B) | → | GPIO 13 | Shift Down / Back |
| Pin 8 | gnd | → | GND | Ground |
| Pin 9 | 3v3 | → | 3.3V | Power |

## ESP32 DevKit Pinout Reference

```
                        ESP32 DevKit
                    ┌─────────────────┐
                    │                 │
               3V3  │ 3V3         G23 │  GPIO23
               GND  │ GND         G22 │  GPIO22 ← oled_scl
          GPIO15    │ G15         TX0 │  TX0
          GPIO2     │ G2          RX0 │  RX0
          GPIO4     │ G4          G21 │  GPIO21 ← oled_sda
          GPIO16    │ RX2         GND │  GND
          GPIO17    │ TX2         G19 │  GPIO19
          GPIO5     │ G5          G18 │  GPIO18
          GPIO18    │ G18         G5  │  GPIO5
          GPIO19    │ G19         G17 │  GPIO17
          GND       │ GND         G16 │  GPIO16
          GPIO21    │ G21         G4  │  GPIO4
          RX0       │ RX0         G0  │  GPIO0
          TX0       │ TX0         G2  │  GPIO2
          GPIO22    │ G22         G15 │  GPIO15
          GPIO23    │ G23         GND │  GND
          GND       │ GND         3V3 │  3V3
                    │                 │
          GPIO13    │ G13         G27 │  GPIO27 ← encoder_push
          GPIO12    │ G12         G26 │  GPIO26 ← encoder_trb
          GPIO14    │ G14         G25 │  GPIO25 ← encoder_tra
          GPIO27    │ G27         G33 │  GPIO33
          GPIO26    │ G26         G32 │  GPIO32
          GPIO25    │ G25         G35 │  GPIO35
          GPIO33    │ G33         G34 │  GPIO34
          GPIO32    │ G32         VN  │  VN
          GPIO35    │ G35         VP  │  VP
          GPIO34    │ G34         EN  │  EN
          VN        │ VN          3V3 │  3V3
          VP        │ VP          GND │  GND
          EN        │ EN              │
                    │                 │
                    └─────────────────┘
                 confirm → G14
                 back    → G13
```

## Breadboard Layout Suggestion

```
ESP32 Side              Dashboard Module Side
-----------             ---------------------

GPIO 14 ──────────────→ Pin 1 (confirm)
GPIO 21 ──────────────→ Pin 2 (oled_sda)
GPIO 22 ──────────────→ Pin 3 (oled_scl)
GPIO 27 ──────────────→ Pin 4 (encoder_push)
GPIO 25 ──────────────→ Pin 5 (encoder_tra)
GPIO 26 ──────────────→ Pin 6 (encoder_trb)
GPIO 13 ──────────────→ Pin 7 (back)
GND ─────────────────→ Pin 8 (gnd)
3.3V ────────────────→ Pin 9 (3v3)
```

## Button Behavior

Both buttons are configured as **INPUT_PULLUP** (internal pull-up resistors):
- **Normal state**: HIGH (3.3V)
- **When pressed**: LOW (GND)
- **No external resistors needed** ✅

The encoder button works the same way.

## Testing Steps

### 1. Power Test
Connect only power (3.3V and GND) first. ESP32 should power on.

### 2. OLED Test
Connect I2C (SDA/SCL). OLED should display startup screen.

### 3. Button Test
Connect buttons. Press each button - serial monitor should show button presses.

### 4. Encoder Test
Connect encoder. Turn it - serial monitor should show rotation. Press - should show encoder button press.

## Common Issues & Solutions

### OLED Not Working
- Check I2C address (should be 0x3C)
- Verify power (3.3V not 5V!)
- Check SDA/SCL not swapped

### Buttons Not Responding
- Check GPIO numbers match code
- Verify buttons connect to GND when pressed
- Check serial monitor for debug messages

### Encoder Not Working
- Verify TRA/TRB not swapped
- Check both tracks connected
- Encoder needs common ground

### ESP32 Won't Boot
- This won't happen now - we removed GPIO 5!
- Check 3.3V power supply is stable
- Ensure no shorts on breadboard

## Why GPIO 14 is Better Than GPIO 5

| Feature | GPIO 5 ❌ | GPIO 14 ✅ |
|---------|-----------|-----------|
| Strapping pin | YES - boot issues | NO - safe |
| Pull-up at boot | Required | Optional |
| Boot mode affects | YES | NO |
| Safe for buttons | NO | YES |
| Conflict risk | HIGH | LOW |

## Additional Notes

- **All pins are 3.3V tolerant** - never exceed 3.3V!
- **I2C pull-ups**: The OLED module likely has pull-ups built in
- **Encoder debouncing**: Handled in software (20ms debounce)
- **Button debouncing**: Handled in software (50ms debounce)

## Complete System Pins (for reference)

Beyond your dashboard, the system also uses:
- GPIO 4: Pedal ADC input
- GPIO 12, 18, 19, 23: Motor control
- GPIO 15: Key switch
- GPIO 16, 17: GPS UART

These are already assigned and shouldn't conflict with your dashboard.

---

**Status**: ✅ Code updated to use GPIO 14
**Ready to wire**: Yes! Use the pinout above
**Safe to power on**: Yes, all safe pins now
