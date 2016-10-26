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
#include <ResourceAttributesConverter.h>
#include <ResourceAttributesUtils.h>

#include <gtest/gtest.h>

using namespace testing;
using namespace OIC::Service;

constexpr char KEY[]{ "key" };

class ResourceAttributesTest: public Test
{
public:
    RCSResourceAttributes resourceAttributes;
};

TEST_F(ResourceAttributesTest, InitialSizeIsZero)
{
    ASSERT_EQ(0U, resourceAttributes.size());
    ASSERT_TRUE(resourceAttributes.empty());
}

TEST_F(ResourceAttributesTest, InsertWithSquareBracket)
{
    resourceAttributes[KEY] = 1;

    ASSERT_EQ(resourceAttributes[KEY], 1);
}

TEST_F(ResourceAttributesTest, ValueThrowsIfTypeDoesNotMatch)
{
     resourceAttributes[KEY] = 1;
    auto& valueRef = resourceAttributes[KEY];

    ASSERT_THROW(valueRef.get< std::string >(), RCSBadGetException);
}

TEST_F(ResourceAttributesTest, GettingWithAtThrowsIfThereIsNoMatchedValue)
{
    ASSERT_THROW(resourceAttributes.at(KEY), RCSInvalidKeyException);
}

TEST_F(ResourceAttributesTest, CopyingValueDoesNotShareState)
{
    const char arbitraryStr[] { "ftryb457" };
    resourceAttributes[KEY] = 1;

    RCSResourceAttributes::Value copied { resourceAttributes[KEY] };
    copied = arbitraryStr;

    ASSERT_EQ(resourceAttributes[KEY], 1);
    ASSERT_EQ(copied, arbitraryStr);
}

TEST_F(ResourceAttributesTest, IsNullWhenAssignmentNullptr)
{
    resourceAttributes[KEY] = nullptr;

    ASSERT_EQ(resourceAttributes[KEY], nullptr);
}

TEST_F(ResourceAttributesTest, ValueChangedIfPutWithSameKey)
{
    resourceAttributes[KEY] = "string";
    resourceAttributes[KEY] = true;

    ASSERT_EQ(resourceAttributes[KEY], true);
}

TEST_F(ResourceAttributesTest, ObjectIsEmptyAfterMoved)
{
    resourceAttributes[KEY] = 1;

    RCSResourceAttributes moved{ std::move(resourceAttributes) };

    ASSERT_TRUE(resourceAttributes.empty());
}

TEST_F(ResourceAttributesTest, GettingWithAtThrowsAfterRemoved)
{
    resourceAttributes[KEY] = 1;

    resourceAttributes.erase(KEY);

    ASSERT_THROW(resourceAttributes.at(KEY), RCSInvalidKeyException);
}

TEST_F(ResourceAttributesTest, NoDataErasedIfKeyDoesNotMatch)
{
    ASSERT_FALSE(resourceAttributes.erase(KEY));
}

TEST_F(ResourceAttributesTest, ChangeValueWithAtGetter)
{
    resourceAttributes[KEY] = 1;

    resourceAttributes.at(KEY) = "after";

    ASSERT_EQ(resourceAttributes[KEY], "after");
}

TEST_F(ResourceAttributesTest, CanHaveNestedResourceAttributes)
{
    constexpr char nestedKey[]{ "nested" };
    constexpr char value[]{ "nested_value" };

    RCSResourceAttributes nested;
    nested[nestedKey] = value;
    resourceAttributes[KEY] = nested;

    ASSERT_EQ(value, resourceAttributes[KEY].get<RCSResourceAttributes>()[nestedKey]);
}

TEST_F(ResourceAttributesTest, ToStringReturnsStringForValue)
{
    resourceAttributes[KEY] = true;

    ASSERT_EQ("true", resourceAttributes[KEY].toString());
}

TEST_F(ResourceAttributesTest, ToStringReturnsEmptyStringForNullValue)
{
    resourceAttributes[KEY] = nullptr;

    ASSERT_EQ("", resourceAttributes[KEY].toString());
}


