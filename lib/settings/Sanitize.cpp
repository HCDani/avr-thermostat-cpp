#include "Sanitize.h"

namespace settings {

namespace {

const uint8_t kTlowDefault = 18;
const uint8_t kThighDefault = 25;
const uint8_t kDegreesMax = 60;

}  // namespace

bool usable(uint8_t tlow, uint8_t thigh) {
    return tlow <= kDegreesMax && thigh <= kDegreesMax && tlow < thigh;
}

void sanitize(uint8_t& tlow, uint8_t& thigh) {
    // Both bytes go back to the defaults together: a pair in the wrong order
    // says nothing about which of the two is the bad one.
    if (!usable(tlow, thigh)) {
        tlow = kTlowDefault;
        thigh = kThighDefault;
    }
}

}  // namespace settings
