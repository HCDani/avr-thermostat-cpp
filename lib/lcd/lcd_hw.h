#pragma once

#include "ILcd.h"
#include "Twi.h"

namespace lcd {

// Grove LCD RGB Backlight: HD44780-compatible text at 0x3E, PCA9633 backlight
// at 0x62. All I2C lives here; the Led and Display drivers only see ILcd.
class LcdHw : public ILcd {
public:
    explicit LcdHw(twi::Twi& bus);

    void init() override;
    void setCursor(uint8_t col, uint8_t row) override;
    void write(const char* s) override;

    // Not part of ILcd: the status row has no business dimming the panel, and
    // section 6.3 wants the backlight for mode indication later.
    void setBacklight(uint8_t red, uint8_t green, uint8_t blue);

private:
    void settle();
    void command(uint8_t cmd);
    void data(uint8_t value);
    void rgbReg(uint8_t reg, uint8_t value);

    twi::Twi& bus_;
};

}  // namespace lcd
