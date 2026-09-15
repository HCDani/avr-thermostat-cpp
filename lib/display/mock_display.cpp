#include "mock_display.h"

namespace display {

MockDisplay::MockDisplay()
    : value_(0), shown_(0), pending_(false), inited_(false), flushed_(false) {}

void MockDisplay::init() { inited_ = true; }

void MockDisplay::showUInt(uint16_t value) { value_ = value; }

void MockDisplay::onTick() { pending_ = true; }

void MockDisplay::service() {
    if (!pending_) {
        return;
    }
    pending_ = false;
    shown_ = value_;
    flushed_ = true;
}

bool MockDisplay::inited() const { return inited_; }

uint16_t MockDisplay::latched() const { return value_; }

uint16_t MockDisplay::shown() const { return shown_; }

bool MockDisplay::flushed() const { return flushed_; }

}  // namespace display
