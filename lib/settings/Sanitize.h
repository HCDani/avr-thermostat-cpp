#pragma once

#include <stdint.h>

namespace settings {

// The setpoint pair rule: 0..60 C, the range the encoder enforces, and tlow
// below thigh. Shared by the hardware driver and the mock so a test and the
// board cannot disagree about it.
bool usable(uint8_t tlow, uint8_t thigh);

// Stored bytes -> usable setpoints, for the read only. An unusable pair is
// replaced by the defaults, because a read has to produce something to run
// with; a blank EEPROM reads 0xFF and that is the normal path on a new board.
// A save has a stored pair to fall back on and so rejects instead: see
// ISettings::save.
void sanitize(uint8_t& tlow, uint8_t& thigh);

}  // namespace settings
