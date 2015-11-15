//
// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#pragma once

namespace DeviceProviders
{
    public enum class TypeId
    {
        Invalid = 0,
        Boolean = 'b',                               // maps to ALLJOYN_BOOLEAN
        Double = 'd',                                // maps to ALLJOYN_DOUBLE
        Dictionary = 'e',                            // maps to an array of ALLJOYN_DICT_ENTRY: a{**}
        Signature = 'g',                             // maps to ALLJOYN_SIGNATURE (string)
        Int32 = 'i',                                 // maps to ALLJOYN_INT32
        Int16 = 'n',                                 // maps to ALLJOYN_INT16
        ObjectPath = 'o',                            // maps to ALLJOYN_OBJECT_PATH (string)
        Uint16 = 'q',                                // maps to ALLLJOYN_UINT16
        Struct = 'r',                                // maps to ALLJOYN_STRUCT
        String = 's',                                // maps to ALLJOYN_STRING
        Uint64 = 't',                                // maps to ALLJOYN_UINT64
        Uint32 = 'u',                                // maps to ALLJOYN_UINT32
        Variant = 'v',                               // maps to ALLJOYN_VARIANT
        Int64 = 'x',                                 // maps to ALLJOYN_INT64
        Uint8 = 'y',                                 // maps to ALLJOYN_BYTE
        ArrayByte = 'a',
        ArrayByteMask = 0xFF,
        BooleanArray    = ('b' << 8) | ArrayByte,    // maps to ALLJOYN_BOOLEAN_ARRAY
        DoubleArray     = ('d' << 8) | ArrayByte,    // maps to ALLJOYN_DOUBLE_ARRAY
        Int32Array      = ('i' << 8) | ArrayByte,    // maps to ALLJOYN_INT32_ARRAY
        Int16Array      = ('n' << 8) | ArrayByte,    // maps to ALLJOYN_INT16_ARRAY
        Uint16Array     = ('q' << 8) | ArrayByte,    // maps to ALLJOYN_UINT16_ARRAY
        Uint64Array     = ('t' << 8) | ArrayByte,    // maps to ALLJOYN_UINT64_ARRAY
        Uint32Array     = ('u' << 8) | ArrayByte,    // maps to ALLJOYN_UINT32_ARRAY
        VariantArray    = ('v' << 8) | ArrayByte,    // no AllJoyn typeid equivalent defined
        Int64Array      = ('x' << 8) | ArrayByte,    // maps to ALLJOYN_INT64_ARRAY
        Uint8Array      = ('y' << 8) | ArrayByte,    // maps to ALLJOYN_BYTE_ARRAY
        SignatureArray  = ('g' << 8) | ArrayByte,    // no AllJoyn typeid equivalent defined
        ObjectPathArray = ('o' << 8) | ArrayByte,    // no AllJoyn typeid equivalent defined
        StringArray     = ('s' << 8) | ArrayByte,    // no AllJoyn typeid equivalent defined
        StructArray     = ('r' << 8) | ArrayByte,    // no AllJoyn typeid equivalent defined
    };

    public interface class ITypeDefinition
    {
        property TypeId Type
        {
            TypeId get();
        }

        // If Type is "Struct" or "StructArray" Fields indicate what fields the Struct has
        property Windows::Foundation::Collections::IVectorView<ITypeDefinition ^> ^ Fields
        {
            Windows::Foundation::Collections::IVectorView<ITypeDefinition ^> ^ get();
        }

        // If Type is "Dictionary" these properties indicate the types of the keys and values. If Type is
        // anything else, these are unused
        property ITypeDefinition ^ KeyType
        {
            ITypeDefinition ^ get();
        }

        property ITypeDefinition ^ ValueType
        {
            ITypeDefinition ^ get();
        }
    };

    public ref struct ParameterInfo sealed
    {
        property Platform::String ^ Name;
        property ITypeDefinition ^ TypeDefinition;
    };

}
