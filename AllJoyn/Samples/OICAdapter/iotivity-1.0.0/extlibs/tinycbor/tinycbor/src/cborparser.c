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
#include "extract_number_p.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "assert_p.h"       /* Always include last */

#ifndef CBOR_PARSER_MAX_RECURSIONS
#  define CBOR_PARSER_MAX_RECURSIONS 1024
#endif

/**
 * \typedef CborValue
 * This type contains one value parsed from the CBOR stream.
 *
 * To get the actual type, use cbor_value_get_type(). Then extract the value
 * using one of the corresponding functions: cbor_value_get_boolean(), cbor_value_get_int64(),
 * cbor_value_get_int(), cbor_value_copy_string(), cbor_value_get_array(), cbor_value_get_map(),
 * cbor_value_get_double(), cbor_value_get_float().
 *
 * In C++ and C11 modes, you can additionally use the cbor_value_get_integer()
 * and cbor_value_get_floating_point() generic functions.
 *
 * \omit
 * Implementation details: the CborValue contains these fields:
 * \list
 *   \li ptr: pointer to the actual data
 *   \li flags: flags from the decoder
 *   \li extra: partially decoded integer value (0, 1 or 2 bytes)
 *   \li remaining: remaining items in this collection after this item or UINT32_MAX if length is unknown
 * \endlist
 * \endomit
 */

static CborError extract_length(const CborParser *parser, const uint8_t **ptr, size_t *len)
{
    uint64_t v;
    CborError err = extract_number(ptr, parser->end, &v);
    if (err)
        return err;

    *len = v;
    if (v != *len)
        return CborErrorDataTooLarge;
    return CborNoError;
}

static bool is_fixed_type(uint8_t type)
{
    return type != CborTextStringType && type != CborByteStringType && type != CborArrayType &&
           type != CborMapType;
}

static CborError preparse_value(CborValue *it)
{
    const CborParser *parser = it->parser;
    it->type = CborInvalidType;

    // are we at the end?
    if (it->ptr == parser->end)
        return CborErrorUnexpectedEOF;

    uint8_t descriptor = *it->ptr;
    uint8_t type = descriptor & MajorTypeMask;
    it->type = type;
    it->flags = 0;
    it->extra = (descriptor &= SmallValueMask);

    if (descriptor > Value64Bit) {
        if (unlikely(descriptor != IndefiniteLength))
            return type == CborSimpleType ? CborErrorUnknownType : CborErrorIllegalNumber;
        if (likely(!is_fixed_type(type))) {
            // special case
            it->flags |= CborIteratorFlag_UnknownLength;
            it->type = type;
            return CborNoError;
        }
        return type == CborSimpleType ? CborErrorUnexpectedBreak : CborErrorIllegalNumber;
    }

    size_t bytesNeeded = descriptor < Value8Bit ? 0 : (1 << (descriptor - Value8Bit));
    if (it->ptr + 1 + bytesNeeded > parser->end)
        return CborErrorUnexpectedEOF;

    uint8_t majortype = type >> MajorTypeShift;
    if (majortype == NegativeIntegerType) {
        it->flags |= CborIteratorFlag_NegativeInteger;
        it->type = CborIntegerType;
    } else if (majortype == SimpleTypesType) {
        switch (descriptor) {
        case FalseValue:
            it->extra = false;
            it->type = CborBooleanType;
            break;

        case SinglePrecisionFloat:
        case DoublePrecisionFloat:
            it->flags |= CborIteratorFlag_IntegerValueTooLarge;
            // fall through
        case TrueValue:
        case NullValue:
        case UndefinedValue:
        case HalfPrecisionFloat:
            it->type = *it->ptr;
            break;

        case SimpleTypeInNextByte:
            it->extra = (uint8_t)it->ptr[1];
#ifndef CBOR_PARSER_NO_STRICT_CHECKS
            if (unlikely(it->extra < 32)) {
                it->type = CborInvalidType;
                return CborErrorIllegalSimpleType;
            }
#endif
            break;

        case 28:
        case 29:
        case 30:
        case Break:
            assert(false);  // these conditions can't be reached
            return CborErrorUnexpectedBreak;
        }
        return CborNoError;
    }

    // try to decode up to 16 bits
    if (descriptor < Value8Bit)
        return CborNoError;

    if (descriptor == Value8Bit)
        it->extra = (uint8_t)it->ptr[1];
    else if (descriptor == Value16Bit)
        it->extra = get16(it->ptr + 1);
    else
        it->flags |= CborIteratorFlag_IntegerValueTooLarge;     // Value32Bit or Value64Bit
    return CborNoError;
}

