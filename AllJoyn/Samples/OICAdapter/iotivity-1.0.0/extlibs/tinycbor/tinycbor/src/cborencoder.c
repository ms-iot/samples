/****************************************************************************
**
** Copyright (C) 2015 Intel Corporation
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#define _BSD_SOURCE 1
#include "cbor.h"
#include "cborconstants_p.h"
#include "compilersupport_p.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "assert_p.h"       /* Always include last */

void cbor_encoder_init(CborEncoder *encoder, uint8_t *buffer, size_t size, int flags)
{
    encoder->ptr = buffer;
    encoder->end = buffer + size;
    encoder->added = 0;
    encoder->flags = flags;
}

static inline void put16(void *where, uint16_t v)
{
    v = cbor_htons(v);
    memcpy(where, &v, sizeof(v));
}

// Note: Since this is currently only used in situations where OOM is the only
// valid error, we KNOW this to be true.  Thus, this function now returns just 'true',
// but if in the future, any function starts returning a non-OOM error, this will need
// to be changed to the test.  At the moment, this is done to prevent more branches
// being created in the tinycbor output
static inline bool isOomError(CborError err)
{
    (void) err;
    return true;
}

static inline void put32(void *where, uint32_t v)
{
    v = cbor_htonl(v);
    memcpy(where, &v, sizeof(v));
}

static inline void put64(void *where, uint64_t v)
{
    v = cbor_htonll(v);
    memcpy(where, &v, sizeof(v));
}

static inline bool would_overflow(CborEncoder *encoder, size_t len)
{
    ptrdiff_t remaining = (ptrdiff_t)encoder->end;
    remaining -= remaining ? (ptrdiff_t)encoder->ptr : encoder->bytes_needed;
    remaining -= (ptrdiff_t)len;
    return unlikely(remaining < 0);
}

static inline void advance_ptr(CborEncoder *encoder, size_t n)
{
    if (encoder->end)
        encoder->ptr += n;
    else
        encoder->bytes_needed += n;
}

static inline CborError append_to_buffer(CborEncoder *encoder, const void *data, size_t len)
{
    if (would_overflow(encoder, len)) {
        if (encoder->end != NULL) {
            len -= encoder->end - encoder->ptr;
            encoder->end = NULL;
            encoder->bytes_needed = 0;
        }

        advance_ptr(encoder, len);
        return CborErrorOutOfMemory;
    }

    memcpy(encoder->ptr, data, len);
    encoder->ptr += len;
    return CborNoError;
}

static inline CborError append_byte_to_buffer(CborEncoder *encoder, uint8_t byte)
{
    return append_to_buffer(encoder, &byte, 1);
}

static inline CborError encode_number_no_update(CborEncoder *encoder, uint64_t ui, uint8_t shiftedMajorType)
{
    /* Little-endian would have been so much more convenient here:
     * We could just write at the beginning of buf but append_to_buffer
     * only the necessary bytes.
     * Since it has to be big endian, do it the other way around:
     * write from the end. */
    uint64_t buf[2];
    uint8_t *const bufend = (uint8_t *)buf + sizeof(buf);
    uint8_t *bufstart = bufend - 1;
    put64(buf + 1, ui);     // we probably have a bunch of zeros in the beginning

    if (ui < Value8Bit) {
        *bufstart += shiftedMajorType;
    } else {
        unsigned more = 0;
        if (ui > 0xffU)
            ++more;
        if (ui > 0xffffU)
            ++more;
        if (ui > 0xffffffffU)
            ++more;
        bufstart -= 1 << more;
        *bufstart = shiftedMajorType + Value8Bit + more;
    }

    return append_to_buffer(encoder, bufstart, bufend - bufstart);
}

static inline CborError encode_number(CborEncoder *encoder, uint64_t ui, uint8_t shiftedMajorType)
{
    ++encoder->added;
    return encode_number_no_update(encoder, ui, shiftedMajorType);
}


CborError cbor_encode_uint(CborEncoder *encoder, uint64_t value)
{
    return encode_number(encoder, value, UnsignedIntegerType << MajorTypeShift);
}

CborError cbor_encode_negative_int(CborEncoder *encoder, uint64_t absolute_value)
{
    return encode_number(encoder, absolute_value, NegativeIntegerType << MajorTypeShift);
}

