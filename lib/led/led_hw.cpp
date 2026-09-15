#include "led_hw.h"

namespace led {

namespace {

// Character code of the filled block LcdHw programs into CGRAM. Slot 1, so the
// code is 0x01 and fits in a C string; slot 0 would be NUL.
const char kOn[] = "\x01";
const char kOff[] = " ";

}  // namespace

LedHw::LedHw(lcd::ILcd& lcd) : lcd_(lcd) {
    for (uint8_t i = 0; i < 8; ++i) {
        state_[i] = false;
    }
}

void LedHw::init() {
    lcd_.init();
    for (uint8_t i = 1; i <= 8; ++i) {
        set(i, false);
    }
}

void LedHw::set(uint8_t ledNo, bool state) {
    const uint8_t index = static_cast<uint8_t>(ledNo - 1);
    state_[index] = state;
    lcd_.setCursor(index, 1);
    lcd_.write(state ? kOn : kOff);
}

bool LedHw::get(uint8_t ledNo) const { return state_[ledNo - 1]; }

}  // namespace led