static CborError preparse_next_value(CborValue *it)
{
    if (it->remaining != UINT32_MAX) {
        // don't decrement the item count if the current item is tag: they don't count
        if (it->type != CborTagType && !--it->remaining) {
            it->type = CborInvalidType;
            return CborNoError;
        }
    } else if (it->remaining == UINT32_MAX && it->ptr != it->parser->end && *it->ptr == (uint8_t)BreakByte) {
        // end of map or array
        ++it->ptr;
        it->type = CborInvalidType;
        it->remaining = 0;
        return CborNoError;
    }

    return preparse_value(it);
}

static CborError advance_internal(CborValue *it)
{
    uint64_t length;
    CborError err = extract_number(&it->ptr, it->parser->end, &length);
    assert(err == CborNoError);

    if (it->type == CborByteStringType || it->type == CborTextStringType) {
        assert(length == (size_t)length);
        assert((it->flags & CborIteratorFlag_UnknownLength) == 0);
        it->ptr += length;
    }

    return preparse_next_value(it);
}

/** \internal
 *
 * Decodes the CBOR integer value when it is larger than the 16 bits available
 * in value->extra. This function requires that value->flags have the
 * CborIteratorFlag_IntegerValueTooLarge flag set.
 *
 * This function is also used to extract single- and double-precision floating
 * point values (SinglePrecisionFloat == Value32Bit and DoublePrecisionFloat ==
 * Value64Bit).
 */
uint64_t _cbor_value_decode_int64_internal(const CborValue *value)
{
    assert(value->flags & CborIteratorFlag_IntegerValueTooLarge ||
           value->type == CborFloatType || value->type == CborDoubleType);

    // since the additional information can only be Value32Bit or Value64Bit,
    // we just need to test for the one bit those two options differ
    assert((*value->ptr & SmallValueMask) == Value32Bit || (*value->ptr & SmallValueMask) == Value64Bit);
    if ((*value->ptr & 1) == (Value32Bit & 1))
        return get32(value->ptr + 1);

    assert((*value->ptr & SmallValueMask) == Value64Bit);
    return get64(value->ptr + 1);
}

/**
 * Initializes the CBOR parser for parsing \a size bytes beginning at \a
 * buffer. Parsing will use flags set in \a flags. The iterator to the first
 * element is returned in \a it.
 *
 * The \a parser structure needs to remain valid throughout the decoding
 * process. It is not thread-safe to share one CborParser among multiple
 * threads iterating at the same time, but the object can be copied so multiple
 * threads can iterate.
 *
 * ### Write how to determine the end pointer
 * ### Write how to do limited-buffer windowed decoding
 */
CborError cbor_parser_init(const uint8_t *buffer, size_t size, int flags, CborParser *parser, CborValue *it)
{
    memset(parser, 0, sizeof(*parser));
    parser->end = buffer + size;
    parser->flags = flags;
    it->parser = parser;
    it->ptr = buffer;
    it->remaining = 1;      // there's one type altogether, usually an array or map
    return preparse_value(it);
}

/**
 * Advances the CBOR value \a it by one fixed-size position. Fixed-size types
 * are: integers, tags, simple types (including boolean, null and undefined
 * values) and floating point types.
 *
 * \sa cbor_value_at_end(), cbor_value_advance(), cbor_value_begin_recurse(), cbor_value_end_recurse()
 */
