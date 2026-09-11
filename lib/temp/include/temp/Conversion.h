#pragma once

#include <stdint.h>

#include "temp/ITemperature.h"

namespace temp {

// Count -> Reading, shared by the hardware driver and the mock so the two
// cannot disagree about where the valid window is or which rail means what.
Reading convert(uint16_t raw);

}  // namespace temp
