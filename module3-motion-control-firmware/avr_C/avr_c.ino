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
#include "watchdog.h"
#include "hal.h"
#include "safety.h"


volatile SystemState system_state;

int main()
{
    // Configuration Routine
    watchdog_config();
    system_state_init(&system_state);
    hal_init();
    safety_init();
    communication_config_usart();
    Controller controller;
    controller_init(&controller);
    
    // The main loop covers:
    //   1. Read sensor values from hardware
    //   2. Process incoming commands from USART
    //   3. Execute controller to compute control signals
    //   4. Write control outputs to hardware
    //   5. Send system state over USART periodically
    while(1)
    {
        wdt_reset();
        
        // Read sensor feedback
        system_state.steering_angle = hal_read_steering_angle();
        system_state.acceleration = hal_read_speed();
        
        // Process ALL incoming commands in buffer
        while (communication_rx_available())
        {
            communication_receive_command();
        }
        
        // Update safety checks
        safety_update();
        
        // Execute controllers
        controller_update(&controller, &system_state, SYSTEM_DELAY_S);
        
        // Write outputs to hardware
        hal_set_steering_pwm(system_state.steering_manipulate_variable);
        hal_set_acceleration_pwm(system_state.acceleration_manipulate_variable);
        hal_set_direction(system_state.direction);
        hal_set_brakes(system_state.breaks);
        
        // Send telemetry (before delay so test gets immediate response)
        communication_send_system_state();
        
        _delay_ms(SYSTEM_DELAY_MS);
    }
}

