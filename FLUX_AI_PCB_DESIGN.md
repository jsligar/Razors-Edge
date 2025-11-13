# Razors-Edge All-in-One Motor Controller PCB Design

## Flux.ai Design Prompt

**Design an all-in-one ESP32-based dual motor controller PCB for a 60V electric vehicle with the following specifications:**

---

## Core Processing

- **ESP32-S3-WROOM-1-N8R2 module** (dual-core Xtensa LX7 @ 240MHz, WiFi enabled)
- 8MB Flash, 2MB PSRAM
- Operating voltage: 3.3V logic
- Improved ADC accuracy for safety-critical throttle sensing
- USB-C connector for programming (CH340C USB-to-UART bridge)
- Reset and boot buttons
- WiFi antenna keep-out zone (15mm × 5mm)

---

## Power System Architecture

### Main Power Input

- **XT90 connector** for 60V battery (54-63V operating range)
- 50A continuous capability, 100A peak (2 seconds)
- Reverse polarity protection: P-channel MOSFET (IRFP260N or similar)
- Main fuse: 60A blade fuse holder
- TVS diode: 75V, 3000W (P6KE75A)

### Buck Converter Stage 1: 60V → 12V

- Switching regulator: **LM5116** or **TPS54160** (60V+ input capable)
- Output: 12V @ 3A for logic and gate drivers
- Switching frequency: 300kHz
- Input bulk capacitor: 470µF/100V electrolytic
- Output capacitor: 220µF/25V + 10µF/25V ceramic
- Inductor: 47µH, 5A, shielded

### Buck Converter Stage 2: 12V → 5V

- Regulator: **LM2576-5.0** or equivalent
- Output: 5V @ 2A for sensors and peripherals
- Output capacitor: 220µF/16V + 10µF/16V

### LDO Stage: 5V → 3.3V

- Regulator: **AMS1117-3.3** or **LD1117V33**
- Output: 3.3V @ 1A for ESP32 and logic
- Output capacitor: 22µF/6.3V + 100nF ceramic

---

## Dual H-Bridge Motor Driver System

### Motor Driver Configuration (per motor - 2x total)

#### MOSFET H-Bridge

- High-side MOSFETs: 2x **IRFB4110** (100V, 180A, 3.7mΩ RDS(on))
- Low-side MOSFETs: 2x **IRFB4110**
- Heat sink mounting: TO-220 packages with thermal vias
- Total per H-bridge: 4 MOSFETs
- **Total MOSFETs on board: 8** (2 motors × 4 MOSFETs each)

#### Gate Drivers (per motor)

- Driver IC: **IR2110** or **MIC4605** (high/low side driver)
- Bootstrap capacitor: 10µF/25V
- Bootstrap diode: UF4007 fast recovery
- Gate resistors: 10Ω per MOSFET gate
- Supply: 12V from buck converter

#### Current Sensing (per motor)

- Shunt resistor: **0.001Ω, 5W, 1%** (Kelvin connection)
- Amplifier: **INA228** (I2C, 85V common-mode)
- I2C address: Left motor **0x41**, Right motor **0x44**
- Filtering: 100nF ceramic + 10µF tantalum

#### PWM Control (ESP32-S3)

- Left motor PWM: **GPIO 10** (ESP32-S3 Pin 15, LEDC Channel 0, 10kHz, 8-bit)
- Left motor DIR: **GPIO 12** (ESP32-S3 Pin 17)
- Right motor PWM: **GPIO 11** (ESP32-S3 Pin 16, LEDC Channel 1, 10kHz, 8-bit)
- Right motor DIR: **GPIO 13** (ESP32-S3 Pin 18)
- Logic level to gate driver: **74HC14** schmitt trigger buffer
- Series resistors: 220Ω between ESP32-S3 and gate drivers

#### Motor Protection

- Flyback diodes: Integrated MOSFET body diodes + external Schottky (**MBRF20200CT**, 200V 20A)
- Overcurrent: Software-based via INA228 monitoring
- Thermal monitoring: NTC thermistors on heatsink (10kΩ @ 25°C)
- Emergency shutdown: GPIO-controlled enable pin on gate drivers

---

## Battery and System Monitoring

### Battery Current/Voltage Monitor

