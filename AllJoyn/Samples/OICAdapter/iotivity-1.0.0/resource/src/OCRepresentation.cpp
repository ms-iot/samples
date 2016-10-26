//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * @file
 *
 * This file contains the implementation of classes and its members related
 * to OCRepresentation.
 */


#include <OCRepresentation.h>

#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "ocpayload.h"
#include "ocrandom.h"
#include "oic_malloc.h"
#include "oic_string.h"

namespace OC
{

    void MessageContainer::setPayload(const OCPayload* rep)
    {
        switch(rep->type)
        {
            case PAYLOAD_TYPE_REPRESENTATION:
                setPayload(reinterpret_cast<const OCRepPayload*>(rep));
                break;
            case PAYLOAD_TYPE_DEVICE:
                setPayload(reinterpret_cast<const OCDevicePayload*>(rep));
                break;
            case PAYLOAD_TYPE_PLATFORM:
                setPayload(reinterpret_cast<const OCPlatformPayload*>(rep));
                break;
            default:
                throw OC::OCException("Invalid Payload type in setPayload");
                break;
        }
    }

    void MessageContainer::setPayload(const OCDevicePayload* payload)
    {
        OCRepresentation rep;
        rep.setUri(payload->uri);
        char uuidString[UUID_STRING_SIZE];
        if(payload->sid && RAND_UUID_OK == OCConvertUuidToString(payload->sid, uuidString))
        {
            rep[OC_RSRVD_DEVICE_ID] = std::string(uuidString);
        }
        else
        {
            rep[OC_RSRVD_DEVICE_ID] = std::string();
        }
        rep[OC_RSRVD_DEVICE_NAME] = payload->deviceName ?
            std::string(payload->deviceName) :
            std::string();
        rep[OC_RSRVD_SPEC_VERSION] = payload->specVersion ?
            std::string(payload->specVersion) :
            std::string();
        rep[OC_RSRVD_DATA_MODEL_VERSION] = payload->dataModelVersion ?
            std::string(payload->dataModelVersion) :
            std::string();
        m_reps.push_back(std::move(rep));
    }

    void MessageContainer::setPayload(const OCPlatformPayload* payload)
    {
        OCRepresentation rep;
        rep.setUri(payload->uri);

        rep[OC_RSRVD_PLATFORM_ID] = payload->info.platformID ?
            std::string(payload->info.platformID) :
            std::string();
        rep[OC_RSRVD_MFG_NAME] = payload->info.manufacturerName ?
            std::string(payload->info.manufacturerName) :
            std::string();
        rep[OC_RSRVD_MFG_URL] = payload->info.manufacturerUrl ?
            std::string(payload->info.manufacturerUrl) :
            std::string();
        rep[OC_RSRVD_MODEL_NUM] = payload->info.modelNumber ?
            std::string(payload->info.modelNumber) :
            std::string();
        rep[OC_RSRVD_MFG_DATE] = payload->info.dateOfManufacture ?
            std::string(payload->info.dateOfManufacture) :
            std::string();
        rep[OC_RSRVD_PLATFORM_VERSION] = payload->info.platformVersion ?
            std::string(payload->info.platformVersion) :
            std::string();
        rep[OC_RSRVD_OS_VERSION] = payload->info.operatingSystemVersion ?
            std::string(payload->info.operatingSystemVersion) :
            std::string();
        rep[OC_RSRVD_HARDWARE_VERSION] = payload->info.hardwareVersion ?
            std::string(payload->info.hardwareVersion) :
            std::string();
        rep[OC_RSRVD_FIRMWARE_VERSION] = payload->info.firmwareVersion ?
            std::string(payload->info.firmwareVersion) :
            std::string();
        rep[OC_RSRVD_SUPPORT_URL] = payload->info.supportUrl ?
            std::string(payload->info.supportUrl) :
            std::string();
        rep[OC_RSRVD_SYSTEM_TIME] = payload->info.systemTime ?
            std::string(payload->info.systemTime) :
            std::string();

        m_reps.push_back(std::move(rep));
    }

    void MessageContainer::setPayload(const OCRepPayload* payload)
    {
        const OCRepPayload* pl = payload;
        while(pl)
        {
            OCRepresentation cur;
            cur.setPayload(pl);

            pl = pl->next;
            this->addRepresentation(cur);
        }
    }

