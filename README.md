# Razors Edge - 60V Electric Vehicle Controller

> **ESP32-based electric vehicle controller for Razor ride-on toy conversion**  
> Advanced dual motor control with safety systems and telemetry

## 🚗 Project Overview

Transform a Razor ride-on toy into a professional electric vehicle with:
- **6-Speed Electronic Transmission** (Park, 1st, 2nd, 3rd, Eco, Sport+)
- **Dual Independent Motor Control** with current monitoring
- **Multi-Layer Safety Systems** with fault detection
- **GPS Geofencing** with automated speed limits and boundaries
- **WiFi Web Interface** for smartphone monitoring and control
- **Advanced Telemetry** (GPS, voltage, current, power, efficiency)
- **OLED Dashboard** with rotary encoder interface
- **4-Zone LED Lighting** with auto modes and animations

## ⚡ System Specifications

- **Platform:** ESP32-WROOM-32 (240MHz dual-core with optimized task distribution)
- **Architecture:** Dual-core real-time system (Core 1: Safety/Motors, Core 0: WiFi/Web)
- **Power:** 60V Li-ion battery (15S, 54-63V range)
- **Motors:** 2× 30V DC motors via Cytron MDD20A driver
- **Current Monitoring:** 3× INA228 sensors (0.001Ω shunts)
- **Display:** 128×64 OLED with rotary encoder
- **GPS:** GT-U7 module for speed/position tracking
- **WiFi:** Built-in ESP32 WiFi for web interface
- **Lighting:** PCA9685 PWM controller for 4-zone LEDs

## 🛠️ Hardware Setup

### Required Components
- ESP32-WROOM-32 development board
- Cytron MDD20A dual motor driver
- 3× Adafruit INA228 current sensors
- PCA9685 PWM driver board
- SSD1306 OLED display (128×64)
- GT-U7 GPS module
- Rotary encoder with push button
- 10kΩ potentiometer (pedal sensor)
- Power converters: 60V→30V (600W), 60V→12V (120W), 12V→5V (3A)

### Pin Assignments
| GPIO | Function | Device |
|------|----------|--------|
| 21 | I2C SDA | All I2C devices |
| 22 | I2C SCL | All I2C devices |
| 32 | Motor PWM L | MDD20A Channel A |
| 12 | Motor PWM R | MDD20A Channel B |
| 34 | Pedal ADC | 10kΩ Potentiometer |
| 15 | Key Switch | SPST Switch (10kΩ pulldown) |
| 25 | Encoder CLK | Rotary Encoder |
| 26 | Encoder DT | Rotary Encoder |
| 27 | Encoder BTN | Push Button |
| 33 | Shift Up | KEY0 Button |
| 13 | Shift Down | KEY1 Button |
| 16 | GPS RX | GT-U7 TX |

### I2C Device Addresses
- **0x40:** Battery Monitor (INA228)
- **0x41:** Left Motor Monitor (INA228)
- **0x44:** Right Motor Monitor (INA228)
- **0x70:** Light Controller (PCA9685)
- **0x3C:** OLED Display (SSD1306)

## 🔧 Software Architecture

### Core Modules
- **PowerManager:** Triple INA228 monitoring, battery SOC, energy tracking
- **MotorController:** Dual PWM control, gear curves, acceleration ramping
- **LightController:** 4-zone LED control, auto modes, gear animations
- **UserInterface:** OLED display management, multiple screens
- **InputHandler:** Pedal ADC, encoder, buttons with debouncing
- **Navigation:** GPS parsing, speed calculation, odometer, geofencing
- **Geofencing:** GPS-based boundaries, speed limits, safety zones
- **WebInterface:** WiFi access point, REST API, mobile dashboard
- **CoreManager:** Dual-core task scheduler with real-time guarantees
- **SafetySystem:** Fault detection, current limiting, watchdog timer
- **StateMachine:** System coordination, state management

### Gear System
| Gear | Max Power | Curve | Character |
|------|-----------|-------|-----------|
| Park | 0% | N/A | Disabled |
| 1st | 40% | Gentle | Learning mode |
| 2nd | 70% | Linear | Normal cruising |
| 3rd | 100% | Sporty | Full power |
| Eco | 60% | Smooth | Battery conservation |
| Sport+ | 110% | Aggressive | 10-second timeout |

### Safety Features
- **Low Voltage Protection:** Shutdown at 54V
- **Overcurrent Protection:** 20A per motor, 20A battery
- **Motor Stall Detection:** High current + no movement
- **Motor Balance Monitoring:** Warn at 15%, fault at 30%
- **Key Switch Enforcement:** No operation without key
- **Watchdog Timer:** Reset if system hangs
- **GPS Geofencing:** Automated speed limits and prohibited zones

