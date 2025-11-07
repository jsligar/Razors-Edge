# Hardware Testing Procedures - Razors Edge

> **⚠️ CRITICAL SAFETY NOTICE**  
> This is a 60V high-voltage system. Follow ALL procedures exactly.  
> Any deviation can cause serious injury, equipment damage, or death.

## 🔧 Pre-Testing Safety Checklist

### **Before ANY Power-On:**
- [ ] **Verify key switch has 10kΩ pull-down resistor on GPIO 15**
- [ ] **Check all ground connections are secure**
- [ ] **Confirm motor wires are NOT connected to MDD20A**
- [ ] **Verify 60V main battery is DISCONNECTED**
- [ ] **Ensure proper PPE: safety glasses, insulated tools**
- [ ] **Clear work area of conductive materials**

### **Initial Power Setup:**
- [ ] **Connect ONLY 5V power to ESP32 (USB or external 5V)**
- [ ] **Do NOT connect 12V or 30V supplies yet**
- [ ] **Keep 60V battery disconnected until final testing**

## 🧪 Phase 1: ESP32 Basic Function Test

### **Step 1.1: Bare ESP32 Test**
```bash
# Upload basic firmware
pio run --target upload

# Monitor serial output
pio device monitor
```

**Expected Results:**
- ESP32 boots successfully
- Serial output shows "Initializing..."
- No random resets or boot loops
- WiFi MAC address displays

**Failure Actions:**
- Check GPIO 15 pull-down resistor
- Verify 5V power supply stability
- Remove all external connections

### **Step 1.2: I2C Bus Scan**
**Connect ONLY:** SDA (GPIO 21), SCL (GPIO 22), 3.3V, GND

**Expected Results:**
```
I2C Device Scan:
No devices found at this stage (normal)
```

**Success Criteria:**
- No I2C errors
- Bus scan completes without hanging
- ESP32 remains stable

## 🔌 Phase 2: I2C Device Testing

### **Step 2.1: Add OLED Display**
**Connect:** SSD1306 OLED (3.3V, GND, SDA, SCL)

**Test Commands:**
```bash
# Monitor for OLED initialization
pio device monitor
```

**Expected Results:**
```
✓ SSD1306 OLED initialized
Display shows: "RAZOR 60V Initializing..."
```

### **Step 2.2: Add Current Sensors (ONE AT A TIME)**

**Connect Battery Monitor First:** INA228 at 0x40
- **Power:** 3.3V, GND
- **I2C:** SDA, SCL  
- **Sense:** 0.001Ω shunt (NO CURRENT FLOW YET)

**Expected Results:**
```
✓ Battery Monitor (INA228) found at 0x40
Voltage: ~0.0V (normal, no power connected)
Current: ~0.0A (normal, no current flow)
```

**Add Left Motor Monitor:** INA228 at 0x41
**Add Right Motor Monitor:** INA228 at 0x44

**Success Criteria:**
- All 3 INA228 sensors detected
- Voltage readings near zero
- No I2C communication errors

### **Step 2.3: Add PCA9685 Light Controller**
**Connect:** PCA9685 at 0x70

**Expected Results:**
```
✓ Light Controller (PCA9685) found at 0x70
LED channels initialized
```

### **Step 2.4: Complete I2C Scan**
**Expected Final Results:**
```
I2C Device Scan Complete:
0x3C: SSD1306 OLED ✓
0x40: Battery Monitor (INA228) ✓  
0x41: Left Motor Monitor (INA228) ✓
0x44: Right Motor Monitor (INA228) ✓
0x70: Light Controller (PCA9685) ✓
All 5 devices found!
```

## 📟 Phase 3: Input/Output Testing

### **Step 3.1: Input Testing**
**Connect:** Potentiometer (pedal), buttons, encoder

**Test Procedure:**
1. Move pedal - watch ADC values change
2. Press buttons - verify debouncing
3. Turn encoder - check direction detection

**Expected Results:**
```
Pedal ADC: 0-4095 range
Buttons: Clean press/release
Encoder: Directional counting
```

### **Step 3.2: GPS Module**
**Connect:** GT-U7 GPS (3.3V, GND, TX→GPIO16)

**Expected Results:**
```
GPS module initialized
Searching for satellites...
(May take 1-5 minutes outdoors)
```

## ⚡ Phase 4: Low-Voltage Power Testing

### **Step 4.1: Add 12V Supply**
**⚠️ DANGER:** Verify polarity before connection

**Connect:** 12V supply to system (NOT motors)
- Powers: PCA9685, any 12V accessories

**Expected Results:**
- System remains stable
- No voltage spikes on 3.3V/5V rails
- PCA9685 LED outputs functional

### **Step 4.2: Add 30V Supply** 
**⚠️ HIGH VOLTAGE:** Use extreme caution

**Connect:** 30V supply to MDD20A driver ONLY
- **DO NOT CONNECT MOTORS YET**

**Expected Results:**
- MDD20A power LED illuminates
- No error LEDs on MDD20A
- ESP32 remains stable and communicating

## 🏁 Phase 5: Motor Testing (Wheels Off Ground)

### **⚠️ EXTREME CAUTION REQUIRED ⚠️**

### **Step 5.1: Connect ONE Motor**
**Safety Setup:**
- Vehicle on jack stands - wheels OFF ground
- Emergency stop within reach
- Connect LEFT motor only

**Initial Test:**
1. Set gear to "1st" 
2. Apply 10% pedal input
3. Verify motor spins correctly
4. Test emergency stop

**Expected Results:**
- Smooth motor acceleration
- No excessive current spikes
- Emergency stop works immediately

### **Step 5.2: Connect BOTH Motors**
**Repeat test with both motors:**
- Monitor current balance
- Check for vibration/noise
- Verify both motors start together

### **Step 5.3: Progressive Speed Testing**
**Test all gears at LOW speeds:**
- 1st gear: 20% max pedal
- 2nd gear: 30% max pedal  
- 3rd gear: 40% max pedal
- Eco gear: 30% max pedal
- **AVOID Sport+ until full testing**

## 🔋 Phase 6: Full System Integration

### **Step 6.1: Add 60V Battery**
**⚠️ LETHAL VOLTAGE - EXTREME CARE**

**Pre-Connection:**
- Verify all connections secure
- Check key switch functionality
- Confirm emergency stop accessible

**Initial Test:**
- Connect 60V battery
- Verify voltage readings on display
- Test low-speed operation

### **Step 6.2: Full Feature Testing**
- All gears including Sport+ (briefly)
- GPS tracking
- Display cycling
- Light control
- Safety fault detection

## 🛑 Emergency Procedures

### **Immediate Shutdown:**
1. **Turn key switch OFF**
2. **Disconnect 60V battery**
3. **Wait 30 seconds for capacitor discharge**

### **If Motor Runs Away:**
1. **Emergency stop button**
2. **Key switch OFF**  
3. **Disconnect 60V battery**
4. **Do NOT touch motors until stopped**

### **Fault Diagnosis:**
- Monitor serial output for fault codes
- Check OLED display for warnings
- Verify current readings within limits

## ✅ Test Completion Checklist

- [ ] All I2C devices detected and functional
- [ ] Motor control smooth and responsive
- [ ] Safety systems trigger correctly
- [ ] Display shows accurate telemetry
- [ ] Emergency stop works in all conditions
- [ ] No excessive heating in any component
- [ ] GPS acquires satellite lock
- [ ] All gear modes function properly

**⚠️ ONLY proceed to road testing after ALL phases pass completely!**

---

**Remember: Safety is paramount. When in doubt, power down and reassess.**