    OCRepPayload* MessageContainer::getPayload() const
    {
        OCRepPayload* root = nullptr;
        for(const auto& r : representations())
        {
            if(!root)
            {
                root = r.getPayload();
            }
            else
            {
                OCRepPayloadAppend(root, r.getPayload());
            }
        }

        return root;
    }

    const std::vector<OCRepresentation>& MessageContainer::representations() const
    {
        return m_reps;
    }

    void MessageContainer::addRepresentation(const OCRepresentation& rep)
    {
        m_reps.push_back(rep);
    }
}

namespace OC
{
    struct get_payload_array: boost::static_visitor<>
    {
        template<typename T>
        void operator()(T& /*arr*/)
        {
            throw std::logic_error("Invalid calc_dimensions_visitor type");
        }

        template<typename T>
        void operator()(std::vector<T>& arr)
        {
            root_size_calc<T>();
            dimensions[0] = arr.size();
            dimensions[1] = 0;
            dimensions[2] = 0;
            dimTotal = calcDimTotal(dimensions);

            array = (void*)OICMalloc(dimTotal * root_size);

            for(size_t i = 0; i < dimensions[0]; ++i)
            {
                copy_to_array(arr[i], array, i);
            }

        }
        template<typename T>
        void operator()(std::vector<std::vector<T>>& arr)
        {
            root_size_calc<T>();
            dimensions[0] = arr.size();
            dimensions[1] = 0;
            dimensions[2] = 0;
            for(size_t i = 0; i < arr.size(); ++i)
            {
                dimensions[1] = std::max(dimensions[1], arr[i].size());
            }
            dimTotal = calcDimTotal(dimensions);
            array = (void*)OICCalloc(1, dimTotal * root_size);

            for(size_t i = 0; i < dimensions[0]; ++i)
            {
                for(size_t j = 0; j < dimensions[1] && j < arr[i].size(); ++j)
                {
                    copy_to_array(arr[i][j], array, i*dimensions[1] + j);
                }
            }
        }
        template<typename T>
        void operator()(std::vector<std::vector<std::vector<T>>>& arr)
        {
            root_size_calc<T>();
            dimensions[0] = arr.size();
            dimensions[1] = 0;
            dimensions[2] = 0;
            for(size_t i = 0; i < arr.size(); ++i)
            {
                dimensions[1] = std::max(dimensions[1], arr[i].size());

                for(size_t j = 0; j < arr[i].size(); ++j)
                {
                    dimensions[2] = std::max(dimensions[2], arr[i][j].size());
                }
            }

            dimTotal = calcDimTotal(dimensions);
            array = (void*)OICCalloc(1, dimTotal * root_size);

            for(size_t i = 0; i < dimensions[0]; ++i)
            {
                for(size_t j = 0; j < dimensions[1] && j < arr[i].size(); ++j)
                {
                    for(size_t k = 0; k < dimensions[2] && k < arr[i][j].size(); ++k)
                    {
                        copy_to_array(arr[i][j][k], array,
                                dimensions[2] * j +
                                dimensions[2] * dimensions[1] * i +
                                k);
                    }
                }
            }
        }

        template<typename T>
        void root_size_calc()
        {
            root_size = sizeof(T);
        }

        template<typename T>
        void copy_to_array(T item, void* array, size_t pos)
        {
            ((T*)array)[pos] = item;
        }

        size_t dimensions[MAX_REP_ARRAY_DEPTH];
        size_t root_size;
        size_t dimTotal;
        void* array;
    };

    template<>
    void get_payload_array::root_size_calc<int>()
    {
        root_size = sizeof(int64_t);
    }

    template<>
    void get_payload_array::root_size_calc<std::string>()
    {
        root_size = sizeof(char*);
    }

    template<>
    void get_payload_array::root_size_calc<OC::OCRepresentation>()
    {
        root_size = sizeof(OCRepPayload*);
    }

    template<>
    void get_payload_array::copy_to_array(int item, void* array, size_t pos)
    {
        ((int64_t*)array)[pos] = item;
    }

#ifndef WIN32
    template<>
    void get_payload_array::copy_to_array(std::_Bit_reference br, void* array, size_t pos)
    {
        ((bool*)array)[pos] = static_cast<bool>(br);
    }
#endif

