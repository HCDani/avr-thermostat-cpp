#pragma once

#include <stdint.h>

// AVR is a Harvard machine: a const array in a normal section is copied from
// flash into SRAM at startup, which on a part with 2 KB of SRAM is not free.
// The lookup table therefore stays in flash and is read a word at a time.
// On the host both of those concerns are imaginary, so the same code compiles
// down to a plain subscript.

#ifdef __AVR__

#include <avr/pgmspace.h>

#define THERMOSTAT_PROGMEM PROGMEM

namespace thermostat {

inline int16_t readTableWord(const int16_t* address) {
    return static_cast<int16_t>(pgm_read_word(address));
}

}  // namespace thermostat

#else

#define THERMOSTAT_PROGMEM

namespace thermostat {

inline int16_t readTableWord(const int16_t* address) { return *address; }

}  // namespace thermostat

#endif
