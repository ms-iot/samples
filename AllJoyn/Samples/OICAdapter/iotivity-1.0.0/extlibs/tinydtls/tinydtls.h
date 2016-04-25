/* tinydtls.h.  Generated from tinydtls.h.in by configure.  */
/* tinydtls -- a very basic DTLS implementation
 *
 * Copyright (C) 2011--2014 Olaf Bergmann <bergmann@tzi.org>
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file tinydtls.h
 * @brief public tinydtls API
 */

#ifndef _DTLS_TINYDTLS_H_
#define _DTLS_TINYDTLS_H_

/** Defined to 1 if tinydtls is built with support for ECC */
#define DTLS_ECC 1

/** Defined to 1 if tinydtls is built with support for PSK */
#define DTLS_PSK 1

/** Defined to 1 if tinydtls is built for Contiki OS */
/* #undef WITH_CONTIKI */

/** Define to 1 if building with X.509 support */
#define DTLS_X509 1

/** Define to 1 if building with Hardware Abstraction Layer */
#define DTLS_CRYPTO_HAL 0

#endif /* _DTLS_TINYDTLS_H_ */