    template<>
    void get_payload_array::copy_to_array(std::string item, void* array, size_t pos)
    {
        ((char**)array)[pos] = OICStrdup(item.c_str());
    }

    template<>
    void get_payload_array::copy_to_array(std::string& item, void* array, size_t pos)
    {
        ((char**)array)[pos] = OICStrdup(item.c_str());
    }

    template<>
    void get_payload_array::copy_to_array(const std::string& item, void* array, size_t pos)
    {
        ((char**)array)[pos] = OICStrdup(item.c_str());
    }

    template<>
    void get_payload_array::copy_to_array(OC::OCRepresentation item, void* array, size_t pos)
    {
        ((OCRepPayload**)array)[pos] = item.getPayload();
    }

    void OCRepresentation::getPayloadArray(OCRepPayload* payload,
                    const OCRepresentation::AttributeItem& item) const
    {
        get_payload_array vis{};
        boost::apply_visitor(vis, m_values[item.attrname()]);


        switch(item.base_type())
        {
            case AttributeType::Integer:
                OCRepPayloadSetIntArrayAsOwner(payload, item.attrname().c_str(),
                        (int64_t*)vis.array,
                        vis.dimensions);
                break;
            case AttributeType::Double:
                OCRepPayloadSetDoubleArrayAsOwner(payload, item.attrname().c_str(),
                        (double*)vis.array,
                        vis.dimensions);
                break;
            case AttributeType::Boolean:
                OCRepPayloadSetBoolArrayAsOwner(payload, item.attrname().c_str(),
                        (bool*)vis.array,
                        vis.dimensions);
                break;
            case AttributeType::String:
                OCRepPayloadSetStringArrayAsOwner(payload, item.attrname().c_str(),
                        (char**)vis.array,
                        vis.dimensions);
                break;
            case AttributeType::OCRepresentation:
                OCRepPayloadSetPropObjectArrayAsOwner(payload, item.attrname().c_str(),
                        (OCRepPayload**)vis.array, vis.dimensions);
                break;
            default:
                throw std::logic_error(std::string("GetPayloadArray: Not Implemented") +
                        std::to_string((int)item.base_type()));
        }
    }

    OCRepPayload* OCRepresentation::getPayload() const
    {
        OCRepPayload* root = OCRepPayloadCreate();
        if(!root)
        {
            throw std::bad_alloc();
        }

        OCRepPayloadSetUri(root, getUri().c_str());

        for(const std::string& type : getResourceTypes())
        {
            OCRepPayloadAddResourceType(root, type.c_str());
        }

        for(const std::string& iface : getResourceInterfaces())
        {
            OCRepPayloadAddInterface(root, iface.c_str());
        }

        for(auto& val : *this)
        {
            switch(val.type())
            {
                case AttributeType::Null:
                    OCRepPayloadSetNull(root, val.attrname().c_str());
                    break;
                case AttributeType::Integer:
                    OCRepPayloadSetPropInt(root, val.attrname().c_str(), static_cast<int>(val));
                    break;
                case AttributeType::Double:
                    OCRepPayloadSetPropDouble(root, val.attrname().c_str(),
                            val.getValue<double>());
                    break;
                case AttributeType::Boolean:
                    OCRepPayloadSetPropBool(root, val.attrname().c_str(), val.getValue<bool>());
                    break;
                case AttributeType::String:
                    OCRepPayloadSetPropString(root, val.attrname().c_str(),
                            static_cast<std::string>(val).c_str());
                    break;
                case AttributeType::OCRepresentation:
                    OCRepPayloadSetPropObjectAsOwner(root, val.attrname().c_str(),
                            static_cast<OCRepresentation>(val).getPayload());
                    break;
                case AttributeType::Vector:
                    getPayloadArray(root, val);
                    break;
                default:
                    throw std::logic_error(std::string("Getpayload: Not Implemented") +
                            std::to_string((int)val.type()));
                    break;
            }
        }

        return root;
    }

    size_t calcArrayDepth(const size_t dimensions[MAX_REP_ARRAY_DEPTH])
    {
        if(dimensions[0] == 0)
        {
            throw std::logic_error("invalid calcArrayDepth");
        }
        else if(dimensions[1] == 0)
        {
            return 1;
        }
        else if (dimensions[2] == 0)
        {
            return 2;
        }
        else
        {
            return 3;
        }
    }

