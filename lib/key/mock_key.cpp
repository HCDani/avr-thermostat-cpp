#include "mock_key.h"

namespace key {

MockKey::MockKey() : inited_(false) {
    for (uint8_t i = 0; i < 3; ++i) {
        down_[i] = false;
    }
}

void MockKey::init() { inited_ = true; }

bool MockKey::get(uint8_t keyNo) const { return down_[keyNo - 1]; }

void MockKey::press(uint8_t keyNo, bool down) { down_[keyNo - 1] = down; }

bool MockKey::inited() const { return inited_; }

}  // namespace key