class ResourceAttributesIteratorTest: public Test
{
public:
    RCSResourceAttributes resourceAttributes;
};

TEST_F(ResourceAttributesIteratorTest, BeginEqualsEndWhenEmpty)
{
    ASSERT_TRUE(resourceAttributes.begin() == resourceAttributes.end());
}

TEST_F(ResourceAttributesIteratorTest, CanIteratesWithForeach)
{
    resourceAttributes["first"] = 1;
    resourceAttributes["second"] = 2;

    int count = 0;

    for (auto& i : resourceAttributes) {
        i.key();
        ++count;
    }

    ASSERT_EQ(2, count);
}

TEST_F(ResourceAttributesIteratorTest, IteratesWithRef)
{
    const char arbitraryStr[] { "ftryb457" };
    resourceAttributes[KEY] = 1;

    for (auto& i : resourceAttributes) {
        i.value() = arbitraryStr;
    }

    ASSERT_EQ(resourceAttributes[KEY], arbitraryStr);
}

TEST_F(ResourceAttributesIteratorTest, IteratorIsCopyable)
{
    RCSResourceAttributes::iterator it;

    it = resourceAttributes.begin();

    ASSERT_TRUE(it == resourceAttributes.begin());
}

TEST_F(ResourceAttributesIteratorTest, IteratorIndicateNextItemAfterIncreased)
{
    resourceAttributes[KEY] = 1;

    RCSResourceAttributes::iterator it = resourceAttributes.begin();

    it++;

    ASSERT_TRUE(it == resourceAttributes.end());
}

TEST_F(ResourceAttributesIteratorTest, IteratorCanBeConvertedIntoConstIterator)
{
    resourceAttributes[KEY] = 1;
    RCSResourceAttributes::const_iterator it { resourceAttributes.begin() };
    it = resourceAttributes.cbegin();

    it++;

    ASSERT_TRUE(it == resourceAttributes.cend());
}

TEST_F(ResourceAttributesIteratorTest, ConstIteratorIsUsedForConst)
{
    resourceAttributes[KEY] = 1;
    const RCSResourceAttributes& constAttrs = resourceAttributes;

    auto iter = constAttrs.begin();

    ASSERT_TRUE((std::is_same<decltype(iter), RCSResourceAttributes::const_iterator>::value));
}


TEST(ResourceAttributesValueTest, MovedValueHasNull)
{
    RCSResourceAttributes::Value one { 1 };
    RCSResourceAttributes::Value another { std::move(one) };

    ASSERT_EQ(nullptr, one);
}

TEST(ResourceAttributesValueTest, MovedValueWithAssignmentHasNull)
{
    RCSResourceAttributes::Value one { 1 };
    RCSResourceAttributes::Value another;

    another = std::move(one);

    ASSERT_EQ(nullptr, one);
}

TEST(ResourceAttributesValueTest, SameValuesAreEqual)
{
    RCSResourceAttributes::Value one { 1 };
    RCSResourceAttributes::Value another { 1 };

    ASSERT_EQ(one, another);
}

TEST(ResourceAttributesValueTest, DifferentValuesAreNotEqual)
{
    RCSResourceAttributes::Value one { 1 };
    RCSResourceAttributes::Value another { 2 };

    ASSERT_NE(one, another);
}

TEST(ResourceAttributesValueTest, ValuesCanBeSwapped)
{
    constexpr int i { 0 };
    constexpr char str[]{ "abc" };

    RCSResourceAttributes::Value intValue { i };
    RCSResourceAttributes::Value strValue { str };

    intValue.swap(strValue);

    ASSERT_EQ(str, intValue);
    ASSERT_EQ(i, strValue);
}

TEST(ResourceAttributesTypeTest, TypeIdMatchesTypeOfValue)
{
    RCSResourceAttributes::Value intValue { 0 };

    ASSERT_EQ(intValue.getType().getId(), RCSResourceAttributes::TypeId::INT);
}

TEST(ResourceAttributesTypeTest, TypeCanBeConstructedFromValue)
{
    RCSResourceAttributes::Value intValue { 1 };

    RCSResourceAttributes::Type t = RCSResourceAttributes::Type::typeOf(0);

    ASSERT_EQ(intValue.getType(), t);
}

