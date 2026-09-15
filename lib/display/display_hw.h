#pragma once

#include <stdint.h>

#include "IDisplay.h"
#include "ILcd.h"

namespace display {

class DisplayHw : public IDisplay {
public:
    explicit DisplayHw(lcd::ILcd& lcd);

    void init() override;
    void setHeading(const char* heading) override;
    void showUInt(uint16_t value) override;
    void onTick() override;
    void service() override;

private:
    lcd::ILcd& lcd_;
    char heading_[6];
    uint16_t value_;
    volatile bool pending_;
};

}  // namespace display
