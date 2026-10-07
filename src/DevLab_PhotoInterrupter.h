/**
 * @file DevLab_PhotoInterrupter.h
 * @brief DevLab 8ITR523S photo interrupter driver over DDP/I2C.
 *
 * The node reads the phototransistor collector on its ADC and also exposes a
 * debounced digital state (with hysteresis) and a pulse counter.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#ifndef DEVLAB_PHOTOINTERRUPTER_H
#define DEVLAB_PHOTOINTERRUPTER_H

#pragma once

#include "DevLabDDP.h"
#include "DevLabDDPConsole.h"
#include "DevLab_I2C_Orchestrator.h"

/* Result of DevLab_PhotoInterrupter::poll(). */
struct DevLab_PhotoInterrupter_State
{
    bool blocked = false;    /* slot currently blocked by an object */
    bool changed = false;    /* the state changed since the previous status read */
    bool adcError = false;   /* the last ADC conversion failed */
};

class DevLab_PhotoInterrupter
{
public:
    /* DDP Device ID of the ITR523S module (DDP_DEVICE_ID in firmware). */
    static constexpr uint16_t DEVICE_ID = 0x0112U;

    /* Factory I2C address (STARTUP_I2C_ADDRESS). */
    static constexpr uint8_t DEFAULT_ADDRESS = 0x31U;

    /* Bits returned by readStatus() (command 0x82). */
    static constexpr uint8_t STATUS_BLOCKED = 0x01U;
    static constexpr uint8_t STATUS_EDGE = 0x02U;
    static constexpr uint8_t STATUS_ADC_ERROR = 0x04U;

    explicit DevLab_PhotoInterrupter(TwoWire &wire = Wire, uint8_t address = DEFAULT_ADDRESS, uint32_t clock = 400000UL)
    : _bus(wire, clock), _ddp(_bus, DEVICE_ID), _address(address), _clock(clock) {}

    bool begin();
    bool begin(uint8_t sdaPin, uint8_t sclPin, uint32_t clock = 400000UL);
    bool beginRecovered(uint8_t sdaPin, uint8_t sclPin, uint32_t timeoutUs = 20000, bool restart = false);

    /* Digital read (command 0x80, same as the standard GPIO0 command 0x40):
     * `blocked` is true while an object interrupts the beam. */
    bool readBlocked(bool &blocked);

    /* Analog read (command 0x81, same as the standard ADC0 command 0x60):
     * 12-bit ADC count of the phototransistor collector, 0..4095. */
    bool readRaw(uint16_t &rawValue);
    bool readPercentage(float &percentage);

    /* Reads the status byte (command 0x82). Reading CLEARS the edge event. */
    bool readStatus(uint8_t &status);

    /* Number of times the slot was blocked since power-up or resetCount()
     * (command 0x83, 16-bit, wraps at 65535). */
    bool readCount(uint16_t &count);
    bool resetCount();

    /* One-call helper over readStatus(). */
    bool poll(DevLab_PhotoInterrupter_State &out);

    bool busReady() const { return _busReady; }
    bool isConnected() const { return _verified; }
    uint8_t address() const { return _address; }
    const DevLabDDP::DeviceInfo &deviceInfo() const { return _info; }
    void printInfo(Print &out = Serial) const;

    DevLabDDP::Master &protocol() { return _ddp; }
    DevLab_I2C_Orchestrator &bus() { return _bus; }

private:
    static constexpr uint8_t CMD_READ_STATE = 0x80U;
    static constexpr uint8_t CMD_READ_RAW = 0x81U;
    static constexpr uint8_t CMD_READ_STATUS = 0x82U;
    static constexpr uint8_t CMD_READ_COUNT = 0x83U;
    static constexpr uint8_t CMD_RESET_COUNT = 0x84U;
    static constexpr uint8_t RESPONSE_DATA_NIBBLE = 0x0CU; /* RESPONSE_DATA (0x3C) low nibble */
    static constexpr uint16_t _kResponseDelayMs = 2U;

    DevLab_I2C_Orchestrator _bus;
    DevLabDDP::Master _ddp;
    uint8_t _address;
    uint32_t _clock;
    bool _busReady = false;
    bool _verified = false;
    DevLabDDP::DeviceInfo _info;
};

#endif
