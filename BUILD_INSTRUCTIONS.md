# Razors Edge ESP32 - Build and Upload Instructions

## Overview
This document contains complete instructions for building and uploading the Razors Edge electric vehicle controller firmware to ESP32.

---

## Prerequisites

- PlatformIO Core (CLI) or PlatformIO IDE extension for VS Code
- ESP32 board connected via USB
- Git (for version control)

---

## Build Environments

The project has multiple build configurations:

### 1. **release** (Production Build)
- Optimized for size and performance (-O2)
- Minimal logging (INFO level)
- No debug symbols
- **Use this for final deployment**

### 2. **esp32dev** (Development Build)
- Balanced optimization (-O1)
- Medium logging (INFO level)
- Colored serial output
- **Recommended for active development**

### 3. **debug** (Debug Build)
- Full logging (DEBUG level)
- Debug symbols included
- Assertions enabled
- Lower optimization (-Og)
- **Use for troubleshooting**

### 4. **test** (Hardware Testing)
- Unit test framework enabled
- Full debug output
- **Use for running hardware tests**

---

## Quick Build Commands

### Compile Only (No Upload)

```bash
# Production build
pio run -e release

# Development build
pio run -e esp32dev

# Debug build
pio run -e debug
```

### Compile and Upload Firmware

```bash
# Production build
pio run -e release --target upload

# Development build
pio run -e esp32dev --target upload

# Debug build
pio run -e debug --target upload
```

### Upload Filesystem (Web Interface Files)

The `data/` folder contains the web interface files (HTML, CSS, JS). Upload separately:

```bash
# Upload filesystem to ESP32
pio run -e release --target uploadfs

# Or for development environment
pio run -e esp32dev --target uploadfs
```

### Monitor Serial Output

```bash
# Monitor serial output (115200 baud)
pio device monitor -e release

# Or combined upload + monitor
pio run -e release --target upload && pio device monitor
```

---

## Full System Upload Procedure

Follow these steps for a complete fresh installation:

### Step 1: Pull Latest Code
```bash
git pull origin claude/code-review-analysis-011CUt2oVAD9ErLxm4sEZe4b
```

### Step 2: Clean Build (Optional but Recommended)
```bash
pio run -e release --target clean
```

### Step 3: Compile Firmware
```bash
pio run -e release
```

### Step 4: Upload Firmware
```bash
pio run -e release --target upload
```

### Step 5: Upload Filesystem
```bash
pio run -e release --target uploadfs
```

### Step 6: Monitor Output
```bash
pio device monitor
```

**Or do it all in one command:**
```bash
pio run -e release --target clean && \
pio run -e release --target upload && \
pio run -e release --target uploadfs && \
pio device monitor
```

---

## Expected Serial Output on Boot

After successful upload, you should see:

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
✓ Navigation initialized
✓ Web Interface initialized
✓ State Machine initialized
✓ Dual-core task system active
  → Core 0: WiFi, Web Interface
  → Core 1: Safety, Motors, Navigation, UI
```

---

## Troubleshooting Build Issues

### Error: "pio: command not found"
**Solution:** Install PlatformIO Core
```bash
pip install platformio
```

### Error: "Port not found" or "Access denied"
**Solution:** Check USB connection and permissions
```bash
# Linux: Add user to dialout group
sudo usermod -a -G dialout $USER

# List available ports
pio device list
```

### Error: Compilation fails with atomic errors
**Solution:** This has been fixed in the latest commit. Pull latest:
```bash
git pull origin claude/code-review-analysis-011CUt2oVAD9ErLxm4sEZe4b
```

### Error: Out of memory / Upload fails
**Solution:** Try erasing flash first
```bash
pio run -e release --target erase
pio run -e release --target upload
```

---

## Build Output Files

After successful build, firmware files are located in:

```
.pio/build/release/
  ├── firmware.bin       # Main firmware
  ├── bootloader.bin     # ESP32 bootloader
  ├── partitions.bin     # Partition table
  └── spiffs.bin         # Filesystem image (if uploadfs was run)
```

---

## Project Structure

```
Razors-Edge/
├── src/                    # Main source code
│   ├── main.cpp           # Entry point
│   ├── CoreManager.cpp    # Dual-core task manager
│   ├── PowerManager.cpp   # Battery/power monitoring
│   ├── MotorController.cpp # Motor control
│   ├── SafetySystem.cpp   # Safety interlocks
│   └── ...
├── data/                  # Web interface files (upload via uploadfs)
│   ├── index.html         # Main dashboard
│   ├── login.html         # Login page
│   ├── css/styles.css     # Stylesheet
│   ├── js/dashboard.js    # Dashboard logic
│   ├── manifest.json      # PWA manifest
│   └── sw.js              # Service worker
├── test/                  # Unit tests
├── platformio.ini         # Build configuration
└── BUILD_INSTRUCTIONS.md  # This file
```

---

## Version Information

Current firmware version: **v1.1.0**

Build system: **PlatformIO 6.x**
Platform: **Espressif32**
Board: **ESP32 Dev Module**
Framework: **Arduino**

---

## Additional Commands

### View Build Configuration
```bash
pio project config
```

### List All Environments
```bash
pio run --list-targets
```

### Update Libraries
```bash
pio lib update
```

### Clean All Builds
```bash
pio run --target clean
```

### Check Code (Linting)
```bash
pio check
```

---

## Notes

- **Upload speed:** 921600 baud (configured in platformio.ini)
- **Monitor speed:** 115200 baud
- **Filesystem:** LittleFS (configured for ESP32)
- **All builds use the same board configuration** - only compiler flags differ

---

## Quick Reference

| Task | Command |
|------|---------|
| Build production | `pio run -e release` |
| Upload firmware | `pio run -e release --target upload` |
| Upload web files | `pio run -e release --target uploadfs` |
| Monitor serial | `pio device monitor` |
| Clean build | `pio run -e release --target clean` |
| Full rebuild | `pio run -e release --target clean && pio run -e release` |

---

**Last Updated:** 2025-11-09
**Branch:** claude/code-review-analysis-011CUt2oVAD9ErLxm4sEZe4b
