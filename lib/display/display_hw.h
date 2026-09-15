#pragma once

#include <stdint.h>

#include "IDisplay.h"
#include "ILcd.h"

namespace display {

// Four digits, leading zeros, right-justified on row 1. Uses ILcd, so the
// same class is exercised on the host against MockLcd.
class DisplayHw : public IDisplay {
public:
    explicit DisplayHw(lcd::ILcd& lcd);

    void init() override;
    void showUInt(uint16_t value) override;
    void onTick() override;
    void service() override;

private:
    lcd::ILcd& lcd_;
    uint16_t value_;
    volatile bool pending_;
};

}  // namespace display