CborError cbor_value_advance_fixed(CborValue *it)
{
    assert(it->type != CborInvalidType);
    assert(is_fixed_type(it->type));
    if (!it->remaining)
        return CborErrorAdvancePastEOF;
    return advance_internal(it);
}

static CborError advance_recursive(CborValue *it, int nestingLevel)
{
    if (is_fixed_type(it->type))
        return advance_internal(it);

    if (!cbor_value_is_container(it)) {
        size_t len = SIZE_MAX;
        return _cbor_value_copy_string(it, NULL, &len, it);
    }

    // map or array
    if (nestingLevel == CBOR_PARSER_MAX_RECURSIONS)
        return CborErrorNestingTooDeep;

    CborError err;
    CborValue recursed;
    err = cbor_value_enter_container(it, &recursed);
    if (err)
        return err;
    while (!cbor_value_at_end(&recursed)) {
        err = advance_recursive(&recursed, nestingLevel + 1);
        if (err)
            return err;
    }
    return cbor_value_leave_container(it, &recursed);
}


/**
 * Advances the CBOR value \a it by one element, skipping over containers.
 * Unlike cbor_value_advance_fixed(), this function can be called on a CBOR
 * value of any type. However, if the type is a container (map or array) or a
 * string with a chunked payload, this function will not run in constant time
 * and will recurse into itself (it will run on O(n) time for the number of
 * elements or chunks and will use O(n) memory for the number of nested
 * containers).
 *
 * \sa cbor_value_at_end(), cbor_value_advance_fixed(), cbor_value_begin_recurse(), cbor_value_end_recurse()
 */
CborError cbor_value_advance(CborValue *it)
{
    assert(it->type != CborInvalidType);
    if (!it->remaining)
        return CborErrorAdvancePastEOF;
    return advance_recursive(it, 0);
}

/**
 * Advances the CBOR value \a it until it no longer points to a tag. If \a it is
 * already not pointing to a tag, then this function returns it unchanged.
 *
 * \sa cbor_value_advance_fixed(), cbor_value_advance()
 */
CborError cbor_value_skip_tag(CborValue *it)
{
    while (cbor_value_is_tag(it)) {
        CborError err = cbor_value_advance_fixed(it);
        if (err)
            return err;
    }
    return CborNoError;
}

/**
 * \fn bool cbor_value_is_container(const CborValue *it)
 *
 * Returns true if the \a it value is a container and requires recursion in
 * order to decode (maps and arrays), false otherwise.
 */

/**
 * Creates a CborValue iterator pointing to the first element of the container
 * represented by \a it and saves it in \a recursed. The \a it container object
 * needs to be kept and passed again to cbor_value_leave_container() in order
 * to continue iterating past this container.
 *
 * \sa cbor_value_is_container(), cbor_value_leave_container(), cbor_value_advance()
 */
CborError cbor_value_enter_container(const CborValue *it, CborValue *recursed)
{
    CborError err;
    assert(cbor_value_is_container(it));
    *recursed = *it;

    if (it->flags & CborIteratorFlag_UnknownLength) {
        recursed->remaining = UINT32_MAX;
        ++recursed->ptr;
        err = preparse_value(recursed);
        if (err != CborErrorUnexpectedBreak)
            return err;
        // actually, break was expected here
        // it's just an empty container
        ++recursed->ptr;
    } else {
        uint64_t len;
        err = extract_number(&recursed->ptr, recursed->parser->end, &len);
        assert(err == CborNoError);

        recursed->remaining = len;
        if (recursed->remaining != len || len == UINT32_MAX) {
            // back track the pointer to indicate where the error occurred
            recursed->ptr = it->ptr;
            return CborErrorDataTooLarge;
        }
        if (recursed->type == CborMapType) {
            // maps have keys and values, so we need to multiply by 2
            if (recursed->remaining > UINT32_MAX / 2) {
                // back track the pointer to indicate where the error occurred
                recursed->ptr = it->ptr;
                return CborErrorDataTooLarge;
            }
            recursed->remaining *= 2;
        }
        if (len != 0)
            return preparse_value(recursed);
    }

    // the case of the empty container
    recursed->type = CborInvalidType;
    recursed->remaining = 0;
    return CborNoError;
}

