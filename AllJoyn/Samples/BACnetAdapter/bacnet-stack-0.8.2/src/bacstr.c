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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>     /* for strlen */
#include "config.h"
#include "bacstr.h"
#include "bits.h"
#if PRINT_ENABLED
#include <stdlib.h>     /* for strtol */
#include <ctype.h>      /* for isalnum */
#endif

/** @file bacstr.c  Manipulate Bit/Char/Octet Strings */

void bitstring_init(
    BACNET_BIT_STRING * bit_string)
{
    int i;

    bit_string->bits_used = 0;
    for (i = 0; i < MAX_BITSTRING_BYTES; i++) {
        bit_string->value[i] = 0;
    }
}

void bitstring_set_bit(
    BACNET_BIT_STRING * bit_string,
    uint8_t bit_number,
    bool value)
{
    uint8_t byte_number = bit_number / 8;
    uint8_t bit_mask = 1;

    if (byte_number < MAX_BITSTRING_BYTES) {
        /* set max bits used */
        if (bit_string->bits_used < (bit_number + 1)) {
            bit_string->bits_used = bit_number + 1;
        }
        bit_mask = bit_mask << (bit_number - (byte_number * 8));
        if (value) {
            bit_string->value[byte_number] |= bit_mask;
        } else {
            bit_string->value[byte_number] &= (~(bit_mask));
        }
    }
}

bool bitstring_bit(
    BACNET_BIT_STRING * bit_string,
    uint8_t bit_number)
{
    bool value = false;
    uint8_t byte_number = bit_number / 8;
    uint8_t bit_mask = 1;

    if (bit_number < (MAX_BITSTRING_BYTES * 8)) {
        bit_mask = bit_mask << (bit_number - (byte_number * 8));
        if (bit_string->value[byte_number] & bit_mask) {
            value = true;
        }
    }

    return value;
}

uint8_t bitstring_bits_used(
    BACNET_BIT_STRING * bit_string)
{
    return bit_string->bits_used;
}

/* returns the number of bytes that a bit string is using */
uint8_t bitstring_bytes_used(
    BACNET_BIT_STRING * bit_string)
{
    uint8_t len = 0;    /* return value */
    uint8_t used_bytes = 0;
    uint8_t last_bit = 0;

    if (bit_string->bits_used) {
        last_bit = bit_string->bits_used - 1;
        used_bytes = last_bit / 8;
        /* add one for the first byte */
        used_bytes++;
        len = used_bytes;
    }

    return len;
}

uint8_t bitstring_octet(
    BACNET_BIT_STRING * bit_string,
    uint8_t octet_index)
{
    uint8_t octet = 0;

    if (bit_string) {
        if (octet_index < MAX_BITSTRING_BYTES) {
            octet = bit_string->value[octet_index];
        }
    }

    return octet;
}

bool bitstring_set_octet(
    BACNET_BIT_STRING * bit_string,
    uint8_t index,
    uint8_t octet)
{
    bool status = false;

    if (bit_string) {
        if (index < MAX_BITSTRING_BYTES) {
            bit_string->value[index] = octet;
            status = true;
        }
    }

    return status;
}

bool bitstring_set_bits_used(
    BACNET_BIT_STRING * bit_string,
    uint8_t bytes_used,
    uint8_t unused_bits)
{
    bool status = false;

    if (bit_string) {
        /* FIXME: check that bytes_used is at least one? */
        bit_string->bits_used = bytes_used * 8;
        bit_string->bits_used -= unused_bits;
        status = true;
    }

    return status;
}

uint8_t bitstring_bits_capacity(
    BACNET_BIT_STRING * bit_string)
{
    if (bit_string) {
        return (MAX_BITSTRING_BYTES * 8);
    } else {
        return 0;
    }
}

bool bitstring_copy(
    BACNET_BIT_STRING * dest,
    BACNET_BIT_STRING * src)
{
    unsigned i;
    bool status = false;

    if (dest && src) {
        dest->bits_used = src->bits_used;
        for (i = 0; i < MAX_BITSTRING_BYTES; i++) {
            dest->value[i] = src->value[i];
        }
        status = true;
    }

    return status;
}

