#pragma once

#include <stdint.h>

namespace lcd {

// Thin LCD layer used by the status-row and numeric-display drivers. The
// application never holds this type; it talks to Led and Display instead.
class ILcd {
public:
    virtual ~ILcd() = default;

    virtual void init() = 0;
    virtual void setCursor(uint8_t col, uint8_t row) = 0;
    virtual void write(const char* s) = 0;
};

}  // namespace lcd