TEST(ResourceAttributesTypeTest, DepthOfNonSequceTypeIsZero)
{
    RCSResourceAttributes::Value intValue { 1 };

    RCSResourceAttributes::Type t = intValue.getType();

    ASSERT_EQ(0U, RCSResourceAttributes::Type::getDepth(t));
}

TEST(ResourceAttributesTypeTest, DepthOfSequceTypeIsNumberOfNested)
{
    typedef std::vector< std::vector< std::vector< int > > > NestedVector;

    RCSResourceAttributes::Type t = RCSResourceAttributes::Type::typeOf(NestedVector{ });

    ASSERT_EQ(3U, RCSResourceAttributes::Type::getDepth(t));
}

TEST(ResourceAttributesTypeTest, BaseTypeOfNonSequceTypeIsItself)
{
    RCSResourceAttributes::Value intValue { 1 };

    RCSResourceAttributes::Type t = intValue.getType();

    ASSERT_EQ(RCSResourceAttributes::TypeId::INT, RCSResourceAttributes::Type::getBaseTypeId(t));
}

TEST(ResourceAttributesTypeTest, BaseTypeOfSequceTypeIsMostNestedType)
{
    typedef std::vector< std::vector< std::vector< RCSResourceAttributes > > > NestedVector;

    RCSResourceAttributes::Type t = RCSResourceAttributes::Type::typeOf(NestedVector{ });

    ASSERT_EQ(RCSResourceAttributes::TypeId::ATTRIBUTES,
            RCSResourceAttributes::Type::getBaseTypeId(t));
}


TEST(ResourceAttributesConverterTest, OCRepresentationCanBeConvertedIntoResourceAttributes)
{
    constexpr double value = 9876;
    OC::OCRepresentation ocRep;
    ocRep[KEY] = value;

    RCSResourceAttributes resourceAttributes{
        ResourceAttributesConverter::fromOCRepresentation(ocRep) };

    ASSERT_TRUE(value == resourceAttributes[KEY]);
}


TEST(ResourceAttributesConverterTest, NestedOCRepresentationCanBeConvertedIntoResourceAttributes)
{
    std::string nested_value { "nested" };
    OC::OCRepresentation ocRep;
    OC::OCRepresentation nested;
    nested[KEY] = nested_value;
    ocRep[KEY] = nested;

    RCSResourceAttributes resourceAttributes{
        ResourceAttributesConverter::fromOCRepresentation(ocRep) };

    ASSERT_EQ(nested_value, resourceAttributes[KEY].get<RCSResourceAttributes>()[KEY]);
}


TEST(ResourceAttributesConverterTest, ResourceAttributesCanBeConvertedIntoOCRepresentation)
{
    double value { 3453453 };
    RCSResourceAttributes resourceAttributes;
    resourceAttributes[KEY] = value;

    OC::OCRepresentation ocRep{
        ResourceAttributesConverter::toOCRepresentation(resourceAttributes) };

    ASSERT_EQ(value, ocRep[KEY].getValue<double>());
}

TEST(ResourceAttributesConverterTest, NestedResourceAttributesCanBeConvertedIntoOCRepresentation)
{
    std::string nested_value { "nested" };
    RCSResourceAttributes resourceAttributes;
    RCSResourceAttributes nested;
    nested[KEY] = nested_value;
    resourceAttributes[KEY] = nested;

    OC::OCRepresentation ocRep{
        ResourceAttributesConverter::toOCRepresentation(resourceAttributes) };

    ASSERT_EQ(nested_value,
            ocRep[KEY].getValue<OC::OCRepresentation>()[KEY].getValue<std::string>());
}

TEST(ResourceAttributesConverterTest, OCRepresentationNullTypeIsNullptrInResourceAttributes)
{
    OC::OCRepresentation ocRep;
    ocRep.setNULL(KEY);

    RCSResourceAttributes resourceAttributes{
        ResourceAttributesConverter::fromOCRepresentation(ocRep) };

    ASSERT_EQ(nullptr, resourceAttributes[KEY]);
}

