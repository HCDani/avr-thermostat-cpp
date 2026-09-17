#pragma once

#include <stdint.h>

#include "ISettings.h"

namespace settings {

// Test mock: the same interface over a RAM-backed store, which starts blank
// (0xFF) the way an unprogrammed EEPROM does. Tests can plant stored bytes to
// exercise the pair rule, and writes() counts only the bytes that actually
// changed, mirroring eeprom_update_byte, so a test can prove that a cancelled
// or unchanged commit spends no erase cycle.
class MockSettings : public ISettings {
public:
    MockSettings();

    void init() override;
    uint8_t tlow() const override;
    uint8_t thigh() const override;
    void save(uint8_t tlow, uint8_t thigh) override;

    // Test controls.
    void setStored(uint8_t tlow, uint8_t thigh);
    uint8_t storedTlow() const;
    uint8_t storedThigh() const;
    uint16_t writes() const;
    bool inited() const;

private:
    uint8_t storedTlow_;
    uint8_t storedThigh_;
    uint8_t tlow_;
    uint8_t thigh_;
    uint16_t writes_;
    bool inited_;
};

}  // namespace settings
