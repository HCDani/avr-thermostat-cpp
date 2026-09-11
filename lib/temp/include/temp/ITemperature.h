#pragma once

#include <stdint.h>

#include "thermostat/Types.h"

namespace temp {

// The two failures are told apart rather than lumped into one error.
// R = R0 * (1023 / count - 1), so a count below the trusted window is an
// implausibly high resistance -- an open lead -- and one above it is a near
// short.
enum class SensorStatus : uint8_t { Ok, Open, Shorted };

// value is meaningful only when status is Ok.
struct Reading {
    SensorStatus status;
    thermostat::DeciCelsius value;
};

// Temperature driver contract. Carries no implementation, so the hardware
// version and the mock are interchangeable and the application cannot tell
// which one it holds.
//
// Sampling is split deliberately: start() and the conversion-complete
// interrupt only move a raw count, and the arithmetic happens in whichever
// call reads it. That keeps the ISR free of arithmetic, as the specification
// requires.
class ITemperature {
public:
    virtual ~ITemperature() = default;

    virtual void init() = 0;

    // Starts one conversion and returns. Called from the 1 Hz tick; the
    // result is latched, not delivered.
    virtual void start() = 0;

    // True once since the last call if the conversion-complete path latched a
    // new count. Self-clearing: each latch sets it, each call that returns
    // true clears it. readRaw() and get() do not.
    virtual bool available() = 0;

    // Latest latched sample. Ten bits on this part.
    virtual uint16_t readRaw() const = 0;

    // Converts a raw count. Touches no hardware, so it is safe to call on any
    // value at any time.
    virtual Reading celsius(uint16_t raw) const = 0;

    // celsius(readRaw()). Call from the main loop, never from an ISR.
    virtual Reading get() const = 0;
};

}  // namespace temp