    template<typename T>
    T OCRepresentation::payload_array_helper_copy(size_t index, const OCRepPayloadValue* pl)
    {
        throw std::logic_error("payload_array_helper_copy: unsupported type");
    }
    template<>
    int OCRepresentation::payload_array_helper_copy<int>(size_t index, const OCRepPayloadValue* pl)
    {
        return pl->arr.iArray[index];
    }
    template<>
    double OCRepresentation::payload_array_helper_copy<double>(size_t index, const OCRepPayloadValue* pl)
    {
        return pl->arr.dArray[index];
    }
    template<>
    bool OCRepresentation::payload_array_helper_copy<bool>(size_t index, const OCRepPayloadValue* pl)
    {
        return pl->arr.bArray[index];
    }
    template<>
    std::string OCRepresentation::payload_array_helper_copy<std::string>(
            size_t index, const OCRepPayloadValue* pl)
    {
        if (pl->arr.strArray[index])
        {
            return std::string(pl->arr.strArray[index]);
        }
        else
        {
            return std::string{};
        }
    }
    template<>
    OCRepresentation OCRepresentation::payload_array_helper_copy<OCRepresentation>(
            size_t index, const OCRepPayloadValue* pl)
    {
        OCRepresentation r;
        if (pl->arr.objArray[index])
        {
            r.setPayload(pl->arr.objArray[index]);
        }
        return r;
    }

    template<typename T>
    void OCRepresentation::payload_array_helper(const OCRepPayloadValue* pl, size_t depth)
    {
        if(depth == 1)
        {
            std::vector<T> val(pl->arr.dimensions[0]);

            for(size_t i = 0; i < pl->arr.dimensions[0]; ++i)
            {
                val[i] = payload_array_helper_copy<T>(i, pl);
            }
            this->setValue(std::string(pl->name), val);
        }
        else if (depth == 2)
        {
            std::vector<std::vector<T>> val(pl->arr.dimensions[0]);
            for(size_t i = 0; i < pl->arr.dimensions[0]; ++i)
            {
                val[i].resize(pl->arr.dimensions[1]);
                for(size_t j = 0; j < pl->arr.dimensions[1]; ++j)
                {
                    val[i][j] = payload_array_helper_copy<T>(
                            i * pl->arr.dimensions[1] + j, pl);
                }
            }
            this->setValue(std::string(pl->name), val);
        }
        else if (depth == 3)
        {
            std::vector<std::vector<std::vector<T>>> val(pl->arr.dimensions[0]);
            for(size_t i = 0; i < pl->arr.dimensions[0]; ++i)
            {
                val[i].resize(pl->arr.dimensions[1]);
                for(size_t j = 0; j < pl->arr.dimensions[1]; ++j)
                {
                    val[i][j].resize(pl->arr.dimensions[2]);
                    for(size_t k = 0; k < pl->arr.dimensions[2]; ++k)
                    {
                        val[i][j][k] = payload_array_helper_copy<T>(
                                pl->arr.dimensions[2] * j +
                                pl->arr.dimensions[2] * pl->arr.dimensions[1] * i +
                                k,
                                pl);
                    }
                }
            }
            this->setValue(std::string(pl->name), val);
        }
        else
        {
            throw std::logic_error("Invalid depth in payload_array_helper");
        }
    }

    void OCRepresentation::setPayloadArray(const OCRepPayloadValue* pl)
    {

        switch(pl->arr.type)
        {
            case OCREP_PROP_INT:
                payload_array_helper<int>(pl, calcArrayDepth(pl->arr.dimensions));
                break;
            case OCREP_PROP_DOUBLE:
                payload_array_helper<double>(pl, calcArrayDepth(pl->arr.dimensions));
                break;
            case OCREP_PROP_BOOL:
                payload_array_helper<bool>(pl, calcArrayDepth(pl->arr.dimensions));
                break;
            case OCREP_PROP_STRING:
                payload_array_helper<std::string>(pl, calcArrayDepth(pl->arr.dimensions));
                break;
            case OCREP_PROP_OBJECT:
                payload_array_helper<OCRepresentation>(pl, calcArrayDepth(pl->arr.dimensions));
                break;
            default:
                throw std::logic_error("setPayload array invalid type");
                break;
        }
    }

