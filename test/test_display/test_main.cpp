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

static void init_brings_up_the_lcd(void) {
    MockLcd lcd;
    DisplayHw display(lcd);

    display.init();

    TEST_ASSERT_TRUE(lcd.inited());
}

static void temp_twenty_three_matches_the_spec_layout(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.setHeading("TEMP");
    display.showUInt(23);
    display.service();

    TEST_ASSERT_EQUAL_INT('T', lcd.cell(0, 0));
    TEST_ASSERT_EQUAL_INT('E', lcd.cell(1, 0));
    TEST_ASSERT_EQUAL_INT('M', lcd.cell(2, 0));
    TEST_ASSERT_EQUAL_INT('P', lcd.cell(3, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(4, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(5, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(6, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(7, 0));
    TEST_ASSERT_EQUAL_INT('3', lcd.cell(8, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(9, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(10, 0));
}

static void thigh_uses_two_spaces_before_the_digits(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.setHeading("THIGH");
    display.showUInt(25);
    display.service();

    TEST_ASSERT_EQUAL_INT('T', lcd.cell(0, 0));
    TEST_ASSERT_EQUAL_INT('H', lcd.cell(1, 0));
    TEST_ASSERT_EQUAL_INT('I', lcd.cell(2, 0));
    TEST_ASSERT_EQUAL_INT('G', lcd.cell(3, 0));
    TEST_ASSERT_EQUAL_INT('H', lcd.cell(4, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(5, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(6, 0));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(7, 0));
    TEST_ASSERT_EQUAL_INT('5', lcd.cell(8, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(9, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(10, 0));
}

static void tlow_eighteen_matches_the_spec_layout(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.setHeading("TLOW");
    display.showUInt(18);
    display.service();

    TEST_ASSERT_EQUAL_INT('T', lcd.cell(0, 0));
    TEST_ASSERT_EQUAL_INT('L', lcd.cell(1, 0));
    TEST_ASSERT_EQUAL_INT('O', lcd.cell(2, 0));
    TEST_ASSERT_EQUAL_INT('W', lcd.cell(3, 0));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(6, 0));
    TEST_ASSERT_EQUAL_INT('1', lcd.cell(7, 0));
    TEST_ASSERT_EQUAL_INT('8', lcd.cell(8, 0));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(10, 0));
}

static void onTick_without_showUInt_still_refreshes(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();
    display.setHeading("TEMP");
    display.showUInt(23);
    display.service();

    display.onTick();
    display.service();

    TEST_ASSERT_EQUAL_INT('3', lcd.cell(8, 0));
}

static void row_two_is_left_alone(void) {
    MockLcd lcd;
    DisplayHw display(lcd);
    display.init();

    display.setHeading("TEMP");
    display.showUInt(23);
    display.service();

    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(0, 1));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(init_brings_up_the_lcd);
    RUN_TEST(temp_twenty_three_matches_the_spec_layout);
    RUN_TEST(thigh_uses_two_spaces_before_the_digits);
    RUN_TEST(tlow_eighteen_matches_the_spec_layout);
    RUN_TEST(onTick_without_showUInt_still_refreshes);
    RUN_TEST(row_two_is_left_alone);
    return UNITY_END();
}
