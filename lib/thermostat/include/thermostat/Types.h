#pragma once

#include <stdint.h>

namespace thermostat {

// A raw 10-bit ADC reading, kept distinct from a temperature so the two cannot
// be mixed in arithmetic. The bug this exists to prevent is comparing a count
// against a setpoint as though the conversion had already happened.
class AdcCount {
public:
    constexpr AdcCount() : value_(0) {}
    constexpr explicit AdcCount(uint16_t value) : value_(value) {}

    constexpr uint16_t raw() const { return value_; }

private:
    uint16_t value_;
};

constexpr bool operator==(AdcCount a, AdcCount b) { return a.raw() == b.raw(); }
constexpr bool operator!=(AdcCount a, AdcCount b) { return !(a == b); }

// Temperature in tenths of a degree Celsius. Fixed point rather than float:
// the ATmega328P has no floating point unit, and 0.1 C is already finer than
// the sensor's 1.5 C accuracy.
class DeciCelsius {
public:
    constexpr DeciCelsius() : value_(0) {}
    constexpr explicit DeciCelsius(int16_t value) : value_(value) {}

    constexpr int16_t raw() const { return value_; }

private:
    int16_t value_;
};

constexpr DeciCelsius operator+(DeciCelsius a, DeciCelsius b) {
    return DeciCelsius(static_cast<int16_t>(a.raw() + b.raw()));
}

constexpr DeciCelsius operator-(DeciCelsius a, DeciCelsius b) {
    return DeciCelsius(static_cast<int16_t>(a.raw() - b.raw()));
}

constexpr bool operator==(DeciCelsius a, DeciCelsius b) { return a.raw() == b.raw(); }
constexpr bool operator!=(DeciCelsius a, DeciCelsius b) { return !(a == b); }
constexpr bool operator<(DeciCelsius a, DeciCelsius b) { return a.raw() < b.raw(); }
constexpr bool operator>(DeciCelsius a, DeciCelsius b) { return b < a; }
constexpr bool operator<=(DeciCelsius a, DeciCelsius b) { return !(b < a); }
constexpr bool operator>=(DeciCelsius a, DeciCelsius b) { return !(a < b); }

}  // namespace thermostat
