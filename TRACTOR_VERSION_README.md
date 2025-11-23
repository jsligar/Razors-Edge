# Tractor Version - Ride-On Tractor Controller

This is a simplified version of the Razor controller designed for a ride-on tractor.

## Key Differences from Original Version

### Hardware Changes
- **3-Position Direction Switch** instead of gear buttons
  - GPIO 25: Forward
  - GPIO 26: Reverse
  - Both LOW = Neutral
- **No OLED Display** - all output via Serial
- **No GPS Module** - navigation features disabled
- **No Power Monitoring** - No sensors at all
  - No battery voltage sensing
  - No current sensing
  - No motor monitoring
  - No power-based safety checks

### Pin Configuration
- Motor PWM Left: GPIO 18
- Motor PWM Right: GPIO 12
- Motor DIR Left: GPIO 19
- Motor DIR Right: GPIO 23
- Pedal ADC: GPIO 4
- Key Switch: GPIO 15
- Direction Forward: GPIO 25
- Direction Reverse: GPIO 26

### Power Configuration
- No battery monitoring
- No voltage sensing
- Speed Limit: 5 MPH (configurable in config.h)

### Direction Control
Three modes:
- **Neutral (N)**: Both switches low, 0% throttle, Red LED
- **Forward (F)**: Forward switch high, 100% throttle, Green LED
- **Reverse (R)**: Reverse switch high, 50% throttle (limited), Yellow LED

### Simplified Safety System
Only monitors:
- Key switch state
- Watchdog timer

All power-based safety checks removed:
- Battery voltage monitoring
- Battery overcurrent
- Motor overcurrent
- Motor stall detection
- Motor imbalance

### Modified Modules

#### config.h
- Removed I2C addresses for OLED and INA228 sensors
- Changed from GearMode enum to DirectionMode enum
- Simplified fault codes
- Updated voltage/speed limits for tractor

#### InputHandler
- Replaced rotary encoder + shift buttons with 3-position direction switch
- Simplified pedal calibration (time-based instead of button-based)
- Added `getDirection()` and `directionChanged()` methods

#### PowerManager
- Removed all INA228 current sensing
- Returns estimated battery voltage (currently fixed at 12V)
- All current/power methods return 0 or stubs

#### UserInterface
- Removed all OLED/Adafruit libraries
- All output goes to Serial monitor
- Simplified status messages

#### Navigation
- Completely stubbed out (all methods return 0/false)
- No GPS functionality

#### StateMachine
- Changed from gear-based to direction-based control
- Removed Sport+ mode and timeout logic
- Simplified to Forward/Neutral/Reverse states

#### SafetySystem
- Removed current-based safety checks
- Only monitors voltage and key switch
- Simplified fault history

#### main.cpp
- Removed I2C initialization
- Updated to use direction switch instead of shift buttons
- Removed Sport+ timeout checking
- Updated status messages

## Building the Project

```bash
platformio run
```

## TODO for Implementation

1. **Test Direction Switch**:
   - Verify switch wiring to GPIO 25 and 26
   - Test Forward/Neutral/Reverse transitions
   - Adjust debounce timing if needed

2. **Calibrate Pedal**:
   - Use calibration sequence in setup (or via web interface)
   - Set min/max ADC values for your pedal

3. **Tune Speed/Throttle**:
   - Adjust max speeds in config.h for your tractor
   - Modify throttle curves if needed
   - May want to limit reverse speed further

4. **Safety Tuning**:
   - Add any tractor-specific safety checks if needed
   - Test emergency stop behavior
   - Consider adding external voltage monitoring if desired

## Notes

- Web interface still available for configuration and monitoring
- Most dual-core functionality preserved
- Motor control and lighting systems unchanged
- Can easily add back features if needed

## Reverting Changes

The original version with full features is available on the main branch.
This tractor version is on branch: `claude/tractor-ride-control-01CyiW5ceyHKo4nzP1e4Dcwa`
