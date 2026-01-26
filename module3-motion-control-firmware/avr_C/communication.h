/* gabriel-teleop-platform - Communication Header
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_COMMUNICATION_H
#define _GABRIEL_TELEOP_COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define UBRR_VALUE 16
#define USART_TIMEOUT_MS 50
#define USART_TIMEOUT_LOOPS (USART_TIMEOUT_MS * 10)
#define SYSTEM_STATE_BUFFER_SIZE 128

static volatile char system_state_buffer[SYSTEM_STATE_BUFFER_SIZE];

void communication_config_usart();
uint8_t communication_check_usart_errors();
uint8_t communication_usart_receive_char_timeout(uint16_t timeout_loops);
void communication_send_system_state();
void communication_receive_command();
uint8_t communication_rx_available();
uint8_t communication_rx_read();

#ifdef __cplusplus
}
#endif

#endif // _GABRIEL_TELEOP_COMMUNICATION_H
