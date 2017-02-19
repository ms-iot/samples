/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2004 Steve Karg

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

/** @file bacdcode.c  Functions to encode/decode BACnet data types */


/* max-segments-accepted
   B'000'      Unspecified number of segments accepted.
   B'001'      2 segments accepted.
   B'010'      4 segments accepted.
   B'011'      8 segments accepted.
   B'100'      16 segments accepted.
   B'101'      32 segments accepted.
   B'110'      64 segments accepted.
   B'111'      Greater than 64 segments accepted.
*/

/* max-APDU-length-accepted
   B'0000'  Up to MinimumMessageSize (50 octets)
   B'0001'  Up to 128 octets
   B'0010'  Up to 206 octets (fits in a LonTalk frame)
   B'0011'  Up to 480 octets (fits in an ARCNET frame)
   B'0100'  Up to 1024 octets
   B'0101'  Up to 1476 octets (fits in an ISO 8802-3 frame)
   B'0110'  reserved by ASHRAE
   B'0111'  reserved by ASHRAE
   B'1000'  reserved by ASHRAE
   B'1001'  reserved by ASHRAE
   B'1010'  reserved by ASHRAE
   B'1011'  reserved by ASHRAE
   B'1100'  reserved by ASHRAE
   B'1101'  reserved by ASHRAE
   B'1110'  reserved by ASHRAE
   B'1111'  reserved by ASHRAE
*/

/* Encoding of BACNET Length/Value/Type tag
   From clause 20.2.1.3.1

   B'000'   interpreted as Value  = FALSE if application class == BOOLEAN
   B'001'   interpreted as Value  = TRUE  if application class == BOOLEAN

   B'000'   interpreted as Length = 0     if application class != BOOLEAN
   B'001'   interpreted as Length = 1
   B'010'   interpreted as Length = 2
   B'011'   interpreted as Length = 3
   B'100'   interpreted as Length = 4
   B'101'   interpreted as Length > 4
   B'110'   interpreted as Type   = Opening Tag
   B'111'   interpreted as Type   = Closing Tag
*/


/* from clause 20.1.2.4 max-segments-accepted */
/* and clause 20.1.2.5 max-APDU-length-accepted */
/* returns the encoded octet */
uint8_t encode_max_segs_max_apdu(
    int max_segs,
    int max_apdu)
{
    uint8_t octet = 0;

    if (max_segs < 2)
        octet = 0;
    else if (max_segs < 4)
        octet = 0x10;
    else if (max_segs < 8)
        octet = 0x20;
    else if (max_segs < 16)
        octet = 0x30;
    else if (max_segs < 32)
        octet = 0x40;
    else if (max_segs < 64)
        octet = 0x50;
    else if (max_segs == 64)
        octet = 0x60;
    else
        octet = 0x70;

    /* max_apdu must be 50 octets minimum */
    if (max_apdu <= 50)
        octet |= 0x00;
    else if (max_apdu <= 128)
        octet |= 0x01;
    /*fits in a LonTalk frame */
    else if (max_apdu <= 206)
        octet |= 0x02;
    /*fits in an ARCNET or MS/TP frame */
    else if (max_apdu <= 480)
        octet |= 0x03;
    else if (max_apdu <= 1024)
        octet |= 0x04;
    /* fits in an ISO 8802-3 frame */
    else if (max_apdu <= 1476)
        octet |= 0x05;

    return octet;
}

/* from clause 20.1.2.4 max-segments-accepted */
/* and clause 20.1.2.5 max-APDU-length-accepted */
/* returns the encoded octet */
int decode_max_segs(
    uint8_t octet)
{
    int max_segs = 0;

    switch (octet & 0xF0) {
        case 0:
            max_segs = 0;
            break;
        case 0x10:
            max_segs = 2;
            break;
        case 0x20:
            max_segs = 4;
            break;
        case 0x30:
            max_segs = 8;
            break;
        case 0x40:
            max_segs = 16;
            break;
        case 0x50:
            max_segs = 32;
            break;
        case 0x60:
            max_segs = 64;
            break;
        case 0x70:
            max_segs = 65;
            break;
        default:
            break;
    }

    return max_segs;
}

int decode_max_apdu(
    uint8_t octet)
{
    int max_apdu = 0;

    switch (octet & 0x0F) {
        case 0:
            max_apdu = 50;
            break;
        case 1:
            max_apdu = 128;
            break;
        case 2:
            max_apdu = 206;
            break;
        case 3:
            max_apdu = 480;
            break;
        case 4:
            max_apdu = 1024;
            break;
        case 5:
            max_apdu = 1476;
            break;
        default:
            break;
    }

    return max_apdu;
}