TEST(ResourceAttributesConverterTest, OCRepresentationHasNullWhenResourceAttributeIsNullptr)
{
    RCSResourceAttributes resourceAttributes;
    resourceAttributes[KEY] = nullptr;

    OC::OCRepresentation ocRep{
        ResourceAttributesConverter::toOCRepresentation(resourceAttributes) };

    ASSERT_TRUE(ocRep.isNULL(KEY));
}

TEST(ResourceAttributesConverterTest, ResourceAttributesWithSequenceTypeCanBeConverted)
{
    typedef std::vector< std::vector< std::vector< int > > > NestedVector;
    constexpr int value { 3453453 };

    RCSResourceAttributes resourceAttributes;
    NestedVector seq(10);
    seq[1].resize(10, std::vector< int >(10));
    seq[1][2][3] = value;
    resourceAttributes[KEY] = seq;

    NestedVector ocSeq = ResourceAttributesConverter::toOCRepresentation(resourceAttributes)[KEY];

    ASSERT_EQ(ocSeq[1][2][3], value);
}

TEST(ResourceAttributesConverterTest, OCRepresentationWithSequenceTypeCanBeConverted)
{
    typedef std::vector< std::vector< std::vector< std::string > > > NestedVector;
    constexpr char value[]{ "some_string" };

    OC::OCRepresentation ocRep;
    NestedVector seq(10);
    seq[1].resize(10, std::vector< std::string >(10));
    seq[1][2][3] = value;
    ocRep[KEY] = seq;

    RCSResourceAttributes resourceAttributes{
        ResourceAttributesConverter::fromOCRepresentation(ocRep) };

    ASSERT_EQ(seq, resourceAttributes[KEY]);
}


class ResourceAttributesUtilTest: public Test
{
public:
    RCSResourceAttributes resourceAttributes;

protected:
    void SetUp()
    {
        resourceAttributes[KEY] = 1;
    }
};

TEST_F(ResourceAttributesUtilTest, EmptyAttributesIsAcceptable)
{
    ASSERT_TRUE(acceptableAttributes(resourceAttributes, RCSResourceAttributes()));
}

TEST_F(ResourceAttributesUtilTest, AttributesItselfIsAcceptable)
{
    ASSERT_TRUE(acceptableAttributes(resourceAttributes, resourceAttributes));
}

TEST_F(ResourceAttributesUtilTest, UnknownKeyIsNotAcceptable)
{
    RCSResourceAttributes newAttrs;
    newAttrs["unknown"] = 1;

    ASSERT_FALSE(acceptableAttributes(resourceAttributes, newAttrs));
}

TEST_F(ResourceAttributesUtilTest, DifferentTypeWithOriginalIsNotAcceptable)
{
    RCSResourceAttributes newAttrs;
    newAttrs[KEY] = "";

    ASSERT_FALSE(acceptableAttributes(resourceAttributes, newAttrs));
}


TEST_F(ResourceAttributesUtilTest, DifferentTypeOfNestedAttributeIsNotAcceptable)
{
    constexpr char KEY_NESTED_ATTR[]{ "nested" };
    constexpr char KEY_NESTED_VALUE[]{ "nested_value" };

    RCSResourceAttributes nested;
    nested[KEY_NESTED_VALUE] = -99;
    resourceAttributes[KEY_NESTED_ATTR] = nested;


    RCSResourceAttributes newAttrs;
    nested[KEY_NESTED_VALUE] = "abc";
    newAttrs[KEY_NESTED_ATTR] = nested;

    ASSERT_FALSE(acceptableAttributes(resourceAttributes, newAttrs));
}

TEST_F(ResourceAttributesUtilTest, ReplaceWillOverwriteOriginal)
{
    constexpr char NEW_VALUE[]{ "newValue" };

    RCSResourceAttributes newAttrs;
    newAttrs[KEY] = NEW_VALUE;

    replaceAttributes(resourceAttributes, newAttrs);

    ASSERT_EQ(NEW_VALUE, resourceAttributes[KEY]);
}
