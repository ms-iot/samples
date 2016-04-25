/* coap_time.h -- Clock Handling
 *
 * Copyright (C) 2010--2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

/**
 * @file coap_time.h
 * @brief Clock Handling
 */

#ifndef _COAP_TIME_H_
#define _COAP_TIME_H_

/*
 ** Make sure we can call this stuff from C++.
 */
#ifdef __cplusplus
extern "C"
{
#endif

#include "config.h"

    /**
     * @defgroup clock Clock Handling
     * Default implementation of internal clock. You should redefine this if
     * you do not have time() and gettimeofday().
     * @{
     */

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef WITH_LWIP

#include <stdint.h>
#include <lwip/sys.h>

    /* lwIP provides ms in sys_now */
#define COAP_TICKS_PER_SECOND 1000

    typedef uint32_t coap_tick_t;

    static inline void coap_ticks_impl(coap_tick_t *t)
    {
        *t = sys_now();
    }

    static inline void coap_clock_init_impl(void)
    {
    }

#define coap_clock_init coap_clock_init_impl

#define coap_ticks coap_ticks_impl

#endif
#ifdef WITH_CONTIKI
#include "clock.h"

    typedef clock_time_t coap_tick_t;

    /**
     * This data type is used to represent the difference between two
     * clock_tick_t values. This data type must have the same size in
     * memory as coap_tick_t to allow wrapping.
     */
    typedef int coap_tick_diff_t;

#define COAP_TICKS_PER_SECOND CLOCK_SECOND

    /** Set at startup to initialize the internal clock (time in seconds). */
    extern clock_time_t clock_offset;

    static inline void
    contiki_clock_init_impl(void)
    {
        clock_init();
        clock_offset = clock_time();
    }

#define coap_clock_init contiki_clock_init_impl

    static inline void
    contiki_ticks_impl(coap_tick_t *t)
    {
        *t = clock_time();
    }

#define coap_ticks contiki_ticks_impl

#endif /* WITH_CONTIKI */
#ifdef WITH_POSIX
    typedef unsigned int coap_tick_t;

    /**
     * This data type is used to represent the difference between two
     * clock_tick_t values. This data type must have the same size in
     * memory as coap_tick_t to allow wrapping.
     */
    typedef int coap_tick_diff_t;

#define COAP_TICKS_PER_SECOND 1024

    /** Set at startup to initialize the internal clock (time in seconds). */
    extern time_t clock_offset;
#endif /* WITH_POSIX */

#ifdef WIN32
    typedef unsigned int coap_tick_t;

    /**
    * This data type is used to represent the difference between two
    * clock_tick_t values. This data type must have the same size in
    * memory as coap_tick_t to allow wrapping.
    */
    typedef int coap_tick_diff_t;

#define COAP_TICKS_PER_SECOND 1000
    /** Set at startup to initialize the internal clock (time in seconds). */
    extern time_t clock_offset;
#endif

#ifdef WITH_ARDUINO
#include "Time.h"
#ifdef ARDUINO_ARCH_SAM
#include <sys/types.h>  // time_t is defined in sys/types.h for ARM compiler
#else
typedef unsigned long time_t; //AVR compiler doesnt define time_t
#endif
typedef time_t coap_tick_t;

/**
 * This data type is used to represent the difference between two
 * clock_tick_t values. This data type must have the same size in
 * memory as coap_tick_t to allow wrapping.
 */
typedef int coap_tick_diff_t;

/* TODO: Ticks per second value for Arduino needs verification from
 * documentation */
#define COAP_TICKS_PER_SECOND 1000

extern time_t clock_offset;

#endif /* WITH_ARDUINO */

#ifndef coap_clock_init
    static inline void coap_clock_init_impl(void)
    {
#ifdef HAVE_TIME_H
        clock_offset = time(NULL);
#else
#  ifdef WITH_ARDUINO
#ifdef __AVR__
    clock_offset = 1; //now();
#else
    clock_offset = now();
#endif
#  else
#    ifdef __GNUC__
    /* Issue a warning when using gcc. Other prepropressors do
     *  not seem to have a similar feature. */
#     warning "cannot initialize clock"
#    endif
    clock_offset = 0;
#  endif /* WITH_ARDUINO */
#endif /* HAVE_TIME */
}
#define coap_clock_init coap_clock_init_impl
#endif /* coap_clock_init */

#ifndef coap_ticks
    static inline void coap_ticks_impl(coap_tick_t *t)
    {
#ifdef HAVE_SYS_TIME_H
        struct timeval tv;
        gettimeofday(&tv, NULL);
        *t = (tv.tv_sec - clock_offset) * COAP_TICKS_PER_SECOND
                + (tv.tv_usec * COAP_TICKS_PER_SECOND / 1000000);
#else
#  ifdef WITH_ARDUINO
    coap_tick_t tv;
#ifdef __AVR__
    tv = 1; //now();
#else
    tv = now();
#endif
    *t = (tv - clock_offset) * COAP_TICKS_PER_SECOND;
#  else
#ifdef HAVE_TIME_H
        time_t tv = time(NULL);
        *t = difftime(tv, clock_offset) * COAP_TICKS_PER_SECOND;

#else
#    error "clock not implemented"
#endif /* HAVE_TIME_H */
#  endif /* WITH_ARDUINO */
#endif /* HAVE_SYS_TIME_H */
}
#define coap_ticks coap_ticks_impl
#endif /* coap_ticks */

    /**
     * Returns @c 1 if and only if @p a is less than @p b where less is
     * defined on a signed data type.
     */
    static inline
    int coap_time_lt(coap_tick_t a, coap_tick_t b)
    {
        return ((coap_tick_diff_t)(a - b)) < 0;
    }

    /**
     * Returns @c 1 if and only if @p a is less than or equal @p b where
     * less is defined on a signed data type.
     */
    static inline
    int coap_time_le(coap_tick_t a, coap_tick_t b)
    {
        return a == b || coap_time_lt(a, b);
    }

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _COAP_TIME_H_ */
