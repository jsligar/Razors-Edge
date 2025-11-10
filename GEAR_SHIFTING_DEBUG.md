# Gear Shifting Troubleshooting Guide

## Problem: Gear shifts not working with A/B buttons

### How Gear Shifting SHOULD Work:

**Button Flow:**
1. **Button A (GPIO 14)** → Shift UP: Park → 1st → 2nd → 3rd → Sport+ → (cycle)
2. **Button B (GPIO 13)** → Shift DOWN: Park → Eco → 3rd → 2nd → 1st → (cycle)

**Code Flow:**
```
InputHandler (Core 1)
  ↓ isShiftUpPressed() / isShiftDownPressed()
CoreManager (Core 1 UI Task)
  ↓ Store in SharedSystemData
main.cpp (Core 0)
  ↓ Read SharedSystemData
StateMachine::handleShiftUp/Down()
  ↓ executeGearChange()
Display + Lights animate
```

---

## Diagnostic Steps

### Step 1: Check Serial Monitor Output

When you press Button A or B, you should see these messages:

**From CoreManager (Core 1):**
```
✓ Shift UP button pressed
```
or
```
✓ Shift DOWN button pressed
```

**From main.cpp (Core 0):**
```
Shift UP requested
```
or
```
Shift DOWN requested
```

**From StateMachine:**
```
Gear: P → 1
```

**What each missing message means:**
- ❌ No "✓ Shift UP button pressed" → **Button not detected by InputHandler**
- ✅ "✓ Shift UP button pressed" but no "Shift UP requested" → **SharedSystemData not being passed correctly**
- ✅ "Shift UP requested" but no "Gear: X → Y" → **State machine blocking the change**

---

### Step 2: Check Button Wiring

**Button A (Shift Up):**
- GPIO 14 (left side of ESP32, pin 12 from top)
- Should connect button between GPIO 14 and GND
- Internal pull-up enabled (button is active-low)

**Button B (Shift Down):**
- GPIO 13 (left side of ESP32, pin 15 from top)
- Should connect button between GPIO 13 and GND
- Internal pull-up enabled (button is active-low)

**Test with multimeter:**
- Button released: GPIO should read ~3.3V
- Button pressed: GPIO should read 0V (GND)

---

### Step 3: Check for Blocking Conditions

**Sport+ mode is blocked if:**
- Battery voltage < 58.0V
- Battery current > 18.0A

**All other gear changes should work without restrictions.**

If trying to enter Sport+, check serial monitor for:
```
Sport+ denied: Battery voltage too low
```
or
```
Sport+ denied: System under high load
```

---

### Step 4: Manual Button Test

Add this test code to `main.cpp` temporarily (inside `loop()`):

```cpp
// DEBUG: Manual button read test
static unsigned long lastDebug = 0;
if (millis() - lastDebug > 1000) {
    int key0 = digitalRead(GPIO_KEY0);
    int key1 = digitalRead(GPIO_KEY1);
    Serial.printf("[DEBUG] KEY0(GPIO14)=%d KEY1(GPIO13)=%d\n", key0, key1);
    lastDebug = millis();
}
```

**Expected output:**
- Buttons released: `KEY0=1 KEY1=1` (pulled high)
- Button A pressed: `KEY0=0 KEY1=1` (pulled low)
- Button B pressed: `KEY0=1 KEY1=0` (pulled low)

---

### Step 5: Check Current Gear

The system needs to know what gear you're currently in. Check serial monitor for:

```
Current Gear: P
```

If the gear display shows something, the system knows the current gear.

---

### Step 6: Verify SharedSystemData

Add this debug code to `main.cpp` inside `loop()`:

```cpp
// DEBUG: SharedSystemData button states
static bool lastShiftUp = false;
static bool lastShiftDown = false;

if (sysData.shiftUpPressed != lastShiftUp) {
    Serial.printf("[DEBUG] shiftUpPressed changed: %d → %d\n",
                  lastShiftUp, sysData.shiftUpPressed);
    lastShiftUp = sysData.shiftUpPressed;
}

if (sysData.shiftDownPressed != lastShiftDown) {
    Serial.printf("[DEBUG] shiftDownPressed changed: %d → %d\n",
                  lastShiftDown, sysData.shiftDownPressed);
    lastShiftDown = sysData.shiftDownPressed;
}
```

This will show when SharedSystemData button flags change.

---

## Common Issues

### Issue 1: Buttons Wired to Wrong Pins
**Symptom:** No "✓ Shift UP/DOWN button pressed" messages

**Fix:** Verify buttons are on GPIO 14 and GPIO 13, not some other pins

### Issue 2: Buttons Wired Backwards (to 3.3V instead of GND)
**Symptom:** Buttons always read as pressed

**Fix:** Button should connect GPIO to GND when pressed

### Issue 3: Pull-up Not Enabled
**Symptom:** Floating/random readings

**Fix:** Code already has `pinMode(GPIO_KEY0, INPUT_PULLUP)` - should be fine

### Issue 4: Debouncing Too Aggressive
**Symptom:** Button presses not registering

**Current debounce:** 50ms (defined in `config.h` as `BUTTON_DEBOUNCE_MS`)

**Fix:** Try reducing to 20ms temporarily for testing

### Issue 5: Button Press Consumed Too Fast
**Symptom:** "✓ Shift UP button pressed" but no "Shift UP requested"

**Explanation:** The button press flag might be cleared before main loop reads it

**Fix:** This shouldn't happen with current code, but if it does, need to add latching

---

## Quick Test Commands

Run these in serial monitor (if available):

```
gear up      - Force shift up
gear down    - Force shift down
gear 1       - Force to 1st gear
status       - Show current system status
```

(Note: These commands may not be implemented yet)

---

## Expected Behavior

**Starting from Park (P):**
1. Press Button A → Should shift to 1st gear
2. Press Button A again → Should shift to 2nd gear
3. Press Button B → Should shift back to 1st gear
4. Press Button B → Should shift to Eco mode
5. Press Button B → Should shift to Park

**Lights should flash** with each gear change (animation)

**OLED should update** to show new gear letter

---

## If Nothing Works

1. **Check core 1 is running:** Should see "✓ Encoder pressed" if you press encoder
2. **Check main loop is running:** Should see continuous sensor readings
3. **Try uploading fresh firmware**
4. **Check for FreeRTOS task crashes** - look for watchdog resets

---

## Next Steps

After following these steps, report back:
1. What serial output you see when pressing buttons
2. Whether the manual button test (Step 4) shows button presses
3. Current gear displayed on screen
4. Any error messages in serial monitor

This will help narrow down where the problem is in the chain!
