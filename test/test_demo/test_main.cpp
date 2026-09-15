#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "Demo.h"
#include "mock_key.h"
#include "mock_led.h"

using key::MockKey;
using led::MockLed;
using logic::Demo;

void setUp(void) {}
void tearDown(void) {}

// Positions 1 and 2 are the key states, 3 to 8 the six operators.
static void expectRow(const MockLed& leds, bool k1, bool k2, bool andv, bool orv,
                      bool xorv, bool nandv, bool norv, bool xnorv) {
    TEST_ASSERT_TRUE(leds.get(1) == k1);
    TEST_ASSERT_TRUE(leds.get(2) == k2);
    TEST_ASSERT_TRUE(leds.get(3) == andv);
    TEST_ASSERT_TRUE(leds.get(4) == orv);
    TEST_ASSERT_TRUE(leds.get(5) == xorv);
    TEST_ASSERT_TRUE(leds.get(6) == nandv);
    TEST_ASSERT_TRUE(leds.get(7) == norv);
    TEST_ASSERT_TRUE(leds.get(8) == xnorv);
}

static void inits_the_drivers(void) {
    MockKey keys;
    MockLed leds;
    Demo demo(keys, leds);

    demo.init();

    TEST_ASSERT_TRUE(keys.inited());
    TEST_ASSERT_TRUE(leds.inited());
}

static void both_released(void) {
    MockKey keys;
    MockLed leds;
    Demo demo(keys, leds);
    demo.init();

    demo.service();

    expectRow(leds, false, false, false, false, false, true, true, true);
}

static void key2_only(void) {
    MockKey keys;
    MockLed leds;
    Demo demo(keys, leds);
    demo.init();

    keys.press(2, true);
    demo.service();

    expectRow(leds, false, true, false, true, true, true, false, false);
}

static void key1_only(void) {
    MockKey keys;
    MockLed leds;
    Demo demo(keys, leds);
    demo.init();

    keys.press(1, true);
    demo.service();

    expectRow(leds, true, false, false, true, true, true, false, false);
}

static void both_pressed(void) {
    MockKey keys;
    MockLed leds;
    Demo demo(keys, leds);
    demo.init();

    keys.press(1, true);
    keys.press(2, true);
    demo.service();

    expectRow(leds, true, true, true, true, false, false, false, true);
}

static void key3_does_not_affect_the_row(void) {
    MockKey keys;
    MockLed leds;
    Demo demo(keys, leds);
    demo.init();

    keys.press(3, true);
    demo.service();

    expectRow(leds, false, false, false, false, false, true, true, true);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(inits_the_drivers);
    RUN_TEST(both_released);
    RUN_TEST(key2_only);
    RUN_TEST(key1_only);
    RUN_TEST(both_pressed);
    RUN_TEST(key3_does_not_affect_the_row);
    return UNITY_END();
}
