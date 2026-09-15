#pragma once

#include <stdint.h>

#include "IEncoder.h"

namespace encoder {

class EncoderHw : public IEncoder {
public:
    EncoderHw();

    void init() override;
    int16_t getDelta() override;
    bool button() const override;

    static void onPinChange();

private:
    volatile int16_t steps_;
    volatile uint8_t prev_;
    volatile int8_t quarter_;
    mutable bool stableButton_;

    static EncoderHw* active_;
};

}  // namespace encoder
