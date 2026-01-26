/* gabriel-teleop-platform - Hardware Abstraction Layer Header
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _GABRIEL_TELEOP_HAL_H
#define _GABRIEL_TELEOP_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void hal_init();
void hal_pwm_init();
void hal_adc_init();
void hal_gpio_init();

void hal_set_steering_pwm(uint16_t value);
void hal_set_acceleration_pwm(uint16_t value);
void hal_set_direction(uint8_t direction);
void hal_set_brakes(uint8_t brakes);

uint16_t hal_read_steering_angle();
uint16_t hal_read_speed();

void hal_emergency_stop();
void hal_disable_all_outputs();

#ifdef __cplusplus
}
#endif

#endif /* _GABRIEL_TELEOP_HAL_H */