    void OCRepresentation::setPayload(const OCRepPayload* pl)
    {
        setUri(pl->uri);

        OCStringLL* ll = pl->types;
        while(ll)
        {
            addResourceType(ll->value);
            ll = ll->next;
        }

        ll = pl->interfaces;
        while(ll)
        {
            addResourceInterface(ll->value);
            ll = ll->next;
        }

        OCRepPayloadValue* val = pl->values;

        while(val)
        {
            switch(val->type)
            {
                case OCREP_PROP_NULL:
                    setNULL(val->name);
                    break;
                case OCREP_PROP_INT:
                    setValue<int>(val->name, val->i);
                    break;
                case OCREP_PROP_DOUBLE:
                    setValue<double>(val->name, val->d);
                    break;
                case OCREP_PROP_BOOL:
                    setValue<bool>(val->name, val->b);
                    break;
                case OCREP_PROP_STRING:
                    setValue<std::string>(val->name, val->str);
                    break;
                case OCREP_PROP_OBJECT:
                    {
                        OCRepresentation cur;
                        cur.setPayload(val->obj);
                        setValue<OCRepresentation>(val->name, cur);
                    }
                    break;
                case OCREP_PROP_ARRAY:
                    setPayloadArray(val);
                    break;
                default:
                    throw std::logic_error(std::string("Not Implemented!") +
                            std::to_string((int)val->type));
                    break;
            }
            val = val->next;
        }
    }

    void OCRepresentation::addChild(const OCRepresentation& rep)
    {
        m_children.push_back(rep);
    }

    void OCRepresentation::clearChildren()
    {
        m_children.clear();
    }

    const std::vector<OCRepresentation>& OCRepresentation::getChildren() const
    {
        return m_children;
    }

    void OCRepresentation::setChildren(const std::vector<OCRepresentation>& children)
    {
        m_children = children;
    }
    void OCRepresentation::setUri(const char* uri)
    {
        m_uri = uri ? uri : "";
    }

    void OCRepresentation::setUri(const std::string& uri)
    {
        m_uri = uri;
    }

    std::string OCRepresentation::getUri() const
    {
        return m_uri;
    }

    const std::vector<std::string>& OCRepresentation::getResourceTypes() const
    {
        return m_resourceTypes;
    }

    void OCRepresentation::setResourceTypes(const std::vector<std::string>& resourceTypes)
    {
        m_resourceTypes = resourceTypes;
    }

    void OCRepresentation::addResourceType(const std::string& str)
    {
        m_resourceTypes.push_back(str);
    }

    const std::vector<std::string>& OCRepresentation::getResourceInterfaces() const
    {
        return m_interfaces;
    }

    void OCRepresentation::addResourceInterface(const std::string& str)
    {
        m_interfaces.push_back(str);
    }

    void OCRepresentation::setResourceInterfaces(const std::vector<std::string>& resourceInterfaces)
    {
        m_interfaces = resourceInterfaces;
    }

    bool OCRepresentation::hasAttribute(const std::string& str) const
    {
        return m_values.find(str) != m_values.end();
    }

    bool OCRepresentation::emptyData() const
    {
        // This logic is meant to determine whether based on the JSON serialization rules
        // if this object will result in empty JSON.  URI is only serialized if there is valid
        // data, ResourceType and Interfaces are only serialized if we are a nothing, a
        // child of a default or link item.
        // Our values array is only printed in the if we are the child of a Batch resource,
        // the parent in a 'default' situation, or not in a child/parent relationship.
        if(!m_uri.empty())
        {
            return false;
        }
        else if ((m_interfaceType == InterfaceType::None
                        || m_interfaceType==InterfaceType::DefaultChild
                        || m_interfaceType==InterfaceType::LinkChild)
                    && (m_resourceTypes.size()>0 || m_interfaces.size()>0))
        {
            return false;
        }
        else if((m_interfaceType == InterfaceType::None
                        || m_interfaceType == InterfaceType::BatchChild
                        || m_interfaceType == InterfaceType::DefaultParent)
                    && m_values.size()>0)
        {
            return false;
        }

        if(m_children.size() > 0)
        {
            return false;
        }

        return true;
    }

    int OCRepresentation::numberOfAttributes() const
    {
        return m_values.size();
    }

    bool OCRepresentation::erase(const std::string& str)
    {
        return m_values.erase(str);
    }

    void OCRepresentation::setNULL(const std::string& str)
    {
        m_values[str] = OC::NullType();
    }

