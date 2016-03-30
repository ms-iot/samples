//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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
#include <OCRepresentation.h>
#include <octypes.h>
#include <ocpayload.h>
#include <ocpayloadcbor.h>
#include <oic_malloc.h>
#include <oic_string.h>

namespace OC
{
    bool operator==(const OC::NullType&, const OC::NullType&)
    {
        return true;
    }

    bool operator==(const OC::OCRepresentation& lhs, const OC::OCRepresentation& rhs)
    {
        return lhs.getUri() == rhs.getUri() &&
            lhs.getChildren() == rhs.getChildren() &&
            lhs.getResourceInterfaces() == rhs.getResourceInterfaces() &&
            lhs.getResourceTypes() == rhs.getResourceTypes() &&
            lhs.m_values == rhs.m_values;
    }
}
// these tests validate the OCRepresentation->OCPayload, OCPayload->CBOR,
// CBOR->OCPayload and OCPayload->OCRepresentation conversions
namespace OCRepresentationEncodingTest
{

    static const char uri1[] = "/testuri";
    static const uint8_t sid1[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    static const char devicename1[] = "device name";
    static const char specver1[] = "spec version";
    static const char dmver1[] = "data model version";
    // Device Payloads
    TEST(DeviceDiscoveryEncoding, Normal)
    {
        OCDevicePayload* device = OCDevicePayloadCreate(
                uri1,
                sid1,
                devicename1,
                specver1,
                dmver1);

        EXPECT_STREQ(uri1, device->uri);
        EXPECT_STREQ(devicename1, device->deviceName);
        EXPECT_STREQ(specver1, device->specVersion);
        EXPECT_STREQ(dmver1, device->dataModelVersion);
        EXPECT_EQ(PAYLOAD_TYPE_DEVICE, ((OCPayload*)device)->type);

        for (uint8_t i = 1; i <= sizeof(sid1); ++i)
        {
            EXPECT_EQ(i, sid1[i - 1]);
        }

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* parsedDevice;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)device, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&parsedDevice, PAYLOAD_TYPE_DEVICE,
                    cborData, cborSize));
        OICFree(cborData);

        EXPECT_STREQ(device->uri, ((OCDevicePayload*)parsedDevice)->uri);
        EXPECT_STREQ(device->deviceName, ((OCDevicePayload*)parsedDevice)->deviceName);
        EXPECT_STREQ(device->specVersion, ((OCDevicePayload*)parsedDevice)->specVersion);
        EXPECT_STREQ(device->dataModelVersion, ((OCDevicePayload*)parsedDevice)->dataModelVersion);
        EXPECT_EQ(device->base.type, ((OCDevicePayload*)parsedDevice)->base.type);

        OCPayloadDestroy((OCPayload*)device);

        OC::MessageContainer mc;
        mc.setPayload(parsedDevice);
        EXPECT_EQ(1u, mc.representations().size());
        const OC::OCRepresentation& r = mc.representations()[0];
        EXPECT_STREQ(uri1, r.getUri().c_str());
        EXPECT_STREQ(devicename1, r.getValue<std::string>(OC_RSRVD_DEVICE_NAME).c_str());
        EXPECT_STREQ(specver1, r.getValue<std::string>(OC_RSRVD_SPEC_VERSION).c_str());
        EXPECT_STREQ(dmver1, r.getValue<std::string>(OC_RSRVD_DATA_MODEL_VERSION).c_str());


        OCPayloadDestroy(parsedDevice);
    }

    static char pfid1[] = "pfid";
    static char mfgnm1[] = "mfgnm";
    static char mfgurl1[] = "mfgurl";
    static char modelnum1[] = "modelnum";
    static char dom1[] = "dom";
    static char pfver1[] = "pfver";
    static char osver1[] = "osver";
    static char hwver1[] = "hwver";
    static char fwver1[] = "fwver";
    static char url1[] = "url";
    static char time1[] = "time";

    // Platform Payloads
    TEST(PlatformDiscoveryEncoding, Normal)
    {
        OCPlatformInfo info {pfid1, mfgnm1, mfgurl1, modelnum1, dom1, pfver1, osver1, hwver1,
            fwver1, url1, time1};
        OCPlatformPayload* platform = OCPlatformPayloadCreate(uri1, &info);
        EXPECT_EQ(PAYLOAD_TYPE_PLATFORM, ((OCPayload*)platform)->type);
        EXPECT_STREQ(uri1, platform->uri);
        EXPECT_STREQ(pfid1, platform->info.platformID);
        EXPECT_STREQ(mfgnm1, platform->info.manufacturerName);
        EXPECT_STREQ(mfgurl1, platform->info.manufacturerUrl);
        EXPECT_STREQ(modelnum1, platform->info.modelNumber);
        EXPECT_STREQ(dom1, platform->info.dateOfManufacture);
        EXPECT_STREQ(pfver1, platform->info.platformVersion);
        EXPECT_STREQ(osver1, platform->info.operatingSystemVersion);
        EXPECT_STREQ(hwver1, platform->info.hardwareVersion);
        EXPECT_STREQ(fwver1, platform->info.firmwareVersion);
        EXPECT_STREQ(url1, platform->info.supportUrl);
        EXPECT_STREQ(time1, platform->info.systemTime);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* parsedPlatform;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)platform, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&parsedPlatform, PAYLOAD_TYPE_PLATFORM,
                    cborData, cborSize));
        OICFree(cborData);

        EXPECT_EQ(platform->base.type, ((OCPlatformPayload*)parsedPlatform)->base.type);
        OCPlatformPayload* platform2 = (OCPlatformPayload*)parsedPlatform;
        EXPECT_STREQ(platform->uri, platform2->uri);
        EXPECT_STREQ(platform->info.platformID, platform2->info.platformID);
        EXPECT_STREQ(platform->info.manufacturerName, platform->info.manufacturerName);
        EXPECT_STREQ(platform->info.manufacturerUrl, platform->info.manufacturerUrl);
        EXPECT_STREQ(platform->info.modelNumber, platform->info.modelNumber);
        EXPECT_STREQ(platform->info.dateOfManufacture, platform->info.dateOfManufacture);
        EXPECT_STREQ(platform->info.platformVersion, platform->info.platformVersion);
        EXPECT_STREQ(platform->info.operatingSystemVersion, platform->info.operatingSystemVersion);
        EXPECT_STREQ(platform->info.hardwareVersion, platform->info.hardwareVersion);
        EXPECT_STREQ(platform->info.firmwareVersion, platform->info.firmwareVersion);
        EXPECT_STREQ(platform->info.supportUrl, platform->info.supportUrl);
        EXPECT_STREQ(platform->info.systemTime, platform2->info.systemTime);

        OCPayloadDestroy((OCPayload*)platform);

        OC::MessageContainer mc;
        mc.setPayload(parsedPlatform);
        EXPECT_EQ(1u, mc.representations().size());
        const OC::OCRepresentation& r = mc.representations()[0];
        EXPECT_STREQ(uri1, r.getUri().c_str());
        EXPECT_STREQ(pfid1, r.getValue<std::string>(OC_RSRVD_PLATFORM_ID).c_str());
        EXPECT_STREQ(mfgnm1, r.getValue<std::string>(OC_RSRVD_MFG_NAME).c_str());
        EXPECT_STREQ(mfgurl1, r.getValue<std::string>(OC_RSRVD_MFG_URL).c_str());
        EXPECT_STREQ(modelnum1, r.getValue<std::string>(OC_RSRVD_MODEL_NUM).c_str());
        EXPECT_STREQ(dom1, r.getValue<std::string>(OC_RSRVD_MFG_DATE).c_str());
        EXPECT_STREQ(pfver1, r.getValue<std::string>(OC_RSRVD_PLATFORM_VERSION).c_str());
        EXPECT_STREQ(osver1, r.getValue<std::string>(OC_RSRVD_OS_VERSION).c_str());
        EXPECT_STREQ(hwver1, r.getValue<std::string>(OC_RSRVD_HARDWARE_VERSION).c_str());
        EXPECT_STREQ(fwver1, r.getValue<std::string>(OC_RSRVD_FIRMWARE_VERSION).c_str());
        EXPECT_STREQ(url1, r.getValue<std::string>(OC_RSRVD_SUPPORT_URL).c_str());
        EXPECT_STREQ(time1, r.getValue<std::string>(OC_RSRVD_SYSTEM_TIME).c_str());

        OCPayloadDestroy(parsedPlatform);
    }

    // Representation Payloads
    TEST(RepresentationEncoding, BaseAttributeTypes)
    {
        OC::OCRepresentation startRep;
        startRep.setNULL("NullAttr");
        startRep.setValue("IntAttr", 77);
        startRep.setValue("DoubleAttr", 3.333);
        startRep.setValue("BoolAttr", true);
        startRep.setValue("StringAttr", std::string("String attr"));
        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        EXPECT_TRUE(r.isNULL("NullAttr"));
        EXPECT_EQ(77, r.getValue<int>("IntAttr"));
        EXPECT_EQ(3.333, r.getValue<double>("DoubleAttr"));
        EXPECT_EQ(true, r.getValue<bool>("BoolAttr"));
        EXPECT_STREQ("String attr", r.getValue<std::string>("StringAttr").c_str());

        OCPayloadDestroy(cparsed);
    }

    TEST(RepresentationEncoding, RepAttribute)
    {
        OC::OCRepresentation startRep;
        OC::OCRepresentation subRep;
        subRep.setNULL("NullAttr");
        subRep.setValue("IntAttr", 77);
        subRep.setValue("DoubleAttr", 3.333);
        subRep.setValue("BoolAttr", true);
        subRep.setValue("StringAttr", std::string("String attr"));
        startRep.setValue("Sub", subRep);

        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        OC::OCRepresentation newSubRep = r["Sub"];

        EXPECT_TRUE(newSubRep.isNULL("NullAttr"));
        EXPECT_EQ(77, newSubRep.getValue<int>("IntAttr"));
        EXPECT_EQ(3.333, newSubRep.getValue<double>("DoubleAttr"));
        EXPECT_EQ(true, newSubRep.getValue<bool>("BoolAttr"));
        EXPECT_STREQ("String attr", newSubRep.getValue<std::string>("StringAttr").c_str());
        OCPayloadDestroy(cparsed);
    }

    TEST(RepresentationEncoding, OneDVectors)
    {
        // Setup
        OC::OCRepresentation startRep;

        OC::OCRepresentation subRep1;
        OC::OCRepresentation subRep2;
        OC::OCRepresentation subRep3;
        subRep1.setNULL("NullAttr");
        subRep1.setValue("IntAttr", 77);
        subRep2.setValue("DoubleAttr", 3.333);
        subRep2.setValue("BoolAttr", true);
        subRep3.setValue("StringAttr", std::string("String attr"));

        std::vector<int> iarr {1,2,3,4,5,6,7,8,9};
        std::vector<double> darr {1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9};
        std::vector<bool> barr {false, true, false, false, true, true};
        std::vector<std::string> strarr {"item1", "item2", "item3", "item4"};
        std::vector<OC::OCRepresentation> objarr {subRep1, subRep2, subRep3};

        startRep["iarr"] = iarr;
        startRep["darr"] = darr;
        startRep["barr"] = barr;
        startRep["strarr"] = strarr;
        startRep["objarr"] = objarr;

        // Encode/decode
        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        // Test
        std::vector<int> iarr2 = r["iarr"];
        std::vector<double> darr2 = r["darr"];
        std::vector<bool> barr2 = r["barr"];
        std::vector<std::string> strarr2 = r["strarr"];
        std::vector<OC::OCRepresentation> objarr2 = r["objarr"];

        EXPECT_EQ(iarr, iarr2);
        EXPECT_EQ(darr, darr2);
        EXPECT_EQ(barr, barr2);
        EXPECT_EQ(strarr, strarr2);
        EXPECT_EQ(objarr, objarr2);
        OCPayloadDestroy(cparsed);
    }

    TEST(RepresentationEncoding, TwoDVectors)
    {
        // Setup
        OC::OCRepresentation startRep;

        OC::OCRepresentation subRep1;
        OC::OCRepresentation subRep2;
        OC::OCRepresentation subRep3;
        subRep1.setNULL("NullAttr");
        subRep1.setValue("IntAttr", 77);
        subRep2.setValue("DoubleAttr", 3.333);
        subRep2.setValue("BoolAttr", true);
        subRep3.setValue("StringAttr", std::string("String attr"));

        std::vector<std::vector<int>> iarr {{1,2,3},{4,5,6},{7,8,9}};
        std::vector<std::vector<double>> darr {{1.1,2.2,3.3},{4.4,5.5,6.6},{7.7,8.8,9.9}};
        std::vector<std::vector<bool>> barr {{false, true}, {false, false}, {true, true}};
        std::vector<std::vector<std::string>> strarr {{"item1", "item2"}, {"item3", "item4"}};
        std::vector<std::vector<OC::OCRepresentation>> objarr
        {{subRep1, subRep2, subRep3}, {subRep3, subRep2, subRep1}};

        startRep["iarr"] = iarr;
        startRep["darr"] = darr;
        startRep["barr"] = barr;
        startRep["strarr"] = strarr;
        startRep["objarr"] = objarr;

        // Encode/decode
        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        // Test
        std::vector<std::vector<int>> iarr2 = r["iarr"];
        std::vector<std::vector<double>> darr2 = r["darr"];
        std::vector<std::vector<bool>> barr2 = r["barr"];
        std::vector<std::vector<std::string>> strarr2 = r["strarr"];
        std::vector<std::vector<OC::OCRepresentation>> objarr2 = r["objarr"];

        EXPECT_EQ(iarr, iarr2);
        EXPECT_EQ(darr, darr2);
        EXPECT_EQ(barr, barr2);
        EXPECT_EQ(strarr, strarr2);
        EXPECT_EQ(objarr, objarr2);
        OCPayloadDestroy(cparsed);
    }

    TEST(RepresentationEncoding, TwoDVectorsJagged)
    {
        // Setup
        OC::OCRepresentation startRep;

        OC::OCRepresentation subRep1;
        OC::OCRepresentation subRep2;
        OC::OCRepresentation subRep3;
        subRep1.setNULL("NullAttr");
        subRep1.setValue("IntAttr", 77);
        subRep2.setValue("DoubleAttr", 3.333);
        subRep2.setValue("BoolAttr", true);
        subRep3.setValue("StringAttr", std::string("String attr"));

        std::vector<std::vector<int>> iarr {{1,2,3},{4,6},{7,8,9}};
        std::vector<std::vector<double>> darr {{1.1,2.2,3.3},{4.4,5.5,6.6},{8.8,9.9}};
        std::vector<std::vector<bool>> barr {{false, true}, {false}, {true, true}};
        std::vector<std::vector<std::string>> strarr {{"item1"}, {"item3", "item4"}};
        std::vector<std::vector<OC::OCRepresentation>> objarr
        {{subRep1, subRep3}, {subRep3, subRep2, subRep1}};

        startRep["iarr"] = iarr;
        startRep["darr"] = darr;
        startRep["barr"] = barr;
        startRep["strarr"] = strarr;
        startRep["objarr"] = objarr;

        // Encode/decode
        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        // Test
        std::vector<std::vector<int>> iarr2 = r["iarr"];
        std::vector<std::vector<double>> darr2 = r["darr"];
        std::vector<std::vector<bool>> barr2 = r["barr"];
        std::vector<std::vector<std::string>> strarr2 = r["strarr"];
        std::vector<std::vector<OC::OCRepresentation>> objarr2 = r["objarr"];

        // Note: due to the way that the CSDK works, all 2d arrays need to be rectangular.
        // Since std::vector doesn't require this, items received on the other side end up
        // being backfilled.  This section removes the backfilling
        iarr2[1].pop_back();
        darr2[2].pop_back();
        barr2[1].pop_back();
        strarr2[0].pop_back();
        objarr2[0].pop_back();

        EXPECT_EQ(iarr, iarr2);
        EXPECT_EQ(darr, darr2);
        EXPECT_EQ(barr, barr2);
        EXPECT_EQ(strarr, strarr2);
        EXPECT_EQ(objarr, objarr2);
        OCPayloadDestroy(cparsed);
    }

    TEST(RepresentationEncoding, ThreeDVectors)
    {
        // Setup
        OC::OCRepresentation startRep;

        OC::OCRepresentation subRep1;
        OC::OCRepresentation subRep2;
        OC::OCRepresentation subRep3;
        subRep1.setNULL("NullAttr");
        subRep1.setValue("IntAttr", 77);
        subRep2.setValue("DoubleAttr", 3.333);
        subRep2.setValue("BoolAttr", true);
        subRep3.setValue("StringAttr", std::string("String attr"));

        std::vector<std::vector<std::vector<int>>> iarr
            {{{1,2,3},{4,5,6}},{{7,8,9},{10,11,12}},{{13,14,15},{16,17,18}}};
        std::vector<std::vector<std::vector<double>>> darr
            {{{1.1,2.2,3.3},{4.4,5.5,6.6}},
                {{7.7,8.7,9.7},{10.7,11.7,12.7}},
                {{13.7,14.7,15.7},{16.7,17.7,18.7}}};
        std::vector<std::vector<std::vector<bool>>> barr
            {{{false, true},{true, false}},{{false, true},{true, false}}};
        std::vector<std::vector<std::vector<std::string>>> strarr
            {
                {{"item1", "item2"},{"item3", "item4"}},
                {{"item5", "item6"},{"item7", "item8"}},
                {{"item9", "item10"},{"item11", ""}}
            };
        std::vector<std::vector<std::vector<OC::OCRepresentation>>> objarr
            {
                {{subRep1, subRep2},{subRep3, subRep1}},
                {{subRep2, subRep3},{subRep2, subRep1}},
                {{subRep3, subRep2},{subRep1, subRep2}}
            };

        startRep["iarr"] = iarr;
        startRep["darr"] = darr;
        startRep["barr"] = barr;
        startRep["strarr"] = strarr;
        startRep["objarr"] = objarr;

        // Encode/decode
        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        // Test
        std::vector<std::vector<std::vector<int>>> iarr2 = r["iarr"];
        std::vector<std::vector<std::vector<double>>> darr2 = r["darr"];
        std::vector<std::vector<std::vector<bool>>> barr2 = r["barr"];
        std::vector<std::vector<std::vector<std::string>>> strarr2 = r["strarr"];
        std::vector<std::vector<std::vector<OC::OCRepresentation>>> objarr2 = r["objarr"];

        EXPECT_EQ(iarr, iarr2);
        EXPECT_EQ(darr, darr2);
        EXPECT_EQ(barr, barr2);
        EXPECT_EQ(strarr, strarr2);
        EXPECT_EQ(objarr, objarr2);
        OCPayloadDestroy(cparsed);
    }

    TEST(RepresentationEncoding, ThreeDVectorsJagged)
    {
        // Setup
        OC::OCRepresentation startRep;

        OC::OCRepresentation subRep1;
        OC::OCRepresentation subRep2;
        OC::OCRepresentation subRep3;
        subRep1.setNULL("NullAttr");
        subRep1.setValue("IntAttr", 77);
        subRep2.setValue("DoubleAttr", 3.333);
        subRep2.setValue("BoolAttr", true);
        subRep3.setValue("StringAttr", std::string("String attr"));

        std::vector<std::vector<std::vector<int>>> iarr
            {
                {{1,2,3},{4,5,6}},
                {{7,8,9},{10,12}},
                {{13,14,15},{16,17,18}}
            };
        std::vector<std::vector<std::vector<double>>> darr
            {
                {{1.1,2.2,3.3},{4.4,5.5,6.6}},
                {{7.7,8.7,9.7},{10.7,12.7}},
                {{13.7,14.7,15.7},{16.7,17.7,18.7}}
            };
        std::vector<std::vector<std::vector<bool>>> barr
            {
                {{false, true},{true}},
                {{false, true},{true, false}}
            };
        std::vector<std::vector<std::vector<std::string>>> strarr
            {
                {{"item1", "item2"},{"item3", "item4"}},
                {{"item5", "item6"},{"item8"}},
                {{"item9", "item10"},{"item11", ""}}
            };
        std::vector<std::vector<std::vector<OC::OCRepresentation>>> objarr
            {
                {{subRep1, subRep2},{subRep3, subRep1}},
                {{subRep2, subRep3},{subRep2}},
                {{subRep3, subRep2}}
            };

        startRep["iarr"] = iarr;
        startRep["darr"] = darr;
        startRep["barr"] = barr;
        startRep["strarr"] = strarr;
        startRep["objarr"] = objarr;

        // Encode/decode
        OC::MessageContainer mc1;
        mc1.addRepresentation(startRep);

        OCRepPayload* cstart = mc1.getPayload();
        EXPECT_EQ(PAYLOAD_TYPE_REPRESENTATION, cstart->base.type);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;
        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)cstart, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));
        OCPayloadDestroy((OCPayload*)cstart);
        OICFree(cborData);

        OC::MessageContainer mc2;
        mc2.setPayload(cparsed);
        EXPECT_EQ(1u, mc2.representations().size());
        const OC::OCRepresentation& r = mc2.representations()[0];

        // Test
        std::vector<std::vector<std::vector<int>>> iarr2 = r["iarr"];
        std::vector<std::vector<std::vector<double>>> darr2 = r["darr"];
        std::vector<std::vector<std::vector<bool>>> barr2 = r["barr"];
        std::vector<std::vector<std::vector<std::string>>> strarr2 = r["strarr"];
        std::vector<std::vector<std::vector<OC::OCRepresentation>>> objarr2 = r["objarr"];

        // Note: due to the way that the CSDK works, all 3d arrays need to be cuboidal.
        // Since std::vector doesn't require this, items received on the other side end up
        // being backfilled.  This section removes the backfilling
        iarr2[1][1].pop_back();
        darr2[1][1].pop_back();
        barr2[0][1].pop_back();
        strarr2[1][1].pop_back();
        objarr2[1][1].pop_back();
        objarr2[2].pop_back();

        EXPECT_EQ(iarr, iarr2);
        EXPECT_EQ(darr, darr2);
        EXPECT_EQ(barr, barr2);
        EXPECT_EQ(strarr, strarr2);
        EXPECT_EQ(objarr, objarr2);
        OCPayloadDestroy(cparsed);
    }

    TEST(DiscoveryRTandIF, SingleItemNormal)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "rt.singleitem");
        OCResourcePayloadAddInterface(resource, "if.singleitem");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next);
        EXPECT_STREQ("rt.singleitem", parsedResource->types->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next);
        EXPECT_STREQ("if.singleitem", parsedResource->interfaces->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }

    TEST(DiscoveryRTandIF, SingleItemFrontTrim)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "    rt.singleitem");
        OCResourcePayloadAddInterface(resource, "    if.singleitem");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next);
        EXPECT_STREQ("rt.singleitem", parsedResource->types->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next);
        EXPECT_STREQ("if.singleitem", parsedResource->interfaces->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(DiscoveryRTandIF, SingleItemBackTrim)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "rt.singleitem    ");
        OCResourcePayloadAddInterface(resource, "if.singleitem    ");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next);
        EXPECT_STREQ("rt.singleitem", parsedResource->types->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next);
        EXPECT_STREQ("if.singleitem", parsedResource->interfaces->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(DiscoveryRTandIF, SingleItemBothTrim)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "    rt.singleitem    ");
        OCResourcePayloadAddInterface(resource, "    if.singleitem     ");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next);
        EXPECT_STREQ("rt.singleitem", parsedResource->types->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next);
        EXPECT_STREQ("if.singleitem", parsedResource->interfaces->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(DiscoveryRTandIF, MultiItemsNormal)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "rt.firstitem");
        OCResourcePayloadAddResourceType(resource, "rt.seconditem");
        OCResourcePayloadAddInterface(resource, "if.firstitem");
        OCResourcePayloadAddInterface(resource, "if.seconditem");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next->next);
        EXPECT_STREQ("rt.firstitem", parsedResource->types->value);
        EXPECT_STREQ("rt.seconditem", parsedResource->types->next->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next->next);
        EXPECT_STREQ("if.firstitem", parsedResource->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedResource->interfaces->next->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(DiscoveryRTandIF, MultiItemExtraLeadSpaces)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "  rt.firstitem");
        OCResourcePayloadAddResourceType(resource, "  rt.seconditem");
        OCResourcePayloadAddInterface(resource, "  if.firstitem");
        OCResourcePayloadAddInterface(resource, "  if.seconditem");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next->next);
        EXPECT_STREQ("rt.firstitem", parsedResource->types->value);
        EXPECT_STREQ("rt.seconditem", parsedResource->types->next->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next->next);
        EXPECT_STREQ("if.firstitem", parsedResource->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedResource->interfaces->next->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(DiscoveryRTandIF, MultiItemExtraTrailSpaces)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "rt.firstitem  ");
        OCResourcePayloadAddResourceType(resource, "rt.seconditem  ");
        OCResourcePayloadAddInterface(resource, "if.firstitem  ");
        OCResourcePayloadAddInterface(resource, "if.seconditem  ");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next->next);
        EXPECT_STREQ("rt.firstitem", parsedResource->types->value);
        EXPECT_STREQ("rt.seconditem", parsedResource->types->next->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next->next);
        EXPECT_STREQ("if.firstitem", parsedResource->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedResource->interfaces->next->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(DiscoveryRTandIF, MultiItemBothSpaces)
    {
        OCDiscoveryPayload* payload = OCDiscoveryPayloadCreate();
        OCResourcePayload* resource = (OCResourcePayload*)OICCalloc(1, sizeof(OCResourcePayload));
        payload->resources = resource;

        OCResourcePayloadAddResourceType(resource, "  rt.firstitem  ");
        OCResourcePayloadAddResourceType(resource, "  rt.seconditem  ");
        OCResourcePayloadAddInterface(resource, "  if.firstitem  ");
        OCResourcePayloadAddInterface(resource, "  if.seconditem  ");
        resource->uri = OICStrdup("/uri/thing");
        resource->sid = (uint8_t*)OICMalloc(16);

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_DISCOVERY,
                    cborData, cborSize));

        EXPECT_EQ(1u, OCDiscoveryPayloadGetResourceCount((OCDiscoveryPayload*)cparsed));
        OCResourcePayload* parsedResource = ((OCDiscoveryPayload*)cparsed)->resources;

        EXPECT_EQ(NULL, parsedResource->next);

        EXPECT_EQ(NULL, parsedResource->types->next->next);
        EXPECT_STREQ("rt.firstitem", parsedResource->types->value);
        EXPECT_STREQ("rt.seconditem", parsedResource->types->next->value);
        EXPECT_EQ(NULL, parsedResource->interfaces->next->next);
        EXPECT_STREQ("if.firstitem", parsedResource->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedResource->interfaces->next->value);

        OICFree(cborData);
        OCPayloadDestroy(cparsed);
        OCDiscoveryPayloadDestroy(payload);
    }
    TEST(RepresentationEncodingRTandIF, SingleItemNormal)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "rt.firstitem");
        OCRepPayloadAddInterface(payload, "if.firstitem");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_EQ(NULL, parsedPayload->types->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, SingleItemFrontTrim)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "  rt.firstitem");
        OCRepPayloadAddInterface(payload, "  if.firstitem");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_EQ(NULL, parsedPayload->types->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, SingleItemBackTrim)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "rt.firstitem  ");
        OCRepPayloadAddInterface(payload, "if.firstitem  ");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_EQ(NULL, parsedPayload->types->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, SingleItemBothTrim)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "  rt.firstitem  ");
        OCRepPayloadAddInterface(payload, "  if.firstitem  ");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_EQ(NULL, parsedPayload->types->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, MultiItemsNormal)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "rt.firstitem");
        OCRepPayloadAddResourceType(payload, "rt.seconditem");
        OCRepPayloadAddInterface(payload, "if.firstitem");
        OCRepPayloadAddInterface(payload, "if.seconditem");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_STREQ("rt.seconditem", parsedPayload->types->next->value);
        EXPECT_EQ(NULL, parsedPayload->types->next->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedPayload->interfaces->next->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, MultiItemExtraLeadSpaces)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "  rt.firstitem");
        OCRepPayloadAddResourceType(payload, "  rt.seconditem");
        OCRepPayloadAddInterface(payload, "  if.firstitem");
        OCRepPayloadAddInterface(payload, "  if.seconditem");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_STREQ("rt.seconditem", parsedPayload->types->next->value);
        EXPECT_EQ(NULL, parsedPayload->types->next->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedPayload->interfaces->next->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, MultiItemExtraTrailSpaces)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "rt.firstitem  ");
        OCRepPayloadAddResourceType(payload, "rt.seconditem  ");
        OCRepPayloadAddInterface(payload, "if.firstitem  ");
        OCRepPayloadAddInterface(payload, "if.seconditem  ");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_STREQ("rt.seconditem", parsedPayload->types->next->value);
        EXPECT_EQ(NULL, parsedPayload->types->next->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedPayload->interfaces->next->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
    TEST(RepresentationEncodingRTandIF, MultiItemExtraMiddleSpaces)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        OCRepPayloadSetUri(payload, "/this/uri");
        OCRepPayloadAddResourceType(payload, "  rt.firstitem  ");
        OCRepPayloadAddResourceType(payload, "  rt.seconditem  ");
        OCRepPayloadAddInterface(payload, "  if.firstitem  ");
        OCRepPayloadAddInterface(payload, "  if.seconditem  ");

        uint8_t* cborData;
        size_t cborSize;
        OCPayload* cparsed;

        EXPECT_EQ(OC_STACK_OK, OCConvertPayload((OCPayload*)payload, &cborData, &cborSize));
        EXPECT_EQ(OC_STACK_OK, OCParsePayload(&cparsed, PAYLOAD_TYPE_REPRESENTATION,
                    cborData, cborSize));

        OCRepPayload* parsedPayload = (OCRepPayload*)cparsed;

        EXPECT_STREQ("rt.firstitem", parsedPayload->types->value);
        EXPECT_STREQ("rt.seconditem", parsedPayload->types->next->value);
        EXPECT_EQ(NULL, parsedPayload->types->next->next);
        EXPECT_STREQ("if.firstitem", parsedPayload->interfaces->value);
        EXPECT_STREQ("if.seconditem", parsedPayload->interfaces->next->value);
        EXPECT_EQ(NULL, parsedPayload->interfaces->next->next);


        OICFree(cborData);
        OCRepPayloadDestroy(payload);
        OCPayloadDestroy(cparsed);
    }
}
