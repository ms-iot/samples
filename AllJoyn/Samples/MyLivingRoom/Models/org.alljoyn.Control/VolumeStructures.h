#pragma once

namespace org { namespace alljoyn { namespace Control {

public ref class VolumeVolumeRange sealed
{
public:
    property int16 Value1
    {
        int16 get() { return m_value1; }
        void set(int16 value) { m_value1 = value; }
    }
     
    property int16 Value2
    {
        int16 get() { return m_value2; }
        void set(int16 value) { m_value2 = value; }
    }
     
    property int16 Value3
    {
        int16 get() { return m_value3; }
        void set(int16 value) { m_value3 = value; }
    }
     
private:
    int16 m_value1;
    int16 m_value2;
    int16 m_value3;
};

public ref class AllJoynMessageArgStructure sealed : Windows::Foundation::Collections::IVector<Platform::Object^>
{
public:
    AllJoynMessageArgStructure()
    {
        m_vector = ref new Platform::Collections::Vector<Platform::Object^>();
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

    virtual void SetAt(unsigned int index, Platform::Object^ value)
    {
        return m_vector->SetAt(index, value);
    }

    virtual void InsertAt(unsigned int index, Platform::Object^ value)
    {
        return m_vector->InsertAt(index, value);
    }

    virtual void Append(Platform::Object^ value)
    {
        return m_vector->Append(value);
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

    virtual void ReplaceAll(const Platform::Array<Platform::Object^>^ items)
    {
        return m_vector->ReplaceAll(items);
    }

private:
    Platform::Collections::Vector<Platform::Object^>^ m_vector;
};
} } } 

partial ref class TypeConversionHelpers
{
internal:
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ org::alljoyn::Control::VolumeVolumeRange^* value)
    {
        UNREFERENCED_PARAMETER(signature);

        (*value) = ref new org::alljoyn::Control::VolumeVolumeRange();

        alljoyn_msgarg argument1;
        alljoyn_msgarg argument2;
        alljoyn_msgarg argument3;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, "(***)", &argument1, &argument2, &argument3));

        int16 value1;
        RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(argument1, "n", &value1));
        (*value)->Value1 = value1;
        int16 value2;
        RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(argument2, "n", &value2));
        (*value)->Value2 = value2;
        int16 value3;
        RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(argument3, "n", &value3));
        (*value)->Value3 = value3;
        
        return ER_OK;
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ org::alljoyn::Control::VolumeVolumeRange^ value)
    {
        UNREFERENCED_PARAMETER(signature);

        auto argument1 = alljoyn_msgarg_create();
        RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(argument1, "n", value->Value1));
        auto argument2 = alljoyn_msgarg_create();
        RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(argument2, "n", value->Value2));
        auto argument3 = alljoyn_msgarg_create();
        RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(argument3, "n", value->Value3));

        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_set(argument, "(***)", argument1, argument2, argument3));
        alljoyn_msgarg_stabilize(argument);
        alljoyn_msgarg_destroy(argument1);
        alljoyn_msgarg_destroy(argument2);
        alljoyn_msgarg_destroy(argument3);

        return ER_OK;
    }
};