//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

#include <RCSResourceAttributes.h>

#include <ResourceAttributesUtils.h>
#include <ResourceAttributesConverter.h>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/advance.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/deref.hpp>

namespace
{

    using namespace OIC::Service;

    class ToStringVisitor: public boost::static_visitor< std::string >
    {
    public:
        ToStringVisitor() = default;
        ToStringVisitor(const ToStringVisitor&) = delete;
        ToStringVisitor(ToStringVisitor&&) = delete;

        ToStringVisitor& operator=(const ToStringVisitor&) = delete;
        ToStringVisitor& operator=(ToStringVisitor&&) = delete;

        template < typename T >
        std::string operator()(const T& value) const
        {
            return boost::lexical_cast<std::string>(value);
        }

        template< typename T >
        std::string operator()(const std::vector< T >&) const
        {
            return "Vector";
        }

        std::string operator()(std::nullptr_t) const
        {
            return "";
        }

        std::string operator()(bool value) const
        {
            return value ? "true" : "false";
        }

        std::string operator()(const std::string& value) const
        {
            return value;
        }

        std::string operator()(const OIC::Service::RCSResourceAttributes&) const
        {
            return "Attributes";
        }
    };

    class TypeVisitor: public boost::static_visitor< RCSResourceAttributes::Type >
    {
    public:
        TypeVisitor() = default;
        TypeVisitor(const TypeVisitor&) = delete;
        TypeVisitor(TypeVisitor&&) = delete;

        TypeVisitor& operator=(const TypeVisitor&) = delete;
        TypeVisitor& operator=(TypeVisitor&&) = delete;

        template< typename T >
        RCSResourceAttributes::Type operator()(const T& value) const
        {
            return RCSResourceAttributes::Type::typeOf(value);
        }

    };

    template< int >
    struct Int2Type {};

    template< typename T >
    struct TypeInfoConverter;

    template< >
    struct TypeInfoConverter< int >
    {
        static constexpr RCSResourceAttributes::TypeId typeId = RCSResourceAttributes::TypeId::INT;
    };

    template< >
    struct TypeInfoConverter< std::nullptr_t >
    {
        static constexpr RCSResourceAttributes::TypeId typeId =
                RCSResourceAttributes::TypeId::NULL_T;
    };

    template< >
    struct TypeInfoConverter< double >
    {
        static constexpr RCSResourceAttributes::TypeId typeId =
                RCSResourceAttributes::TypeId::DOUBLE;
    };

    template< >
    struct TypeInfoConverter< bool >
    {
        static constexpr RCSResourceAttributes::TypeId typeId = RCSResourceAttributes::TypeId::BOOL;
    };

    template< >
    struct TypeInfoConverter< std::string >
    {
        static constexpr RCSResourceAttributes::TypeId typeId =
                RCSResourceAttributes::TypeId::STRING;
    };

    template< >
    struct TypeInfoConverter< RCSResourceAttributes >
    {
        static constexpr RCSResourceAttributes::TypeId typeId =
                RCSResourceAttributes::TypeId::ATTRIBUTES;
    };

    template< typename T >
    struct TypeInfoConverter< std::vector< T > >
    {
        static constexpr RCSResourceAttributes::TypeId typeId =
                RCSResourceAttributes::TypeId::VECTOR;
    };

    template< typename T >
    struct SequenceTraits
    {
        static constexpr size_t depth = 0;
        typedef T base_type;
    };

    template< typename T >
    struct SequenceTraits< std::vector< T > >
    {
        static constexpr size_t depth = SequenceTraits< T >::depth + 1;
        typedef typename SequenceTraits< T >::base_type base_type;
    };

    struct TypeInfo
    {
        RCSResourceAttributes::TypeId m_typeId;
        RCSResourceAttributes::Type m_baseType;
        size_t m_depth;

        template< typename T, typename ST = SequenceTraits < T > >
        constexpr static TypeInfo get()
        {
            return { TypeInfoConverter< T >::typeId ,
                    RCSResourceAttributes::Type::typeOf< typename ST::base_type >(), ST::depth };
        }

        template< typename VARIANT, int POS >
        constexpr static TypeInfo get()
        {
            typedef typename boost::mpl::begin< typename VARIANT::types >::type mpl_begin;
            typedef typename boost::mpl::advance< mpl_begin, boost::mpl::int_< POS > >::type iter;

            return get< typename boost::mpl::deref< iter >::type >();
        }
    };

