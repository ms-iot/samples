/**
 * \file
 *
 * \brief AVR functions for busy-wait delay loops
 *
 * Copyright (c) 2011 - 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#ifndef _CYCLE_COUNTER_H_
#define _CYCLE_COUNTER_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <compiler.h>

/**
 * @name Convenience functions for busy-wait delay loops
 *
 * @def delay_cycles
 * @brief Delay program execution for a specified number of CPU cycles.
 * @param n number of CPU cycles to wait
 *
 * @def cpu_delay_ms
 * @brief Delay program execution for a specified number of milliseconds.
 * @param delay number of milliseconds to wait
 * @param f_cpu CPU frequency in Hertz
 *
 * @def cpu_delay_us
 * @brief Delay program execution for a specified number of microseconds.
 * @param delay number of microseconds to wait
 * @param f_cpu CPU frequency in Hertz
 *
 * @def cpu_ms_2_cy
 * @brief Convert milli-seconds into CPU cycles.
 * @param ms number of milliseconds
 * @param f_cpu CPU frequency in Hertz
 * @return the converted number of CPU cycles
 *
 * @def cpu_us_2_cy
 * @brief Convert micro-seconds into CPU cycles.
 * @param ms number of microseconds
 * @param f_cpu CPU frequency in Hertz
 * @return the converted number of CPU cycles
 *
 * @{
 */
  __always_optimize
    static inline void __portable_avr_delay_cycles (unsigned long n)
  {
    do
      {
	barrier ();
      }
    while (--n);
  }

#if !defined(__DELAY_CYCLE_INTRINSICS__)
#	define delay_cycles            __portable_avr_delay_cycles
#	define cpu_ms_2_cy(ms, f_cpu)  (((uint64_t)(ms) * (f_cpu) + 999) / 6e3)
#	define cpu_us_2_cy(us, f_cpu)  (((uint64_t)(us) * (f_cpu) + 999999ul) / 6e6)
#else
#  if defined(__GNUC__)
#	define delay_cycles            __builtin_avr_delay_cycles
#  elif defined(__ICCAVR__)
#	define delay_cycles            __delay_cycles
#  endif
#	define cpu_ms_2_cy(ms, f_cpu)  (((uint64_t)(ms) * (f_cpu) + 999) / 1e3)
#	define cpu_us_2_cy(us, f_cpu)  (((uint64_t)(us) * (f_cpu) + 999999ul) / 1e6)
#endif

#define cpu_delay_ms(delay, f_cpu) delay_cycles((uint64_t)cpu_ms_2_cy(delay, f_cpu))
#define cpu_delay_us(delay, f_cpu) delay_cycles((uint64_t)cpu_us_2_cy(delay, f_cpu))
//! @}


#ifdef __cplusplus
}
#endif

#endif /* _CYCLE_COUNTER_H_ */
