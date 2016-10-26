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
 * This file contains the declaration of classes and its members related
 * to OCRepresentation.
 */

#ifndef __OCREPRESENTATION_H
#define __OCREPRESENTATION_H


#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <AttributeValue.h>
#include <StringConstants.h>

#ifdef __ANDROID__
#include "OCAndroid.h"
#endif

#include <OCException.h>

namespace OC
{

    enum class InterfaceType
    {
        None,
        LinkParent,
        BatchParent,
        DefaultParent,
        LinkChild,
        BatchChild,
        DefaultChild
    };

    class MessageContainer
    {
        public:
            void setPayload(const OCPayload* rep);

            void setPayload(const OCDevicePayload* rep);

            void setPayload(const OCPlatformPayload* rep);

            void setPayload(const OCRepPayload* rep);

            OCRepPayload* getPayload() const;

            const std::vector<OCRepresentation>& representations() const;

            void addRepresentation(const OCRepresentation& rep);

            const OCRepresentation& operator[](int index) const
            {
                return m_reps[index];
            }

            const OCRepresentation& back() const
            {
                return m_reps.back();
            }
        private:
            std::vector<OCRepresentation> m_reps;
    };
    class OCRepresentation
    {
        public:
            friend bool operator==(const OC::OCRepresentation&, const OC::OCRepresentation&);
            // Note: Implementation of all constructors and destructors
            // are all placed in the same location due to a crash that
            // was observed in Android, where merely constructing/destructing
            // an OCRepresentation object was enough to cause an invalid 'free'.
            // It is believed that this is a result of incompatible compiler
            // options between the gradle JNI and armeabi scons build, however
            // this fix will work in the meantime.
            OCRepresentation(): m_interfaceType(InterfaceType::None){}

            OCRepresentation(OCRepresentation&&) = default;

            OCRepresentation(const OCRepresentation&) = default;

            OCRepresentation& operator=(const OCRepresentation&) = default;

            OCRepresentation& operator=(OCRepresentation&&) = default;

            virtual ~OCRepresentation(){}

            OCRepPayload* getPayload() const;

            void addChild(const OCRepresentation&);

            void clearChildren();

            const std::vector<OCRepresentation>& getChildren() const;

            void setChildren(const std::vector<OCRepresentation>& children);

            void setUri(const char* uri);

            void setUri(const std::string& uri);

            std::string getUri() const;

            const std::vector<std::string>& getResourceTypes() const;

            void setResourceTypes(const std::vector<std::string>& resourceTypes);

            void addResourceType(const std::string& str);

            const std::vector<std::string>& getResourceInterfaces() const;

            void setResourceInterfaces(const std::vector<std::string>& resourceInterfaces);

            void addResourceInterface(const std::string& str);

            bool emptyData() const;

            int numberOfAttributes() const;

            bool erase(const std::string& str);

            template <typename T>
            void setValue(const std::string& str, const T& val)
            {
                m_values[str] = val;
            }

            /**
             *  Retrieve the attribute value associated with the supplied name
             *
             *  @param str Name of the attribute
             *  @param val Value of the attribute
             *  @return The getValue method returns true if the attribute was
             *        found in the representation.  Otherwise it returns false.
             */
            template <typename T>
            bool getValue(const std::string& str, T& val) const
            {
                auto x = m_values.find(str);

                if(x!= m_values.end())
                {
                    val = boost::get<T>(x->second);
                    return true;
                }
                else
                {
                    val = T();
                    return false;
                }
            }

            /**
             *  Return the attribute value associated with the supplied name
             *
             *  @param str Name of the attribute
             *  @return When the representation contains the attribute, the
             *       the associated value is returned.  Otherwise, getValue
             *       returns the default contructed value for the type.
             */
            template <typename T>
            T getValue(const std::string& str) const
            {
                T val = T();
                auto x = m_values.find(str);
                if(x != m_values.end())
                {
                    val = boost::get<T>(x->second);
                }
                return val;
            }