/* from clause 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_tag(
    uint8_t * apdu,
    uint8_t tag_number,
    bool context_specific,
    uint32_t len_value_type)
{
    int len = 1;        /* return value */

    apdu[0] = 0;
    if (context_specific)
        apdu[0] = BIT3;

    /* additional tag byte after this byte */
    /* for extended tag byte */
    if (tag_number <= 14) {
        apdu[0] |= (tag_number << 4);
    } else {
        apdu[0] |= 0xF0;
        apdu[1] = tag_number;
        len++;
    }

    /* NOTE: additional len byte(s) after extended tag byte */
    /* if larger than 4 */
    if (len_value_type <= 4) {
        apdu[0] |= len_value_type;
    } else {
        apdu[0] |= 5;
        if (len_value_type <= 253) {
            apdu[len++] = (uint8_t) len_value_type;
        } else if (len_value_type <= 65535) {
            apdu[len++] = 254;
            len += encode_unsigned16(&apdu[len], (uint16_t) len_value_type);
        } else {
            apdu[len++] = 255;
            len += encode_unsigned32(&apdu[len], len_value_type);
        }
    }

    return len;
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the number of apdu bytes consumed */
int encode_opening_tag(
    uint8_t * apdu,
    uint8_t tag_number)
{
    int len = 1;

    /* set class field to context specific */
    apdu[0] = BIT3;
    /* additional tag byte after this byte for extended tag byte */
    if (tag_number <= 14) {
        apdu[0] |= (tag_number << 4);
    } else {
        apdu[0] |= 0xF0;
        apdu[1] = tag_number;
        len++;
    }
    /* set type field to opening tag */
    apdu[0] |= 6;

    return len;
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the number of apdu bytes consumed */
int encode_closing_tag(
    uint8_t * apdu,
    uint8_t tag_number)
{
    int len = 1;

    /* set class field to context specific */
    apdu[0] = BIT3;
    /* additional tag byte after this byte for extended tag byte */
    if (tag_number <= 14) {
        apdu[0] |= (tag_number << 4);
    } else {
        apdu[0] |= 0xF0;
        apdu[1] = tag_number;
        len++;
    }
    /* set type field to closing tag */
    apdu[0] |= 7;

    return len;
}



int decode_tag_number(
    uint8_t * apdu,
    uint8_t * tag_number)
{
    int len = 1;        /* return value */

    /* decode the tag number first */
    if (IS_EXTENDED_TAG_NUMBER(apdu[0])) {
        /* extended tag */
        if (tag_number) {
            *tag_number = apdu[1];
        }
        len++;
    } else {
        if (tag_number) {
            *tag_number = (uint8_t) (apdu[0] >> 4);
        }
    }

    return len;
}

/* Same as function above, but will safely fail if packet has been truncated */
int decode_tag_number_safe(
    uint8_t * apdu,
    uint32_t apdu_len_remaining,
    uint8_t * tag_number)
{
    int len = 0;        /* return value */

    /* decode the tag number first */
    if (apdu_len_remaining >= 1) {
        if (IS_EXTENDED_TAG_NUMBER(apdu[0]) && apdu_len_remaining >= 2) {
            /* extended tag */
            if (tag_number) {
                *tag_number = apdu[1];
            }
            len = 2;
        } else {
            if (tag_number) {
                *tag_number = (uint8_t) (apdu[0] >> 4);
            }
            len = 1;
        }
    }
    return len;
}

bool decode_is_opening_tag(
    uint8_t * apdu)
{
    return (bool) ((apdu[0] & 0x07) == 6);
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the number of apdu bytes consumed */
bool decode_is_closing_tag(
    uint8_t * apdu)
{
    return (bool) ((apdu[0] & 0x07) == 7);
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the number of apdu bytes consumed */
int decode_tag_number_and_value(
    uint8_t * apdu,
    uint8_t * tag_number,
    uint32_t * value)
{
    int len = 1;
    uint16_t value16;
    uint32_t value32;

    len = decode_tag_number(&apdu[0], tag_number);
    if (IS_EXTENDED_VALUE(apdu[0])) {
        /* tagged as uint32_t */
        if (apdu[len] == 255) {
            len++;
            len += decode_unsigned32(&apdu[len], &value32);
            if (value) {
                *value = value32;
            }
        }
        /* tagged as uint16_t */
        else if (apdu[len] == 254) {
            len++;
            len += decode_unsigned16(&apdu[len], &value16);
            if (value) {
                *value = value16;
            }
        }
        /* no tag - must be uint8_t */
        else {
            if (value) {
                *value = apdu[len];
            }
            len++;
        }
    } else if (IS_OPENING_TAG(apdu[0]) && value) {
        *value = 0;
    } else if (IS_CLOSING_TAG(apdu[0]) && value) {
        /* closing tag */
        *value = 0;
    } else if (value) {
        /* small value */
        *value = apdu[0] & 0x07;
    }

    return len;
}

/* Same as function above, but will safely fail is packet has been truncated */
int decode_tag_number_and_value_safe(
    uint8_t * apdu,
    uint32_t apdu_len_remaining,
    uint8_t * tag_number,
    uint32_t * value)
{
    int len = 0;

    len = decode_tag_number_safe(&apdu[0], apdu_len_remaining, tag_number);

    if (len > 0) {
        apdu_len_remaining -= len;
        if (IS_EXTENDED_VALUE(apdu[0])) {
            /* tagged as uint32_t */
            if (apdu[len] == 255 && apdu_len_remaining >= 5) {
                uint32_t value32;
                len++;
                len += decode_unsigned32(&apdu[len], &value32);
                if (value) {
                    *value = value32;
                }
            }
            /* tagged as uint16_t */
            else if (apdu[len] == 254 && apdu_len_remaining >= 3) {
                uint16_t value16;
                len++;
                len += decode_unsigned16(&apdu[len], &value16);
                if (value) {
                    *value = value16;
                }
            }
            /* no tag - must be uint8_t */
            else if (apdu[len] < 254 && apdu_len_remaining >= 1) {
                if (value) {
                    *value = apdu[len];
                }
                len++;
            } else {
                /* packet is truncated */
                len = 0;
            }
        } else if (IS_OPENING_TAG(apdu[0]) && value) {
            *value = 0;
        } else if (IS_CLOSING_TAG(apdu[0]) && value) {
            /* closing tag */
            *value = 0;
        } else if (value) {
            /* small value */
            *value = apdu[0] & 0x07;
        }
    }
    return len;
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns true if the tag is context specific and matches */
bool decode_is_context_tag(
    uint8_t * apdu,
    uint8_t tag_number)
{
    uint8_t my_tag_number = 0;

    decode_tag_number(apdu, &my_tag_number);
    return (bool) (IS_CONTEXT_SPECIFIC(*apdu) &&
        (my_tag_number == tag_number));
}

bool decode_is_context_tag_with_length(
    uint8_t * apdu,
    uint8_t tag_number,
    int *tag_length)
{
    uint8_t my_tag_number = 0;

    *tag_length = decode_tag_number(apdu, &my_tag_number);

    return (bool) (IS_CONTEXT_SPECIFIC(*apdu) &&
        (my_tag_number == tag_number));
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the true if the tag matches */
bool decode_is_opening_tag_number(
    uint8_t * apdu,
    uint8_t tag_number)
{
    uint8_t my_tag_number = 0;

    decode_tag_number(apdu, &my_tag_number);
    return (bool) (IS_OPENING_TAG(apdu[0]) && (my_tag_number == tag_number));
}

/* from clause 20.2.1.3.2 Constructed Data */
/* returns the number of apdu bytes consumed */
bool decode_is_closing_tag_number(
    uint8_t * apdu,
    uint8_t tag_number)
{
    uint8_t my_tag_number = 0;

    decode_tag_number(apdu, &my_tag_number);
    return (bool) (IS_CLOSING_TAG(apdu[0]) && (my_tag_number == tag_number));
}

/* from clause 20.2.3 Encoding of a Boolean Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_boolean(
    uint8_t * apdu,
    bool boolean_value)
{
    int len = 0;
    uint32_t len_value = 0;

    if (boolean_value) {
        len_value = 1;
    }
    len =
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_BOOLEAN, false, len_value);

    return len;
}

/* context tagged is encoded differently */
int encode_context_boolean(
    uint8_t * apdu,
    uint8_t tag_number,
    bool boolean_value)
{
    int len = 0;        /* return value */

    len = encode_tag(&apdu[0], (uint8_t) tag_number, true, 1);
    apdu[len] = (bool) (boolean_value ? 1 : 0);
    len++;

    return len;
}

bool decode_context_boolean(
    uint8_t * apdu)
{
    bool boolean_value = false;

    if (apdu[0]) {
        boolean_value = true;
    }

    return boolean_value;
}

int decode_context_boolean2(
    uint8_t * apdu,
    uint8_t tag_number,
    bool * boolean_value)
{
    int len = 0;
    if (decode_is_context_tag_with_length(&apdu[len], tag_number, &len)) {
        if (apdu[len]) {
            *boolean_value = true;
        } else {
            *boolean_value = false;
        }
        len++;
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}

/* from clause 20.2.3 Encoding of a Boolean Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
bool decode_boolean(
    uint32_t len_value)
{
    bool boolean_value = false;

    if (len_value) {
        boolean_value = true;
    }

    return boolean_value;
}

/* from clause 20.2.2 Encoding of a Null Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_null(
    uint8_t * apdu)
{
    return encode_tag(&apdu[0], BACNET_APPLICATION_TAG_NULL, false, 0);
}

int encode_context_null(
    uint8_t * apdu,
    uint8_t tag_number)
{
    return encode_tag(&apdu[0], tag_number, true, 0);
}

static uint8_t byte_reverse_bits(
    uint8_t in_byte)
{
    uint8_t out_byte = 0;

    if (in_byte & BIT0) {
        out_byte |= BIT7;
    }
    if (in_byte & BIT1) {
        out_byte |= BIT6;
    }
    if (in_byte & BIT2) {
        out_byte |= BIT5;
    }
    if (in_byte & BIT3) {
        out_byte |= BIT4;
    }
    if (in_byte & BIT4) {
        out_byte |= BIT3;
    }
    if (in_byte & BIT5) {
        out_byte |= BIT2;
    }
    if (in_byte & BIT6) {
        out_byte |= BIT1;
    }
    if (in_byte & BIT7) {
        out_byte |= BIT0;
    }

    return out_byte;
}

/* from clause 20.2.10 Encoding of a Bit String Value */
/* returns the number of apdu bytes consumed */
int decode_bitstring(
    uint8_t * apdu,
    uint32_t len_value,
    BACNET_BIT_STRING * bit_string)
{
    int len = 0;
    uint8_t unused_bits = 0;
    uint32_t i = 0;
    uint32_t bytes_used = 0;


    bitstring_init(bit_string);
    if (len_value) {
        /* the first octet contains the unused bits */
        bytes_used = len_value - 1;
        if (bytes_used <= MAX_BITSTRING_BYTES) {
            len = 1;
            for (i = 0; i < bytes_used; i++) {
                bitstring_set_octet(bit_string, (uint8_t) i,
                    byte_reverse_bits(apdu[len++]));
            }
            unused_bits = (uint8_t) (apdu[0] & 0x07);
            bitstring_set_bits_used(bit_string, (uint8_t) bytes_used,
                unused_bits);
        }
    }

    return len;
}

int decode_context_bitstring(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_BIT_STRING * bit_string)
{
    uint32_t len_value;
    int len = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        len += decode_bitstring(&apdu[len], len_value, bit_string);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}


/* from clause 20.2.10 Encoding of a Bit String Value */
/* returns the number of apdu bytes consumed */
int encode_bitstring(
    uint8_t * apdu,
    BACNET_BIT_STRING * bit_string)
{
    int len = 0;
    uint8_t remaining_used_bits = 0;
    uint8_t used_bytes = 0;
    uint8_t i = 0;

    /* if the bit string is empty, then the first octet shall be zero */
    if (bitstring_bits_used(bit_string) == 0) {
        apdu[len++] = 0;
    } else {
        used_bytes = bitstring_bytes_used(bit_string);
        remaining_used_bits =
            (uint8_t) (bitstring_bits_used(bit_string) - ((used_bytes -
                    1) * 8));
        /* number of unused bits in the subsequent final octet */
        apdu[len++] = (uint8_t) (8 - remaining_used_bits);
        for (i = 0; i < used_bytes; i++) {
            apdu[len++] = byte_reverse_bits(bitstring_octet(bit_string, i));
        }
    }

    return len;
}

int encode_application_bitstring(
    uint8_t * apdu,
    BACNET_BIT_STRING * bit_string)
{
    int len = 0;
    uint32_t bit_string_encoded_length = 1;     /* 1 for the bits remaining octet */

    /* bit string may use more than 1 octet for the tag, so find out how many */
    bit_string_encoded_length += bitstring_bytes_used(bit_string);
    len =
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_BIT_STRING, false,
        bit_string_encoded_length);
    len += encode_bitstring(&apdu[len], bit_string);

    return len;
}

int encode_context_bitstring(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_BIT_STRING * bit_string)
{
    int len = 0;
    uint32_t bit_string_encoded_length = 1;     /* 1 for the bits remaining octet */

    /* bit string may use more than 1 octet for the tag, so find out how many */
    bit_string_encoded_length += bitstring_bytes_used(bit_string);
    len = encode_tag(&apdu[0], tag_number, true, bit_string_encoded_length);
    len += encode_bitstring(&apdu[len], bit_string);

    return len;
}

/* from clause 20.2.14 Encoding of an Object Identifier Value */
/* returns the number of apdu bytes consumed */
int decode_object_id(
    uint8_t * apdu,
    uint16_t * object_type,
    uint32_t * instance)
{
    uint32_t value = 0;
    int len = 0;

    len = decode_unsigned32(apdu, &value);
    *object_type =
        (uint16_t) (((value >> BACNET_INSTANCE_BITS) & BACNET_MAX_OBJECT));
    *instance = (value & BACNET_MAX_INSTANCE);

    return len;
}

int decode_object_id_safe(
    uint8_t * apdu,
    uint32_t len_value,
    uint16_t * object_type,
    uint32_t * instance)
{
    if (len_value != 4) {
        return 0;
    } else {
        return decode_object_id(apdu, object_type, instance);
    }
}

int decode_context_object_id(
    uint8_t * apdu,
    uint8_t tag_number,
    uint16_t * object_type,
    uint32_t * instance)
{
    int len = 0;

    if (decode_is_context_tag_with_length(&apdu[len], tag_number, &len)) {
        len += decode_object_id(&apdu[len], object_type, instance);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}

/* from clause 20.2.14 Encoding of an Object Identifier Value */
/* returns the number of apdu bytes consumed */
int encode_bacnet_object_id(
    uint8_t * apdu,
    int object_type,
    uint32_t instance)
{
    uint32_t value = 0;
    uint32_t type = 0;
    int len = 0;

    type = (uint32_t) object_type;
    value =
        ((type & BACNET_MAX_OBJECT) << BACNET_INSTANCE_BITS) | (instance &
        BACNET_MAX_INSTANCE);
    len = encode_unsigned32(apdu, value);

    return len;
}

/* from clause 20.2.14 Encoding of an Object Identifier Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_context_object_id(
    uint8_t * apdu,
    uint8_t tag_number,
    int object_type,
    uint32_t instance)
{
    int len = 0;

    /* length of object id is 4 octets, as per 20.2.14 */

    len = encode_tag(&apdu[0], tag_number, true, 4);
    len += encode_bacnet_object_id(&apdu[len], object_type, instance);

    return len;
}

/* from clause 20.2.14 Encoding of an Object Identifier Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_object_id(
    uint8_t * apdu,
    int object_type,
    uint32_t instance)
{
    int len = 0;

    /* assumes that the tag only consumes 1 octet */
    len = encode_bacnet_object_id(&apdu[1], object_type, instance);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_OBJECT_ID, false,
        (uint32_t) len);

    return len;
}

#if BACNET_USE_OCTETSTRING

/* from clause 20.2.8 Encoding of an Octet String Value */
/* returns the number of apdu bytes consumed */
int encode_octet_string(
    uint8_t * apdu,
    BACNET_OCTET_STRING * octet_string)
{
    int len = 0;        /* return value */
    uint8_t *value;
    int i = 0;  /* loop counter */

    if (octet_string) {
        /* FIXME: might need to pass in the length of the APDU
           to bounds check since it might not be the only data chunk */
        len = (int) octetstring_length(octet_string);
        value = octetstring_value(octet_string);
        for (i = 0; i < len; i++) {
            apdu[i] = value[i];
        }
    }

    return len;
}

/* from clause 20.2.8 Encoding of an Octet String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_octet_string(
    uint8_t * apdu,
    BACNET_OCTET_STRING * octet_string)
{
    int apdu_len = 0;

    if (octet_string) {
        apdu_len =
            encode_tag(&apdu[0], BACNET_APPLICATION_TAG_OCTET_STRING, false,
            octetstring_length(octet_string));
        /* FIXME: probably need to pass in the length of the APDU
           to bounds check since it might not be the only data chunk */
        if ((apdu_len + octetstring_length(octet_string)) < MAX_APDU) {
            apdu_len += encode_octet_string(&apdu[apdu_len], octet_string);
        } else {
            apdu_len = 0;
        }
    }

    return apdu_len;
}

/* from clause 20.2.8 Encoding of an Octet String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_context_octet_string(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_OCTET_STRING * octet_string)
{
    int apdu_len = 0;

    if (apdu && octet_string) {
        apdu_len =
            encode_tag(&apdu[0], tag_number, true,
            octetstring_length(octet_string));
        if ((apdu_len + octetstring_length(octet_string)) < MAX_APDU) {
            apdu_len += encode_octet_string(&apdu[apdu_len], octet_string);
        } else {
            apdu_len = 0;
        }
    }

    return apdu_len;
}

/* from clause 20.2.8 Encoding of an Octet String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_octet_string(
    uint8_t * apdu,
    uint32_t len_value,
    BACNET_OCTET_STRING * octet_string)
{
    int len = 0;        /* return value */
    bool status = false;

    status = octetstring_init(octet_string, &apdu[0], len_value);
    if (status) {
        len = (int) len_value;
    }

    return len;
}

