/* gabriel-teleop-platform - Motion Control Firmware for ATMega328P
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

// CPU frequency
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h> 
#include <stdio.h>
#include <string.h>

#define SYSTEM_DELAY_MS 10
#define USART_BUFFER_SIZE 26
#define HEADER_BYTE 0xFF // higher than max angle (first data)
#define BYTES_DATA 4

typedef enum {
    STATE_CONFIGURE,
    STATE_SEND_POSITION,
    STATE_RECEIVE_POSITION,
    STATE_CONTROL
} State;

// System Memory
State current_state = STATE_CONFIGURE;
volatile char usart_buffer[USART_BUFFER_SIZE];
// [0.00, 180.00] x 100
volatile uint16_t steering_angle = 90; 
volatile uint16_t steering_setpoint = 90;
// [0.00, 100.00] x 100
volatile uint16_t acceleration = 0;
volatile uint16_t acceleration_setpoint = 0;
// 0 forward and 1 reverse
volatile uint8_t direction = 0;

// Hardware Configuration
void config_usart();

// State Executions and Utils
void control();
void retrieve_setpoints();

// Communication
uint8_t usart_receive_char();
void send_position();
void receive_position();


int main(){
    while(1)
    {
        // Machine State
        switch(current_state)
        {
        case STATE_CONFIGURE:
            config_usart();
            current_state = STATE_SEND_POSITION;
            break;
        case STATE_SEND_POSITION:
            send_position();
            current_state = STATE_RECEIVE_POSITION;
            break;
        case STATE_RECEIVE_POSITION:
            receive_position();
            current_state = STATE_CONTROL;
            break;
        case STATE_CONTROL:
            retrieve_setpoints();
            control();
            current_state = STATE_SEND_POSITION;
            break;
        }

        // System Delay
        _delay_ms(SYSTEM_DELAY_MS);
    }
}

void config_usart()
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
    unsigned int ubrr = 16;
    UBRR0H = ( unsigned char ) ( ubrr>>8) ;
    UBRR0L = ( unsigned char ) ubrr ;
}

uint8_t usart_receive_char() {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void send_position()
{
  sprintf(usart_buffer,"%05u %05u %05u %05u\n", steering_angle, acceleration, steering_setpoint, acceleration_setpoint);
  unsigned char c = 0;
  while( usart_buffer[c] != '\0' )
  {
    while (!( UCSR0A & (1<<UDRE0) )) ;
    UDR0 = usart_buffer[c];
    c++;
  }
}

void receive_position()
{
    uint8_t index = 0;
    uint8_t active = 0;
    char received_char = HEADER_BYTE+1;
    memset((void*)usart_buffer, 0, BYTES_DATA+1);

    // Wait Header
    while (1)
        if (usart_receive_char() == HEADER_BYTE)
            break;

    while (index < BYTES_DATA) {
        usart_buffer[index] = usart_receive_char();
        index++;
    }
}


void retrieve_setpoints()
{
    // Steering
    steering_setpoint = (uint16_t)usart_buffer[1];
    steering_setpoint = steering_setpoint << 8;
    steering_setpoint = steering_setpoint | (uint8_t)usart_buffer[0];

    // Acceleration
    acceleration_setpoint = (uint16_t)usart_buffer[3];
    acceleration_setpoint = acceleration_setpoint << 8;
    acceleration_setpoint = acceleration_setpoint | (uint8_t)usart_buffer[2];
}


void control()
{
    // TO DO: PID Controller
}