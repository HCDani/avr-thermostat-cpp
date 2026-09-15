#pragma once

#include <stdint.h>

#include "ILcd.h"

namespace lcd {

// Records cursor and writes into a 2x16 buffer so LedHw can be tested without
// a bus.
class MockLcd : public ILcd {
public:
    MockLcd();

    void init() override;
    void setCursor(uint8_t col, uint8_t row) override;
    void write(const char* s) override;

    bool inited() const;
    char cell(uint8_t col, uint8_t row) const;
    uint8_t cursorCol() const;
    uint8_t cursorRow() const;

private:
    char grid_[2][16];
    uint8_t col_;
    uint8_t row_;
    bool inited_;
};

}  // namespace lcd
