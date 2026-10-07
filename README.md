# DevLab_PhotoInterrupter

Arduino library for the 8ITR523S slotted photo interrupter module (940 nm LED
plus silicon phototransistor, 2 mm gap) using the DevLab Device Protocol (DDP)
over I2C.

The module is a PY32F003 that samples the phototransistor collector with its
ADC. The same signal is available as a raw 12-bit value and as a digital
blocked/free state (hysteresis + debounce) with a pulse counter. This library
wraps the shared
[`DevLabDDP`](https://github.com/UNIT-Electronics-MX/unit_devlab_ddp_library)
master and the
[`DevLab_Interface`](https://github.com/UNIT-Electronics-MX/unit_devlab_interface_library)
`DevLab_I2C_Orchestrator` bus class into one `DevLab_PhotoInterrupter` object.
Firmware: `unit_firmware_i2c_itr523s_py32`.

Compatible with ESP32, RP2040/RP2350, STM32 and AVR.

# Features

- Device identification (Device ID `0x0112`) before any command
- Digital read `readBlocked()` and analog read `readRaw()` / `readPercentage()`
- Pulse counter (`readCount()`, `resetCount()`) and `poll()` with edge flag
- I2C address scan and reassignment (`0x08` to `0x77`), default address `0x31`
- Bus recovery (`beginRecovered`) for a slave left mid-transaction

# Quick Start

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <DevLab_PhotoInterrupter.h>

DevLab_PhotoInterrupter itr(Wire, DevLab_PhotoInterrupter::DEFAULT_ADDRESS, 400000);

void setup() {
  Serial.begin(115200);
  if (!itr.beginRecovered(SDA, SCL)) {
    Serial.println("ITR523S not found");
    while (1);
  }
}

void loop() {
  bool blocked;
  uint16_t raw;
  if (itr.readBlocked(blocked) && itr.readRaw(raw)) {
    Serial.printf("blocked=%d raw=%u\n", blocked, raw);
  }
  delay(50);
}
```

# Command map

| Command | Method | Response |
|---:|---|---|
| `0x80` (`0x40`) | `readBlocked()` | 1 byte, bit0 = slot blocked |
| `0x81` (`0x60`) | `readRaw()` | uint16 little-endian, 12-bit ADC |
| `0x82` | `readStatus()` | 1 byte of flags; clears the edge bit |
| `0x83` | `readCount()` | uint16 little-endian, times the slot was blocked |
| `0x84` | `resetCount()` | status byte; counter back to 0 |

The standard DDP commands `0x40` (GPIO0) and `0x60` (ADC0) return the same data
as `0x80` and `0x81`. Status bits: `0x01` blocked, `0x02` state changed since
the last read, `0x04` ADC error.

# Wiring and polarity

The collector goes to PA2 with a pull-up: a free slot reads low and a blocked
slot reads high. The digital state is derived from the ADC with thresholds
2800 (blocked) and 1300 (free) and 3 consecutive samples at 1 ms, so the
maximum counting rate is about 150 blockings per second. If the board is wired
the other way, flip `ITR_BLOCKED_IS_HIGH` in the firmware.

# Notes

- `DevLabDDP::deviceName()` reports `unknown` for ID `0x0112` until the DDP
  library adds it; this does not affect identification.
- The firmware and the examples are not yet validated on hardware.

# Folder Structure

```text
DevLab_PhotoInterrupter/
├── examples/
│   ├── adc/readAnalog
│   ├── digital/readBlocked
│   └── i2c/{changeAddress,i2c_scanner}
├── src/DevLab_PhotoInterrupter.{h,cpp}
├── library.properties
├── keywords.txt
└── LICENSE
```

# Author

UNIT Electronics MX - Jonathan Mejorado

# License

MIT License
