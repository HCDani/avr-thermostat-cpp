// Part 1 firmware: Grove buttons on D2/D3/D4 and LCD row 2 as eight status
// cells. The demo writes key 1 and key 2 into positions 1 and 2, then
// AND/OR/XOR/NAND/NOR/XNOR of the two into positions 3 to 8.
// I2C and debounce run in the main loop, never in an ISR.
//
// The same states are traced over USART0 at 115200 8N1, so the keys can be
// watched on the serial monitor as well as on the LCD.
#if defined(__AVR__) && !defined(PIO_UNIT_TESTING)

#include "Demo.h"
#include "IUsart.h"
#include "key_hw.h"
#include "lcd_hw.h"
#include "led_hw.h"
#include "Twi.h"
#include "usart_hw.h"

namespace {

void print(usart::IUsart& out, const char* s) {
    while (*s != '\0') {
        out.write(static_cast<uint8_t>(*s));
        ++s;
    }
}

void printBit(usart::IUsart& out, bool value) {
    out.write(value ? static_cast<uint8_t>('1') : static_cast<uint8_t>('0'));
}

void printHex(usart::IUsart& out, uint8_t value) {
    const char* digits = "0123456789ABCDEF";
    out.write(static_cast<uint8_t>(digits[value >> 4]));
    out.write(static_cast<uint8_t>(digits[value & 0x0F]));
}

}  // namespace

int main() {
    twi::Twi bus;
    lcd::LcdHw lcd(bus);
    led::LedHw leds(lcd);
    key::KeyHw keys;
    logic::Demo demo(keys, leds);
    usart::UsartHw trace;

    trace.init();
    print(trace, "\r\npart1 boot: usart0 115200 8N1\r\n");
    print(trace, "init lcd 0x3E rgb 0x62 (LCD RGB Backlight v2.0), keys D2 D3 D4\r\n");

    demo.init();

    print(trace, bus.ok() ? "lcd acked\r\n" : "lcd did NOT ack, twsr 0x");
    if (!bus.ok()) {
        printHex(trace, bus.status());
        print(trace, " (FF = bus never released; keys still traced below)\r\n");
    }
    // Temporary bring-up label on row 1, which Part 3 will own. If this text is
    // readable then the bus, the init sequence and the backlight are all good,
    // which separates an LCD fault from a row-2 glyph fault.
    // Temporary bring-up label on row 1, which Part 3 will own. If this text is
    // readable then the bus, the init sequence and the backlight are all good,
    // which separates an LCD fault from a row-2 glyph fault.
    lcd.setCursor(0, 0);
    lcd.write("PART1 KEYS");

    print(trace, "drivers up   keys 123  row2 K1 K2 AND OR XOR NAND NOR XNOR\r\n");

    bool previous[3] = {false, false, false};
    bool first = true;

    for (;;) {
        // Latch the debounced states first. Reading them here before service()
        // keeps the traced row in step with the keys: whichever call sees a
        // change first is the one that settles the debounce, and service() must
        // not be the one left behind.
        const bool k1 = keys.get(1);
        const bool k2 = keys.get(2);
        const bool k3 = keys.get(3);

        demo.service();

        if (first || k1 != previous[0] || k2 != previous[1] || k3 != previous[2]) {
            print(trace, "keys ");
            printBit(trace, k1);
            printBit(trace, k2);
            printBit(trace, k3);
            print(trace, "  row2 ");
            for (uint8_t n = 1; n <= 8; ++n) {
                printBit(trace, leds.get(n));
            }
            print(trace, "\r\n");

            previous[0] = k1;
            previous[1] = k2;
            previous[2] = k3;
            first = false;
        }
    }
}

#endif
