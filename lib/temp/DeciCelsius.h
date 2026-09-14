#pragma once
#include <stdint.h>
namespace temp {


// Temperature in tenths of a degree Celsius. Fixed point rather than float:
// the Atmega328P has no floating point unit, and 0.1 C is already finer than the sensor's 1.5 C accuracy.
class DeciCelsius{
public:
    constexpr DeciCelsius() : value_(0){}
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
constexpr bool operator>=(DeciCelsius a, DeciCelsius b) { return !(a < b); }
constexpr bool operator<=(DeciCelsius a, DeciCelsius b) { return !(b < a); }
}// namespace temp