    template< typename VARIANT, int POS >
    constexpr inline std::vector< TypeInfo > getTypeInfo(Int2Type< POS >) noexcept
    {
        auto vec = getTypeInfo< VARIANT >(Int2Type< POS - 1 >{ });
        vec.push_back(TypeInfo::get< VARIANT, POS >());
        return vec;
    }

    template< typename VARIANT >
    constexpr inline std::vector< TypeInfo > getTypeInfo(Int2Type< 0 >) noexcept
    {
        return { TypeInfo::get< VARIANT, 0 >() };
    }

    template< typename VARIANT >
    inline TypeInfo getTypeInfo(int which) noexcept
    {
        static constexpr size_t variantSize = boost::mpl::size< typename VARIANT::types >::value;
        static constexpr size_t variantEnd = variantSize - 1;
        static const std::vector< TypeInfo > typeInfos = getTypeInfo< VARIANT >(
                Int2Type< variantEnd >{ });

        static_assert(variantSize > 0, "Variant has no type!");

        return typeInfos[which];
    }

} // unnamed namespace


namespace OIC
{
    namespace Service
    {

        RCSResourceAttributes::Value::ComparisonHelper::ComparisonHelper(const Value& v) :
                m_valueRef(v)
        {
        }

        bool RCSResourceAttributes::Value::ComparisonHelper::operator==
                (const Value::ComparisonHelper& rhs) const
        {
            return *m_valueRef.m_data == *rhs.m_valueRef.m_data;
        }

        bool operator==(const RCSResourceAttributes::Type& lhs,
                const RCSResourceAttributes::Type& rhs) noexcept
        {
            return lhs.m_which == rhs.m_which;
        }