int decode_context_octet_string(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_OCTET_STRING * octet_string)
{
    int len = 0;        /* return value */
    bool status = false;
    uint32_t len_value = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);

        status = octetstring_init(octet_string, &apdu[len], len_value);

        if (status) {
            len += len_value;
        }
    } else {
        len = BACNET_STATUS_ERROR;
    }

    return len;
}
#endif

/* from clause 20.2.9 Encoding of a Character String Value */
/* returns the number of apdu bytes consumed, or zero if failed */
uint32_t encode_bacnet_character_string_safe(
    uint8_t * apdu,
    uint32_t max_apdu,
    uint8_t encoding,
    char *pString,
    uint32_t length)
{
    uint32_t apdu_len = 1 /*encoding */ ;
    uint32_t i;

    apdu_len += length;
    if (apdu && (apdu_len <= max_apdu)) {
        apdu[0] = encoding;
        for (i = 0; i < length; i++) {
            apdu[1 + i] = (uint8_t) pString[i];
        }
    } else {
        apdu_len = 0;
    }

    return apdu_len;
}

int encode_bacnet_character_string(
    uint8_t * apdu,
    BACNET_CHARACTER_STRING * char_string)
{
    return (int) encode_bacnet_character_string_safe(apdu, MAX_APDU,
        characterstring_encoding(char_string),
        characterstring_value(char_string),
        characterstring_length(char_string));
}

/* from clause 20.2.9 Encoding of a Character String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_character_string(
    uint8_t * apdu,
    BACNET_CHARACTER_STRING * char_string)
{
    int len = 0;
    int string_len = 0;

    string_len =
        (int) characterstring_length(char_string) + 1 /* for encoding */ ;
    len =
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_CHARACTER_STRING, false,
        (uint32_t) string_len);
    if ((len + string_len) < MAX_APDU) {
        len += encode_bacnet_character_string(&apdu[len], char_string);
    } else {
        len = 0;
    }

    return len;
}

int encode_context_character_string(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_CHARACTER_STRING * char_string)
{
    int len = 0;
    int string_len = 0;

    string_len =
        (int) characterstring_length(char_string) + 1 /* for encoding */ ;
    len += encode_tag(&apdu[0], tag_number, true, (uint32_t) string_len);
    if ((len + string_len) < MAX_APDU) {
        len += encode_bacnet_character_string(&apdu[len], char_string);
    } else {
        len = 0;
    }

    return len;
}

/* from clause 20.2.9 Encoding of a Character String Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_character_string(
    uint8_t * apdu,
    uint32_t len_value,
    BACNET_CHARACTER_STRING * char_string)
{
    int len = 0;        /* return value */
    bool status = false;

    status =
        characterstring_init(char_string, apdu[0], (char *) &apdu[1],
        len_value - 1);
    if (status) {
        len = (int) len_value;
    }

    return len;
}

int decode_context_character_string(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_CHARACTER_STRING * char_string)
{
    int len = 0;        /* return value */
    bool status = false;
    uint32_t len_value = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);

        status =
            characterstring_init(char_string, apdu[len],
            (char *) &apdu[len + 1], len_value - 1);
        if (status) {
            len += len_value;
        }
    } else {
        len = BACNET_STATUS_ERROR;
    }

    return len;
}

/* from clause 20.2.4 Encoding of an Unsigned Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_unsigned(
    uint8_t * apdu,
    uint32_t len_value,
    uint32_t * value)
{
    uint16_t unsigned16_value = 0;

    if (value) {
        switch (len_value) {
            case 1:
                *value = apdu[0];
                break;
            case 2:
                decode_unsigned16(&apdu[0], &unsigned16_value);
                *value = unsigned16_value;
                break;
            case 3:
                decode_unsigned24(&apdu[0], value);
                break;
            case 4:
                decode_unsigned32(&apdu[0], value);
                break;
            default:
                *value = 0;
                break;
        }
    }

    return (int) len_value;
}

int decode_context_unsigned(
    uint8_t * apdu,
    uint8_t tag_number,
    uint32_t * value)
{
    uint32_t len_value;
    int len = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        len += decode_unsigned(&apdu[len], len_value, value);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}


/* from clause 20.2.4 Encoding of an Unsigned Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_bacnet_unsigned(
    uint8_t * apdu,
    uint32_t value)
{
    int len = 0;        /* return value */

    if (value < 0x100) {
        apdu[0] = (uint8_t) value;
        len = 1;
    } else if (value < 0x10000) {
        len = encode_unsigned16(&apdu[0], (uint16_t) value);
    } else if (value < 0x1000000) {
        len = encode_unsigned24(&apdu[0], value);
    } else {
        len = encode_unsigned32(&apdu[0], value);
    }

    return len;
}

/* from clause 20.2.4 Encoding of an Unsigned Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_context_unsigned(
    uint8_t * apdu,
    uint8_t tag_number,
    uint32_t value)
{
    int len = 0;

    /* length of unsigned is variable, as per 20.2.4 */
    if (value < 0x100) {
        len = 1;
    } else if (value < 0x10000) {
        len = 2;
    } else if (value < 0x1000000) {
        len = 3;
    } else {
        len = 4;
    }

    len = encode_tag(&apdu[0], tag_number, true, (uint32_t) len);
    len += encode_bacnet_unsigned(&apdu[len], value);

    return len;
}

