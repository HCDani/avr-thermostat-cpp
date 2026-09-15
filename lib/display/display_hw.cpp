#include "display_hw.h"

namespace display {

namespace {

const uint8_t kColumns = 16;

void copyHeading(char* dest, const char* src) {
    uint8_t i = 0;
    while (src[i] != '\0' && i < 5) {
        dest[i] = src[i];
        ++i;
    }
    dest[i] = '\0';
}

void formatRow(char* out, const char* heading, uint16_t value) {
    for (uint8_t i = 0; i < kColumns; ++i) {
        out[i] = ' ';
    }
    out[kColumns] = '\0';

    uint8_t i = 0;
    while (heading[i] != '\0' && i < 5) {
        out[i] = heading[i];
        ++i;
    }

    if (value > 999) {
        value = 999;
    }

    char digits[4];
    uint8_t n = 0;
    uint16_t v = value;
    do {
        digits[n++] = static_cast<char>('0' + (v % 10));
        v = static_cast<uint16_t>(v / 10);
    } while (v != 0);

    uint8_t col = 8;
    uint8_t d = 0;
    while (d < n) {
        out[col] = digits[d];
        if (col == 0) {
            break;
        }
        --col;
        ++d;
    }

    out[9] = ' ';
    out[10] = 'C';
}

}  // namespace

DisplayHw::DisplayHw(lcd::ILcd& lcd) : lcd_(lcd), value_(0), pending_(false) {
    heading_[0] = 'T';
    heading_[1] = 'E';
    heading_[2] = 'M';
    heading_[3] = 'P';
    heading_[4] = '\0';
}

void DisplayHw::init() {
    lcd_.init();
    value_ = 0;
    pending_ = false;
}

void DisplayHw::setHeading(const char* heading) {
    copyHeading(heading_, heading);
    pending_ = true;
}

void DisplayHw::showUInt(uint16_t value) {
    value_ = value;
    pending_ = true;
}

void DisplayHw::onTick() { pending_ = true; }

void DisplayHw::service() {
    if (!pending_) {
        return;
    }
    pending_ = false;

    char row[17];
    formatRow(row, heading_, value_);
    lcd_.setCursor(0, 0);
    lcd_.write(row);
}

}  // namespace display
