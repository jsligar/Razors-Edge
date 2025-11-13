# ESP32-S3-WROOM-1 GPIO Pinout for Razors-Edge Motor Controller

## Chip Specifications
- **Module**: ESP32-S3-WROOM-1 (N8R2 variant recommended)
- **Flash**: 8MB
- **PSRAM**: 2MB (optional but recommended)
- **Package**: 41-pin SMD module
- **CPU**: Dual-core Xtensa LX7 @ 240MHz
- **ADC**: Two 12-bit SAR ADCs (improved linearity over ESP32)
- **PWM**: 8x LEDC channels (up to 14-bit resolution)

## Pin Restrictions and Guidelines

### Strapping Pins (Use with caution):
- **GPIO0**: Boot mode selection (pulled up internally)
- **GPIO3**: JTAG enable (pulled up internally)
- **GPIO45**: VDD_SPI voltage (pulled down internally)
- **GPIO46**: ROM messages enable (pulled down internally)

### Reserved/Special Function Pins:
- **GPIO19, GPIO20**: USB D-, D+ (avoid if using USB)
- **GPIO39-42**: JTAG pins (avoid for production, OK for dev)
- **GPIO43, GPIO44**: UART0 TXD/RXD (USB serial bridge)

### Input-Only Pins:
- None on ESP32-S3 (all GPIO can be input/output)

### ADC Channels:
- **ADC1**: GPIO1-10 (channels 0-9) - **Safe with WiFi**
- **ADC2**: GPIO11-20 (channels 0-9) - **Conflicts with WiFi**

### Recommended Safe Pins for Critical Functions:
- GPIO4-18 (excluding USB pins 19-20)
- GPIO21, GPIO47, GPIO48

---

## Complete GPIO Pin Mapping for Razors-Edge

### Power Supply Pins
```
Pin 1:  GND         → Ground (connect to ground plane)
Pin 2:  3V3         → 3.3V power supply (300mA min, 500mA recommended)
Pin 41: GND         → Ground (connect to ground plane)
Pin 40: GND         → Ground (connect to ground plane)
Pin 38: EN          → Enable (pulled up with 10kΩ, add 0.1µF cap to GND)
```

### I2C Bus (Shared by 5 devices)
```
GPIO8  (Pin 13)     → I2C SDA    (4.7kΩ pull-up to 3.3V)
GPIO9  (Pin 14)     → I2C SCL    (4.7kΩ pull-up to 3.3V)
```
**I2C Devices:**
- 0x3C: SSD1306 OLED Display
- 0x40: PCA9685 16-Channel LED Driver
- 0x41: INA228 Motor Left Current Monitor
- 0x44: INA228 Motor Right Current Monitor
- 0x45: INA228 Battery Monitor (MATEKSYS INA-BM)

### Motor Control (PWM + Direction)
```
GPIO10 (Pin 15)     → Motor Left PWM      (10kHz, 8-bit, LEDC Channel 0)
GPIO11 (Pin 16)     → Motor Right PWM     (10kHz, 8-bit, LEDC Channel 1)
GPIO12 (Pin 17)     → Motor Left Direction (GPIO output)
GPIO13 (Pin 18)     → Motor Right Direction (GPIO output)
```
**Notes:**
- PWM frequency: 10kHz (configurable up to 40MHz)
- Direction pins control H-bridge direction (HIGH=forward, LOW=reverse)
- Add 220Ω series resistors to gate driver inputs

### Analog Inputs (ADC1 - Safe with WiFi)
```
GPIO1  (Pin 4)      → Throttle Pedal (ADC1_CH0, 12-bit, 0-3.3V)
GPIO2  (Pin 5)      → Brake Sensor (ADC1_CH1, 12-bit, 0-3.3V, future use)
```
**ADC Configuration:**
- Resolution: 12-bit (0-4095)
- Attenuation: 11dB (0-3.1V range)
- Use ADC1 channels ONLY (ADC2 conflicts with WiFi)
- Add 100nF ceramic cap + 10kΩ resistor RC filter at ADC input
- Voltage divider for throttle: 0-5V → 0-3.1V (use 10kΩ + 16kΩ)