- **INA228** at I2C address **0x45**
- Shunt: **0.0005Ω, 10W** (50A continuous, 100A peak)
- Measures total battery current (charge/discharge for regen)
- Voltage sensing: Direct 60V input to INA228 (85V rated)

### Power Distribution

- High-current traces: **5mm (200mil) width, 2oz copper**
- Ground plane: Full pour on bottom layer, 2oz copper
- Thermal vias: 0.5mm diameter, array under MOSFETs and shunts
- Motor output terminals: **10mm screw terminals** or **XT90 connectors**

---

## User Interface

### EC11 Rotary Encoder

- CLK: **GPIO 4** (ESP32-S3 Pin 7, 10kΩ pull-up, 100nF debounce)
- DT: **GPIO 5** (ESP32-S3 Pin 8, 10kΩ pull-up, 100nF debounce)
- BTN: **GPIO 6** (ESP32-S3 Pin 9, 10kΩ pull-up, 100nF debounce)
- 5-pin right-angle header

### Shift Buttons

- Button A (Shift UP): **GPIO 7** (ESP32-S3 Pin 12, 10kΩ pull-up, 100nF debounce)
- Button B (Shift DOWN): **GPIO 15** (ESP32-S3 Pin 21, 10kΩ pull-up, 100nF debounce)
- 3-pin headers per button (Signal, GND, optional LED+)

### 128x64 OLED Display (SSD1306)

- I2C address: **0x3C**
- SDA: **GPIO 8** (ESP32-S3 Pin 13, 4.7kΩ pull-up)
- SCL: **GPIO 9** (ESP32-S3 Pin 14, 4.7kΩ pull-up)
- 4-pin header: VCC (3.3V), GND, SDA, SCL

---

## Sensors and Inputs

### GPS Module

- UART interface: RX **GPIO 17** (ESP32-S3 Pin 23), TX **GPIO 18** (ESP32-S3 Pin 24)
- 4-pin header: VCC (3.3V/5V), GND, TX, RX
- Baud: 9600 (NMEA format)
- Use UART1 (Serial1)

### Throttle Pedal (0-5V analog)

- **GPIO 1** (ESP32-S3 Pin 4, ADC1_CH0) - WiFi compatible, improved ADC accuracy
- Voltage divider: 10kΩ + 16kΩ (scales 5V to 3.1V max)
- Low-pass filter: 10kΩ + 100nF RC filter
- ESD protection: TVS diode (5.6V)
- 12-bit resolution (0-4095) with better linearity than ESP32
- 3-pin screw terminal: 5V, Signal, GND

### Key Switch (Ignition)

- **GPIO 16** (ESP32-S3 Pin 22, 10kΩ pull-up)
- Optocoupler isolated: **4N35**
- 2-pin screw terminal

---

## Lighting Control System

### PCA9685 16-Channel PWM Driver

- I2C address: **0x40**
- SDA/SCL: Shared with display (GPIO 8/9 on ESP32-S3)
- Output frequency: 1kHz for LEDs
- 16 outputs via N-channel MOSFETs (IRLZ44N or similar)

### MOSFET Outputs (12V switching)

- **Channels 0-3**: Four headlights (2A each)
- **Channel 4**: Center light (5A)
- **Channels 5-6**: Tail lights (2A each)
- **Channels 8-9**: Turn signals (2A each)
- **Channel 10**: Brake lights (3A)
- **Channel 11**: Reverse lights (2A)

### Per-Channel MOSFET Driver

- N-channel MOSFET: **IRLZ44N** (55V, 47A, logic-level)
- Gate resistor: 100Ω
- Flyback diode: 1N4007
- Screw terminals: Phoenix Contact style, 2-pin per channel

---

## Physical Layout and Thermal Management

### Board Dimensions

- Size: **150mm x 100mm** (allows proper spacing for high-current)
- Layers: **4-layer PCB** (2oz copper on power layers)
- Layer stack: Signal/Ground/Power/Signal

### Thermal Design

- Motor MOSFET heatsink: Shared aluminum extrusion, **100mm x 50mm**
- Thermal interface: Silicone thermal pads
- Thermal vias: Grid array (0.5mm, 1mm pitch) under MOSFETs
- NTC thermistor: Mounted on heatsink, connected to ADC
- Forced air cooling recommendation: 40mm fan (12V, GPIO-controlled)

