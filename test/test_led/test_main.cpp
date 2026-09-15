#include <unity.h>

#ifdef __AVR__
#include "usart/usart_c.h"
#endif

#include "led_hw.h"
#include "mock_lcd.h"

using lcd::MockLcd;
using led::LedHw;

void setUp(void) {}
void tearDown(void) {}

static void init_brings_up_the_lcd_and_clears_the_status_row(void) {
    MockLcd lcd;
    LedHw leds(lcd);

    leds.init();

    TEST_ASSERT_TRUE(lcd.inited());
    for (uint8_t col = 0; col < 8; ++col) {
        TEST_ASSERT_EQUAL_INT(' ', lcd.cell(col, 1));
        TEST_ASSERT_FALSE(leds.get(static_cast<uint8_t>(col + 1)));
    }
}

// The glyph is CGRAM slot 1, character code 0x01. Slot 0 would be code 0x00,
// which cannot sit in a C string.
static void set_writes_the_block_glyph_on_row_two(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();

    leds.set(1, true);
    leds.set(8, true);

    TEST_ASSERT_EQUAL_INT('\x01', lcd.cell(0, 1));
    TEST_ASSERT_EQUAL_INT('\x01', lcd.cell(7, 1));
    TEST_ASSERT_TRUE(leds.get(1));
    TEST_ASSERT_TRUE(leds.get(8));
    TEST_ASSERT_EQUAL_UINT8(1, lcd.cursorRow());
}

static void clearing_an_indicator_writes_a_space(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();

    leds.set(3, true);
    leds.set(3, false);

    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(2, 1));
    TEST_ASSERT_FALSE(leds.get(3));
}

static void row_one_is_left_alone(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();
    leds.set(1, true);

    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(0, 0));
}

static void bar_is_empty_below_the_floor(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();

    leds.bar(210, 280, 209);

    for (uint8_t col = 0; col < 8; ++col) {
        TEST_ASSERT_EQUAL_INT(' ', lcd.cell(col, 1));
        TEST_ASSERT_FALSE(leds.get(static_cast<uint8_t>(col + 1)));
    }
}

static void bar_lights_position_one_at_the_floor(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();

    leds.bar(210, 280, 210);

    TEST_ASSERT_EQUAL_INT('\x01', lcd.cell(0, 1));
    TEST_ASSERT_TRUE(leds.get(1));
    for (uint8_t col = 1; col < 8; ++col) {
        TEST_ASSERT_EQUAL_INT(' ', lcd.cell(col, 1));
    }
}

static void bar_fills_every_degree_from_twenty_one_to_twenty_eight(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();

    leds.bar(210, 280, 270);

    for (uint8_t col = 0; col < 7; ++col) {
        TEST_ASSERT_EQUAL_INT('\x01', lcd.cell(col, 1));
    }
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(7, 1));
}

static void bar_is_full_at_the_ceiling(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();

    leds.bar(210, 280, 280);

    for (uint8_t col = 0; col < 8; ++col) {
        TEST_ASSERT_EQUAL_INT('\x01', lcd.cell(col, 1));
        TEST_ASSERT_TRUE(leds.get(static_cast<uint8_t>(col + 1)));
    }
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(0, 0));
}

static void write_puts_text_after_the_bar(void) {
    MockLcd lcd;
    LedHw leds(lcd);
    leds.init();
    leds.bar(210, 280, 280);

    leds.write(8, "   25.0C");

    TEST_ASSERT_EQUAL_INT('\x01', lcd.cell(7, 1));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(8, 1));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(9, 1));
    TEST_ASSERT_EQUAL_INT(' ', lcd.cell(10, 1));
    TEST_ASSERT_EQUAL_INT('2', lcd.cell(11, 1));
    TEST_ASSERT_EQUAL_INT('5', lcd.cell(12, 1));
    TEST_ASSERT_EQUAL_INT('.', lcd.cell(13, 1));
    TEST_ASSERT_EQUAL_INT('0', lcd.cell(14, 1));
    TEST_ASSERT_EQUAL_INT('C', lcd.cell(15, 1));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(init_brings_up_the_lcd_and_clears_the_status_row);
    RUN_TEST(set_writes_the_block_glyph_on_row_two);
    RUN_TEST(clearing_an_indicator_writes_a_space);
    RUN_TEST(row_one_is_left_alone);
    RUN_TEST(bar_is_empty_below_the_floor);
    RUN_TEST(bar_lights_position_one_at_the_floor);
    RUN_TEST(bar_fills_every_degree_from_twenty_one_to_twenty_eight);
    RUN_TEST(bar_is_full_at_the_ceiling);
    RUN_TEST(write_puts_text_after_the_bar);
    return UNITY_END();
}
