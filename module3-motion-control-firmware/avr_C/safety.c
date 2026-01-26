/* gabriel-teleop-platform - Safety Module Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include "safety.h"
#include "hal.h"
#include "interface.h"
#include "system_state.h"

extern volatile SystemState system_state;

static volatile uint16_t comm_timeout_counter = 0;
static volatile uint8_t failsafe_active = 0;


void safety_init()
{
    comm_timeout_counter = 0;
    failsafe_active = 0;
}

void safety_update()
{
    // Increment timeout counter
    comm_timeout_counter++;
    
    // Check for communication timeout
    if (comm_timeout_counter >= SAFETY_COMM_TIMEOUT_CYCLES)
    {
        if (!failsafe_active)
        {
            safety_trigger_emergency_stop();
        }
    }
    
    // Validate sensor readings
    if (system_state.steering_angle > STEERING_MAX)
    {
        system_state.steering_angle = STEERING_MAX;
    }
    
    if (system_state.acceleration > ACCELERATION_MAX)
    {
        system_state.acceleration = ACCELERATION_MAX;
    }
    
    // Validate setpoints
    if (system_state.steering_angle_setpoint > STEERING_MAX)
    {
        system_state.steering_angle_setpoint = STEERING_MAX;
    }
    
    if (system_state.acceleration_setpoint > ACCELERATION_MAX)
    {
        system_state.acceleration_setpoint = ACCELERATION_MAX;
    }
    
    // Validate manipulate variables
    if (system_state.steering_manipulate_variable > STEERING_MAX)
    {
        system_state.steering_manipulate_variable = STEERING_MAX;
    }
    
    if (system_state.acceleration_manipulate_variable > ACCELERATION_MAX)
    {
        system_state.acceleration_manipulate_variable = ACCELERATION_MAX;
    }
}

void safety_reset_watchdog()
{
    comm_timeout_counter = 0;
    
    // If we were in failsafe, exit it
    if (failsafe_active)
    {
        failsafe_active = 0;
    }
}

uint8_t safety_is_failsafe_active()
{
    return failsafe_active;
}

void safety_trigger_emergency_stop()
{
    failsafe_active = 1;
    
    // Stop all motion
    hal_emergency_stop();
    
    // Reset system state to safe values
    system_state.steering_angle_setpoint = 9000;
    system_state.acceleration_setpoint = 0;
    system_state.steering_manipulate_variable = 0;
    system_state.acceleration_manipulate_variable = 0;
    system_state.breaks = 1;
}