/* from clause 20.2.4 Encoding of an Unsigned Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_unsigned(
    uint8_t * apdu,
    uint32_t value)
{
    int len = 0;

    len = encode_bacnet_unsigned(&apdu[1], value);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_UNSIGNED_INT, false,
        (uint32_t) len);

    return len;
}

/* from clause 20.2.11 Encoding of an Enumerated Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_enumerated(
    uint8_t * apdu,
    uint32_t len_value,
    uint32_t * value)
{
    uint32_t unsigned_value = 0;
    int len;

    len = decode_unsigned(apdu, len_value, &unsigned_value);
    if (value) {
        *value = unsigned_value;
    }

    return len;
}

int decode_context_enumerated(
    uint8_t * apdu,
    uint8_t tag_value,
    uint32_t * value)
{
    int len = 0;
    uint8_t tag_number;
    uint32_t len_value;

    if (decode_is_context_tag(&apdu[len], tag_value)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        len += decode_enumerated(&apdu[len], len_value, value);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}

/* from clause 20.2.11 Encoding of an Enumerated Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_bacnet_enumerated(
    uint8_t * apdu,
    uint32_t value)
{
    return encode_bacnet_unsigned(apdu, value);
}

/* from clause 20.2.11 Encoding of an Enumerated Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_enumerated(
    uint8_t * apdu,
    uint32_t value)
{
    int len = 0;        /* return value */

    /* assumes that the tag only consumes 1 octet */
    len = encode_bacnet_enumerated(&apdu[1], value);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_ENUMERATED, false,
        (uint32_t) len);

    return len;
}

/* from clause 20.2.11 Encoding of an Enumerated Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_context_enumerated(
    uint8_t * apdu,
    uint8_t tag_number,
    uint32_t value)
{
    int len = 0;        /* return value */

    /* length of enumerated is variable, as per 20.2.11 */
    if (value < 0x100) {
        len = 1;
    } else if (value < 0x10000) {
        len = 2;
    } else if (value < 0x1000000) {
        len = 3;
    } else {
        len = 4;
    }

    len = encode_tag(&apdu[0], tag_number, true, (uint32_t) len);
    len += encode_bacnet_enumerated(&apdu[len], value);

    return len;
}

#if BACNET_USE_SIGNED
/* from clause 20.2.5 Encoding of a Signed Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_signed(
    uint8_t * apdu,
    uint32_t len_value,
    int32_t * value)
{
    if (value) {
        switch (len_value) {
            case 1:
                decode_signed8(&apdu[0], value);
                break;
            case 2:
                decode_signed16(&apdu[0], value);
                break;
            case 3:
                decode_signed24(&apdu[0], value);
                break;
            case 4:
                decode_signed32(&apdu[0], value);
                break;
            default:
                *value = 0;
                break;
        }
    }

    return (int) len_value;
}

int decode_context_signed(
    uint8_t * apdu,
    uint8_t tag_number,
    int32_t * value)
{
    uint32_t len_value;
    int len = 0;

    if (decode_is_context_tag(&apdu[len], tag_number)) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        len += decode_signed(&apdu[len], len_value, value);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}

/* from clause 20.2.5 Encoding of a Signed Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_bacnet_signed(
    uint8_t * apdu,
    int32_t value)
{
    int len = 0;        /* return value */

    /* don't encode the leading X'FF' or X'00' of the two's compliment.
       That is, the first octet of any multi-octet encoded value shall
       not be X'00' if the most significant bit (bit 7) of the second
       octet is 0, and the first octet shall not be X'FF' if the most
       significant bit of the second octet is 1. */
    if ((value >= -128) && (value < 128)) {
        len = encode_signed8(&apdu[0], (int8_t) value);
    } else if ((value >= -32768) && (value < 32768)) {
        len = encode_signed16(&apdu[0], (int16_t) value);
    } else if ((value > -8388608) && (value < 8388608)) {
        len = encode_signed24(&apdu[0], value);
    } else {
        len = encode_signed32(&apdu[0], value);
    }

    return len;
}

/* from clause 20.2.5 Encoding of a Signed Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_signed(
    uint8_t * apdu,
    int32_t value)
{
    int len = 0;        /* return value */

    /* assumes that the tag only consumes 1 octet */
    len = encode_bacnet_signed(&apdu[1], value);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_SIGNED_INT, false,
        (uint32_t) len);

    return len;
}

/* from clause 20.2.5 Encoding of a Signed Integer Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_context_signed(
    uint8_t * apdu,
    uint8_t tag_number,
    int32_t value)
{
    int len = 0;        /* return value */

    /* length of signed int is variable, as per 20.2.11 */
    if ((value >= -128) && (value < 128)) {
        len = 1;
    } else if ((value >= -32768) && (value < 32768)) {
        len = 2;
    } else if ((value > -8388608) && (value < 8388608)) {
        len = 3;
    } else {
        len = 4;
    }

    len = encode_tag(&apdu[0], tag_number, true, (uint32_t) len);
    len += encode_bacnet_signed(&apdu[len], value);

    return len;
}
#endif

/* from clause 20.2.6 Encoding of a Real Number Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_real(
    uint8_t * apdu,
    float value)
{
    int len = 0;

    /* assumes that the tag only consumes 1 octet */
    len = encode_bacnet_real(value, &apdu[1]);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_REAL, false,
        (uint32_t) len);

    return len;
}

int encode_context_real(
    uint8_t * apdu,
    uint8_t tag_number,
    float value)
{
    int len = 0;

    /* length of double is 4 octets, as per 20.2.6 */
    len = encode_tag(&apdu[0], tag_number, true, 4);
    len += encode_bacnet_real(value, &apdu[len]);
    return len;
}

#if BACNET_USE_DOUBLE
/* from clause 20.2.7 Encoding of a Double Precision Real Number Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_double(
    uint8_t * apdu,
    double value)
{
    int len = 0;

    /* assumes that the tag only consumes 2 octet */
    len = encode_bacnet_double(value, &apdu[2]);

    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_DOUBLE, false,
        (uint32_t) len);

    return len;
}

int encode_context_double(
    uint8_t * apdu,
    uint8_t tag_number,
    double value)
{
    int len = 0;

    /* length of double is 8 octets, as per 20.2.7 */
    len = encode_tag(&apdu[0], tag_number, true, 8);
    len += encode_bacnet_double(value, &apdu[len]);

    return len;
}
#endif

/* from clause 20.2.13 Encoding of a Time Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_bacnet_time(
    uint8_t * apdu,
    BACNET_TIME * btime)
{
    apdu[0] = btime->hour;
    apdu[1] = btime->min;
    apdu[2] = btime->sec;
    apdu[3] = btime->hundredths;

    return 4;
}

/* from clause 20.2.13 Encoding of a Time Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_time(
    uint8_t * apdu,
    BACNET_TIME * btime)
{
    int len = 0;

    /* assumes that the tag only consumes 1 octet */
    len = encode_bacnet_time(&apdu[1], btime);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_TIME, false,
        (uint32_t) len);

    return len;
}

int encode_context_time(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_TIME * btime)
{
    int len = 0;        /* return value */

    /* length of time is 4 octets, as per 20.2.13 */
    len = encode_tag(&apdu[0], tag_number, true, 4);
    len += encode_bacnet_time(&apdu[len], btime);

    return len;
}

/* from clause 20.2.13 Encoding of a Time Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_bacnet_time(
    uint8_t * apdu,
    BACNET_TIME * btime)
{
    btime->hour = apdu[0];
    btime->min = apdu[1];
    btime->sec = apdu[2];
    btime->hundredths = apdu[3];

    return 4;
}

int decode_bacnet_time_safe(
    uint8_t * apdu,
    uint32_t len_value,
    BACNET_TIME * btime)
{
    if (len_value != 4) {
        btime->hour = 0;
        btime->hundredths = 0;
        btime->min = 0;
        btime->sec = 0;
        return (int) len_value;
    } else {
        return decode_bacnet_time(apdu, btime);
    }
}

int decode_application_time(
    uint8_t * apdu,
    BACNET_TIME * btime)
{
    int len = 0;
    uint8_t tag_number;
    decode_tag_number(&apdu[len], &tag_number);

    if (tag_number == BACNET_APPLICATION_TAG_TIME) {
        len++;
        len += decode_bacnet_time(&apdu[len], btime);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}


int decode_context_bacnet_time(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_TIME * btime)
{
    int len = 0;

    if (decode_is_context_tag_with_length(&apdu[len], tag_number, &len)) {
        len += decode_bacnet_time(&apdu[len], btime);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}


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
    BACNET_DATE * bdate)
{
    /* allow 2 digit years */
    if (bdate->year >= 1900) {
        apdu[0] = (uint8_t) (bdate->year - 1900);
    } else if (bdate->year < 0x100) {
        apdu[0] = (uint8_t) bdate->year;

    } else {
        /*
         ** Don't try and guess what the user meant here. Just fail
         */
        return BACNET_STATUS_ERROR;
    }


    apdu[1] = bdate->month;
    apdu[2] = bdate->day;
    apdu[3] = bdate->wday;

    return 4;
}


