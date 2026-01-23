/* gabriel-teleop-platform - Controller Header
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_CONTROLLER_H
#define _GABRIEL_TELEOP_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pid_controller.h"
#include "system_state.h"

typedef struct
{
    PIDController steering_pid;
    PIDController acceleration_pid;
} Controller;

void controller_init(Controller* controller);
void controller_update(Controller* controller, 
                       SystemState* state, 
                       float dt);


#ifdef __cplusplus
}
#endif

#endif /* _GABRIEL_TELEOP_CONTROLLER_H */
