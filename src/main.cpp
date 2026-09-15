// Part 4 firmware: solar panel control. NTC on A0, Grove keys on D2–D4,
// encoder on D6/D7 with SW on D12 (active low), servo PWM on D9.
//
// Traces state changes over USART0 at 115200: keys, the encoder switch,
// temperature, valve and pump, and saved setpoints.
#if defined(__AVR__) && !defined(PIO_UNIT_TESTING)

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "Adc.h"
#include "IUsart.h"
#include "Solar.h"
#include "display_hw.h"
#include "encoder_hw.h"
#include "key_hw.h"
#include "lcd_hw.h"
#include "led_hw.h"
#include "servo_hw.h"
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

void printBit(usart::IUsart& out, bool value) {
    out.write(value ? static_cast<uint8_t>('1') : static_cast<uint8_t>('0'));
}

}  // namespace

int main() {
    twi::Twi bus;
    lcd::LcdHw lcd(bus);
    led::LedHw leds(lcd);
    display::DisplayHw numeric(lcd);
    adc::Adc adc;
    temp::TemperatureHw sensor(adc, kThermistorPin);
    key::KeyHw keys;
    encoder::EncoderHw encoder;
    servo::ServoHw valve;
    timer::TimerHw clock;
    solar::Solar app(sensor, leds, numeric, keys, encoder, valve);
    usart::UsartHw trace;

    trace.init();
    print(trace, "\r\npart4 boot: ntc A0  keys D2-D4  enc D6/D7  sw D12  servo D9\r\n");

    app.init();
    clock.init(&app);

    print(trace, "drivers up  tlow=18 thigh=25\r\n");

    sei();
    clock.start();

    int16_t prevTemp = 0x7FFF;
    uint8_t prevKeys = 0xFF;
    uint8_t prevSw = 0xFF;
    uint8_t prevTlow = 0xFF;
    uint8_t prevThigh = 0xFF;
    bool prevValve = !app.valveOpen();
    bool prevPump = !app.pump();

    for (;;) {
        app.service();

        const uint8_t keyBits = static_cast<uint8_t>((PIND >> 2) & 0x07);
        if (keyBits != prevKeys) {
            print(trace, "keys=");
            printDec(trace, static_cast<int16_t>(keyBits));
            print(trace, "\r\n");
            prevKeys = keyBits;
        }

        const uint8_t sw = (PINB & static_cast<uint8_t>(_BV(PB4))) != 0 ? 1 : 0;
        if (sw != prevSw) {
            print(trace, sw == 0 ? "sw down\r\n" : "sw up\r\n");
            prevSw = sw;
        }

        if (app.tlow() != prevTlow || app.thigh() != prevThigh) {
            prevTlow = app.tlow();
            prevThigh = app.thigh();
            print(trace, "setpoints tlow=");
            printDec(trace, static_cast<int16_t>(prevTlow));
            print(trace, " thigh=");
            printDec(trace, static_cast<int16_t>(prevThigh));
            print(trace, "\r\n");
        }

        const int16_t now = app.temperature().raw();
        if (now != prevTemp || app.valveOpen() != prevValve || app.pump() != prevPump) {
            prevTemp = now;
            prevValve = app.valveOpen();
            prevPump = app.pump();
            print(trace, "t ");
            printDec(trace, now);
            print(trace, "  valve=");
            printBit(trace, prevValve);
            print(trace, " pump=");
            printBit(trace, prevPump);
            print(trace, "\r\n");
        }
    }
}

#endif