/* returns true if the same length and contents */
bool bitstring_same(
    BACNET_BIT_STRING * bitstring1,
    BACNET_BIT_STRING * bitstring2)
{
    int i = 0;  /* loop counter */
    int bytes_used = 0;
    uint8_t compare_mask = 0;

    if (bitstring1 && bitstring2) {
        if ((bitstring1->bits_used == bitstring2->bits_used) &&
            (bitstring1->bits_used / 8 <= MAX_BITSTRING_BYTES)) {
            bytes_used = (int) (bitstring1->bits_used / 8);
            compare_mask = 0xFF >> (8 - (bitstring1->bits_used % 8));

            for (i = 0; i < bytes_used; i++) {
                if (bitstring1->value[i] != bitstring2->value[i]) {
                    return false;
                }
            }
            if ((bitstring1->value[bytes_used] & compare_mask) !=
                (bitstring2->value[bytes_used] & compare_mask)) {
                return false;
            } else {
                return true;
            }
        }
    }

    return false;
}

#if PRINT_ENABLED
/* converts an null terminated ASCII string to an bitstring.
   Expects "1,0,1,0,1,1" or "101011" as the bits
   returns true if successfully converted and fits; false if too long */
bool bitstring_init_ascii(
    BACNET_BIT_STRING * bit_string,
    const char *ascii)
{
    bool status = false;        /* return value */
    unsigned index = 0; /* offset into buffer */
    uint8_t bit_number = 0;

    if (bit_string) {
        bitstring_init(bit_string);
        if (ascii[0] == 0) {
            /* nothing to decode, so success! */
            status = true;
        } else {
            while (ascii[index] != 0) {
                if (bit_number > bitstring_bits_capacity(bit_string)) {
                    /* too long of a string */
                    status = false;
                    break;
                }
                if (ascii[index] == '1') {
                    bitstring_set_bit(bit_string, bit_number, true);
                    bit_number++;
                    status = true;
                } else if (ascii[index] == '0') {
                    bitstring_set_bit(bit_string, bit_number, false);
                    bit_number++;
                    status = true;
                } else {
                    /* skip non-numeric or alpha */
                    index++;
                    continue;
                }
                /* next character */
                index++;
            }
        }
    }

    return status;
}
#endif

#define CHARACTER_STRING_CAPACITY (MAX_CHARACTER_STRING_BYTES - 1)
/* returns false if the string exceeds capacity
   initialize by using value=NULL */
bool characterstring_init(
    BACNET_CHARACTER_STRING * char_string,
    uint8_t encoding,
    const char *value,
    size_t length)
{
    bool status = false;        /* return value */
    size_t i;   /* counter */

    if (char_string) {
        char_string->length = 0;
        char_string->encoding = encoding;
        /* save a byte at the end for NULL -
           note: assumes printable characters */
        if (length <= CHARACTER_STRING_CAPACITY) {
            if (value) {
                for (i = 0; i < MAX_CHARACTER_STRING_BYTES; i++) {
                    if (i < length) {
                        char_string->value[char_string->length] = value[i];
                        char_string->length++;
                    } else {
                        char_string->value[i] = 0;
                    }
                }
            } else {
                for (i = 0; i < MAX_CHARACTER_STRING_BYTES; i++) {
                    char_string->value[i] = 0;
                }
            }
            status = true;
        }
    }

    return status;
}

bool characterstring_init_ansi(
    BACNET_CHARACTER_STRING * char_string,
    const char *value)
{
    return characterstring_init(char_string, CHARACTER_ANSI_X34, value,
        value ? strlen(value) : 0);
}

bool characterstring_copy(
    BACNET_CHARACTER_STRING * dest,
    BACNET_CHARACTER_STRING * src)
{
    return characterstring_init(dest, characterstring_encoding(src),
        characterstring_value(src), characterstring_length(src));
}

bool characterstring_ansi_copy(
    char *dest,
    size_t dest_max_len,
    BACNET_CHARACTER_STRING * src)
{
    size_t i;   /* counter */

    if (dest && src && (src->encoding == CHARACTER_ANSI_X34) &&
        (src->length < dest_max_len)) {
        for (i = 0; i < dest_max_len; i++) {
            if (i < src->length) {
                dest[i] = src->value[i];
            } else {
                dest[i] = 0;
            }
        }
        return true;
    }

    return false;
}

