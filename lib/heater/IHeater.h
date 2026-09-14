#pragma once

namespace heater {

// The documentation specifies no heater driver -- its application drives a
// valve (servo) and a pump indicator instead. This thermostat switches a
// relay, so it gets a driver of its own, shaped like the documented ones:
// pure interface here, hardware and mock implementing it.
class IHeater {
public:
    virtual ~IHeater() = default;

    virtual void init() = 0;
    virtual void set(bool on) = 0;
    virtual bool get() const = 0;
};

}  // namespace heater
