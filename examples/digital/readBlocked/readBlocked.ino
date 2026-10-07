/**
 * @file readBlocked.ino
 * @brief Digital use of the 8ITR523S photo interrupter: prints BLOCKED / FREE on
 *        every change and the running pulse count (GPIO0 style read).
 *
 * Send 'r' on the Serial monitor to reset the pulse counter.
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

DevLab_PhotoInterrupter itr(I2C_BUS, DevLab_PhotoInterrupter::DEFAULT_ADDRESS, I2C_FREQ);

bool lastBlocked = false;
bool haveLast = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  if (!itr.beginRecovered(SDA_PIN, SCL_PIN)) {
    Serial.println("ITR523S module not found; check wiring, power and firmware");
    return;
  }
  itr.printInfo(Serial);
  Serial.println("Pass an object through the slot. Send 'r' to reset the counter.");
}

void loop() {
  if (!itr.isConnected()) return;

  if (Serial.available() && Serial.read() == 'r') {
    Serial.println(itr.resetCount() ? "Counter reset" : "ERROR: reset failed");
  }

  bool blocked = false;
  if (!itr.readBlocked(blocked)) {
    Serial.println("ERROR: digital read failed");
    delay(500U);
    return;
  }

  if (!haveLast || blocked != lastBlocked) {
    uint16_t count = 0U;
    itr.readCount(count);
    Serial.print(blocked ? "BLOCKED" : "FREE   ");
    Serial.print("  count=");
    Serial.println(count);
    lastBlocked = blocked;
    haveLast = true;
  }

  delay(5U);
}
