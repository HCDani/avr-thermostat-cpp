#include "lcd_hw.h"

#ifdef __AVR__
#include <util/delay.h>
#endif

namespace lcd {

namespace {

const uint8_t kLcdAddress = 0x3E;
const uint8_t kRgbAddress = 0x62;

const uint8_t kControlCommand = 0x80;
const uint8_t kControlData = 0x40;

const uint8_t kClearDisplay = 0x01;
const uint8_t kEntryModeSet = 0x04;
const uint8_t kDisplayControl = 0x08;
const uint8_t kFunctionSet = 0x20;
const uint8_t kSetCgramAddr = 0x40;
const uint8_t kSetDdramAddr = 0x80;

const uint8_t kEntryLeft = 0x02;
const uint8_t kDisplayOn = 0x04;
const uint8_t kTwoLine = 0x08;

// Backlight PWM per channel. This module fixes the LCD bias in hardware and
// ignores the AiP31068 contrast registers, so brightness is the only lever on
// readability; full brightness washes the characters out. 64 was chosen on the
// bench.
const uint8_t kBacklightLevel = 64;

// CGRAM slot holding the filled block, and therefore its character code.
const uint8_t kBlockGlyph = 0x01;

const uint8_t kColumns = 16;

const uint8_t kRgbMode1 = 0x00;
const uint8_t kRgbMode2 = 0x01;
const uint8_t kRgbBlue = 0x02;
const uint8_t kRgbGreen = 0x03;
const uint8_t kRgbRed = 0x04;
const uint8_t kRgbOutput = 0x08;

}  // namespace

LcdHw::LcdHw(twi::Twi& bus) : bus_(bus) {}

// The controller needs about 40 us to execute an instruction and will not
// acknowledge a transfer that arrives while it is still busy. Give it that time
// rather than letting the next transfer be dropped.
void LcdHw::settle() {
#ifdef __AVR__
    _delay_us(50);
#endif
}

void LcdHw::command(uint8_t cmd) {
    const uint8_t packet[2] = {kControlCommand, cmd};
    bus_.write(kLcdAddress, packet, 2);
    settle();
}

void LcdHw::data(uint8_t value) {
    const uint8_t packet[2] = {kControlData, value};
    bus_.write(kLcdAddress, packet, 2);
    settle();
}

void LcdHw::rgbReg(uint8_t reg, uint8_t value) {
    const uint8_t packet[2] = {reg, value};
    bus_.write(kRgbAddress, packet, 2);
}

void LcdHw::init() {
#ifdef __AVR__
    bus_.init();

    _delay_ms(50);
    command(static_cast<uint8_t>(kFunctionSet | kTwoLine));
    _delay_ms(5);
    command(static_cast<uint8_t>(kFunctionSet | kTwoLine));
    _delay_ms(1);
    command(static_cast<uint8_t>(kFunctionSet | kTwoLine));

    command(static_cast<uint8_t>(kDisplayControl | kDisplayOn));
    command(kClearDisplay);
    _delay_ms(2);
    command(static_cast<uint8_t>(kEntryModeSet | kEntryLeft));

    // The filled block the status row uses; the character ROM has none. Slot 1,
    // not slot 0: slot 0's character code is 0x00, which cannot sit in a C
    // string, and the code 0x08 that HD44780 aliases onto it is not dependable
    // on this module's AiP31068.
    command(static_cast<uint8_t>(kSetCgramAddr | (kBlockGlyph << 3)));
    for (uint8_t row = 0; row < 8; ++row) {
        data(0x1F);
    }
    command(kSetDdramAddr);

    // Backlight. MODE2 must stay 0 and LEDOUT must be 0xAA (individual PWM):
    // MODE2 = 0x20 arms group blinking, and with LEDOUT asking for group
    // dimming as well the backlight can stay dark at the default group values.
    rgbReg(kRgbMode1, 0);
    rgbReg(kRgbMode2, 0x00);
    rgbReg(kRgbOutput, 0xAA);
    setBacklight(kBacklightLevel, kBacklightLevel, kBacklightLevel);
#endif
}

void LcdHw::setBacklight(uint8_t red, uint8_t green, uint8_t blue) {
#ifdef __AVR__
    rgbReg(kRgbRed, red);
    rgbReg(kRgbGreen, green);
    rgbReg(kRgbBlue, blue);
#else
    (void)red;
    (void)green;
    (void)blue;
#endif
}

void LcdHw::setCursor(uint8_t col, uint8_t row) {
#ifdef __AVR__
    const uint8_t offsets[2] = {0x00, 0x40};
    command(static_cast<uint8_t>(kSetDdramAddr | (col + offsets[row])));
#else
    (void)col;
    (void)row;
#endif
}

void LcdHw::write(const char* s) {
#ifdef __AVR__
    // One transfer for the whole string, not one per character. The control
    // byte's Co bit is clear, which means every byte after it is data, and a
    // single transfer gives the controller no gap in which to report busy and
    // lose the rest of the string.
    uint8_t packet[1 + kColumns];
    packet[0] = kControlData;

    uint8_t count = 0;
    while (s[count] != '\0' && count < kColumns) {
        packet[1 + count] = static_cast<uint8_t>(s[count]);
        ++count;
    }

    if (count != 0) {
        bus_.write(kLcdAddress, packet, static_cast<uint8_t>(1 + count));
        settle();
    }
#else
    (void)s;
#endif
}

}  // namespace lcd
