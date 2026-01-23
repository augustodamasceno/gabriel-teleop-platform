/* gabriel-teleop-platform - PID Controller Header
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_PID_CONTROLLER_H
#define _GABRIEL_TELEOP_PID_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error;
    float output_min;
    float output_max;
} PIDController;

void pid_init(PIDController* pid, 
              float kp, 
              float ki, 
              float kd, 
              float output_min, 
              float output_max);
void pid_reset(PIDController* pid);
float pid_compute(PIDController* pid, 
                  float setpoint,
                  float process_variable, 
                  float dt);

#ifdef __cplusplus
}
#endif

#endif
