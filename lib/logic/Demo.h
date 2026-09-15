#pragma once

#include "IKey.h"
#include "ILed.h"

namespace logic {

// Part 1.3: row 2 shows key 1 and key 2 in positions 1 and 2, then AND, OR,
// XOR, NAND, NOR and XNOR of the two in positions 3 to 8.
class Demo {
public:
    Demo(key::IKey& keys, led::ILed& leds);

    void init();
    void service();

private:
    key::IKey& keys_;
    led::ILed& leds_;
};

}  // namespace logic
