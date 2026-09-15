#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "display_hw.h"
#include "mock_lcd.h"

using display::DisplayHw;
using lcd::MockLcd;

void setUp(void) {}
void tearDown(void) {}

static void assertRange(const MockLcd& lcd) {
    TEST_ASSERT_EQUAL_INT('B', lcd.cell(0, 0));
    TEST_ASSERT_EQUAL_INT('A', lcd.cell(1, 0));
    TEST_ASSERT_EQUAL_INT('R', lcd.cell(2, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(3, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(4, 0));
    TEST_ASSERT_EQUAL_INT('1', lcd.cell(5, 0));
    TEST_ASSERT_EQUAL_INT('-', lcd.cell(6, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(7, 0));
    TEST_ASSERT_EQUAL_INT('8', lcd.cell(8, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(9, 0));
}

static void init_brings_up_the_lcd(void) {
    MockLcd lcd;
    DisplayHw display(lcd);

    display.init();

    TEST_ASSERT_TRUE(lcd.inited());
}

static void showUInt_does_not_write_until_a_tick_is_serviced(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.showUInt(210);

    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 0));

    display.service();

    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 0));

    display.onTick();
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 0));

    display.service();

    assertRange(lcd);
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(11, 0));
    TEST_ASSERT_EQUAL_INT('1', lcd.cell(12, 0));
    TEST_ASSERT_EQUAL_INT('.', lcd.cell(13, 0));
    TEST_ASSERT_EQUAL_INT('0', lcd.cell(14, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(15, 0));
}

static void a_small_value_keeps_leading_spaces(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.showUInt(21);
    display.onTick();
    display.service();

    assertRange(lcd);
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(11, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(12, 0));
    TEST_ASSERT_EQUAL_INT('.', lcd.cell(13, 0));
    TEST_ASSERT_EQUAL_INT('1', lcd.cell(14, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(15, 0));
}

static void twenty_eight_is_right_justified_with_a_decimal(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.showUInt(280);
    display.onTick();
    display.service();

    assertRange(lcd);
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(11, 0));
    TEST_ASSERT_EQUAL_INT('8', lcd.cell(12, 0));
    TEST_ASSERT_EQUAL_INT('.', lcd.cell(13, 0));
    TEST_ASSERT_EQUAL_INT('0', lcd.cell(14, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(15, 0));
}

static void row_two_is_left_alone(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.showUInt(210);
    display.onTick();
    display.service();

    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(0, 1));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(init_brings_up_the_lcd);
    RUN_TEST(showUInt_does_not_write_until_a_tick_is_serviced);
    RUN_TEST(a_small_value_keeps_leading_spaces);
    RUN_TEST(twenty_eight_is_right_justified_with_a_decimal);
    RUN_TEST(row_two_is_left_alone);
    return UNITY_END();
}