### User Interface - Rotary Encoder
```
GPIO4  (Pin 7)      → Encoder CLK (Track A/TRA)
GPIO5  (Pin 8)      → Encoder DT (Track B/TRB)
GPIO6  (Pin 9)      → Encoder Button (SW)
```
**Hardware:**
- Use 10kΩ pull-up resistors on all encoder pins
- Add 100nF debounce capacitors to GND
- Encoder grounds through common GND

### User Interface - Push Buttons
```
GPIO7  (Pin 12)     → KEY0 - Shift Up/Confirm (Button A)
GPIO15 (Pin 21)     → KEY1 - Shift Down/Back (Button B)
GPIO16 (Pin 22)     → Key Switch (Ignition/Enable)
```
**Hardware:**
- Active LOW with 10kΩ pull-up resistors to 3.3V
- Add 100nF debounce capacitors to GND
- Software debounce: 150ms for gear shifts, 50ms for encoder

### GPS Module (UART)
```
GPIO17 (Pin 23)     → GPS TX (ESP32 RX - receive from GPS)
GPIO18 (Pin 24)     → GPS RX (ESP32 TX - transmit to GPS)
```
**UART Configuration:**
- Baud rate: 9600 (standard NMEA)
- Protocol: 8N1 (8 data bits, no parity, 1 stop bit)
- Use UART1 (Serial1 in Arduino)
- GPS module: NEO-6M or similar NMEA-compatible

### Programming/Debug Pins
```
GPIO43 (Pin 31)     → UART0 TXD (USB serial - programming)
GPIO44 (Pin 32)     → UART0 RXD (USB serial - programming)
GPIO19 (Pin 25)     → USB D- (optional, for native USB)
GPIO20 (Pin 26)     → USB D+ (optional, for native USB)
```
**Notes:**
- GPIO43/44 automatically used by USB-to-UART bridge chip
- GPIO19/20 optional for USB OTG (not required if using UART bridge)
- Add USB-C connector with CH340C or CP2102N UART bridge

### Boot/Flash Mode Control
```
GPIO0  (Pin 3)      → Boot Button (strapping pin)
EN     (Pin 38)     → Reset Button (active LOW)
```
**Boot Mode:**
- GPIO0 LOW during reset → Download/Flash mode
- GPIO0 HIGH during reset → Normal boot mode
- Add 10kΩ pull-up on GPIO0, momentary button to GND
- Add 10kΩ pull-up + 0.1µF cap on EN

### Future Expansion Pins (Available for additional features)
```
GPIO21 (Pin 27)     → Available (good general-purpose pin)
GPIO47 (Pin 35)     → Available (good general-purpose pin)
GPIO48 (Pin 36)     → Available (good general-purpose pin)
```

### Do Not Use (Reserved)
```
GPIO45 (Pin 33)     → VDD_SPI strapping pin
GPIO46 (Pin 34)     → ROM messages strapping pin
GPIO39-42           → JTAG (if debug needed)
```

---

## Pin Summary Table

