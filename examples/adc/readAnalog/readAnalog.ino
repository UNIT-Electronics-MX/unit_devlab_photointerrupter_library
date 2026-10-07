/**
 * @file readAnalog.ino
 * @brief Analog use of the 8ITR523S photo interrupter: prints the 12-bit ADC value
 *        of the phototransistor collector and a text bar, in a format the
 *        Arduino Serial Plotter can graph. Also shows the digital state the
 *        node derives from the same pin, to compare both readings.
 *
 * Compatible with ESP32, RP2040/RP2350, STM32 and AVR.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include <Arduino.h>
#include <Wire.h>
#include <DevLab_PhotoInterrupter.h>

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_RP2350)
  #define I2C_BUS Wire1
  #if defined(PIN_WIRE1_SDA) && defined(PIN_WIRE1_SCL) && \
      PIN_WIRE1_SDA == 24U && PIN_WIRE1_SCL == 25U
    // Pulsar connector pins.
    constexpr int SDA_PIN = 24;
    constexpr int SCL_PIN = 25;
  #else
    // Generic RP2040/RP2350 wiring.
    constexpr int SDA_PIN = 12;
    constexpr int SCL_PIN = 13;
  #endif
  constexpr uint32_t I2C_FREQ = 400000UL;
#elif defined(ARDUINO_ARCH_ESP32)
  #define I2C_BUS Wire
  constexpr int SDA_PIN = 6;
  constexpr int SCL_PIN = 7;
  constexpr uint32_t I2C_FREQ = 400000UL;
#elif defined(ARDUINO_ARCH_STM32)
  // STM32duino: default I2C pins of the selected board.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
  constexpr uint32_t I2C_FREQ = 400000UL;
#elif defined(ARDUINO_ARCH_AVR)
  // AVR has fixed I2C pins (Uno/Nano: A4/A5, Mega: 20/21, Leonardo: 2/3);
  // SDA/SCL come from the board variant and begin() ignores the pin numbers.
  #define I2C_BUS Wire
  constexpr int SDA_PIN = SDA;
  constexpr int SCL_PIN = SCL;
  constexpr uint32_t I2C_FREQ = 100000UL;  // 400 kHz falla con el level shifter en UNO
#else
#error "Use an ESP32, RP2040, RP2350, STM32, or AVR master"
#endif

constexpr uint32_t PRINT_INTERVAL_MS = 50UL;

DevLab_PhotoInterrupter itr(I2C_BUS, DevLab_PhotoInterrupter::DEFAULT_ADDRESS, I2C_FREQ);

void setup() {
  Serial.begin(115200);
  delay(500);
  if (!itr.beginRecovered(SDA_PIN, SCL_PIN)) {
    Serial.println("ITR523S module not found; check wiring, power and firmware");
    return;
  }
  itr.printInfo(Serial);
}

void loop() {
  if (!itr.isConnected()) return;

  uint16_t raw = 0U;
  bool blocked = false;
  if (!itr.readRaw(raw) || !itr.readBlocked(blocked)) {
    Serial.println("ERROR: read failed");
    delay(500U);
    return;
  }

  // Serial Plotter format: label:value pairs.
  Serial.print("raw:");
  Serial.print(raw);
  Serial.print(" percent:");
  Serial.print((raw * 100.0f) / 4095.0f, 1);
  Serial.print(" blocked:");
  Serial.println(blocked ? 4095 : 0);

  delay(PRINT_INTERVAL_MS);
}
