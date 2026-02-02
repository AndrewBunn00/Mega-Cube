# MEGA CUBE - Hardware Reference

Quick reference for hardware setup and troubleshooting.

## Core Components

- **Teensy 4.0** - 600 MHz ARM Cortex-M7 controller
- **4096 PL9823-F8** - Addressable RGB LEDs (**GRB order**, not RGB)
- **ILI9341** - 320×240 LCD with touch
- **ESP8266** - WiFi module (UART @ 460800 baud)
- **5V PSU** - 18A minimum (90W)

## Pin Connections

### LED Control (FlexIO2)
```
Pin 8  → DIN  (Serial data to shift registers)
Pin 9  → WCK  (Word clock/latch)
Pin 10 → BCK  (Bit clock)
```

### LCD (SPI)
```
Pin 11 → MOSI    Pin 15 → TFT_CS    Pin 20 → TFT_DC
Pin 12 → MISO    Pin 18 → TOUCH_CS  Pin 21 → TFT_RESET
Pin 13 → SCK     Pin 19 → TOUCH_IRQ Pin 22 → TFT_LED (backlight)
```

### ESP8266 (UART1)
```
Pin 0 (RX1) ← TX (ESP8266)
Pin 1 (TX1) → RX (ESP8266)
```

**Important**: ESP8266 is 3.3V logic. Teensy 4.0 pins are 3.3V tolerant - direct connection OK.

## Critical Setup Details

### PLL Configuration

Video PLL **must** be exactly 711.1 MHz for PL9823 timing:

```cpp
// 24 MHz × (29 + 1/3) / 4 = 711.1 MHz
CCM_ANALOG_PLL_VIDEO_DIV_SELECT(29);
CCM_ANALOG_PLL_VIDEO_NUM = 1;
CCM_ANALOG_PLL_VIDEO_DENOM = 3;
CCM_ANALOG_PLL_VIDEO_POST_DIV_SELECT(0);
```

Wrong frequency → flickering/corrupt LEDs.

### LED Color Order

**PL9823 uses GRB, not RGB!** Code handles this:

```cpp
// Software writes RGB, hardware outputs GRB
for (int bit = 7; bit >= 0; bit--) {
  channelBuffer[led++] = ((g >> bit) & 1) << 0 |  // Green first
                         ((r >> bit) & 1) << 1 |  // Then red
                         ((b >> bit) & 1) << 2;   // Then blue
}
```

## Power Distribution

### Budget Calculation

Each LED: **60 mA max** (full white)
- Worst case: 4096 × 60 mA = **245 A** (unrealistic)
- Typical: **4-8 A** (mixed colors)
- **Software limited to 18 A** (configurable)

### Power Injection

Critical: Inject power at **multiple points** to prevent voltage drop.

```
PSU 5V ──┬────┬────┬────┬──── (4+ injection points)
         │    │    │    │
       LEDs LEDs LEDs LEDs
         │    │    │    │
GND ─────┴────┴────┴────┴────
```

Minimum: 4 corners
Recommended: 8+ points (corners + midpoints)

### Wire Gauge

- Main power: **18 AWG** (10A+ capacity)
- Distribution: **20-22 AWG** (3-5A sections)
- Data lines: **22-24 AWG**

### Decoupling

- **1000µF** electrolytic near Teensy
- **100µF** every 100 LEDs
- **0.1µF** ceramic near each LED section

## Troubleshooting

### No LEDs Light Up

1. Check 5V power at multiple points (should be 4.75V - 5.25V)
2. Verify DIN/WCK/BCK connections
3. Check shift register wiring and enable pins
4. Confirm PLL configuration (711.1 MHz)

### Wrong Colors

1. **Verify GRB order** in code (should be correct already)
2. Check data line integrity (add pull-up resistors if needed)
3. Verify shift register outputs

### Flickering/Corruption

1. **Insufficient power** - add injection points
2. **Timing issue** - verify PLL frequency
3. **Data signal integrity** - shorten wires, add resistors
4. **Loose connections** - check solder joints

### LCD Issues

- Blank screen: Check 3.3V supply, verify pin connections
- Garbled: Reduce SPI speed in code
- Touch not working: Run calibration routine

### WiFi Issues

- No connection: Check 3.3V to ESP8266, verify GPIO0 is HIGH during boot
- UART fails: Confirm 460800 baud on both sides, check TX↔RX cross-connection

## Software Power Limiting

Current limiter prevents PSU overload:

```cpp
// In config
config.power.max_milliamps = 18000;  // 18A limit
config.power.brightness = 1.0;       // Global scale

// Algorithm scales all LEDs if total current exceeds budget
```

## ESP8266 Programming

To flash ESP8266 firmware:

1. Connect GPIO0 to GND
2. Reset module
3. Upload firmware via UART
4. Disconnect GPIO0, reset to normal mode

## PCB Files

See `Mega-Cube/Electronics/` for:
- KiCAD schematics
- Gerber files (manufacturing-ready)
- Bill of materials

## Construction Docs

See `Mega-Cube/Construction/` for:
- Frame assembly
- LED placement
- Wiring diagrams
