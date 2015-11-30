/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/
#ifndef BACDCODE_H
#define BACDCODE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bacdef.h"
#include "datetime.h"
#include "bacstr.h"
#include "bacint.h"
#include "bacreal.h"
#include "bits.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* from clause 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_tag(
        uint8_t * apdu,
        uint8_t tag_number,
        bool context_specific,
        uint32_t len_value_type);

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the number of apdu bytes consumed */
    int encode_opening_tag(
        uint8_t * apdu,
        uint8_t tag_number);
    int encode_closing_tag(
        uint8_t * apdu,
        uint8_t tag_number);
    int decode_tag_number(
        uint8_t * apdu,
        uint8_t * tag_number);
    int decode_tag_number_safe(
        uint8_t * apdu,
        uint32_t apdu_len_remaining,
        uint8_t * tag_number);
    int decode_tag_number_and_value(
        uint8_t * apdu,
        uint8_t * tag_number,
        uint32_t * value);
    int decode_tag_number_and_value_safe(
        uint8_t * apdu,
        uint32_t apdu_len_remaining,
        uint8_t * tag_number,
        uint32_t * value);
/* returns true if the tag is an opening tag and matches */
    bool decode_is_opening_tag_number(
        uint8_t * apdu,
        uint8_t tag_number);
/* returns true if the tag is a closing tag and matches */
    bool decode_is_closing_tag_number(
        uint8_t * apdu,
        uint8_t tag_number);
/* returns true if the tag is context specific and matches */
    bool decode_is_context_tag(
        uint8_t * apdu,
        uint8_t tag_number);
    bool decode_is_context_tag_with_length(
        uint8_t * apdu,
        uint8_t tag_number,
        int *tag_length);
    /* returns true if the tag is an opening tag */
    bool decode_is_opening_tag(
        uint8_t * apdu);
    /* returns true if the tag is a closing tag */
    bool decode_is_closing_tag(
        uint8_t * apdu);

/* from clause 20.2.2 Encoding of a Null Value */
    int encode_application_null(
        uint8_t * apdu);
    int encode_context_null(
        uint8_t * apdu,
        uint8_t tag_number);

/* from clause 20.2.3 Encoding of a Boolean Value */
    int encode_application_boolean(
        uint8_t * apdu,
        bool boolean_value);
    bool decode_boolean(
        uint32_t len_value);
    int encode_context_boolean(
        uint8_t * apdu,
        uint8_t tag_number,
        bool boolean_value);
    bool decode_context_boolean(
        uint8_t * apdu);

    int decode_context_boolean2(
        uint8_t * apdu,
        uint8_t tag_number,
        bool * boolean_value);

/* from clause 20.2.10 Encoding of a Bit String Value */
/* returns the number of apdu bytes consumed */
    int decode_bitstring(
        uint8_t * apdu,
        uint32_t len_value,
        BACNET_BIT_STRING * bit_string);

    int decode_context_bitstring(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_BIT_STRING * bit_string);
/* returns the number of apdu bytes consumed */
    int encode_bitstring(
        uint8_t * apdu,
        BACNET_BIT_STRING * bit_string);
    int encode_application_bitstring(
        uint8_t * apdu,
        BACNET_BIT_STRING * bit_string);
    int encode_context_bitstring(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_BIT_STRING * bit_string);