/**
 * Updates \a it to point to the next element after the container. The \a
 * recursed object needs to point to the element obtained either by advancing
 * the last element of the container (via cbor_value_advance(),
 * cbor_value_advance_fixed(), a nested cbor_value_leave_container(), or the \c
 * next pointer from cbor_value_copy_string() or cbor_value_dup_string()).
 *
 * \sa cbor_value_enter_container(), cbor_value_at_end()
 */
CborError cbor_value_leave_container(CborValue *it, const CborValue *recursed)
{
    assert(cbor_value_is_container(it));
    assert(recursed->type == CborInvalidType);
    it->ptr = recursed->ptr;
    return preparse_next_value(it);
}

/**
 * Calculates the length of the string in \a value and stores the result in \a
 * len. This function is different from cbor_value_get_string_length() in that
 * it calculates the length even for strings sent in chunks. For that reason,
 * this function may not run in constant time (it will run in O(n) time on the
 * number of chunks).
 *
 * \note On 32-bit platforms, this function will return error condition of \ref
 * CborErrorDataTooLarge if the stream indicates a length that is too big to
 * fit in 32-bit.
 *
 * \sa cbor_value_get_string_length(), cbor_value_copy_string(), cbor_value_is_length_known()
 */
CborError cbor_value_calculate_string_length(const CborValue *value, size_t *len)
{
    *len = SIZE_MAX;
    return _cbor_value_copy_string(value, NULL, len, NULL);
}

/**
 * \fn CborError cbor_value_dup_text_string(const CborValue *value, char **buffer, size_t *buflen, CborValue *next)
 *
 * Allocates memory for the string pointed by \a value and copies it into this
 * buffer. The pointer to the buffer is stored in \a buffer and the number of
 * bytes copied is stored in \a len (those variables must not be NULL).
 *
 * If \c malloc returns a NULL pointer, this function will return error
 * condition \ref CborErrorOutOfMemory.
 *
 * On success, \c{*buffer} will contain a valid pointer that must be freed by
 * calling \c{free()}. This is the case even for zero-length strings.
 *
 * The \a next pointer, if not null, will be updated to point to the next item
 * after this string. If \a value points to the last item, then \a next will be
 * invalid.
 *
 * \note This function does not perform UTF-8 validation on the incoming text
 * string.
 *
 * \sa cbor_value_copy_text_string(), cbor_value_dup_byte_string()
 */

/**
 * \fn CborError cbor_value_dup_byte_string(const CborValue *value, uint8_t **buffer, size_t *buflen, CborValue *next)
 *
 * Allocates memory for the string pointed by \a value and copies it into this
 * buffer. The pointer to the buffer is stored in \a buffer and the number of
 * bytes copied is stored in \a len (those variables must not be NULL).
 *
 * If \c malloc returns a NULL pointer, this function will return error
 * condition \ref CborErrorOutOfMemory.
 *
 * On success, \c{*buffer} will contain a valid pointer that must be freed by
 * calling \c{free()}. This is the case even for zero-length strings.
 *
 * The \a next pointer, if not null, will be updated to point to the next item
 * after this string. If \a value points to the last item, then \a next will be
 * invalid.
 *
 * \sa cbor_value_copy_byte_string(), cbor_value_dup_text_string()
 */
CborError _cbor_value_dup_string(const CborValue *value, void **buffer, size_t *buflen, CborValue *next)
{
    assert(buffer);
    assert(buflen);
    *buflen = SIZE_MAX;
    CborError err = _cbor_value_copy_string(value, NULL, buflen, NULL);
    if (err)
        return err;

    ++*buflen;
    *buffer = malloc(*buflen);
    if (!*buffer) {
        // out of memory
        return CborErrorOutOfMemory;
    }
    err = _cbor_value_copy_string(value, *buffer, buflen, next);
    if (err) {
        free(*buffer);
        return err;
    }
    return CborNoError;
}

