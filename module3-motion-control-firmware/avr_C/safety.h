/* gabriel-teleop-platform - Safety Module Header
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_SAFETY_H
#define _GABRIEL_TELEOP_SAFETY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SAFETY_COMM_TIMEOUT_MS 1000
#define SAFETY_COMM_TIMEOUT_CYCLES (SAFETY_COMM_TIMEOUT_MS / SYSTEM_DELAY_MS)

void safety_init();
void safety_update();
void safety_reset_watchdog();
uint8_t safety_is_failsafe_active();
void safety_trigger_emergency_stop();

#ifdef __cplusplus
}
#endif

#endif /* _GABRIEL_TELEOP_SAFETY_H */