/* returns true if the character encoding and string contents are the same */
bool characterstring_same(
    BACNET_CHARACTER_STRING * dest,
    BACNET_CHARACTER_STRING * src)
{
    size_t i;   /* counter */
    bool same_status = false;

    if (src && dest) {
        if ((src->length == dest->length) && (src->encoding == dest->encoding)) {
            same_status = true;
            for (i = 0; (i < src->length) && same_status; i++) {
                if (src->value[i] != dest->value[i]) {
                    same_status = false;
                }
            }
        }
    } else if (src) {
        if (src->length == 0) {
            same_status = true;
        }
    } else if (dest) {
        if (dest->length == 0) {
            same_status = true;
        }
    }

    return same_status;
}

bool characterstring_ansi_same(
    BACNET_CHARACTER_STRING * dest,
    const char *src)
{
    size_t i;   /* counter */
    bool same_status = false;

    if (src && dest) {
        if ((dest->length == strlen(src)) &&
            (dest->encoding == CHARACTER_ANSI_X34)) {
            same_status = true;
            for (i = 0; (i < dest->length) && same_status; i++) {
                if (src[i] != dest->value[i]) {
                    same_status = false;
                }
            }
        }
    }
    /* NULL matches an empty string in our world */
    else if (src) {
        if (strlen(src) == 0) {
            same_status = true;
        }
    } else if (dest) {
        if (dest->length == 0) {
            same_status = true;
        }
    }

    return same_status;
}

/* returns false if the string exceeds capacity */
bool characterstring_append(
    BACNET_CHARACTER_STRING * char_string,
    const char *value,
    size_t length)
{
    size_t i;   /* counter */
    bool status = false;        /* return value */

    if (char_string) {
        if ((length + char_string->length) <= CHARACTER_STRING_CAPACITY) {
            for (i = 0; i < length; i++) {
                char_string->value[char_string->length] = value[i];
                char_string->length++;
            }
            status = true;
        }
    }

    return status;
}

/* This function sets a new length without changing the value.
   If length exceeds capacity, no modification happens and
   function returns false.  */
bool characterstring_truncate(
    BACNET_CHARACTER_STRING * char_string,
    size_t length)
{
    bool status = false;        /* return value */

    if (char_string) {
        if (length <= CHARACTER_STRING_CAPACITY) {
            char_string->length = length;
            status = true;
        }
    }

    return status;
}

/* Returns the value. */
char *characterstring_value(
    BACNET_CHARACTER_STRING * char_string)
{
    char *value = NULL;

    if (char_string) {
        value = char_string->value;
    }

    return value;
}

/* returns the length. */
size_t characterstring_length(
    BACNET_CHARACTER_STRING * char_string)
{
    size_t length = 0;

    if (char_string) {
        /* FIXME: validate length is within bounds? */
        length = char_string->length;
    }

    return length;
}

size_t characterstring_capacity(
    BACNET_CHARACTER_STRING * char_string)
{
    size_t length = 0;

    if (char_string) {
        length = CHARACTER_STRING_CAPACITY;
    }

    return length;
}

/* returns the encoding. */
uint8_t characterstring_encoding(
    BACNET_CHARACTER_STRING * char_string)
{
    uint8_t encoding = 0;

    if (char_string) {
        encoding = char_string->encoding;
    }

    return encoding;
}

/* returns the encoding. */
bool characterstring_set_encoding(
    BACNET_CHARACTER_STRING * char_string,
    uint8_t encoding)
{
    bool status = false;

    if (char_string) {
        char_string->encoding = encoding;
        status = true;
    }

    return status;
}

/* returns true if string is printable */
/* used to assist in the requirement that
   "The set of characters used in the Object_Name shall be
   restricted to printable characters." */