| GPIO | Pin # | Function | Direction | Special Notes |
|------|-------|----------|-----------|---------------|
| 1 | 4 | Throttle Pedal | Input (ADC1_CH0) | 12-bit ADC, 0-3.1V |
| 2 | 5 | Brake Sensor | Input (ADC1_CH1) | 12-bit ADC, future use |
| 4 | 7 | Encoder CLK | Input | 10kΩ pull-up, 100nF cap |
| 5 | 8 | Encoder DT | Input | 10kΩ pull-up, 100nF cap |
| 6 | 9 | Encoder Button | Input | 10kΩ pull-up, 100nF cap |
| 7 | 12 | KEY0 Button | Input | 10kΩ pull-up, active LOW |
| 8 | 13 | I2C SDA | I/O | 4.7kΩ pull-up |
| 9 | 14 | I2C SCL | I/O | 4.7kΩ pull-up |
| 10 | 15 | Motor Left PWM | Output (LEDC0) | 10kHz, 8-bit PWM |
| 11 | 16 | Motor Right PWM | Output (LEDC1) | 10kHz, 8-bit PWM |
| 12 | 17 | Motor Left Dir | Output | H-bridge direction |
| 13 | 18 | Motor Right Dir | Output | H-bridge direction |
| 15 | 21 | KEY1 Button | Input | 10kΩ pull-up, active LOW |
| 16 | 22 | Key Switch | Input | 10kΩ pull-up, active LOW |
| 17 | 23 | GPS TX → ESP RX | Input (UART1_RX) | 9600 baud |
| 18 | 24 | ESP TX → GPS RX | Output (UART1_TX) | 9600 baud |
| 21 | 27 | Expansion | I/O | Future use |
| 43 | 31 | UART0 TX (USB) | Output | Programming/debug |
| 44 | 32 | UART0 RX (USB) | Input | Programming/debug |
| 47 | 35 | Expansion | I/O | Future use |
| 48 | 36 | Expansion | I/O | Future use |
| 0 | 3 | Boot Button | Input | Strapping pin |
| EN | 38 | Reset Button | Input | Active LOW |

---

## Power Supply Design

### Main Power Rails
```
60V Battery Input
    ↓
[Buck Converter 1] → 12V @ 3A (motor gate drivers, GPS, relays)
    ↓
[Buck Converter 2] → 5V @ 2A (USB power, logic level conversion)
    ↓
[LDO Regulator] → 3.3V @ 500mA (ESP32-S3, I2C devices, ADC reference)
```

### ESP32-S3 Power Pins
```
Pin 1:  GND         (connect to ground plane)
Pin 2:  3V3         (from LDO, add 10µF + 0.1µF bypass caps)
Pin 40: GND         (connect to ground plane)
Pin 41: GND         (connect to ground plane)
```

### Decoupling Capacitors (Critical!)
Place near ESP32-S3 module:
- 1x 10µF ceramic (X7R) on 3V3 pin
- 2x 0.1µF ceramic (X7R) on 3V3 pin
- 1x 10µF tantalum bulk capacitor
- All within 5mm of power pins

---

## PCB Layout Guidelines for Flux.ai

### Layer Stack (4-layer board recommended):
```
Layer 1 (Top):     Component placement, signal traces, I2C bus
Layer 2 (Inner):   Ground plane (solid pour, no splits)
Layer 3 (Inner):   3.3V power plane + 5V/12V islands
Layer 4 (Bottom):  High-current traces, motor PWM, additional GND
```

### Critical Trace Guidelines:

**High-Speed I2C Bus (8MHz capable):**
- Trace width: 0.25mm (10 mil)
- Keep SDA/SCL parallel, same length ±5mm
- Max length: 150mm
- Route away from PWM signals
- 4.7kΩ pull-ups near ESP32-S3

**Motor PWM (10kHz, high current switching):**
- Trace width: 1.0mm (40 mil) minimum
- Keep short as possible (<50mm to gate drivers)
- Route on bottom layer with ground plane above
- Add 220Ω series resistor + ferrite bead
- Separate from ADC traces by >10mm

**ADC Throttle Input (noise-sensitive):**
- Trace width: 0.3mm (12 mil)
- Guard with GND traces on both sides
- Keep away from PWM, motor drivers, WiFi antenna
- Add RC filter at ADC pin: 10kΩ + 100nF
- Route on top layer, GND plane below

**WiFi Antenna:**
- Keep area clear: 15mm × 5mm keepout zone
- No ground plane under antenna trace
- No traces crossing antenna feed
- Position antenna away from motor drivers
- Orient toward outside edge of board

**USB Data Lines (D+/D- if used):**
- Differential pair, 90Ω impedance
- Trace width: 0.3mm, spacing: 0.15mm
- Same length ±0.5mm
- Route away from switching power supplies

### Component Placement:
1. **ESP32-S3**: Center of board, away from high-voltage sections
2. **OLED Display**: Top edge, I2C traces <100mm
3. **INA228 Sensors**: Near shunt resistors (Kelvin connections)
4. **PCA9685**: Near LED output terminal blocks
5. **Power Supplies**: Input side, heat away from ESP32
6. **Motor Drivers**: Opposite side from ESP32, good thermal path

