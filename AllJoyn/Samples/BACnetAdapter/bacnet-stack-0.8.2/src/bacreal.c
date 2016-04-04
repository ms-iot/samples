/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2004-2008 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

#include <string.h>

#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bits.h"
#include "bacstr.h"
#include "bacint.h"
#include "bacreal.h"

/** @file bacreal.c  Encode/Decode Floating Point (Real) Types */

/* NOTE: byte order plays a role in decoding multibyte values */
/* http://www.unixpapa.com/incnote/byteorder.html */
#ifndef BIG_ENDIAN
#error Define BIG_ENDIAN=0 or BIG_ENDIAN=1 for BACnet Stack in compiler settings
#endif

/* from clause 20.2.6 Encoding of a Real Number Value */
/* returns the number of apdu bytes consumed */
int decode_real(
    uint8_t * apdu,
    float *real_value)
{
    union {
        uint8_t byte[4];
        float real_value;
    } my_data;

    /* NOTE: assumes the compiler stores float as IEEE-754 float */
#if BIG_ENDIAN
    my_data.byte[0] = apdu[0];
    my_data.byte[1] = apdu[1];
    my_data.byte[2] = apdu[2];
    my_data.byte[3] = apdu[3];
#else
    my_data.byte[0] = apdu[3];
    my_data.byte[1] = apdu[2];
    my_data.byte[2] = apdu[1];
    my_data.byte[3] = apdu[0];
#endif

    *real_value = my_data.real_value;

    return 4;
}

int decode_real_safe(
    uint8_t * apdu,
    uint32_t len_value,
    float *real_value)
{
    if (len_value != 4) {
        *real_value = 0.0f;
        return (int) len_value;
    } else {
        return decode_real(apdu, real_value);
    }
}

int decode_context_real(
    uint8_t * apdu,
    uint8_t tag_number,
    float *real_value)
{
    uint32_t len_value;
    int len = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        len += decode_real(&apdu[len], real_value);
    } else {
        len = -1;
    }
    return len;
}

/* from clause 20.2.6 Encoding of a Real Number Value */
/* returns the number of apdu bytes consumed */
int encode_bacnet_real(
    float value,
    uint8_t * apdu)
{
    union {
        uint8_t byte[4];
        float real_value;
    } my_data;

    /* NOTE: assumes the compiler stores float as IEEE-754 float */
    my_data.real_value = value;
#if BIG_ENDIAN
    apdu[0] = my_data.byte[0];
    apdu[1] = my_data.byte[1];
    apdu[2] = my_data.byte[2];
    apdu[3] = my_data.byte[3];
#else
    apdu[0] = my_data.byte[3];
    apdu[1] = my_data.byte[2];
    apdu[2] = my_data.byte[1];
    apdu[3] = my_data.byte[0];
#endif

    return 4;
}

#if BACNET_USE_DOUBLE

/* from clause 20.2.7 Encoding of a Double Precision Real Number Value */
/* returns the number of apdu bytes consumed */
int decode_double(
    uint8_t * apdu,
    double *double_value)
{
    union {
        uint8_t byte[8];
        double double_value;
    } my_data;

    /* NOTE: assumes the compiler stores float as IEEE-754 float */
#if BIG_ENDIAN
    my_data.byte[0] = apdu[0];
    my_data.byte[1] = apdu[1];
    my_data.byte[2] = apdu[2];
    my_data.byte[3] = apdu[3];
    my_data.byte[4] = apdu[4];
    my_data.byte[5] = apdu[5];
    my_data.byte[6] = apdu[6];
    my_data.byte[7] = apdu[7];
#else
    my_data.byte[0] = apdu[7];
    my_data.byte[1] = apdu[6];
    my_data.byte[2] = apdu[5];
    my_data.byte[3] = apdu[4];
    my_data.byte[4] = apdu[3];
    my_data.byte[5] = apdu[2];
    my_data.byte[6] = apdu[1];
    my_data.byte[7] = apdu[0];
#endif

    *double_value = my_data.double_value;

    return 8;
}

int decode_double_safe(
    uint8_t * apdu,
    uint32_t len_value,
    double *double_value)
{
    if (len_value != 8) {
        *double_value = 0.0;
        return (int) len_value;
    } else {
        return decode_double(apdu, double_value);
    }
}

/* from clause 20.2.7 Encoding of a Double Precision Real Number Value */
/* returns the number of apdu bytes consumed */
int encode_bacnet_double(
    double value,
    uint8_t * apdu)
{
    union {
        uint8_t byte[8];
        double double_value;
    } my_data;

    /* NOTE: assumes the compiler stores float as IEEE-754 float */
    my_data.double_value = value;
#if BIG_ENDIAN
    apdu[0] = my_data.byte[0];
    apdu[1] = my_data.byte[1];
    apdu[2] = my_data.byte[2];
    apdu[3] = my_data.byte[3];
    apdu[4] = my_data.byte[4];
    apdu[5] = my_data.byte[5];
    apdu[6] = my_data.byte[6];
    apdu[7] = my_data.byte[7];
#else
    apdu[0] = my_data.byte[7];
    apdu[1] = my_data.byte[6];
    apdu[2] = my_data.byte[5];
    apdu[3] = my_data.byte[4];
    apdu[4] = my_data.byte[3];
    apdu[5] = my_data.byte[2];
    apdu[6] = my_data.byte[1];
    apdu[7] = my_data.byte[0];
#endif

    return 8;
}

int decode_context_double(
    uint8_t * apdu,
    uint8_t tag_number,
    double *double_value)
{
    uint32_t len_value;
    int len = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        len += decode_double(&apdu[len], double_value);
    } else {
        len = -1;
    }
    return len;
}
#endif

/* end of decoding_encoding.c */
#ifdef TEST
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "ctest.h"

void testBACreal(
    Test * pTest)
{
    float real_value = 3.14159F, test_real_value = 0.0;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0, test_len = 0;

    len = encode_bacnet_real(real_value, &apdu[0]);
    ct_test(pTest, len == 4);
    test_len = decode_real(&apdu[0], &test_real_value);
    ct_test(pTest, test_len == len);
    ct_test(pTest, test_real_value == real_value);
}

void testBACdouble(
    Test * pTest)
{
    double double_value = 3.1415927, test_double_value = 0.0;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0, test_len = 0;

    len = encode_bacnet_double(double_value, &apdu[0]);
    ct_test(pTest, len == 8);
    test_len = decode_double(&apdu[0], &test_double_value);
    ct_test(pTest, test_len == len);
    ct_test(pTest, test_double_value == double_value);
}

#ifdef TEST_BACNET_REAL
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACreal", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBACreal);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACdouble);
    assert(rc);

    /* configure output */
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BACNET_REAL */
#endif /* TEST */
