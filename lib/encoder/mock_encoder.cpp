#include "mock_encoder.h"

namespace encoder {

MockEncoder::MockEncoder() : pending_(0), down_(false), inited_(false) {}

void MockEncoder::init() { inited_ = true; }

int16_t MockEncoder::getDelta() {
    const int16_t delta = pending_;
    pending_ = 0;
    return delta;
}

bool MockEncoder::button() const { return down_; }

void MockEncoder::turn(int16_t steps) {
    pending_ = static_cast<int16_t>(pending_ + steps);
}

void MockEncoder::press(bool down) { down_ = down; }

bool MockEncoder::inited() const { return inited_; }

}  // namespace encoder
