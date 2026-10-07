# ITR523S DDP examples

| Example | Purpose |
|---|---|
| `digital/readBlocked` | Digital use: prints `BLOCKED` / `FREE` on every change with the pulse count; send `r` to reset the counter. |
| `adc/readAnalog` | Analog use: prints the 12-bit ADC value, a percentage and the derived digital state in Serial Plotter format. |
| `i2c/changeAddress` | Scan the bus and change the I2C address of an ITR523S node. |
| `i2c/i2c_scanner` | List every address that acknowledges on the bus. |

The node reads the phototransistor collector on PA2 (ADC) and derives the
digital state from the same pin with hysteresis and debounce, so both examples
read the same signal. Device ID `0x0112`, factory I2C address `0x31`.

Pins: ESP32 SDA GPIO6 / SCL GPIO7; RP2040/RP2350 `Wire1` GPIO12/13 (GPIO24/25
on Pulsar); STM32 and AVR use the board default pins.
