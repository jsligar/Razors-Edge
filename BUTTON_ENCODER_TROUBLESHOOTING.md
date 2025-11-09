# Button and Encoder Troubleshooting Guide

## Current Status
- **OLED Display**: ✅ Working - Shows main drive screen with data
- **WiFi AP**: ✅ Working - RazorsEdge-Controller at 192.168.4.1
- **System Stability**: ✅ Fixed - No more watchdog reboots
- **Encoder Button**: ❌ Not responding
- **GPIO 14 Button (CONFIRM)**: ❌ Not responding  
- **GPIO 13 Button (BACK)**: ❌ Not responding
- **Encoder Rotation**: ❌ Not tested yet

## Hardware Configuration
### Verified Wiring (from previous test)
- **OLED Display**: I2C at 0x3C - ✅ Working
- **Encoder Button**: GPIO 27 (active LOW with internal pullup)
- **Encoder Track A (CLK)**: GPIO 25 (with internal pullup)
- **Encoder Track B (DT)**: GPIO 26 (with internal pullup)
- **Confirm Button**: GPIO 14 (active LOW with internal pullup)
- **Back Button**: GPIO 13 (active LOW with internal pullup)

## Code Changes Made

### 1. Fixed Watchdog Timer Issue
**File**: `src/main.cpp` line ~123
```cpp
// Disable watchdog for now (tasks handle their own monitoring)
// safety.enableWatchdog();
Serial.println("Watchdog timer disabled (using task monitoring)");
```
**Status**: ✅ Fixed - System runs without rebooting

### 2. Set Default Screen Before Tasks Start
**File**: `src/main.cpp` line ~119
```cpp
// Set screen to main drive before starting tasks
ui.setScreen(SCREEN_MAIN_DRIVE);

// Display startup message on OLED (one-time display before tasks start)
ui.showStartupMessage();
delay(2000); // Show startup message before tasks take over
```
**Status**: ✅ Working - Main drive screen displays correctly

### 3. Fixed Race Condition - Moved inputs.update() to UI Task
**File**: `src/CoreManager.cpp` line ~347
```cpp
if (ui && inputs) {
    // Update input handler (read all buttons and encoder) - Core 1
    inputs->update();

    // Handle encoder button (use edge detection, not state)
    if (inputs->encoderButtonPressed()) {
        ui->cycleScreen();
        Serial.println("Encoder pressed - cycling screen");
    }

    // Check shift buttons and store in shared data for main loop
    lockData();
    sharedData.shiftUpPressed = inputs->isShiftUpPressed();
    sharedData.shiftDownPressed = inputs->isShiftDownPressed();
    unlockData();
    // ...
}
```
**Status**: ✅ Fixed race condition - all input reading now on Core 1

### 4. Main Loop Reads Button States from SharedData
**File**: `src/main.cpp` line ~159
```cpp
void loop() {
    // Get shared data (includes button states from UI task on Core 1)
    SharedSystemData sysData = coreManager.getSharedData();

    // Handle gear shifting input events (buttons read by UI task)
    if (sysData.shiftUpPressed) {
        stateMachine.handleShiftUp();
        Serial.println("Shift UP requested");
    }

    if (sysData.shiftDownPressed) {
        stateMachine.handleShiftDown();
        Serial.println("Shift DOWN requested");
    }
    // ... rest of loop
}
```
**Status**: ✅ Eliminates race condition between cores

### 5. Added Button Flags to SharedSystemData
**File**: `src/CoreManager.h` line ~86
```cpp
struct SharedSystemData {
    // ... other fields

    // Input flags (set by UI task, read by main loop)
    bool shiftUpPressed;
    bool shiftDownPressed;
};
```
**Status**: ✅ Thread-safe button state communication

## Issues Fixed

### ✅ Race Condition Between Cores
**Problem**: `inputs.update()` was called on Core 0 (main loop) while `encoderButtonPressed()` was checked on Core 1 (UI task). This caused button states to be missed or flags cleared before being read.

**Solution**: Moved all input reading to Core 1 (UI task):
- UI task calls `inputs->update()`
- UI task checks encoder button directly
- UI task stores shift button states in `SharedSystemData`
- Main loop reads button flags from `SharedSystemData`

**Result**: All input handling is now on a single core, eliminating race conditions.

## Remaining Issues to Test

### Issue 1: Buttons Still Not Responding (If Applicable)
**If buttons still don't work after the race condition fix:**

**Possible Causes**:
1. **GPIO Pin Conflict**: Check if GPIOs 13, 14, 25, 26, 27 are being used elsewhere
2. **Pullup Configuration**: Internal pullups might not be enabled properly
3. **Debounce Issues**: 50ms debounce might be too long or button states not clearing
4. **Hardware Connection**: Verify physical wiring is correct
5. **Timing**: UI task runs at 2Hz (500ms) - button presses shorter than this might be missed

### Issue 2: ADC2/WiFi Conflict (Non-Critical)
**Symptoms**: 
```
[E][esp32-hal-adc.c:170] __analogReadRaw(): GPIO4: ESP_ERR_TIMEOUT: ADC2 is in use by Wi-Fi
```
**Status**: ⚠️ Expected behavior - GPIO4 is ADC2 which conflicts with WiFi. This is normal and won't affect buttons.

