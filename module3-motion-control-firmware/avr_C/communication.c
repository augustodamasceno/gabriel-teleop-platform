/* gabriel-teleop-platform - Communication Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "communication.h"
#include "system_state.h"
#include "interface.h"


extern volatile SystemState system_state;


void communication_send_system_state()
{
    snprintf(system_state_buffer, 
             SYSTEM_STATE_BUFFER_SIZE, 
             "%05u %05u %05u %05u %05u %05u %01u\n", 
             system_state.steering_angle, 
             system_state.steering_angle_setpoint,
             system_state.steering_manipulate_variable,
             system_state.acceleration, 
             system_state.acceleration_setpoint,
             system_state.acceleration_manipulate_variable,
             system_state.direction);
    
    unsigned char c = 0;
    while( system_state_buffer[c] != '\0' )
    {
        while (!( UCSR0A & (1<<UDRE0) )) ;
        UDR0 = system_state_buffer[c];
        c++;
    }
}

static uint8_t is_valid_command_header(uint8_t header)
{
    return (header == CMD_HEADER_STEERING || 
            header == CMD_HEADER_ACCELERATION || 
            header == CMD_HEADER_DIRECTION);
}

void communication_config_usart()
{
    // Character Size 8
    // Stop Bit 2
    // Baud Rate 115200
    // No Parity 
    // Double Speed Mode
    UCSR0A |= (1 << U2X0);
    UCSR0B |= ( 1 << RXEN0) | ( 1 << TXEN0);
    UCSR0C |= ( 1 << UCSZ01 ) | ( 1 << UCSZ00 );
    
    // Baud Rate Setting for f_osc = 16MHz
    UBRR0H = ( unsigned char ) ( UBRR_VALUE >> 8) ;
    UBRR0L = ( unsigned char ) UBRR_VALUE ;
}

uint8_t communication_check_usart_errors()
{
    uint8_t status = UCSR0A;
    
    // Check for Frame Error, Data OverRun, or Parity Error
    if (status & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
    {
        // Read UDR0 to clear error flags
        uint8_t dummy = UDR0;
        (void)dummy;
        return 1;
    }
    return 0;
}

uint8_t communication_usart_receive_char_timeout(uint16_t timeout_loops)
{
    uint16_t count = 0;
    
    while (!(UCSR0A & (1 << RXC0)))
    {
        if (count++ >= timeout_loops)
        {
            return 0;
        }
        _delay_us(100);
    }
    
    if (communication_check_usart_errors())
    {
        return 0;
    }
    
    return UDR0;
}

void communication_receive_command()
{
    uint8_t header = 0;
    uint16_t header_timeout = 0;
    
    // Wait for valid command header with timeout
    while (1)
    {
        header = communication_usart_receive_char_timeout(USART_TIMEOUT_LOOPS);
        if (is_valid_command_header(header))
        {
            break;
        }
        if (header_timeout++ > 100)
        {
            return;
        }
    }
    
    if (header == CMD_HEADER_STEERING)
    {
        // Receive 2 bytes for steering setpoint (Little-endian)
        uint8_t low_byte = communication_usart_receive_char_timeout(USART_TIMEOUT_LOOPS);
        uint8_t high_byte = communication_usart_receive_char_timeout(USART_TIMEOUT_LOOPS);
        if (low_byte == 0 && high_byte == 0)
        {
            return;
        }
        system_state.steering_angle_setpoint = ((uint16_t)high_byte << 8) | low_byte;
        
        // Validate and clamp
        if (system_state.steering_angle_setpoint > STEERING_MAX)
        {
            system_state.steering_angle_setpoint = STEERING_MAX;
        }
    }
    else if (header == CMD_HEADER_ACCELERATION)
    {
        // Receive 2 bytes for acceleration setpoint (Little-endian)
        uint8_t low_byte = communication_usart_receive_char_timeout(USART_TIMEOUT_LOOPS);
        uint8_t high_byte = communication_usart_receive_char_timeout(USART_TIMEOUT_LOOPS);
        if (low_byte == 0 && high_byte == 0)
        {
            return;
        }
        system_state.acceleration_setpoint = ((uint16_t)high_byte << 8) | low_byte;
        
        // Validate and clamp
        if (system_state.acceleration_setpoint > ACCELERATION_MAX)
        {
            system_state.acceleration_setpoint = ACCELERATION_MAX;
        }
    }
    else if (header == CMD_HEADER_DIRECTION)
    {
        // Receive 1 byte for direction (0 = forward, 1 = reverse)
        uint8_t dir_byte = communication_usart_receive_char_timeout(USART_TIMEOUT_LOOPS);
        if (dir_byte <= 1)
        {
            system_state.direction = dir_byte;
        }
    }
}
