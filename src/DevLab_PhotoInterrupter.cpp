/**
 * @file DevLab_PhotoInterrupter.cpp
 * @brief DevLab 8ITR523S photo interrupter driver over DDP/I2C.
 *
 * @author Jonathan Mejorado
 * @organization UNIT Electronics MX
 */

#include "DevLab_PhotoInterrupter.h"

bool DevLab_PhotoInterrupter::begin() {
    _verified = false;
    _bus.setClock(_clock);
    _busReady = _bus.begin();
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

bool DevLab_PhotoInterrupter::begin(uint8_t sdaPin, uint8_t sclPin, uint32_t clock) {
    _verified = false;
    _clock = clock;
    _bus.setClock(_clock);
    _busReady = _bus.begin(sdaPin, sclPin);
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

bool DevLab_PhotoInterrupter::beginRecovered(uint8_t sdaPin, uint8_t sclPin, uint32_t timeoutUs, bool restart) {
    _verified = false;
    _bus.setClock(_clock);
    _busReady = _bus.beginRecovered(sdaPin, sclPin, timeoutUs, restart);
    if (!_busReady) return false;
    return (_verified = _ddp.matchesExpectedDevice(_address, &_info));
}

void DevLab_PhotoInterrupter::printInfo(Print &out) const {
    DevLabDDP::printDeviceInfo(out, _address, _info, _ddp.expectedDeviceId());
}

bool DevLab_PhotoInterrupter::readBlocked(bool &blocked) {
    if (!_verified) return false;
    uint8_t raw = 0U;
    if (!_ddp.readCommand(_address, CMD_READ_STATE, &raw, 1U, _kResponseDelayMs)) return false;
    blocked = (raw & 0x01U) != 0U;
    return true;
}

bool DevLab_PhotoInterrupter::readRaw(uint16_t &rawValue) {
    if (!_verified) return false;
    uint8_t bytes[2];
    if (!_ddp.readCommand(_address, CMD_READ_RAW, bytes, 2U, _kResponseDelayMs)) return false;
    rawValue = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    return true;
}

bool DevLab_PhotoInterrupter::readPercentage(float &percentage) {
    uint16_t raw;
    if (!readRaw(raw)) return false;
    percentage = (raw * 100.0f) / 4095.0f;
    return true;
}

bool DevLab_PhotoInterrupter::readStatus(uint8_t &status) {
    if (!_verified) return false;
    return _ddp.readCommand(_address, CMD_READ_STATUS, &status, 1U, _kResponseDelayMs);
}

bool DevLab_PhotoInterrupter::readCount(uint16_t &count) {
    if (!_verified) return false;
    uint8_t bytes[2];
    if (!_ddp.readCommand(_address, CMD_READ_COUNT, bytes, 2U, _kResponseDelayMs)) return false;
    count = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    return true;
}

bool DevLab_PhotoInterrupter::resetCount() {
    if (!_verified) return false;
    uint8_t response = 0U;
    if (!_ddp.readCommand(_address, CMD_RESET_COUNT, &response, 1U, _kResponseDelayMs)) return false;
    /* The upper bits of the legacy status byte carry live state. */
    return (response & 0x0FU) == RESPONSE_DATA_NIBBLE;
}

bool DevLab_PhotoInterrupter::poll(DevLab_PhotoInterrupter_State &out) {
    out = DevLab_PhotoInterrupter_State();
    uint8_t status = 0U;
    if (!readStatus(status)) return false;
    out.blocked = (status & STATUS_BLOCKED) != 0U;
    out.changed = (status & STATUS_EDGE) != 0U;
    out.adcError = (status & STATUS_ADC_ERROR) != 0U;
    return true;
}