	   /**
            *  Retrieve the attributevalue structure associated with the supplied name
            *
            *  @param str Name of the attribute
            *  @param attrValue Attribute Value structure
            *  @return The getAttributeValue method returns true if the attribute was
            *        found in the representation.  Otherwise it returns false.
            */
            bool getAttributeValue(const std::string& str, AttributeValue& attrValue) const
            {
                auto x = m_values.find(str);

                if (x != m_values.end())
                {
                    attrValue = x->second;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            std::string getValueToString(const std::string& key) const;
            bool hasAttribute(const std::string& str) const;

            void setNULL(const std::string& str);

            bool isNULL(const std::string& str) const;

            // STL Container stuff
        public:
            class iterator;
            class const_iterator;
            // Shim class to allow iterating and indexing of the OCRepresentation
            // object.
            class AttributeItem
            {
                friend class OCRepresentation;
                friend class iterator;
                friend class const_iterator;
                public:
                    const std::string& attrname() const;
                    AttributeType type() const;
                    AttributeType base_type() const;
                    size_t depth() const;
                    template<typename T>
                    T getValue() const
                    {
                        return boost::get<T>(m_values[m_attrName]);
                    }

                    std::string getValueToString() const;

                    template<typename T>
                    AttributeItem& operator=(T&& rhs)
                    {
                        m_values[m_attrName] = std::forward<T>(rhs);
                        return *this;
                    }

                    AttributeItem& operator=(std::nullptr_t /*rhs*/)
                    {
                        NullType t;
                        m_values[m_attrName] = t;
                        return *this;
                    }

                    // Enable-if required to prevent conversions to alternate types.  This prevents
                    // ambigious conversions in the case where conversions can include a number of
                    // types, such as the string constructor.
                    template<typename T, typename std::enable_if<
                     std::is_same<T, int>::value ||
                     std::is_same<T, double>::value ||
                     std::is_same<T, bool>::value ||
                     std::is_same<T, std::string>::value ||
                     std::is_same<T, OCRepresentation>::value ||
                     std::is_same<T, std::vector<int>>::value ||
                     std::is_same<T, std::vector<std::vector<int>>>::value ||
                     std::is_same<T, std::vector<std::vector<std::vector<int>>>>::value ||
                     std::is_same<T, std::vector<double>>::value ||
                     std::is_same<T, std::vector<std::vector<double>>>::value ||
                     std::is_same<T, std::vector<std::vector<std::vector<double>>>>::value ||
                     std::is_same<T, std::vector<bool>>::value ||
                     std::is_same<T, std::vector<std::vector<bool>>>::value ||
                     std::is_same<T, std::vector<std::vector<std::vector<bool>>>>::value ||
                     std::is_same<T, std::vector<std::string>>::value ||
                     std::is_same<T, std::vector<std::vector<std::string>>>::value ||
                     std::is_same<T, std::vector<std::vector<std::vector<std::string>>>>::value ||
                     std::is_same<T, std::vector<OCRepresentation>>::value ||
                     std::is_same<T, std::vector<std::vector<OCRepresentation>>>::value ||
                     std::is_same<T, std::vector<std::vector<std::vector<OCRepresentation>>>>::value
                     , int>::type = 0// enable_if
                    >
                    operator T() const
                    {
                        return this->getValue<T>();
                    }

                    template<typename T, typename std::enable_if<
                        std::is_same<T, std::nullptr_t>::value
                        , int>::type = 0
                    >
                    operator T() const
                    {
                        this->getValue<NullType>();
                        return nullptr;
                    }

                private:
                    AttributeItem(const std::string& name,
                            std::map<std::string, AttributeValue>& vals);
                    AttributeItem(const AttributeItem&) = default;
                    std::string m_attrName;
                    std::map<std::string, AttributeValue>& m_values;
            };

            // Iterator to allow iteration via STL containers/methods
            class iterator
            {
                friend class OCRepresentation;
                public:
                    typedef iterator self_type;
                    typedef AttributeItem value_type;
                    typedef value_type& reference;
                    typedef value_type* pointer;
                    typedef std::forward_iterator_tag iterator_category;
                    typedef int difference_type;

                    iterator(const iterator&) = default;
                    ~iterator() = default;

                    bool operator ==(const iterator&) const;
                    bool operator !=(const iterator&) const;

                    iterator& operator++();
                    iterator operator++(int);

                    reference operator*();
                    pointer operator->();
                private:
                    iterator(std::map<std::string, AttributeValue>::iterator&& itr,
                            std::map<std::string, AttributeValue>& vals)
                        : m_iterator(std::move(itr)),
                        m_item(m_iterator != vals.end() ? m_iterator->first:"", vals){}
                    std::map<std::string, AttributeValue>::iterator m_iterator;
                    AttributeItem m_item;
            };

            class const_iterator
            {
                friend class OCRepresentation;
                public:
                    typedef iterator self_type;
                    typedef const AttributeItem value_type;
                    typedef value_type& const_reference;
                    typedef value_type* const_pointer;
                    typedef std::forward_iterator_tag iterator_category;
                    typedef int difference_type;

                    const_iterator(const iterator& rhs)
                        :m_iterator(rhs.m_iterator), m_item(rhs.m_item){}
                    const_iterator(const const_iterator&) = default;
                    ~const_iterator() = default;

                    bool operator ==(const const_iterator&) const;
                    bool operator !=(const const_iterator&) const;

                    const_iterator& operator++();
                    const_iterator operator++(int);

                    const_reference operator*() const;
                    const_pointer operator->() const;
                private:
                    const_iterator(std::map<std::string, AttributeValue>::const_iterator&& itr,
                            std::map<std::string, AttributeValue>& vals)
                        : m_iterator(std::move(itr)),
                        m_item(m_iterator != vals.end() ? m_iterator->first: "", vals){}
                    std::map<std::string, AttributeValue>::const_iterator m_iterator;
                    AttributeItem m_item;
            };

            iterator begin();
            const_iterator begin() const;
            const_iterator cbegin() const;
            iterator end();
            const_iterator end() const;
            const_iterator cend() const;
            size_t size() const;
            bool empty() const;

            AttributeItem operator[](const std::string& key);
            const AttributeItem operator[](const std::string& key) const;
        private:
            friend class OCResourceResponse;
            friend class MessageContainer;

            template<typename T>
            void payload_array_helper(const OCRepPayloadValue* pl, size_t depth);
            template<typename T>
            T payload_array_helper_copy(size_t index, const OCRepPayloadValue* pl);
            void setPayload(const OCRepPayload* payload);
            void setPayloadArray(const OCRepPayloadValue* pl);
            void getPayloadArray(OCRepPayload* payload,
                    const OCRepresentation::AttributeItem& item) const;
            // the root node has a slightly different JSON version
            // based on the interface type configured in ResourceResponse.
            // This allows ResourceResponse to set it, so that the save function
            // doesn't serialize things that it isn't supposed to serialize.
            void setInterfaceType(InterfaceType ift)
            {
                m_interfaceType = ift;
            }

            // class used to wrap the 'prop' feature of the save/load
            class Prop
            {
                public:
                    Prop(std::vector<std::string>& resourceTypes,
                            std::vector<std::string>& interfaces)
                    : m_types(resourceTypes), m_interfaces(interfaces)
                    {}

                 /*   Prop(const std::vector<std::string>& resourceTypes,
                            const std::vector<std::string>& interfaces)
                    :m_types(resourceTypes),
                    m_interfaces(interfaces)
                    {}*/
                private:
                    std::vector<std::string>& m_types;
                    std::vector<std::string>& m_interfaces;
            };
        private:
            std::string m_uri;
            std::vector<OCRepresentation> m_children;
            mutable std::map<std::string, AttributeValue> m_values;
            std::vector<std::string> m_resourceTypes;
            std::vector<std::string> m_interfaces;

            InterfaceType m_interfaceType;
    };

    std::ostream& operator <<(std::ostream& os, const OCRepresentation::AttributeItem& ai);
} // namespace OC


#endif //__OCREPRESENTATION_H