// We return uintptr_t so that we can pass memcpy directly as the iteration
// function. The choice is to optimize for memcpy, which is used in the base
// parser API (cbor_value_copy_string), while memcmp is used in convenience API
// only.
typedef uintptr_t (*IterateFunction)(char *, const uint8_t *, size_t);

static uintptr_t iterate_noop(char *dest, const uint8_t *src, size_t len)
{
    (void)dest;
    (void)src;
    (void)len;
    return true;
}

static uintptr_t iterate_memcmp(char *s1, const uint8_t *s2, size_t len)
{
    return memcmp(s1, (const char *)s2, len) == 0;
}

static CborError iterate_string_chunks(const CborValue *value, char *buffer, size_t *buflen,
                                       bool *result, CborValue *next, IterateFunction func)
{
    assert(cbor_value_is_byte_string(value) || cbor_value_is_text_string(value));

    size_t total;
    CborError err;
    const uint8_t *ptr = value->ptr;
    if (cbor_value_is_length_known(value)) {
        // easy case: fixed length
        err = extract_length(value->parser, &ptr, &total);
        if (err)
            return err;
        if (ptr + total > value->parser->end)
            return CborErrorUnexpectedEOF;
        if (total <= *buflen)
            *result = func(buffer, ptr, total);
        else
            *result = false;
        ptr += total;
    } else {
        // chunked
        ++ptr;
        total = 0;
        *result = true;
        while (true) {
            size_t chunkLen;
            size_t newTotal;

            if (ptr == value->parser->end)
                return CborErrorUnexpectedEOF;

            if (*ptr == (uint8_t)BreakByte) {
                ++ptr;
                break;
            }

            // is this the right type?
            if ((*ptr & MajorTypeMask) != value->type)
                return CborErrorIllegalType;

            err = extract_length(value->parser, &ptr, &chunkLen);
            if (err)
                return err;

            if (unlikely(add_check_overflow(total, chunkLen, &newTotal)))
                return CborErrorDataTooLarge;

            if (ptr + chunkLen > value->parser->end)
                return CborErrorUnexpectedEOF;

            if (*result && *buflen >= newTotal)
                *result = func(buffer + total, ptr, chunkLen);
            else
                *result = false;

            ptr += chunkLen;
            total = newTotal;
        }
    }

    // is there enough room for the ending NUL byte?
    if (*result && *buflen > total)
        *result = func(buffer + total, (const uint8_t *)"", 1);
    *buflen = total;

    if (next) {
        *next = *value;
        next->ptr = ptr;
        return preparse_next_value(next);
    }
    return CborNoError;
}

/**
 * \fn CborError cbor_value_copy_text_string(const CborValue *value, char *buffer, size_t *buflen, CborValue *next)
 *
 * Copies the string pointed by \a value into the buffer provided at \a buffer
 * of \a buflen bytes. If \a buffer is a NULL pointer, this function will not
 * copy anything and will only update the \a next value.
 *
 * If the provided buffer length was too small, this function returns an error
 * condition of \ref CborErrorOutOfMemory. If you need to calculate the length
 * of the string in order to preallocate a buffer, use
 * cbor_value_calculate_string_length().
 *
 * On success, this function sets the number of bytes copied to \c{*buflen}. If
 * the buffer is large enough, this function will insert a null byte after the
 * last copied byte, to facilitate manipulation of text strings. That byte is
 * not included in the returned value of \c{*buflen}.
 *
 * The \a next pointer, if not null, will be updated to point to the next item
 * after this string. If \a value points to the last item, then \a next will be
 * invalid.
 *
 * \note This function does not perform UTF-8 validation on the incoming text
 * string.
 *
 * \sa cbor_value_dup_text_string(), cbor_value_copy_byte_string(), cbor_value_get_string_length(), cbor_value_calculate_string_length()
 */

