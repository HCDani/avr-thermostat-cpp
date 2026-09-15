#include "display_hw.h"

namespace display {

namespace {

const uint8_t kColumns = 16;
const uint8_t kRangeLen = 10;
const uint8_t kValueLen = 6;
const char kRange[] = "BAR 21-28C";

// Right-justified 6-character field, e.g. " 25.0C". value is tenths of a
// degree. The range label takes the first ten columns.
void formatCelsius(char* out, uint16_t deci) {
    char tmp[8];
    uint8_t n = 0;

    tmp[n++] = 'C';
    tmp[n++] = static_cast<char>('0' + (deci % 10));
    tmp[n++] = '.';
    uint16_t v = static_cast<uint16_t>(deci / 10);
    do {
        tmp[n++] = static_cast<char>('0' + (v % 10));
        v = static_cast<uint16_t>(v / 10);
    } while (v != 0);

    uint8_t i = 0;
    while (i + n < kValueLen) {
        out[i++] = ' ';
    }
    while (n > 0) {
        --n;
        out[i++] = tmp[n];
    }
}

void formatRow(char* out, uint16_t value) {
    for (uint8_t i = 0; i < kRangeLen; ++i) {
        out[i] = kRange[i];
    }
    formatCelsius(out + kRangeLen, value);
    out[kColumns] = '\0';
}

}  // namespace

DisplayHw::DisplayHw(lcd::ILcd& lcd) : lcd_(lcd), value_(0), pending_(false) {}

void DisplayHw::init() {
    lcd_.init();
    value_ = 0;
    pending_ = false;
}

void DisplayHw::showUInt(uint16_t value) { value_ = value; }

void DisplayHw::onTick() { pending_ = true; }

void DisplayHw::service() {
    if (!pending_) {
        return;
    }
    pending_ = false;

    char row[17];
    formatRow(row, value_);
    lcd_.setCursor(0, 0);
    lcd_.write(row);
}

}  // namespace display