        bool operator!=(const RCSResourceAttributes::Type& lhs,
                const RCSResourceAttributes::Type& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        bool operator==(const RCSResourceAttributes::Value::ComparisonHelper& lhs,
                const RCSResourceAttributes::Value::ComparisonHelper& rhs)
        {
            return lhs.operator==(rhs);
        }

        bool operator!=(const RCSResourceAttributes::Value::ComparisonHelper& lhs,
                const RCSResourceAttributes::Value::ComparisonHelper& rhs)
        {
            return !lhs.operator==(rhs);
        }

        bool operator==(const RCSResourceAttributes& lhs, const RCSResourceAttributes& rhs)
        {
            return lhs.m_values == rhs.m_values;
        }

        bool operator!=(const RCSResourceAttributes& lhs, const RCSResourceAttributes& rhs)
        {
            return !(lhs == rhs);
        }

        auto RCSResourceAttributes::Type::getId() const noexcept -> TypeId
        {
            return ::getTypeInfo< ValueVariant >(m_which).m_typeId;
        }

        auto RCSResourceAttributes::Type::getBaseTypeId(const Type& t) noexcept -> TypeId
        {
            return ::getTypeInfo< ValueVariant >(t.m_which).m_baseType.getId();
        }

        size_t RCSResourceAttributes::Type::getDepth(const Type& t) noexcept
        {
            return ::getTypeInfo< ValueVariant >(t.m_which).m_depth;
        }


        RCSResourceAttributes::Value::Value() :
                m_data{ new ValueVariant{} }
        {
        }

        RCSResourceAttributes::Value::Value(const Value& from) :
                m_data{ new ValueVariant{ *from.m_data } }
        {
        }

        RCSResourceAttributes::Value::Value(Value&& from) noexcept :
                m_data{ new ValueVariant{} }
        {
            m_data.swap(from.m_data);
        }

        RCSResourceAttributes::Value::Value(const char* value) :
                m_data{ new ValueVariant{ std::string{ value } } }
        {
        }

        auto RCSResourceAttributes::Value::operator=(const Value& rhs) -> Value&
        {
            *m_data = *rhs.m_data;
            return *this;
        }

        auto RCSResourceAttributes::Value::operator=(Value&& rhs) -> Value&
        {
            *m_data = ValueVariant{};
            m_data->swap(*rhs.m_data);
            return *this;
        }

        auto RCSResourceAttributes::Value::operator=(const char* rhs) -> Value&
        {
            *m_data = std::string{ rhs };
            return *this;
        }

        auto RCSResourceAttributes::Value::operator=(std::nullptr_t) -> Value&
        {
            *m_data = nullptr;
            return *this;
        }

        auto RCSResourceAttributes::Value::getType() const -> Type
        {
            return boost::apply_visitor(TypeVisitor(), *m_data);
        }

        std::string RCSResourceAttributes::Value::toString() const
        {
            return boost::apply_visitor(ToStringVisitor(), *m_data);
        }

        void RCSResourceAttributes::Value::swap(Value& rhs) noexcept
        {
            m_data.swap(rhs.m_data);
        }

        auto RCSResourceAttributes::KeyValuePair::KeyVisitor::operator()(
                iterator* iter) const noexcept -> result_type
        {
            return iter->m_cur->first;
        }

        auto RCSResourceAttributes::KeyValuePair::KeyVisitor::operator()(
                const_iterator* iter) const noexcept -> result_type
        {
            return iter->m_cur->first;
        }

        auto RCSResourceAttributes::KeyValuePair::ValueVisitor::operator() (iterator* iter) noexcept
                -> result_type
        {
            return iter->m_cur->second;
        }

        auto RCSResourceAttributes::KeyValuePair::ValueVisitor::operator() (const_iterator*)
                -> result_type
        {
            // should not reach here.
            throw RCSBadGetException("");
        }

        auto RCSResourceAttributes::KeyValuePair::ConstValueVisitor::operator()(
                iterator*iter) const noexcept -> result_type
        {
            return iter->m_cur->second;
        }

        auto RCSResourceAttributes::KeyValuePair::ConstValueVisitor::operator()(
                const_iterator* iter) const noexcept -> result_type
        {
            return iter->m_cur->second;
        }

        auto RCSResourceAttributes::KeyValuePair::key() const noexcept -> const std::string&
        {
            return boost::apply_visitor(m_keyVisitor, m_iterRef);
        }

        auto RCSResourceAttributes::KeyValuePair::value() const noexcept -> const Value&
        {
            return boost::apply_visitor(m_constValueVisitor, m_iterRef);
        }

        auto RCSResourceAttributes::KeyValuePair::value() -> Value&
        {
            return boost::apply_visitor(m_valueVisitor, m_iterRef);
        }

        RCSResourceAttributes::KeyValuePair::KeyValuePair(boost::variant<iterator*,
                const_iterator*>&& ref) noexcept :
                m_iterRef{ ref }
        {
        }


        RCSResourceAttributes::iterator::iterator() :
                m_cur{ base_iterator{ } },
                m_keyValuePair{ this }
        {
        }

        RCSResourceAttributes::iterator::iterator(base_iterator&& iter) :
                m_cur{ std::move(iter) },
                m_keyValuePair{ this }
        {
        }

        auto RCSResourceAttributes::iterator::operator*() -> KeyValuePair&
        {
            return m_keyValuePair;
        }

        auto RCSResourceAttributes::iterator::iterator::operator->() -> KeyValuePair*
        {
            return &m_keyValuePair;
        }

        auto RCSResourceAttributes::iterator::operator++() -> iterator&
        {
            ++m_cur;
            return *this;
        }

        auto RCSResourceAttributes::iterator::operator++(int) -> iterator
        {
            iterator iter(*this);
            ++(*this);
            return iter;
        }

        bool RCSResourceAttributes::iterator::operator==(const iterator& rhs) const
        {
            return m_cur == rhs.m_cur;
        }

        bool RCSResourceAttributes::iterator::operator!=(const iterator& rhs) const
        {
            return !(*this == rhs);
        }


        RCSResourceAttributes::const_iterator::const_iterator() :
                m_cur{ base_iterator{} }, m_keyValuePair{ this }
        {
        }

        RCSResourceAttributes::const_iterator::const_iterator(base_iterator&& iter) :
                m_cur{ iter }, m_keyValuePair{ this }
        {
        }

        RCSResourceAttributes::const_iterator::const_iterator(
                const RCSResourceAttributes::iterator& iter) :
                m_cur{ iter.m_cur }, m_keyValuePair{ this }
        {
        }

        auto RCSResourceAttributes::const_iterator::operator=(
                const RCSResourceAttributes::iterator& iter) -> const_iterator&
        {
            m_cur = iter.m_cur;
            return *this;
        }

        auto RCSResourceAttributes::const_iterator::operator*() const -> reference
        {
            return m_keyValuePair;
        }

        auto RCSResourceAttributes::const_iterator::operator->() const -> pointer
        {
            return &m_keyValuePair;
        }

        auto RCSResourceAttributes::const_iterator::operator++() -> const_iterator&
        {
            ++m_cur;
            return *this;
        }

        auto RCSResourceAttributes::const_iterator::operator++(int) -> const_iterator
        {
            const_iterator iter(*this);
            ++(*this);
            return iter;
        }

        bool RCSResourceAttributes::const_iterator::operator==(const const_iterator& rhs) const
        {
            return m_cur == rhs.m_cur;
        }

        bool RCSResourceAttributes::const_iterator::operator!=(const const_iterator& rhs) const
        {
            return !(*this == rhs);
        }


        auto RCSResourceAttributes::begin() noexcept -> iterator
        {
            return iterator{ m_values.begin() };
        }

        auto RCSResourceAttributes::end() noexcept -> iterator
        {
            return iterator{ m_values.end() };
        }

        auto RCSResourceAttributes::begin() const noexcept -> const_iterator
        {
            return const_iterator{ m_values.begin() };
        }

        auto RCSResourceAttributes::end() const noexcept -> const_iterator
        {
            return const_iterator{ m_values.end() };
        }

        auto RCSResourceAttributes::cbegin() const noexcept -> const_iterator
        {
            return const_iterator{ m_values.begin() };
        }

        auto RCSResourceAttributes::cend() const noexcept -> const_iterator
        {
            return const_iterator{ m_values.end() };
        }

        auto RCSResourceAttributes::operator[](const std::string& key) -> Value&
        {
            return m_values[key];
        }

        auto RCSResourceAttributes::operator[](std::string&& key) -> Value&
        {
            return m_values[std::move(key)];
        }

        auto RCSResourceAttributes::at(const std::string& key) -> Value&
        {
            try
            {
                return m_values.at(key);
            }
            catch (const std::out_of_range&)
            {
                throw RCSInvalidKeyException{ "No attribute named '" + key + "'" };
            }
        }

        auto RCSResourceAttributes::at(const std::string& key) const -> const Value&
        {
            try
            {
                return m_values.at(key);
            }
            catch (const std::out_of_range&)
            {
                throw RCSInvalidKeyException{ "No attribute named '" + key + "'" };
            }
        }

        void RCSResourceAttributes::clear() noexcept
        {
            return m_values.clear();
        }

        bool RCSResourceAttributes::erase(const std::string& key)
        {
            return m_values.erase(key) == 1U;
        }

        bool RCSResourceAttributes::contains(const std::string& key) const
        {
            return m_values.find(key) != m_values.end();
        }

        bool RCSResourceAttributes::empty() const noexcept
        {
            return m_values.empty();
        }

        size_t RCSResourceAttributes::size() const noexcept
        {
            return m_values.size();
        }


        bool acceptableAttributeValue(const RCSResourceAttributes::Value& dest,
                const RCSResourceAttributes::Value& value)
        {
            if (dest.getType() != value.getType())
            {
                return false;
            }

            static_assert(RCSResourceAttributes::is_supported_type< RCSResourceAttributes >::value,
                    "RCSResourceAttributes doesn't have RCSResourceAttributes recursively.");
            if (dest.getType().getId() == RCSResourceAttributes::TypeId::ATTRIBUTES
                    && !acceptableAttributes(dest.get< RCSResourceAttributes >(),
                            value.get< RCSResourceAttributes >()))
            {
                return false;
            }

            return true;
        }

        bool acceptableAttributes(const RCSResourceAttributes& dest, const RCSResourceAttributes& attr)
        {
            for (const auto& kv : attr)
            {
                if (!dest.contains(kv.key()))
                {
                    return false;
                }

                if (!acceptableAttributeValue(dest.at(kv.key()), kv.value()))
                {
                    return false;
                }
            }

            return true;
        }

        AttrKeyValuePairs replaceAttributes(RCSResourceAttributes& dest,
                const RCSResourceAttributes& newAttrs)
        {
            AttrKeyValuePairs replacedList;

            for (const auto& kv : newAttrs)
            {
                if (dest[kv.key()] != kv.value())
                {
                    RCSResourceAttributes::Value replacedValue;
                    replacedValue.swap(dest[kv.key()]);
                    dest[kv.key()] = kv.value();

                    replacedList.push_back(AttrKeyValuePair{ kv.key(), std::move(replacedValue) });
                }
            }

            return replacedList;
        }

    }
}
