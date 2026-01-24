/* gabriel-teleop-platform - System State Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include "system_state.h"


void system_state_init(SystemState* state)
{
    state->steering_angle = 9000;
    state->steering_angle_setpoint = 9000;
    state->steering_manipulate_variable = 0;
    state->acceleration = 0;
    state->acceleration_setpoint = 0;
    state->acceleration_manipulate_variable = 0;
    state->direction = 0;
}