### Ground Strategy:
- **Single-point ground architecture**
- Heavy ground plane on Layer 2 (2oz copper)
- Star-ground point near battery negative terminal
- Separate analog ground (AGND) for ADC reference
- Join AGND to DGND with ferrite bead at star point

---

## Firmware Pin Configuration (Arduino Framework)

```cpp
// ESP32-S3-WROOM-1 Pin Definitions
#define GPIO_SDA                8
#define GPIO_SCL                9

#define GPIO_MOTOR_PWM_LEFT     10
#define GPIO_MOTOR_PWM_RIGHT    11
#define GPIO_MOTOR_DIR_LEFT     12
#define GPIO_MOTOR_DIR_RIGHT    13

#define GPIO_PEDAL_ADC          1   // ADC1_CH0
#define GPIO_BRAKE_ADC          2   // ADC1_CH1 (future)
#define GPIO_KEY_SWITCH         16

#define GPIO_ENCODER_CLK        4
#define GPIO_ENCODER_DT         5
#define GPIO_ENCODER_BTN        6
#define GPIO_KEY0               7
#define GPIO_KEY1               15

#define GPIO_GPS_RX             17  // GPS TX → ESP RX
#define GPIO_GPS_TX             18  // ESP TX → GPS RX

// ADC Configuration
#define ADC_RESOLUTION          12  // 12-bit (0-4095)
#define ADC_ATTENUATION         ADC_11db  // 0-3.1V range
```

### Arduino Setup Code:
```cpp
void setup() {
    // Configure ADC for throttle pedal (improved accuracy on S3)
    analogReadResolution(12);  // 12-bit resolution
    analogSetAttenuation(ADC_11db);  // 0-3.1V range
    adcAttachPin(GPIO_PEDAL_ADC);

    // Configure PWM for motors
    ledcSetup(0, 10000, 8);  // Channel 0, 10kHz, 8-bit
    ledcSetup(1, 10000, 8);  // Channel 1, 10kHz, 8-bit
    ledcAttachPin(GPIO_MOTOR_PWM_LEFT, 0);
    ledcAttachPin(GPIO_MOTOR_PWM_RIGHT, 1);

    // Configure direction pins
    pinMode(GPIO_MOTOR_DIR_LEFT, OUTPUT);
    pinMode(GPIO_MOTOR_DIR_RIGHT, OUTPUT);

    // Configure I2C
    Wire.begin(GPIO_SDA, GPIO_SCL, 400000);  // 400kHz I2C

    // Configure UART for GPS
    Serial1.begin(9600, SERIAL_8N1, GPIO_GPS_RX, GPIO_GPS_TX);
}
```

---

## Migration Notes from ESP32-WROOM-32

### Pin Changes Required:
| Old ESP32 Pin | New ESP32-S3 Pin | Function |
|---------------|------------------|----------|
| GPIO21 → GPIO8 | I2C SDA |
| GPIO22 → GPIO9 | I2C SCL |
| GPIO18 → GPIO10 | Motor Left PWM |
| GPIO12 → GPIO11 | Motor Right PWM |
| GPIO19 → GPIO12 | Motor Left Dir |
| GPIO23 → GPIO13 | Motor Right Dir |
| GPIO36 → GPIO1 | Throttle ADC (ADC1_CH0) |
| GPIO25 → GPIO4 | Encoder CLK |
| GPIO26 → GPIO5 | Encoder DT |
| GPIO27 → GPIO6 | Encoder Button |
| GPIO14 → GPIO7 | KEY0 Button |
| GPIO13 → GPIO15 | KEY1 Button |
| GPIO15 → GPIO16 | Key Switch |
| GPIO16 → GPIO17 | GPS RX |
| GPIO17 → GPIO18 | GPS TX |

