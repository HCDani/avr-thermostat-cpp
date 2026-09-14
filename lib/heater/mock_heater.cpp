#include "mock_heater.h"

namespace heater {

MockHeater::MockHeater() : on_(false), transitions_(0) {}

void MockHeater::init() {}

void MockHeater::set(bool on) {
    if (on != on_) {
        ++transitions_;
    }
    on_ = on;
}

bool MockHeater::get() const { return on_; }

uint16_t MockHeater::transitions() const { return transitions_; }

}  // namespace heater
