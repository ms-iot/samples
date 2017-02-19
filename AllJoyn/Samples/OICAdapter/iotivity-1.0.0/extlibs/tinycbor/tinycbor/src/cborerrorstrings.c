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

#include "cbor.h"

#ifndef _
#  define _(msg)    msg
#endif

const char *cbor_error_string(CborError error)
{
    switch (error) {
    case CborNoError:
        return "";

    case CborUnknownError:
        return _("unknown error");

    case CborErrorOutOfMemory:
        return _("out of memory/need more memory");

    case CborErrorUnknownLength:
        return _("unknown length (attempted to get the length of a map/array/string of indeterminate length");

    case CborErrorAdvancePastEOF:
        return _("attempted to advance past EOF");

    case CborErrorIO:
        return _("I/O error");

    case CborErrorGarbageAtEnd:
        return _("garbage after the end of the content");

    case CborErrorUnexpectedEOF:
        return _("unexpected end of data");

    case CborErrorUnexpectedBreak:
        return _("unexpected 'break' byte");

    case CborErrorUnknownType:
        return _("illegal byte (encodes future extension type)");

    case CborErrorIllegalType:
        return _("mismatched string type in chunked string");

    case CborErrorIllegalNumber:
        return _("illegal initial byte (encodes unspecified additional information)");

    case CborErrorIllegalSimpleType:
        return _("illegal encoding of simple type smaller than 32");

    case CborErrorUnknownSimpleType:
        return _("unknown simple type");

    case CborErrorUnknownTag:
        return _("unknown tag");

    case CborErrorInappropriateTagForType:
        return _("inappropriate tag for type");

    case CborErrorDuplicateObjectKeys:
        return _("duplicate keys in object");

    case CborErrorInvalidUtf8TextString:
        return _("invalid UTF-8 content in string");

    case CborErrorTooManyItems:
        return _("too many items added to encoder");

    case CborErrorTooFewItems:
        return _("too few items added to encoder");

    case CborErrorDataTooLarge:
        return _("internal error: data too large");

    case CborErrorNestingTooDeep:
        return _("internal error: too many nested containers found in recursive function");

    case CborErrorUnsupportedType:
        return _("unsupported type");

    case CborErrorJsonObjectKeyIsAggregate:
        return _("conversion to JSON failed: key in object is an array or map");

    case CborErrorJsonObjectKeyNotString:
        return _("conversion to JSON failed: key in object is not a string");


    case CborErrorInternalError:
        return _("internal error");
    }
    return cbor_error_string(CborUnknownError);
}
