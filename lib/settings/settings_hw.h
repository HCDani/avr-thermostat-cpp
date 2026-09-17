#pragma once

#include <stdint.h>

#include "ISettings.h"

namespace settings {

// Two EEPROM bytes at addresses 0 and 1. The only component in the project
// that includes <avr/eeprom.h>. init() reads the cells once and the running
// values live in RAM from then on, because a byte write blocks for roughly
// 3.4 ms and a cell endures only about 100,000 erase cycles.
class SettingsHw : public ISettings {
public:
    SettingsHw();

    void init() override;
    uint8_t tlow() const override;
    uint8_t thigh() const override;
    void save(uint8_t tlow, uint8_t thigh) override;

private:
    uint8_t tlow_;
    uint8_t thigh_;
};

}  // namespace settings
