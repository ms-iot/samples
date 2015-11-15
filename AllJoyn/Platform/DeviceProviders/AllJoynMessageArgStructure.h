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

#include "collection.h"
#include "ITypeDefinition.h"
#include "AllJoynTypeDefinition.h"

namespace DeviceProviders
{
    public ref class AllJoynMessageArgStructure sealed : Windows::Foundation::Collections::IVector<Platform::Object^>
    {
        DEBUG_LIFETIME_DECL(AllJoynMessageArgStructure);

    public:
        AllJoynMessageArgStructure(ITypeDefinition^ typeDefinition)
        {
            m_vector = ref new Platform::Collections::Vector<Platform::Object^>();
            AllJoynSignature = AllJoynTypeDefinition::GetAllJoynSignatureString(typeDefinition);
        }

        virtual Windows::Foundation::Collections::IIterator<Platform::Object^>^ First()
        {
            return m_vector->First();
        }

        virtual Platform::Object^ GetAt(unsigned int index)
        {
            return m_vector->GetAt(index);
        }

        virtual property unsigned int Size {
            virtual unsigned int get()
            {
                return m_vector->Size;
            }
        }

        virtual bool IndexOf(Platform::Object^ value, unsigned int * index)
        {
            return m_vector->IndexOf(value, index);
        }

        virtual unsigned int GetMany(unsigned int startIndex, Platform::WriteOnlyArray<Platform::Object^>^ items)
        {
            return m_vector->GetMany(startIndex, items);
        }

        virtual Windows::Foundation::Collections::IVectorView<Platform::Object^>^ GetView()
        {
            return m_vector->GetView();
        }

        virtual void SetAt(unsigned int index, Platform::Object^ item)
        {
            return m_vector->SetAt(index, item);
        }

        virtual void InsertAt(unsigned int index, Platform::Object^ item)
        {
            return m_vector->InsertAt(index, item);
        }

        virtual void Append(Platform::Object^ item)
        {
            return m_vector->Append(item);
        }

        virtual void RemoveAt(unsigned int index)
        {
            return m_vector->RemoveAt(index);
        }

        virtual void RemoveAtEnd()
        {
            return m_vector->RemoveAtEnd();
        }

        virtual void Clear()
        {
            return m_vector->Clear();
        }

        virtual void ReplaceAll(const Platform::Array<Platform::Object^>^ arr)
        {
            return m_vector->ReplaceAll(arr);
        }

    internal:
        AllJoynMessageArgStructure(std::string _allJoynSignature)
        {
            m_vector = ref new Platform::Collections::Vector<Platform::Object^>();
            AllJoynSignature = _allJoynSignature;
        }

        std::string AllJoynSignature;

    private:
        Platform::Collections::Vector<Platform::Object^>^ m_vector;
    };
}