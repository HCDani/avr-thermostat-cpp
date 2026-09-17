#pragma once

#include <stdint.h>

#include "IRelay.h"

namespace relay {

// Test mock. Counts transitions so a test can prove the pump was not
// chattered, which a plain get() cannot show.
class MockRelay : public IRelay {
public:
    MockRelay();

    void init() override;
    void set(bool on) override;
    bool get() const override;

    uint16_t transitions() const;
    bool inited() const;

private:
    bool on_;
    uint16_t transitions_;
    bool inited_;
};

}  // namespace relay
