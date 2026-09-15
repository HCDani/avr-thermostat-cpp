#include "mock_lcd.h"

namespace lcd {

MockLcd::MockLcd() : col_(0), row_(0), inited_(false) {
    for (uint8_t r = 0; r < 2; ++r) {
        for (uint8_t c = 0; c < 16; ++c) {
            grid_[r][c] = ' ';
        }
    }
}

void MockLcd::init() { inited_ = true; }

void MockLcd::setCursor(uint8_t col, uint8_t row) {
    col_ = col;
    row_ = row;
}

void MockLcd::write(const char* s) {
    while (*s != '\0' && col_ < 16 && row_ < 2) {
        grid_[row_][col_] = *s;
        ++col_;
        ++s;
    }
}

bool MockLcd::inited() const { return inited_; }

char MockLcd::cell(uint8_t col, uint8_t row) const { return grid_[row][col]; }

uint8_t MockLcd::cursorCol() const { return col_; }

uint8_t MockLcd::cursorRow() const { return row_; }

}  // namespace lcd
