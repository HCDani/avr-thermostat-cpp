#include "Demo.h"

namespace logic {

Demo::Demo(key::IKey& keys, led::ILed& leds) : keys_(keys), leds_(leds) {}

void Demo::init() {
    keys_.init();
    leds_.init();
}

void Demo::service() {
    const bool a = keys_.get(1);
    const bool b = keys_.get(2);

    // Positions 1 and 2 are the inputs themselves, so the operands can be read
    // off the row beside the results.
    leds_.set(1, a);
    leds_.set(2, b);

    leds_.set(3, a && b);
    leds_.set(4, a || b);
    leds_.set(5, a != b);
    leds_.set(6, !(a && b));
    leds_.set(7, !(a || b));
    leds_.set(8, a == b);
}

}  // namespace logic
