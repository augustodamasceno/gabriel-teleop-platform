/* gabriel-teleop-platform - Motion Control Firmware for ATMega328P
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "controller.h"
#include "communication.h"
#include "interface.h"
#include "system_state.h"


void config_watchdog();

volatile SystemState system_state;

int main()
{
    // Configuration Routine
    config_watchdog();
    communication_config_usart();
    Controller controller;
    controller_init(&controller);
    
    // The main loop covers:
    //   1. Send the system state over USART periodically
    //   2. Controller Execution
    // The interruptions covers:
    //   1. Receiving command via USART and updating system state accordingly
    while(1)
    {
        wdt_reset();
        
        communication_send_system_state();
        controller_update(&controller, &system_state, SYSTEM_DELAY_S);
        
        _delay_ms(SYSTEM_DELAY_MS);
    }
}

void config_watchdog()
{
    // Clear WDRF in MCUSR to allow changes to WDE and watchdog prescalers
    MCUSR &= ~(1 << WDRF);
    // Set WDCE and WDE for timed sequence
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    // Set new prescaler (WDP1 for 64ms) and enable WDE (must be within 4 cycles)
    WDTCSR = (1 << WDE) | (1 << WDP1);
}