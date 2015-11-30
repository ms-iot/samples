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

using DeviceProviders;
using System;
using System.Collections.Generic;
using System.Linq;
using Windows.Data.Json;

namespace AllJoynVoice
{
    class AllJoynTypes
    {
        static public object Convert(ITypeDefinition type, IJsonValue val)
        {
            try
            {
                switch (type.Type)
                {
                    case TypeId.Boolean:
                        return val.GetBoolean();
                    case TypeId.Double:
                        return val.GetNumber();
                    case TypeId.Dictionary:
                        return val.GetArray()
                                  .ToList()
                                  .Select(x =>
                                      new KeyValuePair<object, object>(
                                          Convert(type.KeyType, x.GetArray()[0]),
                                          Convert(type.ValueType, x.GetArray()[1])
                                      )
                                  );
                    case TypeId.Signature:
                        return val.GetString();
                    case TypeId.Int32:
                        return (Int32)val.GetNumber();
                    case TypeId.Int16:
                        return (Int16)val.GetNumber();
                    case TypeId.ObjectPath:
                        return val.GetString();
                    case TypeId.Uint16:
                        return (UInt16)val.GetNumber();
                    case TypeId.Struct:
                        return ConvertStruct(type, val);
                    case TypeId.String:
                        return val.GetString();
                    case TypeId.Uint64:
                        return (UInt64)val.GetNumber();
                    case TypeId.Uint32:
                        return (UInt32)val.GetNumber();
                    case TypeId.Variant:
                        return ConvertVariant(val);
                    case TypeId.Int64:
                        return (Int64)val.GetNumber();
                    case TypeId.Uint8:
                        return (byte)val.GetNumber();
                    case TypeId.BooleanArray:
                        return val.GetArray().Select(x => x.GetBoolean()).Cast<object>().ToList();
                    case TypeId.DoubleArray:
                        return val.GetArray().Select(x => x.GetNumber()).Cast<object>().ToList();
                    case TypeId.Int32Array:
                        return val.GetArray().Select(x => (Int32)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.Int16Array:
                        return val.GetArray().Select(x => (Int16)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.Uint16Array:
                        return val.GetArray().Select(x => (UInt16)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.Uint64Array:
                        return val.GetArray().Select(x => (UInt64)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.Uint32Array:
                        return val.GetArray().Select(x => (UInt32)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.VariantArray:
                        return val.GetArray().Select(ConvertVariant).Cast<object>().ToList();
                    case TypeId.Int64Array:
                        return val.GetArray().Select(x => (Int64)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.Uint8Array:
                        return val.GetArray().Select(x => (byte)x.GetNumber()).Cast<object>().ToList();
                    case TypeId.SignatureArray:
                        return val.GetArray().Select(x => x.GetString()).Cast<object>().ToList();
                    case TypeId.ObjectPathArray:
                        return val.GetArray().Select(x => x.GetString()).Cast<object>().ToList();
                    case TypeId.StringArray:
                        return val.GetArray().Select(x => x.GetString()).Cast<object>().ToList();
                    case TypeId.StructArray:
                        return val.GetArray().Select(x => ConvertStruct(type, x)).Cast<object>().ToList();
                }
            }
            catch (Exception) { };

            throw Logger.LogException("AllJoyn cast", new InvalidCastException());
        }

        private static object ConvertVariant(IJsonValue arg)
        {
            JsonObject obj = arg.GetObject();

            string signatureString = obj.Keys.Single();
            ITypeDefinition signature = AllJoynTypeDefinition.CreateTypeDefintions(signatureString).Single();

            return Convert(signature, obj.Values.Single());
        }

        static private object ConvertStruct(ITypeDefinition type, IJsonValue val)
        {
            AllJoynMessageArgStructure @struct = new AllJoynMessageArgStructure(type);

            IEnumerable<object> fields = Enumerable.Zip(type.Fields, val.GetArray(), Convert);

            foreach (object elem in fields)
            {
                @struct.Add(elem);
            }

            return @struct;
        }
    }
}