/* from clause 20.2.12 Encoding of a Date Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int encode_application_date(
    uint8_t * apdu,
    BACNET_DATE * bdate)
{
    int len = 0;

    /* assumes that the tag only consumes 1 octet */
    len = encode_bacnet_date(&apdu[1], bdate);
    len +=
        encode_tag(&apdu[0], BACNET_APPLICATION_TAG_DATE, false,
        (uint32_t) len);

    return len;

}

int encode_context_date(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_DATE * bdate)
{
    int len = 0;        /* return value */

    /* length of date is 4 octets, as per 20.2.12 */
    len = encode_tag(&apdu[0], tag_number, true, 4);
    len += encode_bacnet_date(&apdu[len], bdate);

    return len;
}

/* from clause 20.2.12 Encoding of a Date Value */
/* and 20.2.1 General Rules for Encoding BACnet Tags */
/* returns the number of apdu bytes consumed */
int decode_date(
    uint8_t * apdu,
    BACNET_DATE * bdate)
{
    bdate->year = (uint16_t) (apdu[0] + 1900);
    bdate->month = apdu[1];
    bdate->day = apdu[2];
    bdate->wday = apdu[3];

    return 4;
}

int decode_date_safe(
    uint8_t * apdu,
    uint32_t len_value,
    BACNET_DATE * bdate)
{
    if (len_value != 4) {
        bdate->day = 0;
        bdate->month = 0;
        bdate->wday = 0;
        bdate->year = 0;
        return (int) len_value;
    } else {
        return decode_date(apdu, bdate);
    }
}


int decode_application_date(
    uint8_t * apdu,
    BACNET_DATE * bdate)
{
    int len = 0;
    uint8_t tag_number;
    decode_tag_number(&apdu[len], &tag_number);

    if (tag_number == BACNET_APPLICATION_TAG_DATE) {
        len++;
        len += decode_date(&apdu[len], bdate);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}

int decode_context_date(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_DATE * bdate)
{
    int len = 0;

    if (decode_is_context_tag_with_length(&apdu[len], tag_number, &len)) {
        len += decode_date(&apdu[len], bdate);
    } else {
        len = BACNET_STATUS_ERROR;
    }
    return len;
}



/* returns the number of apdu bytes consumed */
int encode_simple_ack(
    uint8_t * apdu,
    uint8_t invoke_id,
    uint8_t service_choice)
{
    apdu[0] = PDU_TYPE_SIMPLE_ACK;
    apdu[1] = invoke_id;
    apdu[2] = service_choice;

    return 3;
}

/* end of decoding_encoding.c */
#ifdef TEST
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "ctest.h"

static int get_apdu_len(
    bool extended_tag,
    uint32_t value)
{
    int test_len = 1;

    if (extended_tag) {
        test_len++;
    }
    if (value <= 4) {
        test_len += 0;  /* do nothing... */
    } else if (value <= 253) {
        test_len += 1;
    } else if (value <= 65535) {
        test_len += 3;
    } else {
        test_len += 5;
    }

    return test_len;
}

static void print_apdu(
    uint8_t * pBlock,
    uint32_t num)
{
    size_t lines = 0;   /* number of lines to print */
    size_t line = 0;    /* line of text counter */
    size_t last_line = 0;       /* line on which the last text resided */
    unsigned long count = 0;    /* address to print */
    unsigned int i = 0; /* counter */

    if (pBlock && num) {
        /* how many lines to print? */
        num--;  /* adjust */
        lines = (num / 16) + 1;
        last_line = num % 16;

        /* create the line */
        for (line = 0; line < lines; line++) {
            /* start with the address */
            printf("%08lX: ", count);
            /* hex representation */
            for (i = 0; i < 16; i++) {
                if (((line == (lines - 1)) && (i <= last_line)) ||
                    (line != (lines - 1))) {
                    printf("%02X ", (unsigned) (0x00FF & pBlock[i]));
                } else {
                    printf("-- ");
                }
            }
            printf(" ");
            /* print the characters if valid */
            for (i = 0; i < 16; i++) {
                if (((line == (lines - 1)) && (i <= last_line)) ||
                    (line != (lines - 1))) {
                    if (isprint(pBlock[i])) {
                        printf("%c", pBlock[i]);
                    } else {
                        printf(".");
                    }
                } else {
                    printf(".");
                }
            }
            printf("\r\n");
            pBlock += 16;
            count += 16;
        }
    }

    return;
}

void testBACDCodeTags(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    uint8_t tag_number = 0, test_tag_number = 0;
    int len = 0, test_len = 0;
    uint32_t value = 0, test_value = 0;

    for (tag_number = 0;; tag_number++) {
        len = encode_opening_tag(&apdu[0], tag_number);
        test_len = get_apdu_len(IS_EXTENDED_TAG_NUMBER(apdu[0]), 0);
        ct_test(pTest, len == test_len);
        len = decode_tag_number_and_value(&apdu[0], &test_tag_number, &value);
        ct_test(pTest, value == 0);
        ct_test(pTest, len == test_len);
        ct_test(pTest, tag_number == test_tag_number);
        ct_test(pTest, IS_OPENING_TAG(apdu[0]) == true);
        ct_test(pTest, IS_CLOSING_TAG(apdu[0]) == false);
        len = encode_closing_tag(&apdu[0], tag_number);
        ct_test(pTest, len == test_len);
        len = decode_tag_number_and_value(&apdu[0], &test_tag_number, &value);
        ct_test(pTest, len == test_len);
        ct_test(pTest, value == 0);
        ct_test(pTest, tag_number == test_tag_number);
        ct_test(pTest, IS_OPENING_TAG(apdu[0]) == false);
        ct_test(pTest, IS_CLOSING_TAG(apdu[0]) == true);
        /* test the len-value-type portion */
        for (value = 1;; value = value << 1) {
            len = encode_tag(&apdu[0], tag_number, false, value);
            len =
                decode_tag_number_and_value(&apdu[0], &test_tag_number,
                &test_value);
            ct_test(pTest, tag_number == test_tag_number);
            ct_test(pTest, value == test_value);
            test_len = get_apdu_len(IS_EXTENDED_TAG_NUMBER(apdu[0]), value);
            ct_test(pTest, len == test_len);
            /* stop at the the last value */
            if (value & BIT31) {
                break;
            }
        }
        /* stop after the last tag number */
        if (tag_number == 255) {
            break;
        }
    }

    return;
}

void testBACDCodeEnumerated(
    Test * pTest)
{
    uint8_t array[5] = { 0 };
    uint8_t encoded_array[5] = { 0 };
    uint32_t value = 1;
    uint32_t decoded_value = 0;
    int i = 0, apdu_len = 0;
    int len = 0;
    uint8_t apdu[MAX_APDU] = { 0 };
    uint8_t tag_number = 0;
    uint32_t len_value = 0;

    for (i = 0; i < 31; i++) {
        apdu_len = encode_application_enumerated(&array[0], value);
        len = decode_tag_number_and_value(&array[0], &tag_number, &len_value);
        len += decode_enumerated(&array[len], len_value, &decoded_value);
        ct_test(pTest, decoded_value == value);
        ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_ENUMERATED);
        ct_test(pTest, len == apdu_len);
        /* encode back the value */
        encode_application_enumerated(&encoded_array[0], decoded_value);
        ct_test(pTest, memcmp(&array[0], &encoded_array[0],
                sizeof(array)) == 0);
        /* an enumerated will take up to 4 octects */
        /* plus a one octet for the tag */
        apdu_len = encode_application_enumerated(&apdu[0], value);
        len = decode_tag_number_and_value(&apdu[0], &tag_number, NULL);
        ct_test(pTest, len == 1);
        ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_ENUMERATED);
        ct_test(pTest, IS_CONTEXT_SPECIFIC(apdu[0]) == false);
        /* context specific encoding */
        apdu_len = encode_context_enumerated(&apdu[0], 3, value);
        ct_test(pTest, IS_CONTEXT_SPECIFIC(apdu[0]) == true);
        len = decode_tag_number_and_value(&apdu[0], &tag_number, NULL);
        ct_test(pTest, len == 1);
        ct_test(pTest, tag_number == 3);
        /* test the interesting values */
        value = value << 1;
    }

    return;
}

void testBACDCodeReal(
    Test * pTest)
{
    uint8_t real_array[4] = { 0 };
    uint8_t encoded_array[4] = { 0 };
    float value = 42.123F;
    float decoded_value = 0.0F;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0, apdu_len = 0;
    uint8_t tag_number = 0;
    uint32_t long_value = 0;

    encode_bacnet_real(value, &real_array[0]);
    decode_real(&real_array[0], &decoded_value);
    ct_test(pTest, decoded_value == value);
    encode_bacnet_real(value, &encoded_array[0]);
    ct_test(pTest, memcmp(&real_array, &encoded_array,
            sizeof(real_array)) == 0);

    /* a real will take up 4 octects plus a one octet tag */
    apdu_len = encode_application_real(&apdu[0], value);
    ct_test(pTest, apdu_len == 5);
    /* len tells us how many octets were used for encoding the value */
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &long_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_REAL);
    ct_test(pTest, IS_CONTEXT_SPECIFIC(apdu[0]) == false);
    ct_test(pTest, len == 1);
    ct_test(pTest, long_value == 4);
    decode_real(&apdu[len], &decoded_value);
    ct_test(pTest, decoded_value == value);

    return;
}

