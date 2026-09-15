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

void LedHw::bar(int16_t lo, int16_t hi, int16_t value) {
    uint8_t lit = 0;
    if (value >= hi) {
        lit = 8;
    } else if (value >= lo) {
        const int32_t span = static_cast<int32_t>(hi) - static_cast<int32_t>(lo);
        for (uint8_t i = 0; i < 8; ++i) {
            const int16_t threshold =
                static_cast<int16_t>(static_cast<int32_t>(lo) + (static_cast<int32_t>(i) * span) / 7);
            if (value >= threshold) {
                lit = static_cast<uint8_t>(i + 1);
            }
        }
    }

    char row[9];
    for (uint8_t i = 0; i < 8; ++i) {
        const bool on = i < lit;
        state_[i] = on;
        row[i] = on ? kOn[0] : kOff[0];
    }
    row[8] = '\0';

    // One transfer for the whole bar, so the controller cannot NACK between cells.
    lcd_.setCursor(0, 1);
    lcd_.write(row);
}

void LedHw::write(uint8_t col, const char* s) {
    lcd_.setCursor(col, 1);
    lcd_.write(s);
}

}  // namespace led
