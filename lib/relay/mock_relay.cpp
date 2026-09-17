#include "mock_relay.h"

namespace relay {

MockRelay::MockRelay() : on_(false), transitions_(0), inited_(false) {}

void MockRelay::init() {
    on_ = false;
    inited_ = true;
}

void MockRelay::set(bool on) {
    if (on != on_) {
        ++transitions_;
    }
    on_ = on;
}

bool MockRelay::get() const { return on_; }

uint16_t MockRelay::transitions() const { return transitions_; }

bool MockRelay::inited() const { return inited_; }

}  // namespace relay
