#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "IKey.h"
#include "mock_key.h"

using key::IKey;
using key::MockKey;

void setUp(void) {}
void tearDown(void) {}

static void keys_start_released(void) {
    MockKey keys;
    TEST_ASSERT_FALSE(keys.get(1));
    TEST_ASSERT_FALSE(keys.get(2));
    TEST_ASSERT_FALSE(keys.get(3));
}

static void press_is_visible_through_the_interface(void) {
    MockKey keys;
    IKey& asInterface = keys;

    keys.press(2, true);

    TEST_ASSERT_FALSE(asInterface.get(1));
    TEST_ASSERT_TRUE(asInterface.get(2));
    TEST_ASSERT_FALSE(asInterface.get(3));

    keys.press(2, false);
    TEST_ASSERT_FALSE(asInterface.get(2));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(keys_start_released);
    RUN_TEST(press_is_visible_through_the_interface);
    return UNITY_END();
}