/* printable character: a character that represents a printable
symbol as opposed to a device control character. These
include, but are not limited to, upper- and lowercase letters,
punctuation marks, and mathematical symbols. The exact set
depends upon the character set being used. In ANSI X3.4 the
printable characters are represented by single octets in the range
X'20' - X'7E'.*/
bool characterstring_printable(
    BACNET_CHARACTER_STRING * char_string)
{
    bool status = false;        /* return value */
    size_t i;   /* counter */

    if (char_string) {
        if (char_string->encoding == CHARACTER_ANSI_X34) {
            status = true;
            for (i = 0; i < MAX_CHARACTER_STRING_BYTES; i++) {
                if (i < char_string->length) {
                    if ((char_string->value[i] < 0x20) ||
                        (char_string->value[i] > 0x7E)) {
                        status = false;
                    }
                } else {
                    break;
                }
            }
        } else {
            status = true;
        }
    }

    return status;
}

/* Basic UTF-8 manipulation routines
  by Jeff Bezanson
  placed in the public domain Fall 2005 */
static const char trailingBytesForUTF8[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4,
    4, 4, 4, 5, 5, 5, 5
};

/* based on the valid_utf8 routine from the PCRE library by Philip Hazel
   length is in bytes, since without knowing whether the string is valid
   it's hard to know how many characters there are! */
bool utf8_isvalid(
    const char *str,
    size_t length)
{
    const unsigned char *p, *pend = (unsigned char *) str + length;
    unsigned char c;
    size_t ab;

    /* empty string is valid */
    if (length == 0) {
        return true;
    }
    for (p = (const unsigned char *) str; p < pend; p++) {
        c = *p;
        /* null in middle of string */
        if (c == 0) {
            return false;
        }
        /* ASCII character */
        if (c < 128) {
            continue;
        }
        if ((c & 0xc0) != 0xc0) {
            return false;
        }
        ab = (size_t) trailingBytesForUTF8[c];
        if (length < ab) {
            return false;
        }
        length -= ab;

        p++;
        /* Check top bits in the second byte */
        if ((*p & 0xc0) != 0x80) {
            return false;
        }
        /* Check for overlong sequences for each different length */
        switch (ab) {
                /* Check for xx00 000x */
            case 1:
                if ((c & 0x3e) == 0)
                    return false;
                continue;       /* We know there aren't any more bytes to check */

                /* Check for 1110 0000, xx0x xxxx */
            case 2:
                if (c == 0xe0 && (*p & 0x20) == 0)
                    return false;
                break;

                /* Check for 1111 0000, xx00 xxxx */
            case 3:
                if (c == 0xf0 && (*p & 0x30) == 0)
                    return false;
                break;

                /* Check for 1111 1000, xx00 0xxx */
            case 4:
                if (c == 0xf8 && (*p & 0x38) == 0)
                    return false;
                break;

                /* Check for leading 0xfe or 0xff,
                   and then for 1111 1100, xx00 00xx */
            case 5:
                if (c == 0xfe || c == 0xff || (c == 0xfc && (*p & 0x3c) == 0))
                    return false;
                break;
        }

        /* Check for valid bytes after the 2nd, if any; all must start 10 */
        while (--ab > 0) {
            if ((*(++p) & 0xc0) != 0x80)
                return false;
        }
    }

    return true;
}

bool characterstring_valid(
    BACNET_CHARACTER_STRING * char_string)
{
    bool valid = false; /* return value */

    if (char_string->encoding < MAX_CHARACTER_STRING_ENCODING) {
        if (char_string->encoding == CHARACTER_UTF8) {
            if (utf8_isvalid(char_string->value, char_string->length)) {
                valid = true;
            }
        } else {
            valid = true;
        }
    }

    return valid;
}

#if BACNET_USE_OCTETSTRING
/* returns false if the string exceeds capacity
   initialize by using value=NULL */
bool octetstring_init(
    BACNET_OCTET_STRING * octet_string,
    uint8_t * value,
    size_t length)
{
    bool status = false;        /* return value */
    size_t i;   /* counter */

    if (octet_string && (length <= MAX_OCTET_STRING_BYTES)) {
        octet_string->length = 0;
        if (value) {
            for (i = 0; i < MAX_OCTET_STRING_BYTES; i++) {
                if (i < length) {
                    octet_string->value[i] = value[i];
                } else {
                    octet_string->value[i] = 0;
                }
            }
            octet_string->length = length;
        } else {
            for (i = 0; i < MAX_OCTET_STRING_BYTES; i++) {
                octet_string->value[i] = 0;
            }
        }
        status = true;
    }

    return status;
}