    bool OCRepresentation::isNULL(const std::string& str) const
    {
        auto x = m_values.find(str);

        if(m_values.end() != x)
        {
            return x->second.which() == AttributeValueNullIndex;
        }
        else
        {
            throw OCException(OC::Exception::INVALID_ATTRIBUTE+ str);
        }
    }
}

namespace OC
{
    std::ostream& operator <<(std::ostream& os, const AttributeType at)
    {
        switch(at)
        {
            case AttributeType::Null:
                os << "Null";
                break;
            case AttributeType::Integer:
                os << "Integer";
                break;
            case AttributeType::Double:
                os << "Double";
                break;
            case AttributeType::Boolean:
                os << "Boolean";
                break;
            case AttributeType::String:
                os << "String";
                break;
            case AttributeType::OCRepresentation:
                os << "OCRepresentation";
                break;
            case AttributeType::Vector:
                os << "Vector";
                break;
        }
        return os;
    }
}

// STL Container For OCRepresentation
namespace OC
{
    OCRepresentation::AttributeItem::AttributeItem(const std::string& name,
            std::map<std::string, AttributeValue>& vals):
            m_attrName(name), m_values(vals){}

    OCRepresentation::AttributeItem OCRepresentation::operator[](const std::string& key)
    {
        OCRepresentation::AttributeItem attr{key, m_values};
        return std::move(attr);
    }

    const OCRepresentation::AttributeItem OCRepresentation::operator[](const std::string& key) const
    {
        OCRepresentation::AttributeItem attr{key, m_values};
        return std::move(attr);
    }

    const std::string& OCRepresentation::AttributeItem::attrname() const
    {
        return m_attrName;
    }

    template<typename T, typename = void>
    struct type_info
    {
        // contains the actual type
        typedef T type;
        // contains the inner most vector-type
        typedef T base_type;
        // contains the AttributeType for this item
        constexpr static AttributeType enum_type =
            AttributeTypeConvert<T>::type;
        // contains the AttributeType for this base-type
        constexpr static AttributeType enum_base_type =
            AttributeTypeConvert<T>::type;
        // depth of the vector
        constexpr static size_t depth = 0;
    };

    template<typename T>
    struct type_info<T, typename std::enable_if<is_vector<T>::value>::type>
    {
        typedef T type;
        typedef typename type_info<typename T::value_type>::base_type base_type;
        constexpr static AttributeType enum_type = AttributeType::Vector;
        constexpr static AttributeType enum_base_type =
            type_info<typename T::value_type>::enum_base_type;
        constexpr static size_t depth = 1 +
            type_info<typename T::value_type>::depth;
    };

    struct type_introspection_visitor : boost::static_visitor<>
    {
        AttributeType type;
        AttributeType base_type;
        size_t depth;

        type_introspection_visitor() : boost::static_visitor<>(),
            type(AttributeType::Null), base_type(AttributeType::Null), depth(0){}

        template <typename T>
        void operator()(T const& /*item*/)
        {
            type = type_info<T>::enum_type;
            base_type = type_info<T>::enum_base_type;
            depth = type_info<T>::depth;
        }
    };

    AttributeType OCRepresentation::AttributeItem::type() const
    {
        type_introspection_visitor vis;
        boost::apply_visitor(vis, m_values[m_attrName]);
        return vis.type;
    }

    AttributeType OCRepresentation::AttributeItem::base_type() const
    {
        type_introspection_visitor vis;
        boost::apply_visitor(vis, m_values[m_attrName]);
        return vis.base_type;
    }

    size_t OCRepresentation::AttributeItem::depth() const
    {
        type_introspection_visitor vis;
        boost::apply_visitor(vis, m_values[m_attrName]);
        return vis.depth;
    }

    OCRepresentation::iterator OCRepresentation::begin()
    {
        return OCRepresentation::iterator(m_values.begin(), m_values);
    }

    OCRepresentation::const_iterator OCRepresentation::begin() const
    {
         return OCRepresentation::const_iterator(m_values.begin(), m_values);
    }

    OCRepresentation::const_iterator OCRepresentation::cbegin() const
    {
        return OCRepresentation::const_iterator(m_values.cbegin(), m_values);
    }

    OCRepresentation::iterator OCRepresentation::end()
    {
        return OCRepresentation::iterator(m_values.end(), m_values);
    }