/* from clause 20.2.6 Encoding of a Real Number Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_application_real(
        uint8_t * apdu,
        float value);
    int encode_context_real(
        uint8_t * apdu,
        uint8_t tag_number,
        float value);

/* from clause 20.2.7 Encoding of a Double Precision Real Number Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_application_double(
        uint8_t * apdu,
        double value);

    int encode_context_double(
        uint8_t * apdu,
        uint8_t tag_number,
        double value);

/* from clause 20.2.14 Encoding of an Object Identifier Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int decode_object_id(
        uint8_t * apdu,
        uint16_t * object_type,
        uint32_t * instance);

    int decode_object_id_safe(
        uint8_t * apdu,
        uint32_t len_value,
        uint16_t * object_type,
        uint32_t * instance);

    int decode_context_object_id(
        uint8_t * apdu,
        uint8_t tag_number,
        uint16_t * object_type,
        uint32_t * instance);

    int encode_bacnet_object_id(
        uint8_t * apdu,
        int object_type,
        uint32_t instance);
    int encode_context_object_id(
        uint8_t * apdu,
        uint8_t tag_number,
        int object_type,
        uint32_t instance);
    int encode_application_object_id(
        uint8_t * apdu,
        int object_type,
        uint32_t instance);

/* from clause 20.2.8 Encoding of an Octet String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_octet_string(
        uint8_t * apdu,
        BACNET_OCTET_STRING * octet_string);
    int encode_application_octet_string(
        uint8_t * apdu,
        BACNET_OCTET_STRING * octet_string);
    int encode_context_octet_string(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_OCTET_STRING * octet_string);
    int decode_octet_string(
        uint8_t * apdu,
        uint32_t len_value,
        BACNET_OCTET_STRING * octet_string);
    int decode_context_octet_string(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_OCTET_STRING * octet_string);


/* from clause 20.2.9 Encoding of a Character String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    uint32_t encode_bacnet_character_string_safe(
        uint8_t * apdu,
        uint32_t max_apdu,
        uint8_t encoding,
        char *pString,
        uint32_t length);
    int encode_bacnet_character_string(
        uint8_t * apdu,
        BACNET_CHARACTER_STRING * char_string);
    int encode_application_character_string(
        uint8_t * apdu,
        BACNET_CHARACTER_STRING * char_string);
    int encode_context_character_string(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_CHARACTER_STRING * char_string);
    int decode_character_string(
        uint8_t * apdu,
        uint32_t len_value,
        BACNET_CHARACTER_STRING * char_string);
    int decode_context_character_string(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_CHARACTER_STRING * char_string);


/* from clause 20.2.4 Encoding of an Unsigned Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_bacnet_unsigned(
        uint8_t * apdu,
        uint32_t value);
    int encode_context_unsigned(
        uint8_t * apdu,
        uint8_t tag_number,
        uint32_t value);
    int encode_application_unsigned(
        uint8_t * apdu,
        uint32_t value);
    int decode_unsigned(
        uint8_t * apdu,
        uint32_t len_value,
        uint32_t * value);
    int decode_context_unsigned(
        uint8_t * apdu,
        uint8_t tag_number,
        uint32_t * value);

/* from clause 20.2.5 Encoding of a Signed Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_bacnet_signed(
        uint8_t * apdu,
        int32_t value);
    int encode_application_signed(
        uint8_t * apdu,
        int32_t value);
    int encode_context_signed(
        uint8_t * apdu,
        uint8_t tag_number,
        int32_t value);
    int decode_signed(
        uint8_t * apdu,
        uint32_t len_value,
        int32_t * value);
    int decode_context_signed(
        uint8_t * apdu,
        uint8_t tag_number,
        int32_t * value);


/* from clause 20.2.11 Encoding of an Enumerated Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int decode_enumerated(
        uint8_t * apdu,
        uint32_t len_value,
        uint32_t * value);
    int decode_context_enumerated(
        uint8_t * apdu,
        uint8_t tag_value,
        uint32_t * value);
    int encode_bacnet_enumerated(
        uint8_t * apdu,
        uint32_t value);
    int encode_application_enumerated(
        uint8_t * apdu,
        uint32_t value);
    int encode_context_enumerated(
        uint8_t * apdu,
        uint8_t tag_number,
        uint32_t value);

/* from clause 20.2.13 Encoding of a Time Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_bacnet_time(
        uint8_t * apdu,
        BACNET_TIME * btime);
    int encode_application_time(
        uint8_t * apdu,
        BACNET_TIME * btime);
    int decode_bacnet_time(
        uint8_t * apdu,
        BACNET_TIME * btime);
    int decode_bacnet_time_safe(
        uint8_t * apdu,
        uint32_t len_value,
        BACNET_TIME * btime);
    int encode_context_time(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_TIME * btime);
    int decode_application_time(
        uint8_t * apdu,
        BACNET_TIME * btime);
    int decode_context_bacnet_time(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_TIME * btime);


/* BACnet Date */
/* year = years since 1900 */
/* month 1=Jan */
/* day = day of month */
/* wday 1=Monday...7=Sunday */

/* from clause 20.2.12 Encoding of a Date Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
    int encode_bacnet_date(
        uint8_t * apdu,
        BACNET_DATE * bdate);
    int encode_application_date(
        uint8_t * apdu,
        BACNET_DATE * bdate);
    int encode_context_date(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_DATE * bdate);
    int decode_date(
        uint8_t * apdu,
        BACNET_DATE * bdate);
    int decode_date_safe(
        uint8_t * apdu,
        uint32_t len_value,
        BACNET_DATE * bdate);
    int decode_application_date(
        uint8_t * apdu,
        BACNET_DATE * bdate);
    int decode_context_date(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_DATE * bdate);

/* from clause 20.1.2.4 max-segments-accepted */
/* and clause 20.1.2.5 max-APDU-length-accepted */
/* returns the encoded octet */
    uint8_t encode_max_segs_max_apdu(
        int max_segs,
        int max_apdu);
    int decode_max_segs(
        uint8_t octet);
    int decode_max_apdu(
        uint8_t octet);

/* returns the number of apdu bytes consumed */
    int encode_simple_ack(
        uint8_t * apdu,
        uint8_t invoke_id,
        uint8_t service_choice);

/* from clause 20.2.1.2 Tag Number */
/* true if extended tag numbering is used */
#define IS_EXTENDED_TAG_NUMBER(x) ((x & 0xF0) == 0xF0)

/* from clause 20.2.1.3.1 Primitive Data */
/* true if the extended value is used */
#define IS_EXTENDED_VALUE(x) ((x & 0x07) == 5)

/* from clause 20.2.1.1 Class */
/* true if the tag is context specific */
#define IS_CONTEXT_SPECIFIC(x) ((x & BIT3) == BIT3)

/* from clause 20.2.1.3.2 Constructed Data */
/* true if the tag is an opening tag */
#define IS_OPENING_TAG(x) ((x & 0x07) == 6)

/* from clause 20.2.1.3.2 Constructed Data */
/* true if the tag is a closing tag */
#define IS_CLOSING_TAG(x) ((x & 0x07) == 7)


#ifdef __cplusplus

}
#endif /* __cplusplus */
#endif
