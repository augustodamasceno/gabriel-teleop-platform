/* gabriel-teleop-platform - PID Controller Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "pid_controller.h"

void pid_init(PIDController* pid, 
              float kp, 
              float ki, 
              float kd, 
              float output_min, 
              float output_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

void pid_reset(PIDController* pid)
{
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

float pid_compute(PIDController* pid, 
                  float setpoint, 
                  float process_variable, 
                  float dt)
{
    float error = setpoint - process_variable;
    float p = pid->kp * error;

    pid->integral += error * dt;
    float i = pid->ki * pid->integral;
    
    float derivative = (error - pid->prev_error) / dt;
    float d = pid->kd * derivative;
    
    float manipulate_variable = p + i + d;
    
    if (manipulate_variable > pid->output_max)
    {
        manipulate_variable = pid->output_max;
    }
    else if (manipulate_variable < pid->output_min)
    {
        manipulate_variable = pid->output_min;
    }
    
    pid->prev_error = error;
    
    return manipulate_variable;
}