#if PRINT_ENABLED
/* converts an null terminated ASCII Hex string to an octet string.
   returns true if successfully converted and fits; false if too long */
bool octetstring_init_ascii_hex(
    BACNET_OCTET_STRING * octet_string,
    const char *ascii_hex)
{
    bool status = false;        /* return value */
    unsigned index = 0; /* offset into buffer */
    uint8_t value = 0;
    char hex_pair_string[3] = "";

    if (octet_string) {
        octet_string->length = 0;
        if (ascii_hex[0] == 0) {
            /* nothing to decode, so success! */
            status = true;
        } else {
            while (ascii_hex[index] != 0) {
                if (!isalnum(ascii_hex[index])) {
                    /* skip non-numeric or alpha */
                    index++;
                    continue;
                }
                if (ascii_hex[index + 1] == 0) {
                    break;
                }
                hex_pair_string[0] = ascii_hex[index];
                hex_pair_string[1] = ascii_hex[index + 1];
                value = (uint8_t) strtol(hex_pair_string, NULL, 16);
                if (octet_string->length <= MAX_OCTET_STRING_BYTES) {
                    octet_string->value[octet_string->length] = value;
                    octet_string->length++;
                    /* at least one pair was decoded */
                    status = true;
                } else {
                    break;
                    status = false;
                }
                /* set up for next pair */
                index += 2;
            }
        }
    }

    return status;
}
#endif

bool octetstring_copy(
    BACNET_OCTET_STRING * dest,
    BACNET_OCTET_STRING * src)
{
    return octetstring_init(dest, octetstring_value(src),
        octetstring_length(src));
}

/* returns the number of bytes copied, or 0 if the dest
   cannot hold entire octetstring value */
size_t octetstring_copy_value(
    uint8_t * dest,
    size_t length,
    BACNET_OCTET_STRING * src)
{
    size_t bytes_copied = 0;
    size_t i;   /* counter */

    if (src && dest) {
        if (length <= src->length) {
            for (i = 0; i < src->length; i++) {
                dest[i] = src->value[i];
            }
            bytes_copied = src->length;
        }
    }

    return bytes_copied;
}

/* returns false if the string exceeds capacity */
bool octetstring_append(
    BACNET_OCTET_STRING * octet_string,
    uint8_t * value,
    size_t length)
{
    size_t i;   /* counter */
    bool status = false;        /* return value */

    if (octet_string) {
        if ((length + octet_string->length) <= MAX_OCTET_STRING_BYTES) {
            for (i = 0; i < length; i++) {
                octet_string->value[octet_string->length] = value[i];
                octet_string->length++;
            }
            status = true;
        }
    }

    return status;
}

/* This function sets a new length without changing the value.
   If length exceeds capacity, no modification happens and
   function returns false.  */
bool octetstring_truncate(
    BACNET_OCTET_STRING * octet_string,
    size_t length)
{
    bool status = false;        /* return value */

    if (octet_string) {
        if (length <= MAX_OCTET_STRING_BYTES) {
            octet_string->length = length;
            status = true;
        }
    }

    return status;
}

/* returns a pointer to the value. */
uint8_t *octetstring_value(
    BACNET_OCTET_STRING * octet_string)
{
    uint8_t *value = NULL;

    if (octet_string) {
        value = octet_string->value;
    }

    return value;
}

/* returns the length. */
size_t octetstring_length(
    BACNET_OCTET_STRING * octet_string)
{
    size_t length = 0;

    if (octet_string) {
        /* FIXME: validate length is within bounds? */
        length = octet_string->length;
    }

    return length;
}

/* returns the maximum capacity. */
size_t octetstring_capacity(
    BACNET_OCTET_STRING * octet_string)
{
    size_t length = 0;

    if (octet_string) {
        /* FIXME: validate length is within bounds? */
        length = MAX_OCTET_STRING_BYTES;
    }

    return length;
}

