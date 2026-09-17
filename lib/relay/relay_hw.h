#pragma once

#include "IRelay.h"

namespace relay {

// Grove Relay SIG on D5 (PD5). The same command is written to D13 (PB5), the
// Uno's onboard LED, so the pump state is visible without looking at the
// module. COM0B is not touched: Timer 0 owns TCCR0A.
class RelayHw : public IRelay {
public:
    RelayHw();

    void init() override;
    void set(bool on) override;
    bool get() const override;

private:
    bool on_;
};

}  // namespace relay
