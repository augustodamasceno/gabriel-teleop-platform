/* gabriel-teleop-platform - System State Header
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_SYSTEM_STATE_H
#define _GABRIEL_TELEOP_SYSTEM_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "pid_controller.h"

typedef struct
{
    uint16_t steering_angle;
    uint16_t steering_angle_setpoint;
    uint16_t steering_manipulate_variable;
    uint16_t acceleration;
    uint16_t acceleration_setpoint;
    uint16_t acceleration_manipulate_variable;
    uint8_t direction;
} SystemState;


void system_state_init(SystemState* state);

#ifdef __cplusplus
}
#endif

#endif