    OCRepresentation::const_iterator OCRepresentation::end() const
    {
        return OCRepresentation::const_iterator(m_values.end(), m_values);
    }

    OCRepresentation::const_iterator OCRepresentation::cend() const
    {
        return OCRepresentation::const_iterator(m_values.cend(), m_values);
    }

    size_t OCRepresentation::size() const
    {
        return m_values.size();
    }

    bool OCRepresentation::empty() const
    {
        return m_values.empty();
    }

    bool OCRepresentation::iterator::operator==(const OCRepresentation::iterator& rhs) const
    {
        return m_iterator == rhs.m_iterator;
    }

    bool OCRepresentation::iterator::operator!=(const OCRepresentation::iterator& rhs) const
    {
        return m_iterator != rhs.m_iterator;
    }

    bool OCRepresentation::const_iterator::operator==(
            const OCRepresentation::const_iterator& rhs) const
    {
        return m_iterator == rhs.m_iterator;
    }

    bool OCRepresentation::const_iterator::operator!=(
            const OCRepresentation::const_iterator& rhs) const
    {
        return m_iterator != rhs.m_iterator;
    }

    OCRepresentation::iterator::reference OCRepresentation::iterator::operator*()
    {
        return m_item;
    }

    OCRepresentation::const_iterator::const_reference
        OCRepresentation::const_iterator::operator*() const
    {
        return m_item;
    }

    OCRepresentation::iterator::pointer OCRepresentation::iterator::operator->()
    {
        return &m_item;
    }

    OCRepresentation::const_iterator::const_pointer
        OCRepresentation::const_iterator::operator->() const
    {
        return &m_item;
    }

    OCRepresentation::iterator& OCRepresentation::iterator::operator++()
    {
        m_iterator++;
        if(m_iterator != m_item.m_values.end())
        {
            m_item.m_attrName = m_iterator->first;
        }
        else
        {
            m_item.m_attrName = "";
        }
        return *this;
    }

    OCRepresentation::const_iterator& OCRepresentation::const_iterator::operator++()
    {
        m_iterator++;
        if(m_iterator != m_item.m_values.end())
        {
            m_item.m_attrName = m_iterator->first;
        }
        else
        {
            m_item.m_attrName = "";
        }
        return *this;
    }

    OCRepresentation::iterator OCRepresentation::iterator::operator++(int)
    {
        OCRepresentation::iterator itr(*this);
        ++(*this);
        return itr;
    }

    OCRepresentation::const_iterator OCRepresentation::const_iterator::operator++(int)
    {
        OCRepresentation::const_iterator itr(*this);
        ++(*this);
        return itr;
    }

    struct to_string_visitor : boost::static_visitor<>
    {
        std::string str;
        template <typename T>
        void operator()(T const& item)
        {
            str = boost::lexical_cast<std::string>(item);
        }

        template <typename T>
        void operator()(std::vector<T> const& item)
        {
            to_string_visitor vis;
            std::ostringstream stream;
            stream << "[";

            for(const auto& i : item)
            {
                vis(i);
                stream << vis.str  << " ";
            }
            stream << "]";
            str = stream.str();
        }
    };

    template<>
    void to_string_visitor::operator()(bool const& item)
    {
        str = item ? "true" : "false";
    }

    template<>
    void to_string_visitor::operator()(std::string const& item)
    {
        str = item;
    }

    template<>
    void to_string_visitor::operator()(NullType const& /*item*/)
    {
        str = "(null)";
    }

    template<>
    void to_string_visitor::operator()(OCRepresentation const& /*item*/)
    {
        str = "OC::OCRepresentation";
    }

    std::string OCRepresentation::getValueToString(const std::string& key) const
    {
        auto x = m_values.find(key);
        if(x != m_values.end())
        {
            to_string_visitor vis;
            boost::apply_visitor(vis, x->second);
            return std::move(vis.str);
        }

        return "";
    }

    std::string OCRepresentation::AttributeItem::getValueToString() const
    {
        to_string_visitor vis;
        boost::apply_visitor(vis, m_values[m_attrName]);
        return std::move(vis.str);
    }

    std::ostream& operator<<(std::ostream& os, const OCRepresentation::AttributeItem& ai)
    {
        os << ai.getValueToString();
        return os;
    }
}