void testBACDCodeDouble(
    Test * pTest)
{
    uint8_t double_array[8] = { 0 };
    uint8_t encoded_array[8] = { 0 };
    double value = 42.123;
    double decoded_value = 0.0;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0, apdu_len = 0;
    uint8_t tag_number = 0;
    uint32_t long_value = 0;

    encode_bacnet_double(value, &double_array[0]);
    decode_double(&double_array[0], &decoded_value);
    ct_test(pTest, decoded_value == value);
    encode_bacnet_double(value, &encoded_array[0]);
    ct_test(pTest, memcmp(&double_array, &encoded_array,
            sizeof(double_array)) == 0);

    /* a real will take up 4 octects plus a one octet tag */
    apdu_len = encode_application_double(&apdu[0], value);
    ct_test(pTest, apdu_len == 10);
    /* len tells us how many octets were used for encoding the value */
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &long_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_DOUBLE);
    ct_test(pTest, IS_CONTEXT_SPECIFIC(apdu[0]) == false);
    ct_test(pTest, len == 2);
    ct_test(pTest, long_value == 8);
    decode_double(&apdu[len], &decoded_value);
    ct_test(pTest, decoded_value == value);

    return;
}

void testBACDCodeUnsignedValue(
    Test * pTest,
    uint32_t value)
{
    uint8_t array[5] = { 0 };
    uint8_t encoded_array[5] = { 0 };
    uint32_t decoded_value = 0;
    int len;
    uint8_t apdu[MAX_APDU] = { 0 };
    uint8_t tag_number = 0;
    uint32_t len_value = 0;

    len_value = encode_application_unsigned(&array[0], value);
    len = decode_tag_number_and_value(&array[0], &tag_number, &len_value);
    len = decode_unsigned(&array[len], len_value, &decoded_value);
    ct_test(pTest, decoded_value == value);
    if (decoded_value != value) {
        printf("value=%lu decoded_value=%lu\n", (unsigned long) value,
            (unsigned long) decoded_value);
        print_apdu(&array[0], sizeof(array));
    }
    encode_application_unsigned(&encoded_array[0], decoded_value);
    ct_test(pTest, memcmp(&array[0], &encoded_array[0], sizeof(array)) == 0);
    /* an unsigned will take up to 4 octects */
    /* plus a one octet for the tag */
    encode_application_unsigned(&apdu[0], value);
    /* apdu_len varies... */
    len = decode_tag_number_and_value(&apdu[0], &tag_number, NULL);
    ct_test(pTest, len == 1);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_UNSIGNED_INT);
    ct_test(pTest, IS_CONTEXT_SPECIFIC(apdu[0]) == false);
}

void testBACDCodeUnsigned(
    Test * pTest)
{
    uint32_t value = 1;
    int i;

    for (i = 0; i < 32; i++) {
        testBACDCodeUnsignedValue(pTest, value - 1);
        testBACDCodeUnsignedValue(pTest, value);
        testBACDCodeUnsignedValue(pTest, value + 1);
        value = value << 1;
    }

    return;
}

void testBACnetUnsigned(
    Test * pTest)
{
    uint8_t apdu[32] = { 0 };
    uint32_t value = 0, test_value = 0;
    int len = 0, test_len = 0;

    for (value = 0;; value += 0xFF) {
        len = encode_bacnet_unsigned(&apdu[0], value);
        test_len = decode_unsigned(&apdu[0], len, &test_value);
        ct_test(pTest, len == test_len);
        ct_test(pTest, value == test_value);
        if (value == 0xFFFFFFFF)
            break;
    }
}

void testBACDCodeSignedValue(
    Test * pTest,
    int32_t value)
{
    uint8_t array[5] = { 0 };
    uint8_t encoded_array[5] = { 0 };
    int32_t decoded_value = 0;
    int len = 0;
    uint8_t apdu[MAX_APDU] = { 0 };
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    int diff = 0;

    len = encode_application_signed(&array[0], value);
    len = decode_tag_number_and_value(&array[0], &tag_number, &len_value);
    len = decode_signed(&array[len], len_value, &decoded_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_SIGNED_INT);
    ct_test(pTest, decoded_value == value);
    if (decoded_value != value) {
        printf("value=%ld decoded_value=%ld\n", (long) value,
            (long) decoded_value);
        print_apdu(&array[0], sizeof(array));
    }
    encode_application_signed(&encoded_array[0], decoded_value);
    diff = memcmp(&array[0], &encoded_array[0], sizeof(array));
    ct_test(pTest, diff == 0);
    if (diff) {
        printf("value=%ld decoded_value=%ld\n", (long) value,
            (long) decoded_value);
        print_apdu(&array[0], sizeof(array));
        print_apdu(&encoded_array[0], sizeof(array));
    }
    /* a signed int will take up to 4 octects */
    /* plus a one octet for the tag */
    encode_application_signed(&apdu[0], value);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, NULL);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_SIGNED_INT);
    ct_test(pTest, IS_CONTEXT_SPECIFIC(apdu[0]) == false);

    return;
}

void testBACDCodeSigned(
    Test * pTest)
{
    int value = 1;
    int i = 0;

    for (i = 0; i < 32; i++) {
        testBACDCodeSignedValue(pTest, value - 1);
        testBACDCodeSignedValue(pTest, value);
        testBACDCodeSignedValue(pTest, value + 1);
        value = value << 1;
    }

    testBACDCodeSignedValue(pTest, -1);
    value = -2;
    for (i = 0; i < 32; i++) {
        testBACDCodeSignedValue(pTest, value - 1);
        testBACDCodeSignedValue(pTest, value);
        testBACDCodeSignedValue(pTest, value + 1);
        value = value << 1;
    }

    return;
}

void testBACnetSigned(
    Test * pTest)
{
    uint8_t apdu[32] = { 0 };
    int32_t value = 0, test_value = 0;
    int len = 0, test_len = 0;

    for (value = -2147483647; value < 0; value += 127) {
        len = encode_bacnet_signed(&apdu[0], value);
        test_len = decode_signed(&apdu[0], len, &test_value);
        ct_test(pTest, len == test_len);
        ct_test(pTest, value == test_value);
    }
    for (value = 2147483647; value > 0; value -= 127) {
        len = encode_bacnet_signed(&apdu[0], value);
        test_len = decode_signed(&apdu[0], len, &test_value);
        ct_test(pTest, len == test_len);
        ct_test(pTest, value == test_value);
    }
}

void testBACDCodeOctetString(
    Test * pTest)
{
    uint8_t array[MAX_APDU] = { 0 };
    uint8_t encoded_array[MAX_APDU] = { 0 };
    BACNET_OCTET_STRING octet_string;
    BACNET_OCTET_STRING test_octet_string;
    uint8_t test_value[MAX_APDU] = { "" };
    int i;      /* for loop counter */
    int apdu_len;
    int len;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    bool status = false;
    int diff = 0;       /* for memcmp */

    status = octetstring_init(&octet_string, NULL, 0);
    ct_test(pTest, status == true);
    apdu_len = encode_application_octet_string(&array[0], &octet_string);
    len = decode_tag_number_and_value(&array[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OCTET_STRING);
    len += decode_octet_string(&array[len], len_value, &test_octet_string);
    ct_test(pTest, apdu_len == len);
    diff =
        memcmp(octetstring_value(&octet_string), &test_value[0],
        octetstring_length(&octet_string));
    ct_test(pTest, diff == 0);

    for (i = 0; i < (MAX_APDU - 6); i++) {
        test_value[i] = '0' + (i % 10);
        status = octetstring_init(&octet_string, test_value, i);
        ct_test(pTest, status == true);
        apdu_len =
            encode_application_octet_string(&encoded_array[0], &octet_string);
        len =
            decode_tag_number_and_value(&encoded_array[0], &tag_number,
            &len_value);
        ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OCTET_STRING);
        len +=
            decode_octet_string(&encoded_array[len], len_value,
            &test_octet_string);
        if (apdu_len != len) {
            printf("test octet string=#%d\n", i);
        }
        ct_test(pTest, apdu_len == len);
        diff =
            memcmp(octetstring_value(&octet_string), &test_value[0],
            octetstring_length(&octet_string));
        if (diff) {
            printf("test octet string=#%d\n", i);
        }
        ct_test(pTest, diff == 0);
    }

    return;
}

void testBACDCodeCharacterString(
    Test * pTest)
{
    uint8_t array[MAX_APDU] = { 0 };
    uint8_t encoded_array[MAX_APDU] = { 0 };
    BACNET_CHARACTER_STRING char_string;
    BACNET_CHARACTER_STRING test_char_string;
    char test_value[MAX_APDU] = { "" };
    int i;      /* for loop counter */
    int apdu_len;
    int len;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    int diff = 0;       /* for comparison */
    bool status = false;

    status = characterstring_init(&char_string, CHARACTER_ANSI_X34, NULL, 0);
    ct_test(pTest, status == true);
    apdu_len = encode_application_character_string(&array[0], &char_string);
    len = decode_tag_number_and_value(&array[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_CHARACTER_STRING);
    len += decode_character_string(&array[len], len_value, &test_char_string);
    ct_test(pTest, apdu_len == len);
    diff =
        memcmp(characterstring_value(&char_string), &test_value[0],
        characterstring_length(&char_string));
    ct_test(pTest, diff == 0);
    for (i = 0; i < MAX_CHARACTER_STRING_BYTES - 1; i++) {
        test_value[i] = 'S';
        test_value[i + 1] = '\0';
        status = characterstring_init_ansi(&char_string, test_value);
        ct_test(pTest, status == true);
        apdu_len =
            encode_application_character_string(&encoded_array[0],
            &char_string);
        len =
            decode_tag_number_and_value(&encoded_array[0], &tag_number,
            &len_value);
        ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_CHARACTER_STRING);
        len +=
            decode_character_string(&encoded_array[len], len_value,
            &test_char_string);
        if (apdu_len != len) {
            printf("test string=#%d apdu_len=%d len=%d\n", i, apdu_len, len);
        }
        ct_test(pTest, apdu_len == len);
        diff =
            memcmp(characterstring_value(&char_string), &test_value[0],
            characterstring_length(&char_string));
        if (diff) {
            printf("test string=#%d\n", i);
        }
        ct_test(pTest, diff == 0);
    }

    return;
}

