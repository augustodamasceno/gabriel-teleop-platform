/* gabriel-teleop-platform - Hardware Abstraction Layer Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include "hal.h"
#include "pinout.h"
#include "interface.h"


void hal_init()
{
    hal_gpio_init();
    hal_pwm_init();
    hal_adc_init();
}

void hal_gpio_init()
{
    // Set direction and brake pins as outputs
    DDRB |= (1 << DIRECTION_PIN);
    DDRB |= (1 << BRAKES_PIN);
    
    // Initialize to safe state (brakes engaged)
    PORTB &= ~(1 << DIRECTION_PIN);
    PORTB |= (1 << BRAKES_PIN);
}

void hal_pwm_init()
{
    // Configure Timer0 for Fast PWM mode
    // WGM02:0 = 011 (Fast PWM, TOP=0xFF)
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    TCCR0B &= ~(1 << WGM02);
    
    // Set prescaler to 64: CS02:0 = 011
    // PWM frequency = 16MHz / (64 * 256) = 976.5625 Hz
    TCCR0B |= (1 << CS01) | (1 << CS00);
    TCCR0B &= ~(1 << CS02);
    
    // Enable PWM on OC0A (PD6 - Steering) and OC0B (PD5 - Acceleration)
    // Non-inverting mode: COM0A1:0 = 10, COM0B1:0 = 10
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
    TCCR0A &= ~((1 << COM0A0) | (1 << COM0B0));
    
    // Set PWM pins as outputs
    DDRD |= (1 << WRITE_STEERING_PIN);
    DDRD |= (1 << WRITE_ACCELERATION_PIN);
    
    // Initialize PWM duty cycles to 0
    OCR0A = 0;
    OCR0B = 0;
}

void hal_adc_init()
{
    // AVCC as reference with external capacitor on AREF pin
    ADMUX = (1 << REFS0);
    
    // Enable ADC, set prescaler to 128 for 125kHz ADC clock at 16MHz
    // ADEN=1, ADPS2:0=111
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void hal_set_steering_pwm(uint16_t value)
{
    // Map from 0-18000 to 0-255
    if (value > STEERING_MAX)
    {
        value = STEERING_MAX;
    }
    OCR0A = (uint8_t)((value * 255UL) / STEERING_MAX);
}

void hal_set_acceleration_pwm(uint16_t value)
{
    // Map from 0-10000 to 0-255
    if (value > ACCELERATION_MAX)
    {
        value = ACCELERATION_MAX;
    }
    OCR0B = (uint8_t)((value * 255UL) / ACCELERATION_MAX);
}

void hal_set_direction(uint8_t direction)
{
    if (direction == 0)
    {
        PORTB &= ~(1 << DIRECTION_PIN);
    }
    else
    {
        PORTB |= (1 << DIRECTION_PIN);
    }
}

void hal_set_brakes(uint8_t brakes)
{
    if (brakes == 0)
    {
        PORTB &= ~(1 << BRAKES_PIN);
    }
    else
    {
        PORTB |= (1 << BRAKES_PIN);
    }
}

static uint16_t hal_read_adc(uint8_t channel)
{
    // Select ADC channel (0-7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));
    
    // Return 10-bit result
    return ADC;
}

uint16_t hal_read_steering_angle()
{
    // Read from ADC0 (PC0)
    uint16_t adc_value = hal_read_adc(0);
    
    // Map 10-bit ADC (0-1023) to steering range (0-18000)
    return (uint16_t)((adc_value * (uint32_t)STEERING_MAX) / 1023UL);
}

uint16_t hal_read_speed()
{
    // Read from ADC1 (PC1)
    uint16_t adc_value = hal_read_adc(1);
    
    // Map 10-bit ADC (0-1023) to acceleration range (0-10000)
    return (uint16_t)((adc_value * (uint32_t)ACCELERATION_MAX) / 1023UL);
}

void hal_emergency_stop()
{
    hal_disable_all_outputs();
    PORTB |= (1 << BRAKES_PIN);
}

void hal_disable_all_outputs()
{
    OCR0A = 0;
    OCR0B = 0;
    PORTB &= ~(1 << DIRECTION_PIN);
}
