#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// C entry points for Unity, which is compiled as C and cannot call UsartHw.
// Same hardware as UsartHw: USART0 at 115200 8N1, transmit only.
void usart_hw_init(void);
void usart_hw_write(uint8_t byte);
void usart_hw_flush(void);

#ifdef __cplusplus
}
#endif
