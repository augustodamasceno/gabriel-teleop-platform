/* gabriel-teleop-platform - Controller Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "controller.h"
#include "interface.h"


void controller_init(Controller* controller)
{
    pid_init(&controller->steering_pid, 
             STEERING_PID_KP, 
             STEERING_PID_KI, 
             STEERING_PID_KD, 
             STEERING_PID_OUTPUT_MIN, 
             STEERING_PID_OUTPUT_MAX);
    
    pid_init(&controller->acceleration_pid, 
             ACCELERATION_PID_KP, 
             ACCELERATION_PID_KI, 
             ACCELERATION_PID_KD, 
             ACCELERATION_PID_OUTPUT_MIN, 
             ACCELERATION_PID_OUTPUT_MAX);
}

void controller_update(Controller* controller, 
                       SystemState* state, 
                       float dt)
{
    state->steering_manipulate_variable = 
        (uint16_t)pid_compute(&controller->steering_pid, 
                              (float)state->steering_angle_setpoint, 
                              (float)state->steering_angle, 
                              dt);

    state->acceleration_manipulate_variable = 
        (uint16_t)pid_compute(&controller->acceleration_pid, 
                              (float)state->acceleration_setpoint, 
                              (float)state->acceleration, 
                              dt);
}
