#include "Adc.h"

// Only the implementation touches registers; adc/Adc.h stays AVR-free so host
// code can name the contract without pulling in avr-libc.
#ifdef __AVR__

#include <avr/interrupt.h>
#include <avr/io.h>

namespace {

// The ISR is a free function at global scope and cannot be handed a this
// pointer, so the registered callback lives here.
hal::ConversionCallback g_callback = 0;

}  // namespace

namespace adc {

void Adc::start(uint8_t pin, ConversionCallback callback) {
    g_callback = callback;

    // AVcc as reference, result right-adjusted, low four bits pick the channel.
    ADMUX = static_cast<uint8_t>(_BV(REFS0) | (pin & 0x0F));

    // Prescaler /128 puts the ADC clock at 125 kHz on a 16 MHz part. Outside
    // 50-200 kHz the lowest bits of the result stop meaning anything.
    ADCSRA = static_cast<uint8_t>(_BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0));
    ADCSRA = static_cast<uint8_t>(ADCSRA | _BV(ADSC));
}

}  // namespace adc

ISR(ADC_vect) {
    // ADC is the 16-bit register pair; right-adjusted it holds all ten bits.
    if (g_callback != 0) {
        g_callback(static_cast<uint16_t>(ADC));
    }
}

#endif
