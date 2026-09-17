#pragma once

namespace relay {

// Pump contactor (specification 6.3). A binary actuator, nothing more: the
// application decides, this driver writes the pin. D5 is OC0B, so the
// hardware implementation drives GPIO and leaves COM0B at 00.
class IRelay {
public:
    virtual ~IRelay() = default;

    virtual void init() = 0;
    virtual void set(bool on) = 0;
    virtual bool get() const = 0;
};

}  // namespace relay
