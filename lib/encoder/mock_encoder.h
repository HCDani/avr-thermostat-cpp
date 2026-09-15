#pragma once

#include <stdint.h>

#include "IEncoder.h"

namespace encoder {

class MockEncoder : public IEncoder {
public:
    MockEncoder();

    void init() override;
    int16_t getDelta() override;
    bool button() const override;

    void turn(int16_t steps);
    void press(bool down);
    bool inited() const;

private:
    int16_t pending_;
    bool down_;
    bool inited_;
};

}  // namespace encoder
