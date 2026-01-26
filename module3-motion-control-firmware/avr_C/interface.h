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

// Command Definitions (use 181+ to avoid collision with data and state header)
#define CMD_HEADER_STEERING 0xB5      // 181
#define CMD_HEADER_ACCELERATION 0xB6  // 182
#define CMD_HEADER_DIRECTION 0xB7     // 183
#define CMD_HEADER_BRAKES 0xB8        // 184

// System State Header (3 bytes: 0xC3 0x3C 0xA5 - unique sync pattern)
#define SYSTEM_STATE_HEADER_1 0xC3
#define SYSTEM_STATE_HEADER_2 0x3C
#define SYSTEM_STATE_HEADER_3 0xA5

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
