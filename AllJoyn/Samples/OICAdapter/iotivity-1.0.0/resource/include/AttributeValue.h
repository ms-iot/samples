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
 * This file contains the definition of the internally used type
 * AttributeValue.
 */

#ifndef __ATTRIBUTEVALUE_H
#define __ATTRIBUTEVALUE_H

// These defines are required to get the boost::variant to hold more than 20 items.
// documentation requires that you use a power of 10
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30
#include <boost/variant.hpp>
#include <iosfwd>
#include <OCUtilities.h>
namespace OC
{
    class OCRepresentation;

    struct NullType{};

    // Since null needs to be encoded in a special fashion in JSON, the encoder
    // needs to know the index of the NullType Sentinel  Any time the code does a special
    // case for the NullType, we use the AttributeValueNullIndex.  This MUST be kept up to date
    // with the variant's which() for NullType.
    static const int AttributeValueNullIndex = 0;
    typedef boost::variant<
        NullType, // Note: this handles the null-type and must match the above static const
        int,
        double,
        bool,
        std::string,
        OC::OCRepresentation,

        // Sequences:
        std::vector<int>,
        std::vector<double>,
        std::vector<bool>,
        std::vector<std::string>,
        std::vector<OC::OCRepresentation>,

        // Nested sequences:
        std::vector<std::vector<int>>,
        std::vector<std::vector<std::vector<int>>>,

        std::vector<std::vector<double>>,
        std::vector<std::vector<std::vector<double>>>,

        std::vector<std::vector<bool>>,
        std::vector<std::vector<std::vector<bool>>>,

        std::vector<std::vector<std::string>>,
        std::vector<std::vector<std::vector<std::string>>>,

        std::vector<std::vector<OC::OCRepresentation>>,
        std::vector<std::vector<std::vector<OC::OCRepresentation>>>
    > AttributeValue;

    enum class AttributeType
    {
        Null,
        Integer,
        Double,
        Boolean,
        String,
        OCRepresentation,
        Vector
    };

    template<typename T>
    struct AttributeTypeConvert{};

    template<>
    struct AttributeTypeConvert<NullType>
    {
        constexpr static AttributeType type = AttributeType::Null;
    };

    template<>
    struct AttributeTypeConvert<int>
    {
        constexpr static AttributeType type = AttributeType::Integer;
    };

    template<>
    struct AttributeTypeConvert<double>
    {
        constexpr static AttributeType type = AttributeType::Double;
    };

    template<>
    struct AttributeTypeConvert<bool>
    {
        constexpr static AttributeType type = AttributeType::Boolean;
    };

    template<>
    struct AttributeTypeConvert<std::string>
    {
        constexpr static AttributeType type = AttributeType::String;
    };

    template<>
    struct AttributeTypeConvert<OCRepresentation>
    {
        constexpr static AttributeType type = AttributeType::OCRepresentation;
    };

    std::ostream& operator << (std::ostream& os, const AttributeType at);
}
#endif // __ATTRIBUTEVALUE_H