### Geofencing System
The advanced geofencing system provides GPS-based safety controls:
- **Safe Zones:** Designated safe operating areas
- **Speed Limits:** Automatic speed reduction in specific areas
- **Prohibited Zones:** No-entry areas with automatic stop
- **Emergency Boundaries:** Auto return-to-home beyond limits
- **Real-time Monitoring:** Continuous GPS position tracking

For detailed geofencing setup and configuration, see [GEOFENCING_GUIDE.md](GEOFENCING_GUIDE.md).

### Dual-Core Real-Time Architecture
Optimized task distribution across ESP32's dual-core processor:
- **Core 1 (Real-Time):** Safety monitoring (100Hz), motor control (50Hz), navigation (10Hz)
- **Core 0 (Communication):** WiFi stack, web interface, background tasks
- **Thread-Safe Communication:** Mutex-protected shared data between cores
- **Deterministic Timing:** Guaranteed response times for safety-critical functions
- **Performance Monitoring:** Real-time task execution and memory usage tracking

For complete architecture details, see [DUAL_CORE_ARCHITECTURE.md](DUAL_CORE_ARCHITECTURE.md).

### WiFi Web Interface
Monitor and control your vehicle remotely via smartphone:
- **Real-time Dashboard:** Battery, motors, GPS, geofencing status
- **Remote Controls:** Emergency stop, gear changes, trip reset
- **Geofence Management:** Add/remove boundaries, set speed limits
- **Mobile Optimized:** Responsive design for all devices

**Quick Connect:** Look for WiFi network `RazorsEdge-Controller`, password `RazorEdge2025`, then browse to `http://192.168.4.1`

For complete WiFi setup and mobile app development, see [WIFI_GUIDE.md](WIFI_GUIDE.md).

## 📋 Getting Started

### 1. Development Environment
```bash
# Install PlatformIO
pip install platformio

# Clone or download project
# Open in VS Code with PlatformIO extension
```

### 2. Hardware Assembly
1. Wire ESP32 according to pin assignments
2. Connect all I2C devices to bus (SDA/SCL)
3. Install current sensors in power paths
4. Connect motors to MDD20A driver
5. Wire power distribution system
6. **CRITICAL:** Install 10kΩ pull-down on GPIO 15

### 3. Initial Testing
```bash
# Compile project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### 4. Progressive Testing
1. **I2C Scan:** Verify all 5 devices respond
2. **Power Monitoring:** Check voltage/current readings
3. **Display Test:** Verify OLED shows startup screen
4. **Input Test:** Check pedal, buttons, encoder
5. **Motor Test:** Low power with wheels off ground
6. **Integration:** Full system test in safe area

## 🔍 Troubleshooting

### Common Issues

**I2C Device Not Found**
- Check wiring (SDA/SCL, power, ground)
- Verify device addresses
- Add pull-up resistors if needed

**Motor Won't Run**
- Verify key switch is ON
- Check 30V motor power supply
- Ensure MDD20A error LED is off
- Confirm PWM signals present

**Erratic Pedal Reading**
- Increase ADC filtering
- Check 3.3V supply stability
- Calibrate min/max values

**ESP32 Won't Boot**
- Verify GPIO 15 has 10kΩ pull-down
- Check 5V power supply
- Remove all connections, test bare ESP32

### Serial Debug Output
Monitor serial at 115200 baud for:
- Module initialization status
- I2C device detection
- Current/voltage readings
- Fault codes and warnings
- State transitions

## ⚠️ Safety Warnings

**DANGER - High Voltage System**
- 60V can cause serious injury or death
- Always disconnect battery before working
- Use appropriate safety equipment
- Test thoroughly before road use

**Motor Safety**
- Sport+ mode can damage motors if overused
- Monitor motor temperatures during testing
- Respect current limits
- Emergency stop must always be accessible

**Battery Safety**
- Use proper Li-ion charging procedures
- Never discharge below 54V (3.6V/cell)
- Monitor cell balance
- Install appropriate fuses

## 📚 Documentation

- **PROJECT_CONTEXT.md:** Complete technical specifications
- **platformio.ini:** Library dependencies and build config
- **src/config.h:** All pin assignments and constants
- **src/*.h:** Module header files with API documentation

## 🤝 Contributing

This is a specialized embedded project. Contributions welcome for:
- Additional safety features
- Performance optimizations
- Documentation improvements
- Testing procedures

## 📄 License

This project is for educational and experimental use. Use at your own risk.

**⚠️ WARNING:** This involves high-voltage electrical systems and motor control. Improper implementation can cause injury, property damage, or death. Only qualified individuals should attempt this project.

---

**Build Status:** ✅ Code Complete - Ready for Hardware Testing  
**Last Updated:** November 2025  
**Platform:** ESP32 + PlatformIO + Arduino Framework