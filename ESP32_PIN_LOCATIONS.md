# ESP32 Physical Pin Locations for GT-U7 GPS

## What You Need to Find on Your ESP32:

You need to connect to these **GPIO numbers**:
- **GPIO 16** (for GPS TX data)
- **GPIO 17** (for GPS RX data)
- **3.3V** (for power)
- **GND** (for ground)

---

## Common ESP32 DevKit V1 (30-pin) Pinout

This is the most common ESP32 board. Pins are on both sides.

```
                    ESP32 DevKit V1
                    ===============
                         USB
                       ┌─────┐
         Left Side     │     │     Right Side
         =========     └─────┘     ==========

    3V3  ●  ○ 1 ───────────────  1 ○  ● GND
    EN   ●  ○ 2                  2 ○  ● GPIO 23
 GPIO 36 ●  ○ 3                  3 ○  ● GPIO 22
 GPIO 39 ●  ○ 4                  4 ○  ● GPIO 1 (TX0)
 GPIO 34 ●  ○ 5                  5 ○  ● GPIO 3 (RX0)
 GPIO 35 ●  ○ 6                  6 ○  ● GPIO 21
 GPIO 32 ●  ○ 7                  7 ○  ● GND
 GPIO 33 ●  ○ 8                  8 ○  ● GPIO 19
 GPIO 25 ●  ○ 9                  9 ○  ● GPIO 18
 GPIO 26 ●  ○ 10                10 ○  ● GPIO 5
 GPIO 27 ●  ○ 11                11 ○  ● GPIO 17  ← GPS RX connects here!
 GPIO 14 ●  ○ 12                12 ○  ● GPIO 16  ← GPS TX connects here!
 GPIO 12 ●  ○ 13                13 ○  ● GPIO 4
    GND  ●  ○ 14                14 ○  ● GPIO 0
 GPIO 13 ●  ○ 15                15 ○  ● GPIO 2

```

## 🎯 Connection Points on ESP32 DevKit V1:

**RIGHT side of the board (near USB):**
- **Pin 11** = GPIO 17 → Connect **GT-U7 RX** (white/yellow wire)
- **Pin 12** = GPIO 16 → Connect **GT-U7 TX** (green wire)

**LEFT side of the board:**
- **Pin 1** = 3.3V → Connect **GT-U7 VCC** (red wire)
- **Pin 14** = GND → Connect **GT-U7 GND** (black wire)

---

## Visual Connection Guide

```
GT-U7 GPS Module                    ESP32 DevKit V1
================                    ===============

Pin 1: VCC (red wire)    ──────→   LEFT Pin 1: 3.3V
Pin 2: RX (yellow wire)  ──────→   RIGHT Pin 11: GPIO 17
Pin 3: TX (green wire)   ──────→   RIGHT Pin 12: GPIO 16
Pin 4: GND (black wire)  ──────→   LEFT Pin 14: GND
Pin 5: PPS               ──────    (not connected)
```

---

## How to Find GPIO 16 and GPIO 17 on YOUR Board

### Method 1: Look at the Silkscreen
- Most ESP32 boards have GPIO numbers printed on the PCB
- Look for "GPIO16" or "16" and "GPIO17" or "17"
- Sometimes labeled as "IO16" and "IO17"

### Method 2: Check Pin Labels
Some boards might show:
- **RX2** = GPIO 16 (UART2 RX)
- **TX2** = GPIO 17 (UART2 TX)

But **don't confuse with**:
- **RX0** / **TX0** = GPIO 3 / GPIO 1 (used for USB serial, NOT for GPS!)

### Method 3: Count from Photos
1. Hold board with USB port at TOP
2. Count pins from top on the RIGHT side
3. GPIO 17 = 11th pin from top (right side)
4. GPIO 16 = 12th pin from top (right side)

---

## Common ESP32 Board Variations

### ESP32-WROOM-32 DevKit (38-pin)
- **GPIO 16**: Usually on right side, around pin 27-29
- **GPIO 17**: Usually next to GPIO 16
- Check your specific board's pinout diagram

### NodeMCU-32S
- **GPIO 16**: Labeled as "D0" or "RX2"
- **GPIO 17**: Labeled as "D1" or "TX2"

### ESP32-C3 / ESP32-S3 (Different chip!)
⚠️ **These use different GPIO numbers!**
- Check your board documentation
- Different pinout than ESP32-WROOM

---