/* returns true if the same length and contents */
bool octetstring_value_same(
    BACNET_OCTET_STRING * octet_string1,
    BACNET_OCTET_STRING * octet_string2)
{
    size_t i = 0;       /* loop counter */

    if (octet_string1 && octet_string2) {
        if ((octet_string1->length == octet_string2->length) &&
            (octet_string1->length <= MAX_OCTET_STRING_BYTES)) {
            for (i = 0; i < octet_string1->length; i++) {
                if (octet_string1->value[i] != octet_string2->value[i]) {
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}
#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ctest.h"

void testBitString(
    Test * pTest)
{
    uint8_t bit = 0;
    int max_bit;
    BACNET_BIT_STRING bit_string;
    BACNET_BIT_STRING bit_string2;
    BACNET_BIT_STRING bit_string3;

    bitstring_init(&bit_string);
    /* verify initialization */
    ct_test(pTest, bitstring_bits_used(&bit_string) == 0);
    for (bit = 0; bit < (MAX_BITSTRING_BYTES * 8); bit++) {
        ct_test(pTest, bitstring_bit(&bit_string, bit) == false);
    }

    /* test for true */
    for (bit = 0; bit < (MAX_BITSTRING_BYTES * 8); bit++) {
        bitstring_set_bit(&bit_string, bit, true);
        ct_test(pTest, bitstring_bits_used(&bit_string) == (bit + 1));
        ct_test(pTest, bitstring_bit(&bit_string, bit) == true);
    }
    /* test for false */
    bitstring_init(&bit_string);
    for (bit = 0; bit < (MAX_BITSTRING_BYTES * 8); bit++) {
        bitstring_set_bit(&bit_string, bit, false);
        ct_test(pTest, bitstring_bits_used(&bit_string) == (bit + 1));
        ct_test(pTest, bitstring_bit(&bit_string, bit) == false);
    }

    /* test for compare equals */
    srand(time(NULL));
    for (max_bit = 0; max_bit < (MAX_BITSTRING_BYTES * 8); max_bit++) {
        bitstring_init(&bit_string);
        bitstring_init(&bit_string2);
        for (bit = 0; bit < max_bit; bit++) {
            bool bit_value = rand() % 2;
            bitstring_set_bit(&bit_string, bit, bit_value);
            bitstring_set_bit(&bit_string2, bit, bit_value);
        }
        ct_test(pTest, bitstring_same(&bit_string, &bit_string2));
    }
    /* test for compare not equals */
    for (max_bit = 1; max_bit < (MAX_BITSTRING_BYTES * 8); max_bit++) {
        bitstring_init(&bit_string);
        bitstring_init(&bit_string2);
        bitstring_init(&bit_string3);
        for (bit = 0; bit < max_bit; bit++) {
            bool bit_value = rand() % 2;
            bitstring_set_bit(&bit_string, bit, bit_value);
            bitstring_set_bit(&bit_string2, bit, bit_value);
            bitstring_set_bit(&bit_string3, bit, bit_value);
        }
        /* Set the first bit of bit_string2 and the last bit of bit_string3 to be different */
        bitstring_set_bit(&bit_string2, 0, !bitstring_bit(&bit_string, 0));
        bitstring_set_bit(&bit_string3, max_bit - 1,
            !bitstring_bit(&bit_string, max_bit - 1));
        ct_test(pTest, !bitstring_same(&bit_string, &bit_string2));
        ct_test(pTest, !bitstring_same(&bit_string, &bit_string3));
    }
}

void testCharacterString(
    Test * pTest)
{
    BACNET_CHARACTER_STRING bacnet_string;
    char *value = "Joshua,Mary,Anna,Christopher";
    char test_value[MAX_APDU] = "Patricia";
    char test_append_value[MAX_APDU] = " and the Kids";
    char test_append_string[MAX_APDU] = "";
    bool status = false;
    size_t length = 0;
    size_t test_length = 0;
    size_t i = 0;

    /* verify initialization */
    status = characterstring_init(&bacnet_string, CHARACTER_ANSI_X34, NULL, 0);
    ct_test(pTest, status == true);
    ct_test(pTest, characterstring_length(&bacnet_string) == 0);
    ct_test(pTest,
        characterstring_encoding(&bacnet_string) == CHARACTER_ANSI_X34);
    /* bounds check */
    status =
        characterstring_init(&bacnet_string, CHARACTER_ANSI_X34, NULL,
        characterstring_capacity(&bacnet_string) + 1);
    ct_test(pTest, status == false);
    status =
        characterstring_truncate(&bacnet_string,
        characterstring_capacity(&bacnet_string) + 1);
    ct_test(pTest, status == false);
    status =
        characterstring_truncate(&bacnet_string,
        characterstring_capacity(&bacnet_string));
    ct_test(pTest, status == true);

    test_length = strlen(test_value);
    status =
        characterstring_init(&bacnet_string, CHARACTER_ANSI_X34,
        &test_value[0], test_length);
    ct_test(pTest, status == true);
    value = characterstring_value(&bacnet_string);
    length = characterstring_length(&bacnet_string);
    ct_test(pTest, length == test_length);
    for (i = 0; i < test_length; i++) {
        ct_test(pTest, value[i] == test_value[i]);
    }
    test_length = strlen(test_append_value);
    status =
        characterstring_append(&bacnet_string, &test_append_value[0],
        test_length);
    strcat(test_append_string, test_value);
    strcat(test_append_string, test_append_value);
    test_length = strlen(test_append_string);
    ct_test(pTest, status == true);
    length = characterstring_length(&bacnet_string);
    value = characterstring_value(&bacnet_string);
    ct_test(pTest, length == test_length);
    for (i = 0; i < test_length; i++) {
        ct_test(pTest, value[i] == test_append_string[i]);
    }
}

void testOctetString(
    Test * pTest)
{
    BACNET_OCTET_STRING bacnet_string;
    uint8_t *value = NULL;
    uint8_t test_value[MAX_APDU] = "Patricia";
    uint8_t test_append_value[MAX_APDU] = " and the Kids";
    uint8_t test_append_string[MAX_APDU] = "";
    bool status = false;
    size_t length = 0;
    size_t test_length = 0;
    size_t i = 0;

    /* verify initialization */
    status = octetstring_init(&bacnet_string, NULL, 0);
    ct_test(pTest, status == true);
    ct_test(pTest, octetstring_length(&bacnet_string) == 0);
    value = octetstring_value(&bacnet_string);
    for (i = 0; i < octetstring_capacity(&bacnet_string); i++) {
        ct_test(pTest, value[i] == 0);
    }
    /* bounds check */
    status =
        octetstring_init(&bacnet_string, NULL,
        octetstring_capacity(&bacnet_string) + 1);
    ct_test(pTest, status == false);
    status =
        octetstring_init(&bacnet_string, NULL,
        octetstring_capacity(&bacnet_string));
    ct_test(pTest, status == true);
    status =
        octetstring_truncate(&bacnet_string,
        octetstring_capacity(&bacnet_string) + 1);
    ct_test(pTest, status == false);
    status =
        octetstring_truncate(&bacnet_string,
        octetstring_capacity(&bacnet_string));
    ct_test(pTest, status == true);

    test_length = strlen((char *) test_value);
    status = octetstring_init(&bacnet_string, &test_value[0], test_length);
    ct_test(pTest, status == true);
    length = octetstring_length(&bacnet_string);
    value = octetstring_value(&bacnet_string);
    ct_test(pTest, length == test_length);
    for (i = 0; i < test_length; i++) {
        ct_test(pTest, value[i] == test_value[i]);
    }

    test_length = strlen((char *) test_append_value);
    status =
        octetstring_append(&bacnet_string, &test_append_value[0], test_length);
    strcat((char *) test_append_string, (char *) test_value);
    strcat((char *) test_append_string, (char *) test_append_value);
    test_length = strlen((char *) test_append_string);
    ct_test(pTest, status == true);
    length = octetstring_length(&bacnet_string);
    value = octetstring_value(&bacnet_string);
    ct_test(pTest, length == test_length);
    for (i = 0; i < test_length; i++) {
        ct_test(pTest, value[i] == test_append_string[i]);
    }
}

#ifdef TEST_BACSTR
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Strings", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBitString);
    assert(rc);
    rc = ct_addTestFunction(pTest, testCharacterString);
    assert(rc);
    rc = ct_addTestFunction(pTest, testOctetString);
    assert(rc);
    /* configure output */
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BACSTR */
#endif /* TEST */