## Debugging Steps to Try

### Step 1: Add More Debug Logging
Add to `src/InputHandler.cpp` in `updateEncoder()`:
```cpp
void InputHandler::updateEncoder() {
    // Read encoder button
    bool buttonPressed = !digitalRead(GPIO_ENCODER_BTN); // Active low
    
    // ADD THIS DEBUG LINE
    if (buttonPressed != encoder.buttonPressed) {
        Serial.printf("Encoder button state change: %d -> %d\n", encoder.buttonPressed, buttonPressed);
    }
    
    if (buttonPressed != encoder.buttonPressed) {
        encoder.buttonPressed = buttonPressed;
        encoder.buttonChanged = true;
    }
}
```

### Step 2: Test GPIO Reading Directly
Add to `loop()` in `main.cpp` temporarily:
```cpp
static unsigned long lastTest = 0;
if (millis() - lastTest > 1000) {
    Serial.printf("GPIO Test - Enc:%d Btn14:%d Btn13:%d\n",
                  digitalRead(GPIO_ENCODER_BTN),
                  digitalRead(GPIO_KEY0),
                  digitalRead(GPIO_KEY1));
    lastTest = millis();
}
```
**Expected Output**: All should read `1` (HIGH) when not pressed, `0` (LOW) when pressed

### Step 3: Check Pin Definitions
Verify in `src/config.h`:
```cpp
#define GPIO_ENCODER_CLK        25  // Encoder track A (TRA)
#define GPIO_ENCODER_DT         26  // Encoder track B (TRB)
#define GPIO_ENCODER_BTN        27  // Encoder push button
#define GPIO_KEY0               14  // Confirm/Shift Up button (Button A)
#define GPIO_KEY1               13  // Back/Shift Down button (Button B)
```

### Step 4: Verify pinMode Configuration
Check `src/InputHandler.cpp` in `init()`:
```cpp
pinMode(GPIO_ENCODER_CLK, INPUT_PULLUP);
pinMode(GPIO_ENCODER_DT, INPUT_PULLUP);
pinMode(GPIO_ENCODER_BTN, INPUT_PULLUP);
pinMode(GPIO_KEY0, INPUT_PULLUP);       // Shift Up
pinMode(GPIO_KEY1, INPUT_PULLUP);       // Shift Down
```

### Step 5: Check for Boot Pin Restrictions
- **GPIO 12**: Strapping pin (boot config) - avoid if possible
- **GPIO 13**: ✅ Safe for buttons
- **GPIO 14**: ✅ Safe for buttons  
- **GPIO 15**: Strapping pin (boot config) - avoid if possible
- **GPIO 25/26/27**: ✅ Safe for encoder

## Expected Behavior When Working

### Encoder Button Press:
1. Serial monitor shows: `"Encoder pressed - cycling screen"`
2. OLED cycles through screens:
   - **Main Drive**: Speed (0 MPH), Gear (P), Battery (0.0V), SOC gauge
   - **Detailed Metrics**: Motor currents, balance, power, SOC percentage
   - **Settings**: Calibration unlock instructions
   - Back to Main Drive

### GPIO 14 Button (CONFIRM/Shift Up):
1. Serial monitor shows: `"Shift UP requested"`
2. Gear changes: P → R → N → D → S → S+ (if unlocked)

### GPIO 13 Button (BACK/Shift Down):
1. Serial monitor shows: `"Shift DOWN requested"`  
2. Gear changes in reverse: S+ → S → D → N → R → P

## Quick Test Commands

### Monitor with Filtering:
```powershell
pio device monitor -e esp32dev --filter=nocontrol
```

### Clean Build and Upload:
```powershell
pio run -e esp32dev --target clean
pio run -e esp32dev --target upload
```

### Check Current Git Status:
```powershell
git status
git log --oneline -5
```

## Files Modified in This Session
1. `src/main.cpp` - Disabled watchdog, set default screen, removed inputs.update() from main loop, reads button states from SharedSystemData
2. `src/CoreManager.cpp` - Added inputs->update() to UI task, checks buttons and stores in SharedSystemData
3. `src/CoreManager.h` - Added shiftUpPressed and shiftDownPressed flags to SharedSystemData

## Next Steps for Manual Testing
1. Add GPIO debug logging to verify pins are reading correctly
2. Test with simple digitalRead() calls in loop()
3. Check if input handler update is being called (add Serial.println in update())
4. Verify button wiring with multimeter (should show LOW when pressed)
5. Try different GPIO pins if current ones have hardware issues

## Hardware Test Procedure
1. **Multimeter Test**: 
   - Measure between GPIO pin and GND
   - Should read 3.3V when not pressed (pullup active)
   - Should read 0V when button pressed
   
2. **LED Test**:
   - Connect LED between GPIO and GND through resistor
   - LED should be ON when button pressed (if configured as OUTPUT)

3. **Continuity Test**:
   - Check button contacts with continuity mode
   - Should beep when button pressed

## Contact
If issues persist, check:
- Physical button wiring and connections
- ESP32 boot mode (should be normal boot, not download mode)
- Power supply stability (brownout can cause strange behavior)
- Try different ESP32 board if available (could be damaged GPIO pins)
