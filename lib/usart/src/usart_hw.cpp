#include "usart/usart_hw.h"

#include "usart/usart_c.h"

#ifdef __AVR__

#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU
#error "F_CPU is not defined; PlatformIO sets it from the board manifest."
#endif

namespace {

const unsigned long kBaud = 115200UL;

// U2X=1: UBRR = F_CPU / (8 * baud) - 1. At 16 MHz that is 16, 2.1% error,
// which the Uno's USB-serial link accepts. Normal mode would be worse.
const unsigned long kUbrr = F_CPU / 8 / kBaud - 1UL;

static_assert(kUbrr <= 4095UL, "baud divisor exceeds UBRR's 12 bits");

void configure() {
    UBRR0H = static_cast<uint8_t>(kUbrr >> 8);
    UBRR0L = static_cast<uint8_t>(kUbrr & 0xFF);
    UCSR0A = static_cast<uint8_t>(_BV(U2X0));
    UCSR0B = static_cast<uint8_t>(_BV(TXEN0));
    UCSR0C = static_cast<uint8_t>(_BV(UCSZ01) | _BV(UCSZ00));
}

void put(uint8_t byte) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UCSR0A = static_cast<uint8_t>(UCSR0A | _BV(TXC0));
    UDR0 = byte;
}

void waitIdle() { loop_until_bit_is_set(UCSR0A, TXC0); }

}  // namespace

extern "C" void usart_hw_init(void) { configure(); }

extern "C" void usart_hw_write(uint8_t byte) { put(byte); }

extern "C" void usart_hw_flush(void) { waitIdle(); }

namespace usart {

void UsartHw::init() { configure(); }

void UsartHw::write(uint8_t byte) { put(byte); }

}  // namespace usart

#endif
