/* gabriel-teleop-platform - Module 3 Pinout Definitions
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_PINOUT_H
#define _GABRIEL_TELEOP_PINOUT_H

#ifdef __cplusplus
extern "C" {
#endif

// Pin 1 of PORTD (Arduino Digital Pin 1)
#define USART_TX_PIN             PD1    
// Pin 0 of PORTD (Arduino Digital Pin 0)  
#define USART_RX_PIN             PD0      
// Pin 0 of PORTC (Arduino Analog Pin A0, ADC0)
#define READ_STEERING_PIN        PC0      
 // Pin 6 of PORTD (Arduino Digital Pin 6, OC0A)
#define WRITE_STEERING_PIN       PD6     
// Pin 1 of PORTC (Arduino Analog Pin A1, ADC1)
#define READ_SPEED_PIN           PC1      
// Pin 5 of PORTD (Arduino Digital Pin 5, OC0B)
#define WRITE_ACCELERATION_PIN   PD5    
// Pin 0 of PORTB (Arduino Digital Pin 8)
#define DIRECTION_PIN           PB0      
// Pin 1 of PORTB (Arduino Digital Pin 9)
#define BRAKES_PIN              PB1      

#ifdef __cplusplus
}
#endif

#endif /* _GABRIEL_TELEOP_PINOUT_H */