void testBACDCodeObject(
    Test * pTest)
{
    uint8_t object_array[4] = {
        0
    };
    uint8_t encoded_array[4] = {
        0
    };
    uint16_t type = OBJECT_BINARY_INPUT;
    uint16_t decoded_type = OBJECT_ANALOG_OUTPUT;
    uint32_t instance = 123;
    uint32_t decoded_instance = 0;

    encode_bacnet_object_id(&encoded_array[0], type, instance);
    decode_object_id(&encoded_array[0], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == type);
    ct_test(pTest, decoded_instance == instance);
    encode_bacnet_object_id(&object_array[0], type, instance);
    ct_test(pTest, memcmp(&object_array[0], &encoded_array[0],
            sizeof(object_array)) == 0);
    for (type = 0; type < 1024; type++) {
        for (instance = 0; instance <= BACNET_MAX_INSTANCE; instance += 1024) {
            encode_bacnet_object_id(&encoded_array[0], type, instance);
            decode_object_id(&encoded_array[0], &decoded_type,
                &decoded_instance);
            ct_test(pTest, decoded_type == type);
            ct_test(pTest, decoded_instance == instance);
            encode_bacnet_object_id(&object_array[0], type, instance);
            ct_test(pTest, memcmp(&object_array[0], &encoded_array[0],
                    sizeof(object_array)) == 0);
        }
    }

    return;
}

void testBACDCodeMaxSegsApdu(
    Test * pTest)
{
    int max_segs[8] = { 0, 2, 4, 8, 16, 32, 64, 65 };
    int max_apdu[6] = { 50, 128, 206, 480, 1024, 1476 };
    int i = 0;
    int j = 0;
    uint8_t octet = 0;

    /* test */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 6; j++) {
            octet = encode_max_segs_max_apdu(max_segs[i], max_apdu[j]);
            ct_test(pTest, max_segs[i] == decode_max_segs(octet));
            ct_test(pTest, max_apdu[j] == decode_max_apdu(octet));
        }
    }
}

void testBACDCodeBitString(
    Test * pTest)
{
    uint8_t bit = 0;
    BACNET_BIT_STRING bit_string;
    BACNET_BIT_STRING decoded_bit_string;
    uint8_t apdu[MAX_APDU] = { 0 };
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    int len = 0;

    bitstring_init(&bit_string);
    /* verify initialization */
    ct_test(pTest, bitstring_bits_used(&bit_string) == 0);
    for (bit = 0; bit < (MAX_BITSTRING_BYTES * 8); bit++) {
        ct_test(pTest, bitstring_bit(&bit_string, bit) == false);
    }
    /* test encode/decode -- true */
    for (bit = 0; bit < (MAX_BITSTRING_BYTES * 8); bit++) {
        bitstring_set_bit(&bit_string, bit, true);
        ct_test(pTest, bitstring_bits_used(&bit_string) == (bit + 1));
        ct_test(pTest, bitstring_bit(&bit_string, bit) == true);
        /* encode */
        len = encode_application_bitstring(&apdu[0], &bit_string);
        /* decode */
        len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
        ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_BIT_STRING);
        len += decode_bitstring(&apdu[len], len_value, &decoded_bit_string);
        ct_test(pTest, bitstring_bits_used(&decoded_bit_string) == (bit + 1));
        ct_test(pTest, bitstring_bit(&decoded_bit_string, bit) == true);
    }
    /* test encode/decode -- false */
    bitstring_init(&bit_string);
    for (bit = 0; bit < (MAX_BITSTRING_BYTES * 8); bit++) {
        bitstring_set_bit(&bit_string, bit, false);
        ct_test(pTest, bitstring_bits_used(&bit_string) == (bit + 1));
        ct_test(pTest, bitstring_bit(&bit_string, bit) == false);
        /* encode */
        len = encode_application_bitstring(&apdu[0], &bit_string);
        /* decode */
        len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
        ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_BIT_STRING);
        len += decode_bitstring(&apdu[len], len_value, &decoded_bit_string);
        ct_test(pTest, bitstring_bits_used(&decoded_bit_string) == (bit + 1));
        ct_test(pTest, bitstring_bit(&decoded_bit_string, bit) == false);
    }
}

void testUnsignedContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    /* 32 bit number */
    uint32_t in = 0xdeadbeef;
    uint32_t out;

    outLen2 = decode_context_unsigned(apdu, 9, &out);

    in = 0xdeadbeef;
    inLen = encode_context_unsigned(apdu, 10, in);
    outLen = decode_context_unsigned(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    inLen = encode_context_unsigned(apdu, large_context_tag, in);
    outLen = decode_context_unsigned(apdu, large_context_tag, &out);
    outLen2 = decode_context_unsigned(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    /* 16 bit number */
    in = 0xdead;
    inLen = encode_context_unsigned(apdu, 10, in);
    outLen = decode_context_unsigned(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_unsigned(apdu, large_context_tag, in);
    outLen = decode_context_unsigned(apdu, large_context_tag, &out);
    outLen2 = decode_context_unsigned(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    /* 8 bit number */
    in = 0xde;
    inLen = encode_context_unsigned(apdu, 10, in);
    outLen = decode_context_unsigned(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_unsigned(apdu, large_context_tag, in);
    outLen = decode_context_unsigned(apdu, large_context_tag, &out);
    outLen2 = decode_context_unsigned(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    /* 4 bit number */
    in = 0xd;
    inLen = encode_context_unsigned(apdu, 10, in);
    outLen = decode_context_unsigned(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_unsigned(apdu, large_context_tag, in);
    outLen = decode_context_unsigned(apdu, large_context_tag, &out);
    outLen2 = decode_context_unsigned(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    /* 2 bit number */
    in = 0x2;
    inLen = encode_context_unsigned(apdu, 10, in);
    outLen = decode_context_unsigned(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_unsigned(apdu, large_context_tag, in);
    outLen = decode_context_unsigned(apdu, large_context_tag, &out);
    outLen2 = decode_context_unsigned(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

}

void testSignedContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;


    /* 32 bit number */
    int32_t in = 0xdeadbeef;
    int32_t out;

    outLen2 = decode_context_signed(apdu, 9, &out);

    in = 0xdeadbeef;
    inLen = encode_context_signed(apdu, 10, in);
    outLen = decode_context_signed(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    inLen = encode_context_signed(apdu, large_context_tag, in);
    outLen = decode_context_signed(apdu, large_context_tag, &out);
    outLen2 = decode_context_signed(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    /* 16 bit number */
    in = 0xdead;
    inLen = encode_context_signed(apdu, 10, in);
    outLen = decode_context_signed(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_signed(apdu, large_context_tag, in);
    outLen = decode_context_signed(apdu, large_context_tag, &out);
    outLen2 = decode_context_signed(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    /* 8 bit number */
    in = 0xde;
    inLen = encode_context_signed(apdu, 10, in);
    outLen = decode_context_signed(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_signed(apdu, large_context_tag, in);
    outLen = decode_context_signed(apdu, large_context_tag, &out);
    outLen2 = decode_context_signed(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    /* 4 bit number */
    in = 0xd;
    inLen = encode_context_signed(apdu, 10, in);
    outLen = decode_context_signed(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_signed(apdu, large_context_tag, in);
    outLen = decode_context_signed(apdu, large_context_tag, &out);
    outLen2 = decode_context_signed(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    /* 2 bit number */
    in = 0x2;
    inLen = encode_context_signed(apdu, 10, in);
    outLen = decode_context_signed(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_signed(apdu, large_context_tag, in);
    outLen = decode_context_signed(apdu, large_context_tag, &out);
    outLen2 = decode_context_signed(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

}

void testEnumeratedContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    /* 32 bit number */
    uint32_t in = 0xdeadbeef;
    uint32_t out;

    outLen2 = decode_context_enumerated(apdu, 9, &out);

    in = 0xdeadbeef;
    inLen = encode_context_enumerated(apdu, 10, in);
    outLen = decode_context_enumerated(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    inLen = encode_context_enumerated(apdu, large_context_tag, in);
    outLen = decode_context_enumerated(apdu, large_context_tag, &out);
    outLen2 = decode_context_enumerated(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    /* 16 bit number */
    in = 0xdead;
    inLen = encode_context_enumerated(apdu, 10, in);
    outLen = decode_context_enumerated(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_enumerated(apdu, large_context_tag, in);
    outLen = decode_context_enumerated(apdu, large_context_tag, &out);
    outLen2 = decode_context_enumerated(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    /* 8 bit number */
    in = 0xde;
    inLen = encode_context_enumerated(apdu, 10, in);
    outLen = decode_context_enumerated(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_enumerated(apdu, large_context_tag, in);
    outLen = decode_context_enumerated(apdu, large_context_tag, &out);
    outLen2 = decode_context_enumerated(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    /* 4 bit number */
    in = 0xd;
    inLen = encode_context_enumerated(apdu, 10, in);
    outLen = decode_context_enumerated(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_enumerated(apdu, large_context_tag, in);
    outLen = decode_context_enumerated(apdu, large_context_tag, &out);
    outLen2 = decode_context_enumerated(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    /* 2 bit number */
    in = 0x2;
    inLen = encode_context_enumerated(apdu, 10, in);
    outLen = decode_context_enumerated(apdu, 10, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_enumerated(apdu, large_context_tag, in);
    outLen = decode_context_enumerated(apdu, large_context_tag, &out);
    outLen2 = decode_context_enumerated(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
}

void testFloatContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    /* 32 bit number */
    float in;
    float out;


    in = 0.1234f;
    inLen = encode_context_real(apdu, 10, in);
    outLen = decode_context_real(apdu, 10, &out);
    outLen2 = decode_context_real(apdu, 9, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    inLen = encode_context_real(apdu, large_context_tag, in);
    outLen = decode_context_real(apdu, large_context_tag, &out);
    outLen2 = decode_context_real(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    in = 0.0f;
    inLen = encode_context_real(apdu, 10, in);
    outLen = decode_context_real(apdu, 10, &out);
    outLen2 = decode_context_real(apdu, 9, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_real(apdu, large_context_tag, in);
    outLen = decode_context_real(apdu, large_context_tag, &out);
    outLen2 = decode_context_real(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
}

void testDoubleContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    /* 64 bit number */
    double in;
    double out;


    in = 0.1234;
    inLen = encode_context_double(apdu, 10, in);
    outLen = decode_context_double(apdu, 10, &out);
    outLen2 = decode_context_double(apdu, 9, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    inLen = encode_context_double(apdu, large_context_tag, in);
    outLen = decode_context_double(apdu, large_context_tag, &out);
    outLen2 = decode_context_double(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    in = 0.0;
    inLen = encode_context_double(apdu, 10, in);
    outLen = decode_context_double(apdu, 10, &out);
    outLen2 = decode_context_double(apdu, 9, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);

    inLen = encode_context_double(apdu, large_context_tag, in);
    outLen = decode_context_double(apdu, large_context_tag, &out);
    outLen2 = decode_context_double(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in == out);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
}

void testObjectIDContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    /* 32 bit number */
    uint16_t in_type;
    uint32_t in_id;

    uint16_t out_type;
    uint32_t out_id;

    in_type = 0xde;
    in_id = 0xbeef;

    inLen = encode_context_object_id(apdu, 10, in_type, in_id);
    outLen = decode_context_object_id(apdu, 10, &out_type, &out_id);
    outLen2 = decode_context_object_id(apdu, 9, &out_type, &out_id);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in_type == out_type);
    ct_test(pTest, in_id == out_id);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);

    inLen = encode_context_object_id(apdu, large_context_tag, in_type, in_id);
    outLen =
        decode_context_object_id(apdu, large_context_tag, &out_type, &out_id);
    outLen2 =
        decode_context_object_id(apdu, large_context_tag - 1, &out_type,
        &out_id);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in_type == out_type);
    ct_test(pTest, in_id == out_id);
    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
}

void testCharacterStringContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;


    BACNET_CHARACTER_STRING in;
    BACNET_CHARACTER_STRING out;

    characterstring_init_ansi(&in, "This is a test");

    inLen = encode_context_character_string(apdu, 10, &in);
    outLen = decode_context_character_string(apdu, 10, &out);
    outLen2 = decode_context_character_string(apdu, 9, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.length == out.length);
    ct_test(pTest, in.encoding == out.encoding);
    ct_test(pTest, strcmp(in.value, out.value) == 0);

    inLen = encode_context_character_string(apdu, large_context_tag, &in);
    outLen = decode_context_character_string(apdu, large_context_tag, &out);
    outLen2 =
        decode_context_character_string(apdu, large_context_tag - 1, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.length == out.length);
    ct_test(pTest, in.encoding == out.encoding);
    ct_test(pTest, strcmp(in.value, out.value) == 0);
}

void testBitStringContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    BACNET_BIT_STRING in;
    BACNET_BIT_STRING out;

    bitstring_init(&in);
    bitstring_set_bit(&in, 1, true);
    bitstring_set_bit(&in, 3, true);
    bitstring_set_bit(&in, 6, true);
    bitstring_set_bit(&in, 10, false);
    bitstring_set_bit(&in, 11, true);
    bitstring_set_bit(&in, 12, false);

    inLen = encode_context_bitstring(apdu, 10, &in);
    outLen = decode_context_bitstring(apdu, 10, &out);
    outLen2 = decode_context_bitstring(apdu, 9, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.bits_used == out.bits_used);
    ct_test(pTest, memcmp(in.value, out.value, MAX_BITSTRING_BYTES) == 0);

    inLen = encode_context_bitstring(apdu, large_context_tag, &in);
    outLen = decode_context_bitstring(apdu, large_context_tag, &out);
    outLen2 = decode_context_bitstring(apdu, large_context_tag - 1, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.bits_used == out.bits_used);
    ct_test(pTest, memcmp(in.value, out.value, MAX_BITSTRING_BYTES) == 0);
}

void testOctetStringContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;


    BACNET_OCTET_STRING in;
    BACNET_OCTET_STRING out;

    uint8_t initData[] = { 0xde, 0xad, 0xbe, 0xef };

    octetstring_init(&in, initData, sizeof(initData));

    inLen = encode_context_octet_string(apdu, 10, &in);
    outLen = decode_context_octet_string(apdu, 10, &out);
    outLen2 = decode_context_octet_string(apdu, 9, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.length == out.length);
    ct_test(pTest, octetstring_value_same(&in, &out));

    inLen = encode_context_octet_string(apdu, large_context_tag, &in);
    outLen = decode_context_octet_string(apdu, large_context_tag, &out);
    outLen2 = decode_context_octet_string(apdu, large_context_tag - 1, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.length == out.length);
    ct_test(pTest, octetstring_value_same(&in, &out));

}

void testTimeContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;

    BACNET_TIME in;
    BACNET_TIME out;

    in.hour = 10;
    in.hundredths = 20;
    in.min = 30;
    in.sec = 40;

    inLen = encode_context_time(apdu, 10, &in);
    outLen = decode_context_bacnet_time(apdu, 10, &out);
    outLen2 = decode_context_bacnet_time(apdu, 9, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.hour == out.hour);
    ct_test(pTest, in.hundredths == out.hundredths);
    ct_test(pTest, in.min == out.min);
    ct_test(pTest, in.sec == out.sec);

    inLen = encode_context_time(apdu, large_context_tag, &in);
    outLen = decode_context_bacnet_time(apdu, large_context_tag, &out);
    outLen2 = decode_context_bacnet_time(apdu, large_context_tag - 1, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.hour == out.hour);
    ct_test(pTest, in.hundredths == out.hundredths);
    ct_test(pTest, in.min == out.min);
    ct_test(pTest, in.sec == out.sec);


}

void testDateContextDecodes(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU];
    int inLen;
    int outLen;
    int outLen2;
    uint8_t large_context_tag = 0xfe;


    BACNET_DATE in;
    BACNET_DATE out;

    in.day = 3;
    in.month = 10;
    in.wday = 5;
    in.year = 1945;

    inLen = encode_context_date(apdu, 10, &in);
    outLen = decode_context_date(apdu, 10, &out);
    outLen2 = decode_context_date(apdu, 9, &out);

    ct_test(pTest, outLen2 == BACNET_STATUS_ERROR);
    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.day == out.day);
    ct_test(pTest, in.month == out.month);
    ct_test(pTest, in.wday == out.wday);
    ct_test(pTest, in.year == out.year);

    /* Test large tags */
    inLen = encode_context_date(apdu, large_context_tag, &in);
    outLen = decode_context_date(apdu, large_context_tag, &out);
    outLen2 = decode_context_date(apdu, large_context_tag - 1, &out);

    ct_test(pTest, inLen == outLen);
    ct_test(pTest, in.day == out.day);
    ct_test(pTest, in.month == out.month);
    ct_test(pTest, in.wday == out.wday);
    ct_test(pTest, in.year == out.year);
}


#ifdef TEST_DECODE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACDCode", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBACDCodeTags);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeReal);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeUnsigned);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACnetUnsigned);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeSigned);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACnetSigned);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeEnumerated);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeOctetString);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeCharacterString);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeObject);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeMaxSegsApdu);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACDCodeBitString);
    assert(rc);

    rc = ct_addTestFunction(pTest, testUnsignedContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testSignedContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testEnumeratedContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testCharacterStringContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testFloatContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testDoubleContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testObjectIDContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testBitStringContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testTimeContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testDateContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testOctetStringContextDecodes);
    assert(rc);

    rc = ct_addTestFunction(pTest, testBACDCodeDouble);
    assert(rc);
    /* configure output */
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_DECODE */
#endif /* TEST */
