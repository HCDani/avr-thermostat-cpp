#pragma once

// Present for every Unity build: PlatformIO includes this as soon as it
// exists, including on the host. Host output stays putchar; only the target
// redirects through USART0.
#if defined(__AVR__)

#include <stdint.h>

void usart_hw_init(void);
void usart_hw_write(uint8_t byte);
void usart_hw_flush(void);

#define UNITY_OUTPUT_START() usart_hw_init()
#define UNITY_OUTPUT_CHAR(c) usart_hw_write((uint8_t)(c))
#define UNITY_OUTPUT_FLUSH() usart_hw_flush()
#define UNITY_OUTPUT_COMPLETE() usart_hw_flush()

#endif