### Component Placement

- **Power section**: Left side (XT90 input, MOSFETs, shunts, caps)
- **Logic section**: Right side (ESP32, sensors, USB-C)
- **I/O connectors**: Edge placement for cable access
- **High-current path**: Straight traces, minimal vias
- **MOSFETs**: Arranged for optimal heatsink contact

---

## Connectors Summary

### Power

- **1x XT90**: Main battery input (60V)
- **2x 10mm screw terminal**: Motor outputs (Left/Right)

### User Interface

- **1x USB-C**: Programming and debug
- **1x 5-pin header**: EC11 encoder
- **2x 3-pin header**: Shift buttons A & B
- **1x 4-pin header**: OLED display

### Sensors

- **1x 4-pin header**: GPS module
- **1x 3-pin screw terminal**: Throttle pedal
- **1x 2-pin screw terminal**: Brake switch
- **1x 2-pin screw terminal**: Key switch

### Lighting (16x 2-pin screw terminals)

- **Channels 0-3**: Headlights
- **Channel 4**: Center light
- **Channels 5-6**: Tail lights
- **Channels 8-9**: Turn signals
- **Channel 10**: Brake lights
- **Channel 11**: Reverse lights

---

## Protection and Safety

### Electrical Protection

- Main fuse: **60A blade fuse**
- Per-motor fuses: **25A blade fuses**
- Reverse polarity: P-MOSFET (automatic)
- Overvoltage: **75V TVS** on battery input
- Overcurrent: INA228 monitoring + software shutdown
- Thermal: NTC + software shutdown at **85°C**

### Isolation

- Optocoupler: Brake input isolated from logic
- Current sensing: Kelvin shunt connections
- Gate driver: Isolated high-side drive via bootstrap

### ESD Protection

- TVS diodes on all external connections
- ESD suppressors on USB-C (**USBLC6**)
- Spark gaps on motor outputs (optional)

---

## Additional Features

### LED Indicators

- **Power (green)**: 3.3V rail active
- **WiFi (blue)**: GPIO 2 (ESP32-S3 Pin 5)
- **Motor L (yellow)**: PWM activity monitor
- **Motor R (yellow)**: PWM activity monitor
- **Fault (red)**: GPIO 3 (ESP32-S3 Pin 6) - optional status LED

### Test Points

- Battery voltage
- 12V rail
- 5V rail
- 3.3V rail
- Motor PWM signals
- I2C bus (SDA, SCL)
- Current shunt voltages

### Mounting

- **8x M4 mounting holes** with keepout zones
- Anti-vibration: Use with rubber grommets
- Enclosure-ready: Fits 160mm x 110mm box

### Silkscreen

- Voltage warnings: **"DANGER 60V"**
- Polarity marks on all connectors
- Pin functions labeled
- Component values for critical parts
- Board name: **"Razors-Edge Controller v2.0"**
- **NerdBillyFab** logo

---

## Design Notes

### Critical Specifications

- Motor outputs: **60V, 20A continuous per motor**
- Regenerative braking capable (current flows both ways)
- Total board current: **50A nominal, 100A peak (2 sec)**
- Operating temperature: **-20°C to +70°C** (with heatsink)
- Minimum PWM frequency: **10kHz** (reduces motor noise)

### Assembly Notes

- Hand-solderable through-hole for high-power (MOSFETs, terminals)
- SMD for logic components (0805 minimum)
- Conformal coating recommended for outdoor use
- Heatsink: Must make contact with all 8 motor MOSFETs

### Firmware Compatibility

- All GPIO assignments updated for ESP32-S3-WROOM-1 (see config.h)
- Pin-compatible with existing Razors-Edge firmware (Arduino framework)
- I2C addresses pre-configured (0x3C, 0x40, 0x41, 0x44, 0x45)
- PWM frequency: 10kHz via ESP32-S3 LEDC (8-bit resolution)
- Dual-core FreeRTOS task architecture fully supported
- Improved ADC accuracy for throttle safety (12-bit with better linearity)
- Migration guide: See ESP32-S3_PINOUT_MAPPING.md

---

## Complete GPIO Pin Mapping (ESP32-S3-WROOM-1)