/**
 * \fn CborError cbor_value_copy_byte_string(const CborValue *value, uint8_t *buffer, size_t *buflen, CborValue *next)
 *
 * Copies the string pointed by \a value into the buffer provided at \a buffer
 * of \a buflen bytes. If \a buffer is a NULL pointer, this function will not
 * copy anything and will only update the \a next value.
 *
 * If the provided buffer length was too small, this function returns an error
 * condition of \ref CborErrorOutOfMemory. If you need to calculate the length
 * of the string in order to preallocate a buffer, use
 * cbor_value_calculate_string_length().
 *
 * On success, this function sets the number of bytes copied to \c{*buflen}. If
 * the buffer is large enough, this function will insert a null byte after the
 * last copied byte, to facilitate manipulation of null-terminated strings.
 * That byte is not included in the returned value of \c{*buflen}.
 *
 * The \a next pointer, if not null, will be updated to point to the next item
 * after this string. If \a value points to the last item, then \a next will be
 * invalid.
 *
 * \sa cbor_value_dup_text_string(), cbor_value_copy_text_string(), cbor_value_get_string_length(), cbor_value_calculate_string_length()
 */

CborError _cbor_value_copy_string(const CborValue *value, void *buffer,
                                 size_t *buflen, CborValue *next)
{
    bool copied_all;
    CborError err = iterate_string_chunks(value, (char*)buffer, buflen, &copied_all, next,
                                          buffer ? (IterateFunction)memcpy : iterate_noop);
    return err ? err :
                 copied_all ? CborNoError : CborErrorOutOfMemory;
}

/**
 * Compares the entry \a value with the string \a string and store the result
 * in \a result. If the value is different from \a string or if it is not a
 * text string, \a result will contain \c false.
 *
 * The entry at \a value may be a tagged string. If \a is not a string or a
 * tagged string, the comparison result will be false.
 */
CborError cbor_value_text_string_equals(const CborValue *value, const char *string, bool *result)
{
    CborValue copy = *value;
    CborError err = cbor_value_skip_tag(&copy);
    if (err)
        return err;
    if (!cbor_value_is_text_string(&copy)) {
        *result = false;
        return CborNoError;
    }

    size_t len = strlen(string);
    return iterate_string_chunks(&copy, CONST_CAST(char *, string), &len, result, NULL, iterate_memcmp);
}

/**
 * Attempts to find the value in map \a map that corresponds to the text string
 * entry \a string. If the item is found, it is stored in \a result. If no item
 * is found matching the key, then \a result will contain an element of type
 * \ref CborInvalidType.
 *
 * \note This function may be expensive to execute.
 */
CborError cbor_value_map_find_value(const CborValue *map, const char *string, CborValue *element)
{
    assert(cbor_value_is_map(map));
    size_t len = strlen(string);
    CborError err = cbor_value_enter_container(map, element);
    if (err)
        goto error;

    while (!cbor_value_at_end(element)) {
        // find the non-tag so we can compare
        err = cbor_value_skip_tag(element);
        if (err)
            goto error;
        if (cbor_value_is_text_string(element)) {
            bool equals;
            size_t dummyLen = len;
            err = iterate_string_chunks(element, CONST_CAST(char *, string), &dummyLen,
                                        &equals, element, iterate_memcmp);
            if (err)
                goto error;
            if (equals)
                return preparse_value(element);
        } else {
            // skip this key
            err = cbor_value_advance(element);
            if (err)
                goto error;
        }

        // skip this value
        err = cbor_value_skip_tag(element);
        if (err)
            goto error;
        err = cbor_value_advance(element);
        if (err)
            goto error;
    }

    // not found
    element->type = CborInvalidType;
    return CborNoError;

error:
    element->type = CborInvalidType;
    return err;
}

/**
 * Extracts a half-precision floating point from \a value and stores it in \a
 * result.
 */
CborError cbor_value_get_half_float(const CborValue *value, void *result)
{
    assert(value->type == CborHalfFloatType);

    // size has been computed already
    uint16_t v = get16(value->ptr + 1);
    memcpy(result, &v, sizeof(v));
    return CborNoError;
}
