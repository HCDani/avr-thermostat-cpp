#pragma once

#include <stdint.h>

namespace settings {

// The two setpoints, surviving a power cycle (specification 6.4). Whole
// degrees Celsius rather than DeciCelsius: that is the unit the encoder edits
// in, and it is what fits the two bytes the specification allots.
class ISettings {
public:
    virtual ~ISettings() = default;

    virtual void init() = 0;
    virtual uint8_t tlow() const = 0;
    virtual uint8_t thigh() const = 0;

    // A pair outside 0..60 C, or one where tlow is not below thigh, is not
    // stored and changes nothing: an unusable edit is cancelled, not turned
    // into some other value the caller did not ask for. The caller should
    // take tlow() and thigh() back afterwards rather than assume its own
    // values were stored.
    virtual void save(uint8_t tlow, uint8_t thigh) = 0;
};

}  // namespace settings