```
GPIO 1  (Pin 4)  → Throttle ADC (ADC1_CH0, 12-bit, 0-3.1V)
GPIO 2  (Pin 5)  → WiFi LED (blue) - optional status indicator
GPIO 3  (Pin 6)  → Fault LED (red) - optional status indicator
GPIO 4  (Pin 7)  → EC11 Encoder CLK (Track A)
GPIO 5  (Pin 8)  → EC11 Encoder DT (Track B)
GPIO 6  (Pin 9)  → EC11 Encoder Button
GPIO 7  (Pin 12) → Shift Button A (UP)
GPIO 8  (Pin 13) → I2C SDA (OLED, PCA9685, INA228s)
GPIO 9  (Pin 14) → I2C SCL (OLED, PCA9685, INA228s)
GPIO 10 (Pin 15) → Left Motor PWM (LEDC Channel 0, 10kHz)
GPIO 11 (Pin 16) → Right Motor PWM (LEDC Channel 1, 10kHz)
GPIO 12 (Pin 17) → Left Motor Direction
GPIO 13 (Pin 18) → Right Motor Direction
GPIO 15 (Pin 21) → Shift Button B (DOWN)
GPIO 16 (Pin 22) → Key Switch (Ignition)
GPIO 17 (Pin 23) → GPS RX (ESP32 receives from GPS TX)
GPIO 18 (Pin 24) → GPS TX (ESP32 transmits to GPS RX)
GPIO 21 (Pin 27) → Expansion / Future Use
GPIO 43 (Pin 31) → UART0 TX (USB programming via CH340C)
GPIO 44 (Pin 32) → UART0 RX (USB programming via CH340C)
GPIO 0  (Pin 3)  → Boot Button (strapping pin, 10kΩ pull-up)
GPIO 47 (Pin 35) → Expansion / Future Use
GPIO 48 (Pin 36) → Expansion / Future Use
```

---

## I2C Device Addresses

```
0x3C → OLED Display (SSD1306)
0x40 → PCA9685 LED/Light Driver
0x41 → Left Motor Current Monitor (INA228)
0x44 → Right Motor Current Monitor (INA228)
0x45 → Battery Current/Voltage Monitor (INA228)
```

---

## Bill of Materials Highlights

### Critical Components

- **8x IRFB4110** - N-channel MOSFETs (motor H-bridges)
- **2x IR2110** - Gate drivers
- **3x INA228** - Current/voltage monitors
- **1x PCA9685** - 16-channel PWM driver
- **1x ESP32-S3-WROOM-1-N8R2** - Main processor (8MB Flash, 2MB PSRAM)
- **1x CH340C** - USB-to-UART bridge
- **1x LM5116** or **TPS54160** - 60V→12V buck converter
- **1x LM2576-5.0** - 12V→5V buck converter
- **1x AMS1117-3.3** - 5V→3.3V LDO
- **3x 0.001Ω 5W** - Motor current shunts
- **1x 0.0005Ω 10W** - Battery current shunt
- **2x 74HC14** - Schmitt trigger buffers
- **1x USB-C** connector (16-pin)
- **1x XT90** - Main power connector
- **2x XT90** or **10mm terminals** - Motor outputs
- **Heatsink**: 100mm × 50mm aluminum extrusion

### Protection Components

- **1x P6KE75A** - 75V TVS diode (battery)
- **1x IRFP260N** - P-channel MOSFET (reverse polarity)
- **2x MBRF20200CT** - Schottky diodes (flyback)
- **Multiple 1N4007** - Flyback diodes for lights
- **1x 60A blade fuse** - Main protection
- **2x 25A blade fuses** - Per-motor protection

---

## Feature Summary

This is a **complete, production-ready motor controller** that integrates:

✅ Dual 60V/20A H-bridge motor drivers
✅ Regenerative braking capability
✅ Precise current and voltage monitoring
✅ 16-channel lighting control
✅ GPS navigation
✅ WiFi connectivity
✅ OLED display interface
✅ Rotary encoder + buttons
✅ Throttle pedal input
✅ Comprehensive safety features
✅ All on a single PCB!

---

**Generated for**: NerdBillyFab Razors-Edge Project
**Firmware**: https://github.com/jsligar/Razors-Edge
**Design Tool**: flux.ai PCB Designer
**Board Version**: 2.0 (All-in-One Controller)
