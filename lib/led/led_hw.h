#pragma once

#include <stdint.h>

#include "ILcd.h"
#include "ILed.h"

namespace led {

// Status row: a filled CGRAM block for on, a space for off. Uses ILcd, so the
// same class is exercised on the host against MockLcd.
class LedHw : public ILed {
public:
    explicit LedHw(lcd::ILcd& lcd);

    void init() override;
    void set(uint8_t ledNo, bool state) override;
    bool get(uint8_t ledNo) const override;
    void bar(int16_t lo, int16_t hi, int16_t value) override;
    void write(uint8_t col, const char* s) override;

private:
    lcd::ILcd& lcd_;
    bool state_[8];
};

}  // namespace led
