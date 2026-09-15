#pragma once

#include <stdint.h>

#include "IKey.h"

namespace key {

// D2, D3, D4 — keys 1..3. HIGH is pressed (Grove onboard pull-down).
class KeyHw : public IKey {
public:
    KeyHw();

    void init() override;
    bool get(uint8_t keyNo) const override;

private:
    uint8_t pinBit(uint8_t keyNo) const;
    bool read(uint8_t keyNo) const;

    mutable bool stable_[3];
};

}  // namespace key
