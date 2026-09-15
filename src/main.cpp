// Part 2+3 firmware: NTC on A0, sampled once per 1 Hz tick. Bar on LCD row 2
// spanning 21–28 C; tenths of a degree on the numeric display (row 1). The
// tick only starts a conversion and marks a refresh; I2C and interpolation
// run in the main loop.
#if defined(__AVR__) && !defined(PIO_UNIT_TESTING)

#include <avr/interrupt.h>
#include <stdint.h>

#include "Adc.h"
#include "IUsart.h"
#include "Thermometer.h"
#include "display_hw.h"
#include "lcd_hw.h"
#include "led_hw.h"
#include "temp_hw.h"
#include "timer_hw.h"
#include "Twi.h"
#include "usart_hw.h"

namespace {

const uint8_t kThermistorPin = 0;

void print(usart::IUsart& out, const char* s) {
    while (*s != '\0') {
        out.write(static_cast<uint8_t>(*s));
        ++s;
    }
}

void printBit(usart::IUsart& out, bool value) {
    out.write(value ? static_cast<uint8_t>('1') : static_cast<uint8_t>('0'));
}

void printDec(usart::IUsart& out, int16_t value) {
    if (value < 0) {
        out.write(static_cast<uint8_t>('-'));
        value = static_cast<int16_t>(-value);
    }
    char buf[6];
    uint8_t n = 0;
    do {
        buf[n++] = static_cast<char>('0' + (value % 10));
        value = static_cast<int16_t>(value / 10);
    } while (value != 0);
    while (n > 0) {
        --n;
        out.write(static_cast<uint8_t>(buf[n]));
    }
}

}  // namespace

int main() {
    twi::Twi bus;
    lcd::LcdHw lcd(bus);
    led::LedHw leds(lcd);
    display::DisplayHw numeric(lcd);
    adc::Adc adc;
    temp::TemperatureHw sensor(adc, kThermistorPin);
    timer::TimerHw clock;
    thermo::Thermometer thermo(sensor, leds, numeric);
    usart::UsartHw trace;

    trace.init();
    print(trace, "\r\npart3 boot: usart0 115200 8N1  ntc A0  bar 21-28 C\r\n");

    thermo.init();
    clock.init(&thermo);

    print(trace, "drivers up\r\n");

    sei();
    clock.start();

    int16_t previous = 0x7FFF;

    for (;;) {
        thermo.service();

        const int16_t now = thermo.temperature().raw();
        if (now != previous) {
            print(trace, "t ");
            printDec(trace, now);
            print(trace, "  bar ");
            for (uint8_t n = 1; n <= 8; ++n) {
                printBit(trace, leds.get(n));
            }
            print(trace, "\r\n");
            previous = now;
        }
    }
}

#endif
