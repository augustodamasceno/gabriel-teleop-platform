/* gabriel-teleop-platform - Watchdog Timer Implementation
 *
 * Copyright (c) 2026, Augusto Damasceno.
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/wdt.h>
#include "watchdog.h"


void watchdog_config()
{
    // Clear WDRF in MCUSR to allow changes to WDE and watchdog prescalers
    MCUSR &= ~(1 << WDRF);
    // Set WDCE and WDE for timed sequence
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    // Set new prescaler (WDP1 for 64ms) and enable WDE (must be within 4 cycles)
    WDTCSR = (1 << WDE) | (1 << WDP1);
}
