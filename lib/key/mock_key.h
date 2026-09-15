#pragma once

#include <stdint.h>

#include "IKey.h"

namespace key {

// Test mock. press() is the test control; get() is what the application sees.
class MockKey : public IKey {
public:
    MockKey();

    void init() override;
    bool get(uint8_t keyNo) const override;

    void press(uint8_t keyNo, bool down);
    bool inited() const;

private:
    bool down_[3];
    bool inited_;
};

}  // namespace key
