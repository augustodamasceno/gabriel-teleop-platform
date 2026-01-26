/* gabriel-teleop-platform - Communication Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "communication.h"
#include "system_state.h"
#include "interface.h"
#include "safety.h"


extern volatile SystemState system_state;

static volatile uint8_t rx_buffer[16];
static volatile uint8_t rx_write_pos = 0;
static volatile uint8_t rx_read_pos = 0;


void communication_send_system_state()
{
    // Send 3-byte header for packet synchronization (0xC3 0x3C 0xA5)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = SYSTEM_STATE_HEADER_1;
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = SYSTEM_STATE_HEADER_2;
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = SYSTEM_STATE_HEADER_3;
    
    // Send steering_angle (low byte, high byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.steering_angle & 0xFF);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.steering_angle >> 8);
    
    // Send steering_angle_setpoint (low byte, high byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.steering_angle_setpoint & 0xFF);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.steering_angle_setpoint >> 8);
    
    // Send steering_manipulate_variable (low byte, high byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.steering_manipulate_variable & 0xFF);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.steering_manipulate_variable >> 8);
    
    // Send acceleration (low byte, high byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.acceleration & 0xFF);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.acceleration >> 8);
    
    // Send acceleration_setpoint (low byte, high byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.acceleration_setpoint & 0xFF);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.acceleration_setpoint >> 8);
    
    // Send acceleration_manipulate_variable (low byte, high byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.acceleration_manipulate_variable & 0xFF);
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = (uint8_t)(system_state.acceleration_manipulate_variable >> 8);
    
    // Send direction (1 byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = system_state.direction;

    // Send breaks (1 byte)
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = system_state.breaks;
}

static uint8_t is_valid_command_header(uint8_t header)
{
    return (header == CMD_HEADER_STEERING || 
            header == CMD_HEADER_ACCELERATION || 
            header == CMD_HEADER_DIRECTION ||
            header == CMD_HEADER_BRAKES);
}

void communication_config_usart()
{
    // Character Size 8
    // Stop Bit 2
    // Baud Rate 115200
    // No Parity 
    // Double Speed Mode
    UCSR0A |= (1 << U2X0);
    UCSR0B |= ( 1 << RXEN0) | ( 1 << TXEN0) | (1 << RXCIE0);
    UCSR0C |= ( 1 << UCSZ01 ) | ( 1 << UCSZ00 );
    
    // Baud Rate Setting for f_osc = 16MHz
    UBRR0H = ( unsigned char ) ( UBRR_VALUE >> 8) ;
    UBRR0L = ( unsigned char ) UBRR_VALUE ;
    
    // Enable global interrupts
    sei();
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

static uint8_t communication_usart_receive_char_timeout_direct(uint8_t *data, uint16_t timeout_loops)
{
    uint16_t count = 0;
    
    while (!(UCSR0A & (1 << RXC0)))
    {
        if (count++ >= timeout_loops)
        {
            return 0;  // Timeout
        }
        _delay_us(100);
    }
    
    if (communication_check_usart_errors())
    {
        return 0;  // Error
    }
    
    *data = UDR0;
    return 1;  // Success
}

static uint8_t communication_rx_read_timeout(uint8_t *data, uint16_t timeout_ms)
{
    uint16_t elapsed = 0;
    
    while (!communication_rx_available())
    {
        if (elapsed >= timeout_ms)
        {
            return 0;  // Timeout
        }
        _delay_ms(1);
        elapsed++;
    }
    
    *data = communication_rx_read();
    return 1;  // Success
}

void communication_receive_command()
{
    // Check if we have enough data in buffer for a command
    if (!communication_rx_available())
    {
        return;
    }
    
    uint8_t header = 0;
    uint16_t header_timeout = 0;
    
    // Wait for valid command header with timeout
    while (communication_rx_available())
    {
        header = communication_rx_read();
        if (is_valid_command_header(header))
        {
            break;
        }
        if (header_timeout++ > 100)
        {
            return;
        }
    }
    
    if (!is_valid_command_header(header))
    {
        return;
    }
    
    if (header == CMD_HEADER_STEERING)
    {
        uint8_t low_byte, high_byte;
        
        // Wait for data bytes in buffer (with timeout)
        uint16_t timeout = 0;
        while ((!communication_rx_available()) && (timeout++ < 1000))
        {
            _delay_us(10);
        }
        if (!communication_rx_available())
        {
            return;  // Timeout
        }
        low_byte = communication_rx_read();
        
        timeout = 0;
        while ((!communication_rx_available()) && (timeout++ < 1000))
        {
            _delay_us(10);
        }
        if (!communication_rx_available())
        {
            return;  // Timeout
        }
        high_byte = communication_rx_read();
        
        system_state.steering_angle_setpoint = ((uint16_t)high_byte << 8) | low_byte;
        
        // Validate and clamp
        if (system_state.steering_angle_setpoint > STEERING_MAX)
        {
            system_state.steering_angle_setpoint = STEERING_MAX;
        }
        safety_reset_watchdog();
    }
    else if (header == CMD_HEADER_ACCELERATION)
    {
        uint8_t low_byte, high_byte;
        
        // Wait for data bytes in buffer (with timeout)
        uint16_t timeout = 0;
        while ((!communication_rx_available()) && (timeout++ < 1000))
        {
            _delay_us(10);
        }
        if (!communication_rx_available())
        {
            return;  // Timeout
        }
        low_byte = communication_rx_read();
        
        timeout = 0;
        while ((!communication_rx_available()) && (timeout++ < 1000))
        {
            _delay_us(10);
        }
        if (!communication_rx_available())
        {
            return;  // Timeout
        }
        high_byte = communication_rx_read();
        
        system_state.acceleration_setpoint = ((uint16_t)high_byte << 8) | low_byte;
        
        // Validate and clamp
        if (system_state.acceleration_setpoint > ACCELERATION_MAX)
        {
            system_state.acceleration_setpoint = ACCELERATION_MAX;
        }
        safety_reset_watchdog();
    }
    else if (header == CMD_HEADER_DIRECTION)
    {
        uint8_t dir_byte;
        
        // Wait for data byte in buffer (with timeout)
        uint16_t timeout = 0;
        while ((!communication_rx_available()) && (timeout++ < 1000))
        {
            _delay_us(10);
        }
        if (!communication_rx_available())
        {
            return;  // Timeout
        }
        dir_byte = communication_rx_read();
        
        if (dir_byte <= 1)
        {
            system_state.direction = dir_byte;
            safety_reset_watchdog();
        }
    }
    else if (header == CMD_HEADER_BRAKES)
    {
        uint8_t brake_byte;
        
        // Wait for data byte in buffer (with timeout)
        uint16_t timeout = 0;
        while ((!communication_rx_available()) && (timeout++ < 1000))
        {
            _delay_us(10);
        }
        if (!communication_rx_available())
        {
            return;  // Timeout
        }
        brake_byte = communication_rx_read();
        
        if (brake_byte <= 1)
        {
            system_state.breaks = brake_byte;
            safety_reset_watchdog();
        }
    }
}

// USART RX Complete Interrupt Service Routine
ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    
    // Store in circular buffer
    uint8_t next_pos = (rx_write_pos + 1) % 16;
    if (next_pos != rx_read_pos)
    {
        rx_buffer[rx_write_pos] = data;
        rx_write_pos = next_pos;
    }
}

uint8_t communication_rx_available()
{
    return (rx_write_pos != rx_read_pos);
}

uint8_t communication_rx_read()
{
    if (rx_write_pos == rx_read_pos)
    {
        return 0;
    }
    
    uint8_t data = rx_buffer[rx_read_pos];
    rx_read_pos = (rx_read_pos + 1) % 16;
    return data;
}
