/* gabriel-teleop-platform - Module 3 C Interface
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_INTERFACE_H
#define _GABRIEL_TELEOP_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

// System Delay in milliseconds
#define SYSTEM_DELAY_MS 10
#define SYSTEM_DELAY_S ( (float)SYSTEM_DELAY_MS / 1000.0f )

// Command Definitions
#define CMD_HEADER_STEERING 0xFF
#define CMD_HEADER_ACCELERATION 0xFE
#define CMD_HEADER_DIRECTION 0xFD

// PID Steering Parameters
#define STEERING_MIN 0
#define STEERING_MAX 18000
#define STEERING_PID_KP 1.0f
#define STEERING_PID_KI 0.1f
#define STEERING_PID_KD 0.05f
#define STEERING_PID_OUTPUT_MIN 0.0f
#define STEERING_PID_OUTPUT_MAX 18000.0f

// PID Acceleration Parameters
#define ACCELERATION_MIN 0
#define ACCELERATION_MAX 10000
#define ACCELERATION_PID_KP 1.0f
#define ACCELERATION_PID_KI 0.1f
#define ACCELERATION_PID_KD 0.05f
#define ACCELERATION_PID_OUTPUT_MIN 0.0f
#define ACCELERATION_PID_OUTPUT_MAX 10000.0f

#ifdef __cplusplus
}
#endif

#endif /* _GABRIEL_TELEOP_INTERFACE_H */