## ⚠️ Critical: DO NOT Use These Pins

**Wrong pins that look similar:**
- ❌ **TX** (GPIO 1) - This is for USB serial, not GPS!
- ❌ **RX** (GPIO 3) - This is for USB serial, not GPS!
- ❌ **GPIO 6-11** - Used for internal flash, will cause crashes!

**Correct pins:**
- ✅ **GPIO 16** - Specifically this number
- ✅ **GPIO 17** - Specifically this number

---

## Step-by-Step Physical Connection

### Step 1: Identify Your Board Type
- Take a photo of your ESP32
- Look for text on the board: "ESP32 DevKit", "NodeMCU-32S", etc.
- Count the total pins (30 or 38 usually)

### Step 2: Locate GPIO 16 and GPIO 17
**On ESP32 DevKit V1 (most common):**
- Face the board with USB port at TOP
- Look at RIGHT side
- Count down from top: 1, 2, 3... 11, 12
- **Pin 11 = GPIO 17**
- **Pin 12 = GPIO 16**
- Should be labeled on the board!

### Step 3: Locate Power Pins
**3.3V pin:**
- Usually top-left corner
- Labeled "3V3" or "3.3V"

**GND pin:**
- Multiple GND pins available
- Labeled "GND" or "G"
- Use any GND pin

### Step 4: Insert Wires
**Breadboard method:**
1. Insert ESP32 into breadboard (straddles center gap)
2. Insert GT-U7 female headers into breadboard
3. Use jumper wires to connect

**Direct connection:**
- Use female-to-female jumper wires
- Connect GT-U7 5-pin header directly to ESP32 pins

---

## Wire Color Convention (Suggested)

Use different colored wires to avoid confusion:
- 🔴 **Red** = VCC (3.3V power)
- ⚫ **Black** = GND (ground)
- 🟢 **Green** = GT-U7 TX → ESP32 GPIO 16
- 🟡 **Yellow** = GT-U7 RX → ESP32 GPIO 17
- 🔵 **Blue** = PPS (not used, leave disconnected)

---

## Double-Check Your Connections

Before powering on:

| GT-U7 Pin | Wire Color | ESP32 Pin | GPIO Number |
|-----------|------------|-----------|-------------|
| VCC       | Red        | 3.3V      | Power       |
| RX        | Yellow     | RIGHT 11  | GPIO 17     |
| TX        | Green      | RIGHT 12  | GPIO 16     |
| GND       | Black      | GND       | Ground      |
| PPS       | -          | -         | Not used    |

**Verify:**
- ✅ GT-U7 USB is **unplugged**
- ✅ GT-U7 TX → ESP32 GPIO **16** (not 17!)
- ✅ GT-U7 RX → ESP32 GPIO **17** (not 16!)
- ✅ Using 3.3V (not 5V)
- ✅ GND connected

---

## Still Not Sure? Take a Photo!

If you're unsure about your specific board:
1. Take a clear photo of your ESP32 board
2. Show both sides with pin labels visible
3. Note the board name/model printed on it

Common boards:
- **ESP32 DevKit V1** (30 pins total, 15 per side)
- **ESP32-WROOM-32** (38 pins total, 19 per side)
- **NodeMCU-32S** (30 pins, labeled D0-D13)
- **ESP32-C3 / S2 / S3** (different chip entirely)

---

## Quick Test After Wiring

Once wired, upload this tiny test to verify connections:

```cpp
void setup() {
  Serial.begin(115200);
  Serial.println("Testing GPIO 16/17...");
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for GPS data on GPIO 16/17...");
}

void loop() {
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
```

**Expected result:** You should see NMEA sentences scrolling in serial monitor.

---

## Common Issues

### "Nothing happens"
- Check: GT-U7 LED blinking? (Power is connected)
- Check: USB disconnected from GT-U7?
- Check: Correct GPIO numbers?

### "Still no data"
- Try swapping GPIO 16 and 17 connections (maybe they're reversed)
- Double-check it's GPIO 16/17, not pin numbers
- Verify with multimeter: 3.3V on VCC, 0V on GND

---

## Next Steps

After wiring is confirmed:
1. Upload full Razors Edge firmware
2. Open serial monitor (115200 baud)
3. Look for: `GPS: Sats=0 Fix=0...` every 5 seconds
4. Take ESP32 outside for satellite fix

Good luck! Let me know which ESP32 board you have if you need more specific help! 🛰️