### Code Changes Required:
1. Update all `#define GPIO_*` definitions in `config.h`
2. Test ADC calibration (S3 has better linearity - may need recalibration)
3. Update PWM channel configuration (syntax is the same)
4. Verify I2C speed at 400kHz (S3 handles this better)
5. No other code changes needed (Arduino API compatible)

### Benefits of ESP32-S3:
- ✅ **Improved ADC accuracy** for safety-critical throttle sensing
- ✅ **Better WiFi performance** with less current spikes
- ✅ **Faster CPU** (240MHz Xtensa LX7 vs 160MHz LX6)
- ✅ **More GPIO** (45 vs 34) for future expansion
- ✅ **Better USB support** for debugging and programming
- ✅ **Pin-compatible firmware** with minimal changes

---

## Testing and Validation Checklist

### Power-Up Sequence:
1. ☐ Measure 3.3V rail stability (±50mV max ripple)
2. ☐ Verify all I2C devices detected (5 devices at correct addresses)
3. ☐ Test encoder rotation and button press
4. ☐ Verify GPIO buttons (KEY0, KEY1, Key Switch)
5. ☐ Validate ADC readings (throttle pedal full range)
6. ☐ Test GPS UART communication (NMEA sentences)
7. ☐ Verify PWM outputs with oscilloscope (10kHz, clean edges)
8. ☐ Test WiFi connectivity and web interface
9. ☐ Validate motor direction control (forward/reverse)
10. ☐ Full system integration test with motors under load

### ADC Calibration Procedure:
1. With throttle at 0%: Record ADC value (should be ~100-200)
2. With throttle at 100%: Record ADC value (should be ~3900-4095)
3. Update `PEDAL_ADC_MIN` and `PEDAL_ADC_MAX` in config.h
4. Test linearity at 25%, 50%, 75% throttle positions
5. Verify smooth response curve with no dead zones

---

## Bill of Materials (Key Components)

| Component | Part Number | Quantity | Notes |
|-----------|-------------|----------|-------|
| ESP32-S3-WROOM-1-N8R2 | Espressif | 1 | 8MB Flash, 2MB PSRAM |
| CH340C | WCH | 1 | USB-to-UART bridge |
| AMS1117-3.3 | AMS | 1 | 3.3V LDO regulator |
| 10µF Ceramic Cap (0805) | X7R | 4 | Power decoupling |
| 0.1µF Ceramic Cap (0603) | X7R | 10 | Bypass capacitors |
| 10kΩ Resistor (0603) | 1% | 10 | Pull-up resistors |
| 4.7kΩ Resistor (0603) | 1% | 2 | I2C pull-ups |
| Tactile Switch (6mm) | SMD | 2 | Boot + Reset buttons |
| USB-C Connector | SMD | 1 | Programming/power |

---

## Flux.ai Prompt Integration

**Include this exact pinout in your Flux.ai PCB design prompt:**

"Use ESP32-S3-WROOM-1-N8R2 module with the following GPIO mapping:
- I2C: SDA=GPIO8, SCL=GPIO9 (4.7kΩ pull-ups)
- Motor PWM: Left=GPIO10 (LEDC0), Right=GPIO11 (LEDC1) at 10kHz
- Motor Direction: Left=GPIO12, Right=GPIO13
- ADC (throttle): GPIO1 (ADC1_CH0) with RC filter
- Encoder: CLK=GPIO4, DT=GPIO5, BTN=GPIO6 (10kΩ pull-ups)
- Buttons: KEY0=GPIO7, KEY1=GPIO15, KeySwitch=GPIO16
- GPS UART: RX=GPIO17, TX=GPIO18
- Boot: GPIO0 with pull-up and button to GND
- Reset: EN pin with pull-up and button to GND
- USB: CH340C on GPIO43/44 (UART0)

Power: 3.3V @ 500mA from LDO, add 10µF + 2x0.1µF decoupling caps.
Use 4-layer board: Layer 1=signals, Layer 2=GND plane, Layer 3=3V3 plane, Layer 4=power traces."

---

**Document Version:** 1.0
**Last Updated:** 2025-11-13
**Compatible Firmware:** Razors-Edge v1.0+
**Target Board:** Custom motor controller PCB for flux.ai