CborError cbor_encode_int(CborEncoder *encoder, int64_t value)
{
    // adapted from code in RFC 7049 appendix C (pseudocode)
    uint64_t ui = value >> 63;              // extend sign to whole length
    uint8_t majorType = ui & 0x20;          // extract major type
    ui ^= value;                            // complement negatives
    return encode_number(encoder, ui, majorType);
}

CborError cbor_encode_simple_value(CborEncoder *encoder, uint8_t value)
{
#ifndef CBOR_ENCODER_NO_CHECK_USER
    // check if this is a valid simple type
    if (value >= HalfPrecisionFloat && value <= Break)
        return CborErrorIllegalSimpleType;
#endif
    return encode_number(encoder, value, SimpleTypesType << MajorTypeShift);
}

CborError cbor_encode_floating_point(CborEncoder *encoder, CborType fpType, const void *value)
{
    uint8_t buf[1 + sizeof(uint64_t)];
    assert(fpType == CborHalfFloatType || fpType == CborFloatType || fpType == CborDoubleType);
    buf[0] = fpType;

    unsigned size = 2U << (fpType - CborHalfFloatType);
    if (size == 8)
        put64(buf + 1, *(const uint64_t*)value);
    else if (size == 4)
        put32(buf + 1, *(const uint32_t*)value);
    else
        put16(buf + 1, *(const uint16_t*)value);
    ++encoder->added;
    return append_to_buffer(encoder, buf, size + 1);
}

CborError cbor_encode_tag(CborEncoder *encoder, CborTag tag)
{
    // tags don't count towards the number of elements in an array or map
    return encode_number_no_update(encoder, tag, TagType << MajorTypeShift);
}

static CborError encode_string(CborEncoder *encoder, size_t length, uint8_t shiftedMajorType, const void *string)
{
    CborError err = encode_number(encoder, length, shiftedMajorType);
    if (err && !isOomError(err))
        return err;
    return append_to_buffer(encoder, string, length);
}

CborError cbor_encode_byte_string(CborEncoder *encoder, const uint8_t *string, size_t length)
{
    return encode_string(encoder, length, ByteStringType << MajorTypeShift, string);
}

CborError cbor_encode_text_string(CborEncoder *encoder, const char *string, size_t length)
{
    return encode_string(encoder, length, TextStringType << MajorTypeShift, string);
}

#ifdef __GNUC__
__attribute__((noinline))
#endif
static CborError create_container(CborEncoder *encoder, CborEncoder *container, size_t length, uint8_t shiftedMajorType)
{
    CborError err;
    container->ptr = encoder->ptr;
    container->end = encoder->end;
    ++encoder->added;
    container->added = 0;

    cbor_static_assert(((MapType << MajorTypeShift) & CborIteratorFlag_ContainerIsMap) == CborIteratorFlag_ContainerIsMap);
    cbor_static_assert(((ArrayType << MajorTypeShift) & CborIteratorFlag_ContainerIsMap) == 0);
    container->flags = shiftedMajorType & CborIteratorFlag_ContainerIsMap;

    if (length == CborIndefiniteLength) {
        container->flags |= CborIteratorFlag_UnknownLength;
        err = append_byte_to_buffer(container, shiftedMajorType + IndefiniteLength);
    } else {
        err = encode_number_no_update(container, length, shiftedMajorType);
    }
    if (err && !isOomError(err))
        return err;

    return CborNoError;
}

CborError cbor_encoder_create_array(CborEncoder *encoder, CborEncoder *arrayEncoder, size_t length)
{
    return create_container(encoder, arrayEncoder, length, ArrayType << MajorTypeShift);
}

CborError cbor_encoder_create_map(CborEncoder *encoder, CborEncoder *mapEncoder, size_t length)
{
    if (length != CborIndefiniteLength && length > SIZE_MAX / 2)
        return CborErrorDataTooLarge;
    return create_container(encoder, mapEncoder, length, MapType << MajorTypeShift);
}

CborError cbor_encoder_close_container(CborEncoder *encoder, const CborEncoder *containerEncoder)
{
    if (encoder->end)
        encoder->ptr = containerEncoder->ptr;
    else
        encoder->bytes_needed = containerEncoder->bytes_needed;
    encoder->end = containerEncoder->end;
    if (containerEncoder->flags & CborIteratorFlag_UnknownLength)
        return append_byte_to_buffer(encoder, BreakByte);
    return CborNoError;
}
