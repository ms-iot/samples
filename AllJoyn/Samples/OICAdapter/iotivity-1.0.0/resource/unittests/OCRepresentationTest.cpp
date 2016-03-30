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

#include <gtest/gtest.h>
#include <OCApi.h>
#include <string>
#include <limits>
#include <boost/lexical_cast.hpp>
namespace OCRepresentationTest
{
    using namespace OC;
    using std::string;
    using std::vector;

    void parsedEqual(double expected, const std::string& actualStr)
    {
        double actual = boost::lexical_cast<double>(actualStr);
        EXPECT_GE(actual, expected - .0000001);
        EXPECT_LE(actual, expected + .0000001);
    }

    // getValueToString(all types)
    TEST(OCRepresentationValueToString, Null)
    {
        static const std::string AttrName = "NullTest";
        OCRepresentation rep;
        rep.setNULL(AttrName);

        EXPECT_TRUE(rep.isNULL(AttrName));
        EXPECT_EQ("(null)", rep.getValueToString(AttrName));
        EXPECT_EQ("(null)", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, Integer)
    {
        static const std::string AttrName = "IntTest";
        OCRepresentation rep;

        rep.setValue(AttrName, -5);
        EXPECT_EQ("-5", rep.getValueToString(AttrName));
        EXPECT_EQ("-5", rep[AttrName].getValueToString());

        rep.setValue(AttrName, 0);
        EXPECT_EQ("0", rep.getValueToString(AttrName));
        EXPECT_EQ("0", rep[AttrName].getValueToString());

        rep.setValue(AttrName, 5);
        EXPECT_EQ("5", rep.getValueToString(AttrName));
        EXPECT_EQ("5", rep[AttrName].getValueToString());

        rep.setValue(AttrName, 54321);
        EXPECT_EQ("54321", rep.getValueToString(AttrName));
        EXPECT_EQ("54321", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, Double)
    {
        static const std::string AttrName = "DoubleTest";
        OCRepresentation rep;

        rep.setValue(AttrName, -5.0);
        parsedEqual(-5.0, rep.getValueToString(AttrName));
        parsedEqual(-5.0, rep[AttrName].getValueToString());

        rep.setValue(AttrName, 0.0);
        parsedEqual(0.0, rep.getValueToString(AttrName));
        parsedEqual(0.0, rep[AttrName].getValueToString());

        rep.setValue(AttrName, 5.0);
        parsedEqual(5.0, rep.getValueToString(AttrName));
        parsedEqual(5.0, rep[AttrName].getValueToString());

        rep.setValue(AttrName, 54321.0);
        parsedEqual(54321.0, rep.getValueToString(AttrName));
        parsedEqual(54321.0, rep[AttrName].getValueToString());

        rep.setValue(AttrName, 3.55);
        parsedEqual(3.55, rep.getValueToString(AttrName));
        parsedEqual(3.55, rep[AttrName].getValueToString());

        rep.setValue(AttrName, -4.95);
        parsedEqual(-4.95, rep.getValueToString(AttrName));
        parsedEqual(-4.95, rep[AttrName].getValueToString());

        rep.setValue(AttrName, 99999.5555);
        parsedEqual(99999.5555, rep.getValueToString(AttrName));
        parsedEqual(99999.5555, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, Boolean)
    {
        static const std::string AttrName = "BooleanTest";
        OCRepresentation rep;

        rep.setValue(AttrName, false);
        EXPECT_EQ("false", rep.getValueToString(AttrName));
        EXPECT_EQ("false", rep[AttrName].getValueToString());

        rep.setValue(AttrName, true);
        EXPECT_EQ("true", rep.getValueToString(AttrName));
        EXPECT_EQ("true", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, String)
    {
        static const std::string AttrName = "StringTest";
        OCRepresentation rep;

        rep.setValue(AttrName, std::string("test 1"));
        EXPECT_EQ("test 1", rep.getValueToString(AttrName));
        EXPECT_EQ("test 1", rep[AttrName].getValueToString());

        rep.setValue(AttrName, std::string("test 2"));
        EXPECT_EQ("test 2", rep.getValueToString(AttrName));
        EXPECT_EQ("test 2", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, SubRepresentation)
    {
        static const std::string AttrName = "SubRepTest";
        OCRepresentation rep;
        OCRepresentation sub1;
        OCRepresentation sub2;

        rep.setValue(AttrName, sub1);
        EXPECT_EQ("OC::OCRepresentation", rep.getValueToString(AttrName));
        EXPECT_EQ("OC::OCRepresentation", rep[AttrName].getValueToString());

        rep.setValue(AttrName, sub2);
        EXPECT_EQ("OC::OCRepresentation", rep.getValueToString(AttrName));
        EXPECT_EQ("OC::OCRepresentation", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, IntegerVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<int> vect {1,2,3,4,5,6,7,8,9};
        vector<int> vect2 {-5,-3,-1,0,5,3,2};
        rep.setValue(AttrName, vect);
        EXPECT_EQ("[1 2 3 4 5 6 7 8 9 ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[1 2 3 4 5 6 7 8 9 ]", rep[AttrName].getValueToString());

        rep.setValue(AttrName, vect2);
        EXPECT_EQ("[-5 -3 -1 0 5 3 2 ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[-5 -3 -1 0 5 3 2 ]", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, IntegerVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<int> vect1 {1,2,3,4,5,6,7,8,9};
        vector<int> vect2 {-5,-3,-1,0,5,3,2};
        vector<vector<int>> vect{vect1, vect2};

        rep.setValue(AttrName, vect);
        static const string Expected = "[[1 2 3 4 5 6 7 8 9 ] [-5 -3 -1 0 5 3 2 ] ]";
        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, IntegerVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<int> vect11 {1,2,3,4,5,6,7,8,9};
        vector<int> vect12 {-5,-3,-1,0,5,3,2};
        vector<vector<int>> vect1{vect11, vect12};
        vector<int> vect21 {2,0,1,6,9,3,8};
        vector<int> vect22 {9,7,8,100003};
        vector<vector<int>> vect2{vect21, vect22};
        vector<vector<vector<int>>> vect{vect1, vect2};
        rep.setValue(AttrName, vect);
        static const std::string Expected =
            "[[[1 2 3 4 5 6 7 8 9 ] [-5 -3 -1 0 5 3 2 ] ] [[2 0 1 6 9 3 8 ] [9 7 8 100003 ] ] ]";
        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, DoubleVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<double> vect {3.12, -5.3, 7.5, 1.110};
        vector<double> vect2 {2.1, -555.5, 0.0001, -0.2};
        rep.setValue(AttrName, vect);
        EXPECT_EQ(rep.getValueToString(AttrName), rep[AttrName].getValueToString());

        rep.setValue(AttrName, vect2);
        EXPECT_EQ(rep.getValueToString(AttrName), rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, DoubleVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<double> vect1 {30.1,5.88,-22.0,0};
        vector<double> vect2 {2.1,-55.5,0.1100,-.2};
        vector<vector<double>> vect{vect1, vect2};

        rep.setValue(AttrName, vect);
        EXPECT_EQ(rep.getValueToString(AttrName), rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, DoubleVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<double> vect11 {3.01, 5.88, -22.0, 0.0};
        vector<double> vect12 {99.3,8.0,.01236,-.22};
        vector<vector<double>> vect1{vect11, vect12};
        vector<double> vect21 {9.0,-1};
        vector<double> vect22 {-99.2};
        vector<vector<double>> vect2{vect21, vect22};
        vector<vector<vector<double>>> vect{vect1, vect2};
        rep.setValue(AttrName, vect);
        EXPECT_EQ(rep.getValueToString(AttrName), rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, BooleanVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<bool> vect {true, false, false, true};
        vector<bool> vect2 {false, false, false, true};
        rep.setValue(AttrName, vect);
        EXPECT_EQ("[true false false true ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[true false false true ]", rep[AttrName].getValueToString());

        rep.setValue(AttrName, vect2);
        EXPECT_EQ("[false false false true ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[false false false true ]", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, BooleanVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<bool> vect1 {true, false, false, true};
        vector<bool> vect2 {false, false, false, true};
        vector<vector<bool>> vect{vect1, vect2};

        rep.setValue(AttrName, vect);
        static const string Expected="[[true false false true ] [false false false true ] ]";

        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, BooleanVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<bool> vect11 {true, false, false, true};
        vector<bool> vect12 {false, false, false, true};
        vector<vector<bool>> vect1{vect11, vect12};
        vector<bool> vect21 {false, true, true, false};
        vector<bool> vect22 {true, true, true, false};
        vector<vector<bool>> vect2{vect21, vect22};
        vector<vector<vector<bool>>> vect{vect1, vect2};
        rep.setValue(AttrName, vect);
        static const std::string Expected =
            "[[[true false false true ] [false false false true ] ]"
            " [[false true true false ] [true true true false ] ] ]";
        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, StringVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<string> vect {"s1", "s2", "s3"};
        vector<string> vect2 {"s4", "s5", "s6"};
        rep.setValue(AttrName, vect);
        EXPECT_EQ("[s1 s2 s3 ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[s1 s2 s3 ]", rep[AttrName].getValueToString());

        rep.setValue(AttrName, vect2);
        EXPECT_EQ("[s4 s5 s6 ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[s4 s5 s6 ]", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, StringVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<string> vect1 {"s1", "s2", "s3"};
        vector<string> vect2 {"s4", "s5", "s6"};
        vector<vector<string>> vect{vect1, vect2};

        rep.setValue(AttrName, vect);
        static const string Expected="[[s1 s2 s3 ] [s4 s5 s6 ] ]";

        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, StringVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        vector<string> vect11 {"s1", "s2", "s3"};
        vector<string> vect12 {"s4", "s5", "s6"};
        vector<vector<string>> vect1{vect11, vect12};
        vector<string> vect21 {"s7", "s8"};
        vector<string> vect22 {"s9"};
        vector<vector<string>> vect2{vect21, vect22};
        vector<vector<vector<string>>> vect{vect1, vect2};
        rep.setValue(AttrName, vect);
        static const std::string Expected =
            "[[[s1 s2 s3 ] [s4 s5 s6 ] ]"
            " [[s7 s8 ] [s9 ] ] ]";
        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, SubRepresentationVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        OCRepresentation sub1;
        OCRepresentation sub2;
        vector<OCRepresentation> vect {sub1, sub2};
        rep.setValue(AttrName, vect);
        EXPECT_EQ("[OC::OCRepresentation OC::OCRepresentation ]", rep.getValueToString(AttrName));
        EXPECT_EQ("[OC::OCRepresentation OC::OCRepresentation ]", rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, SubRepresentationVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        OCRepresentation sub1;
        OCRepresentation sub2;
        OCRepresentation sub3;
        OCRepresentation sub4;
        vector<OCRepresentation> vect1 {sub1, sub2};
        vector<OCRepresentation> vect2 {sub3, sub4};
        vector<vector<OCRepresentation>> vect{vect1, vect2};
        rep.setValue(AttrName, vect);
        static const string Expected = "[[OC::OCRepresentation OC::OCRepresentation ]"
            " [OC::OCRepresentation OC::OCRepresentation ] ]";
        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    TEST(OCRepresentationValueToString, SubRepresentationVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;

        OCRepresentation sub1;
        OCRepresentation sub2;
        OCRepresentation sub3;
        OCRepresentation sub4;
        OCRepresentation sub5;
        OCRepresentation sub6;

        vector<OCRepresentation> vect11 {sub1, sub2};
        vector<OCRepresentation> vect12 {sub3, sub4};
        vector<vector<OCRepresentation>> vect1{vect11, vect12};
        vector<OCRepresentation> vect21 {sub5};
        vector<OCRepresentation> vect22 {sub6};
        vector<vector<OCRepresentation>> vect2{vect21, vect22};
        vector<vector<vector<OCRepresentation>>> vect{vect1, vect2};

        rep.setValue(AttrName, vect);
        static const string Expected =
            "[[[OC::OCRepresentation OC::OCRepresentation ] "
            "[OC::OCRepresentation OC::OCRepresentation ] ] "
            "[[OC::OCRepresentation ] [OC::OCRepresentation ] ] ]";
        EXPECT_EQ(Expected, rep.getValueToString(AttrName));
        EXPECT_EQ(Expected, rep[AttrName].getValueToString());
    }

    // Subscript get/set (all types)
    TEST(OCRepresentationSubscript, NullPtr)
    {
        static const std::string AttrName = "NullTest";
        OCRepresentation rep;
        rep[AttrName] = nullptr;
        EXPECT_TRUE(rep.isNULL(AttrName));

        std::nullptr_t repout = rep[AttrName];
        std::nullptr_t repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(nullptr, repout);
        EXPECT_EQ(nullptr, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, Integer)
    {
        static const std::string AttrName = "IntTest";
        OCRepresentation rep;
        int repsource = -5;
        rep[AttrName] = repsource;
        int repout = rep[AttrName];
        int repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, Double)
    {
        static const std::string AttrName = "DoubleTest";
        OCRepresentation rep;
        double repsource = -5.33;
        rep[AttrName] = repsource;
        double repout = rep[AttrName];
        double repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        int badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    //Disabled this test due to older GCC v4.6 fails for this test.
    //We will enable it when we have a fix for it.
    TEST(OCRepresentationSubscript, DISABLED_Boolean)
    {
        static const std::string AttrName = "BooleanTest";
        OCRepresentation rep;
        bool repsource = true;
        rep[AttrName] = repsource;
        bool repout = rep[AttrName];
        bool repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        repsource = false;
        rep[AttrName] = repsource;
        repout = rep[AttrName];

        EXPECT_EQ(repsource, repout);

        int badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, String)
    {
        static const std::string AttrName = "StringTest";
        OCRepresentation rep;
        string repsource = "This is a string!";
        rep[AttrName] = repsource;
        string repout = rep[AttrName];
        string repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        (void)badout;
    }

    TEST(OCRepresentationSubscript, SubRepresentation)
    {
        static const std::string AttrName = "SubRepresentationTest";
        OCRepresentation rep;
        OCRepresentation repsource;
        repsource.setUri("This is a uri");

        rep[AttrName] = repsource;
        OCRepresentation repout = rep[AttrName];
        OCRepresentation repout2;
        repout2 = rep[AttrName];

        //OCRepresentation doesn't overload equality, so this just compares
        //the value we set to ensure we got the same one out;
        EXPECT_EQ(repsource.getUri(), repout.getUri());
        EXPECT_EQ(repsource.getUri(), repout2.getUri());

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, IntegerVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<int> repsource {1,2,3,4};
        rep[AttrName] = repsource;
        vector<int> repout = rep[AttrName];
        vector<int> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, IntegerVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<int>> repsource {{1,2,3,4},{5,6,7,8}};
        rep[AttrName] = repsource;
        vector<vector<int>> repout = rep[AttrName];
        vector<vector<int>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, IntegerVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<vector<int>>> repsource {{{1,2,3,4},{5,6,7,8}},{{9,10,11},{21,13}}};
        rep[AttrName] = repsource;
        vector<vector<vector<int>>> repout = rep[AttrName];
        vector<vector<vector<int>>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, DoubleVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<double> repsource {1.1,2.2,3.2,4.2};
        rep[AttrName] = repsource;
        vector<double> repout = rep[AttrName];
        vector<double> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, DoubleVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<double>> repsource {{1.1,2.2,3.2,4.3},{5.5,6.6,7.8,.8}};
        rep[AttrName] = repsource;
        vector<vector<double>> repout = rep[AttrName];
        vector<vector<double>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, DoubleVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<vector<double>>> repsource {{{1.1,2.5,3.5,4.4},{.5,.6,7.8,8.9}},
            {{9.8,10.8,11.8},{2.1,1.3}}};
        rep[AttrName] = repsource;
        vector<vector<vector<double>>> repout = rep[AttrName];
        vector<vector<vector<double>>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, BooleanVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<bool> repsource {false, false, true};
        rep[AttrName] = repsource;
        vector<bool> repout = rep[AttrName];
        vector<bool> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, BooleanVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<bool>> repsource {{true, true},{false, true}};
        rep[AttrName] = repsource;
        vector<vector<bool>> repout = rep[AttrName];
        vector<vector<bool>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, BooleanVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<vector<bool>>> repsource {{{true, true, false},{true}},
            {{true, false, false},{false, true, true}}};
        rep[AttrName] = repsource;
        vector<vector<vector<bool>>> repout = rep[AttrName];
        vector<vector<vector<bool>>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, StringVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<string> repsource {"str1", "str2"};
        rep[AttrName] = repsource;
        vector<string> repout = rep[AttrName];
        vector<string> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, StringVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<string>> repsource {{"str1", "str2"},{"str3", "str4"}};
        rep[AttrName] = repsource;
        vector<vector<string>> repout = rep[AttrName];
        vector<vector<string>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, StringVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        vector<vector<vector<string>>> repsource {{{"str1", "str2"},{"str3", "str4"}},
            {{"str5"},{"str6"}}};
        rep[AttrName] = repsource;
        vector<vector<vector<string>>> repout = rep[AttrName];
        vector<vector<vector<string>>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(repsource, repout);
        EXPECT_EQ(repsource, repout2);

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, SubRepresentationVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        OCRepresentation inner1, inner2;
        inner1.setUri("inner1");
        inner2.setUri("inner2");
        vector<OCRepresentation> repsource {inner1, inner2};
        rep[AttrName] = repsource;
        vector<OCRepresentation> repout = rep[AttrName];
        vector<OCRepresentation> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(2u, repout.size());
        EXPECT_EQ(inner1.getUri(), repout[0].getUri());
        EXPECT_EQ(inner2.getUri(), repout[1].getUri());
        EXPECT_EQ(2u, repout2.size());
        EXPECT_EQ(inner1.getUri(), repout2[0].getUri());
        EXPECT_EQ(inner2.getUri(), repout2[1].getUri());

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, SubRepresentationVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        OCRepresentation inner1, inner2, inner3, inner4;
        inner1.setUri("inner1");
        inner2.setUri("inner2");
        inner3.setUri("inner3");
        inner4.setUri("inner4");

        vector<vector<OCRepresentation>> repsource {{inner1, inner2}, {inner3, inner4}};
        rep[AttrName] = repsource;
        vector<vector<OCRepresentation>> repout = rep[AttrName];
        vector<vector<OCRepresentation>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(2u, repout.size());
        EXPECT_EQ(2u, repout[0].size());
        EXPECT_EQ(inner1.getUri(), repout[0][0].getUri());
        EXPECT_EQ(inner2.getUri(), repout[0][1].getUri());
        EXPECT_EQ(2u, repout.size());
        EXPECT_EQ(2u, repout[1].size());
        EXPECT_EQ(inner3.getUri(), repout[1][0].getUri());
        EXPECT_EQ(inner4.getUri(), repout[1][1].getUri());

        EXPECT_EQ(2u, repout2.size());
        EXPECT_EQ(2u, repout2[0].size());
        EXPECT_EQ(inner1.getUri(), repout2[0][0].getUri());
        EXPECT_EQ(inner2.getUri(), repout2[0][1].getUri());
        EXPECT_EQ(2u, repout2.size());
        EXPECT_EQ(2u, repout2[1].size());
        EXPECT_EQ(inner3.getUri(), repout2[1][0].getUri());
        EXPECT_EQ(inner4.getUri(), repout2[1][1].getUri());

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationSubscript, SubRepresentationVectorVectorVector)
    {
        static const std::string AttrName = "VectorTest";
        OCRepresentation rep;
        OCRepresentation inner1, inner2, inner3, inner4, inner5, inner6, inner7, inner8;
        inner1.setUri("inner1");
        inner2.setUri("inner2");
        inner3.setUri("inner3");
        inner4.setUri("inner4");
        inner5.setUri("inner5");
        inner6.setUri("inner6");
        inner7.setUri("inner7");
        inner8.setUri("inner8");

        vector<vector<vector<OCRepresentation>>> repsource
            {{{inner1, inner2},{inner3, inner4}},{{inner5, inner6},{inner7,inner8}}};

        rep[AttrName] = repsource;
        vector<vector<vector<OCRepresentation>>> repout = rep[AttrName];
        vector<vector<vector<OCRepresentation>>> repout2;
        repout2 = rep[AttrName];

        EXPECT_EQ(2u, repout.size());
        EXPECT_EQ(2u, repout[0].size());
        EXPECT_EQ(2u, repout[0][0].size());
        EXPECT_EQ(inner1.getUri(), repout[0][0][0].getUri());
        EXPECT_EQ(inner2.getUri(), repout[0][0][1].getUri());
        EXPECT_EQ(2u, repout[0][1].size());
        EXPECT_EQ(inner3.getUri(), repout[0][1][0].getUri());
        EXPECT_EQ(inner4.getUri(), repout[0][1][1].getUri());
        EXPECT_EQ(2u, repout[1].size());
        EXPECT_EQ(2u, repout[1][0].size());
        EXPECT_EQ(inner5.getUri(), repout[1][0][0].getUri());
        EXPECT_EQ(inner6.getUri(), repout[1][0][1].getUri());
        EXPECT_EQ(2u, repout[1][1].size());
        EXPECT_EQ(inner7.getUri(), repout[1][1][0].getUri());
        EXPECT_EQ(inner8.getUri(), repout[1][1][1].getUri());

        EXPECT_EQ(2u, repout2.size());
        EXPECT_EQ(2u, repout2[0].size());
        EXPECT_EQ(2u, repout2[0][0].size());
        EXPECT_EQ(inner1.getUri(), repout2[0][0][0].getUri());
        EXPECT_EQ(inner2.getUri(), repout2[0][0][1].getUri());
        EXPECT_EQ(2u, repout[0][1].size());
        EXPECT_EQ(inner3.getUri(), repout2[0][1][0].getUri());
        EXPECT_EQ(inner4.getUri(), repout2[0][1][1].getUri());
        EXPECT_EQ(2u, repout[1].size());
        EXPECT_EQ(2u, repout[1][0].size());
        EXPECT_EQ(inner5.getUri(), repout2[1][0][0].getUri());
        EXPECT_EQ(inner6.getUri(), repout2[1][0][1].getUri());
        EXPECT_EQ(2u, repout[1][1].size());
        EXPECT_EQ(inner7.getUri(), repout2[1][1][0].getUri());
        EXPECT_EQ(inner8.getUri(), repout2[1][1][1].getUri());

        double badout;
        EXPECT_THROW(badout = rep[AttrName], boost::bad_get);
        string badoutstr;
        EXPECT_THROW(badoutstr = rep[AttrName], boost::bad_get);
        (void)badout;
        (void)badoutstr;
    }

    TEST(OCRepresentationIterator, constiterator)
    {
        OCRepresentation rep;

        EXPECT_TRUE(rep.empty());
        rep.setValue("int", 8);
        EXPECT_FALSE(rep.empty());
        rep.setValue("double", 8.8);
        rep.setValue("bool", true);
        rep.setValue("string", std::string("this is a string"));

        EXPECT_EQ(4u, rep.size());
        EXPECT_FALSE(rep.empty());

        OCRepresentation::const_iterator itr = rep.cbegin();
        OCRepresentation::const_iterator endItr = rep.cend();
        for(;itr!=endItr;++itr);

        const OCRepresentation& rep2(rep);
        OCRepresentation::const_iterator itr2 = rep2.begin();
        OCRepresentation::const_iterator endItr2 = rep2.end();
        for(;itr2!=endItr2;++itr2);

    }

    TEST(OCRepresentationIterator, constautoiterator)
    {
        OCRepresentation rep;

        EXPECT_TRUE(rep.empty());
        rep.setValue("int", 8);
        EXPECT_FALSE(rep.empty());
        rep.setValue("double", 8.8);
        rep.setValue("bool", true);
        rep.setValue("string", std::string("this is a string"));

        EXPECT_EQ(4u, rep.size());
        EXPECT_FALSE(rep.empty());

        for(const auto& a : rep)
        {
            (void)a;
        }

        const OCRepresentation& rep2(rep);
        for(const auto& a : rep2)
        {
            (void)a;
        }
    }
    TEST(OCRepresentationIterator, autoiterator)
    {
        OCRepresentation rep;

        EXPECT_TRUE(rep.empty());
        rep.setValue("int", 8);
        EXPECT_FALSE(rep.empty());
        rep.setValue("double", 8.8);
        rep.setValue("bool", true);
        rep.setValue("string", std::string("this is a string"));

        EXPECT_EQ(4u, rep.size());
        EXPECT_FALSE(rep.empty());

        for(auto& cur : rep)
        {
            if(cur.attrname() == "int")
            {
                EXPECT_EQ("int", cur.attrname());
                EXPECT_EQ(AttributeType::Integer, cur.type());
                EXPECT_EQ(AttributeType::Integer, cur.base_type());
                EXPECT_EQ(0u, cur.depth());
                int curInt = cur.getValue<int>();
                EXPECT_EQ(8, curInt);
            }
            if(cur.attrname() == "double")
            {
                EXPECT_EQ("double", cur.attrname());
                EXPECT_EQ(AttributeType::Double, cur.type());
                EXPECT_EQ(AttributeType::Double, cur.base_type());
                EXPECT_EQ(0u, cur.depth());
                double curDouble = cur.getValue<double>();
                EXPECT_EQ(8.8, curDouble);
            }
            if(cur.attrname() == "bool")
            {
                EXPECT_EQ("bool", cur.attrname());
                EXPECT_EQ(AttributeType::Boolean, cur.type());
                EXPECT_EQ(AttributeType::Boolean, cur.base_type());
                EXPECT_EQ(0u, cur.depth());
                bool curBool = cur.getValue<bool>();
                EXPECT_EQ(true, curBool);
            }
            if(cur.attrname() == "string")
            {
                EXPECT_EQ("string", cur.attrname());
                EXPECT_EQ(AttributeType::String, cur.type());
                EXPECT_EQ(AttributeType::String, cur.base_type());
                EXPECT_EQ(0u, cur.depth());
                string curStr = cur.getValue<string>();
                EXPECT_EQ("this is a string", curStr);
            }
        }
    }
    // Iterator usage
    TEST(OCRepresentationIterator, iterator)
    {
        OCRepresentation rep;
        OCRepresentation sub1;
        sub1.setUri("sub rep1 URI");
        OCRepresentation sub2;
        sub2.setUri("sub rep2 URI");
        OCRepresentation sub3;
        sub3.setUri("sub rep3 URI");
        OCRepresentation sub4;
        sub4.setUri("sub rep4 URI");
        OCRepresentation sub5;
        sub5.setUri("sub rep5 URI");
        OCRepresentation sub6;
        sub6.setUri("sub rep6 URI");


        EXPECT_TRUE(rep.empty());
        rep.setValue("int", 8);
        EXPECT_FALSE(rep.empty());
        rep.setValue("double", 8.8);
        rep.setValue("bool", true);
        rep.setValue("string", std::string("this is a string"));
        rep.setValue("rep", sub1);

        vector<int> intv {1,2,3,4};
        rep.setValue("intv", intv);
        vector<double> doublev {1.1,2.2,3.3,4.4};
        rep.setValue("doublev", doublev);
        vector<bool> boolv{false, false, true};
        rep.setValue("boolv", boolv);
        vector<string> strv{"abc", "def", "ghi"};
        rep.setValue("strv", strv);
        vector<OCRepresentation> repv { sub1, sub2 };
        rep.setValue("repv", repv);

        vector<vector<int>> intvv{{1,2,3},{4,5,6}};
        rep.setValue("intvv", intvv);
        vector<vector<vector<int>>> intvvv{{{1,2},{3,4}},{{5,6},{8,7}}};
        rep.setValue("intvvv", intvvv);

        vector<vector<double>> doublevv{{1.1,2.1,3},{4.4,5.4,6.4}};
        rep.setValue("doublevv", doublevv);
        vector<vector<vector<double>>> doublevvv{{{1.1,2.1},{3.1,4.1}},{{5.1,6.1},{8.1,7.1}}};
        rep.setValue("doublevvv", doublevvv);

        vector<vector<bool>> boolvv{{false, true},{true, false}};
        rep.setValue("boolvv", boolvv);
        vector<vector<vector<bool>>> boolvvv{{{true, false},{true}},{{false},{true}}};
        rep.setValue("boolvvv", boolvvv);

        vector<vector<string>> strvv{{"abc", "def"},{"wer", "qwer"}};
        rep.setValue("strvv", strvv);
        vector<vector<vector<string>>> strvvv{{{"wqr", "xcv"},{"234"}},{{"we"},{"wert"}}};
        rep.setValue("strvvv", strvvv);

        vector<vector<OCRepresentation>> repvv{{sub1, sub2},{sub3, sub4}};
        rep.setValue("repvv", repvv);
        vector<vector<vector<OCRepresentation>>> repvvv{{{sub5},{sub6}},{{sub3},{sub2}}};
        rep.setValue("repvvv", repvvv);

        EXPECT_EQ(20u, rep.size());
        EXPECT_FALSE(rep.empty());

        OCRepresentation::iterator itr= rep.begin();
        OCRepresentation::iterator endItr = rep.end();

        for(;itr!=endItr;++itr)
        {
            if(itr->attrname() == "int")
            {
                EXPECT_EQ("int", itr->attrname());
                EXPECT_EQ(AttributeType::Integer, itr->type());
                EXPECT_EQ(AttributeType::Integer, itr->base_type());
                EXPECT_EQ(0u, itr->depth());
                int curInt = (*itr).getValue<int>();
                EXPECT_EQ(8, curInt);
            }
            else if (itr->attrname() == "double")
            {
                EXPECT_EQ("double", itr->attrname());
                EXPECT_EQ(AttributeType::Double, itr->type());
                EXPECT_EQ(AttributeType::Double, itr->base_type());
                EXPECT_EQ(0u, itr->depth());
                double curDouble = (*itr).getValue<double>();
                EXPECT_EQ(8.8, curDouble);
            }
            else if (itr->attrname() == "bool")
            {
                EXPECT_EQ("bool", itr->attrname());
                EXPECT_EQ(AttributeType::Boolean, itr->type());
                EXPECT_EQ(AttributeType::Boolean, itr->base_type());
                EXPECT_EQ(0u, itr->depth());
                bool curBool = (*itr).getValue<bool>();
                EXPECT_EQ(true, curBool);
            }
            else if (itr->attrname() == "string")
            {
                EXPECT_EQ("string", itr->attrname());
                EXPECT_EQ(AttributeType::String, itr->type());
                EXPECT_EQ(AttributeType::String, itr->base_type());
                EXPECT_EQ(0u, itr->depth());
                string curString = (*itr).getValue<string>();
                EXPECT_EQ("this is a string", curString);
            }
            else if (itr->attrname() == "rep")
            {
                EXPECT_EQ("rep", itr->attrname());
                EXPECT_EQ(AttributeType::OCRepresentation, itr->type());
                EXPECT_EQ(AttributeType::OCRepresentation, itr->base_type());
                EXPECT_EQ(0u, itr->depth());
                OCRepresentation curRep = (*itr).getValue<OCRepresentation>();
                EXPECT_EQ(sub1.getUri(), curRep.getUri());
            }
            else if (itr->attrname() == "intv")
            {
                EXPECT_EQ("intv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Integer, itr->base_type());
                EXPECT_EQ(1u, itr->depth());
                vector<int> curv = (*itr).getValue<vector<int>>();
                EXPECT_EQ(intv, curv);
            }
            else if (itr->attrname() == "doublev")
            {
                EXPECT_EQ("doublev", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Double, itr->base_type());
                EXPECT_EQ(1u, itr->depth());
                vector<double> curv = (*itr).getValue<vector<double>>();
                EXPECT_EQ(doublev, curv);
            }
            else if (itr->attrname() == "boolv")
            {
                EXPECT_EQ("boolv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Boolean, itr->base_type());
                EXPECT_EQ(1u, itr->depth());
                vector<bool> curv = (*itr).getValue<vector<bool>>();
                EXPECT_EQ(boolv, curv);
            }
            else if (itr->attrname() == "strv")
            {
                EXPECT_EQ("strv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::String, itr->base_type());
                EXPECT_EQ(1u, itr->depth());
                vector<string> curv = (*itr).getValue<vector<string>>();
                EXPECT_EQ(strv, curv);
            }
            else if (itr->attrname() == "repv")
            {
                EXPECT_EQ("repv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::OCRepresentation, itr->base_type());
                EXPECT_EQ(1u, itr->depth());
                vector<OCRepresentation> curv = (*itr).getValue<vector<OCRepresentation>>();
                EXPECT_EQ(2u, repv.size());
                EXPECT_EQ(sub1.getUri(), repv[0].getUri());
                EXPECT_EQ(sub2.getUri(), repv[1].getUri());
            }
            else if (itr->attrname() == "intvv")
            {
                EXPECT_EQ("intvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Integer, itr->base_type());
                EXPECT_EQ(2u, itr->depth());
                vector<vector<int>> curv = (*itr).getValue<vector<vector<int>>>();
                EXPECT_EQ(intvv, curv);
            }
            else if (itr->attrname() == "intvvv")
            {
                EXPECT_EQ("intvvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Integer, itr->base_type());
                EXPECT_EQ(3u, itr->depth());
                vector<vector<vector<int>>> curv = (*itr).getValue<vector<vector<vector<int>>>>();
                EXPECT_EQ(intvvv, curv);
            }
            else if (itr->attrname() == "doublevv")
            {
                EXPECT_EQ("doublevv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Double, itr->base_type());
                EXPECT_EQ(2u, itr->depth());
                vector<vector<double>> curv = (*itr).getValue<vector<vector<double>>>();
                EXPECT_EQ(doublevv, curv);
            }
            else if (itr->attrname() == "doublevvv")
            {
                EXPECT_EQ("doublevvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Double, itr->base_type());
                EXPECT_EQ(3u, itr->depth());
                vector<vector<vector<double>>> curv =
                    (*itr).getValue<vector<vector<vector<double>>>>();
                EXPECT_EQ(doublevvv, curv);
            }
            else if (itr->attrname() == "boolvv")
            {
                EXPECT_EQ("boolvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Boolean, itr->base_type());
                EXPECT_EQ(2u, itr->depth());
                vector<vector<bool>> curv = (*itr).getValue<vector<vector<bool>>>();
                EXPECT_EQ(boolvv, curv);
            }
            else if (itr->attrname() == "boolvvv")
            {
                EXPECT_EQ("boolvvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::Boolean, itr->base_type());
                EXPECT_EQ(3u, itr->depth());
                vector<vector<vector<bool>>> curv = (*itr).getValue<vector<vector<vector<bool>>>>();
                EXPECT_EQ(boolvvv, curv);
            }
            else if (itr->attrname() == "strvv")
            {
                EXPECT_EQ("strvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::String, itr->base_type());
                EXPECT_EQ(2u, itr->depth());
                vector<vector<string>> curv = (*itr).getValue<vector<vector<string>>>();
                EXPECT_EQ(strvv, curv);
            }
            else if (itr->attrname() == "strvvv")
            {
                EXPECT_EQ("strvvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::String, itr->base_type());
                EXPECT_EQ(3u, itr->depth());
                vector<vector<vector<string>>> curv =
                    (*itr).getValue<vector<vector<vector<string>>>>();
                EXPECT_EQ(strvvv, curv);
            }
            else if (itr->attrname() == "repvv")
            {
                EXPECT_EQ("repvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::OCRepresentation, itr->base_type());
                EXPECT_EQ(2u, itr->depth());
                vector<vector<OCRepresentation>> curv =
                    (*itr).getValue<vector<vector<OCRepresentation>>>();
                EXPECT_EQ(2u, curv.size());
                EXPECT_EQ(2u, curv[0].size());
                EXPECT_EQ(sub1.getUri(), curv[0][0].getUri());
                EXPECT_EQ(sub2.getUri(), curv[0][1].getUri());
                EXPECT_EQ(2u, curv[1].size());
                EXPECT_EQ(sub3.getUri(), curv[1][0].getUri());
                EXPECT_EQ(sub4.getUri(), curv[1][1].getUri());
            }
            else if (itr->attrname() == "repvvv")
            {
                EXPECT_EQ("repvvv", itr->attrname());
                EXPECT_EQ(AttributeType::Vector, itr->type());
                EXPECT_EQ(AttributeType::OCRepresentation, itr->base_type());
                EXPECT_EQ(3u, itr->depth());
                vector<vector<vector<OCRepresentation>>> curv =
                    (*itr).getValue<vector<vector<vector<OCRepresentation>>>>();
                EXPECT_EQ(2u, curv.size());
                EXPECT_EQ(2u, curv[0].size());
                EXPECT_EQ(1u, curv[0][0].size());
                EXPECT_EQ(sub5.getUri(), curv[0][0][0].getUri());
                EXPECT_EQ(1u, curv[0][1].size());
                EXPECT_EQ(sub6.getUri(), curv[0][1][0].getUri());
                EXPECT_EQ(1u, curv[1][0].size());
                EXPECT_EQ(sub3.getUri(), curv[1][0][0].getUri());
                EXPECT_EQ(1u, curv[1][1].size());
                EXPECT_EQ(sub2.getUri(), curv[1][1][0].getUri());
            }
            else
            {
                EXPECT_TRUE(false) << itr->attrname();
            }
        }
    }
}
