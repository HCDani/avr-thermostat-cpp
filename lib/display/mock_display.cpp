#include "mock_display.h"

namespace display {

MockDisplay::MockDisplay()
    : value_(0), shown_(0), pending_(false), inited_(false), flushed_(false) {
    heading_[0] = 'T';
    heading_[1] = 'E';
    heading_[2] = 'M';
    heading_[3] = 'P';
    heading_[4] = '\0';
}

void MockDisplay::init() { inited_ = true; }

void MockDisplay::setHeading(const char* heading) {
    uint8_t i = 0;
    while (heading[i] != '\0' && i < 5) {
        heading_[i] = heading[i];
        ++i;
    }
    heading_[i] = '\0';
    pending_ = true;
}

void MockDisplay::showUInt(uint16_t value) {
    value_ = value;
    pending_ = true;
}

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

const char* MockDisplay::heading() const { return heading_; }

bool MockDisplay::flushed() const { return flushed_; }

}  // namespace display
