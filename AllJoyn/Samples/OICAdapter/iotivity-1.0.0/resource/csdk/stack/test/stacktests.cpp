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


extern "C"
{
    #include "ocstack.h"
    #include "ocstackinternal.h"
    #include "logger.h"
    #include "oic_malloc.h"
}

#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <stdint.h>

#include "gtest_helper.h"

using namespace std;

namespace itst = iotivity::test;

#define DEFAULT_CONTEXT_VALUE 0x99

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------
static const char TAG[] = "TestHarness";

char gDeviceUUID[] = "myDeviceUUID";
char gManufacturerName[] = "myName";
char gTooLongManufacturerName[] = "extremelylongmanufacturername";
char gManufacturerUrl[] = "www.foooooooooooooooo.baaaaaaaaaaaaar";

std::chrono::seconds const SHORT_TEST_TIMEOUT = std::chrono::seconds(5);

//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------
extern "C"  OCStackApplicationResult asyncDoResourcesCallback(void* ctx,
        OCDoHandle /*handle*/, OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Entering asyncDoResourcesCallback");

    EXPECT_EQ(OC_STACK_OK, clientResponse->result);

    if(ctx == (void*)DEFAULT_CONTEXT_VALUE) {
        OC_LOG_V(INFO, TAG, "Callback Context recvd successfully");
    }
    OC_LOG_V(INFO, TAG, "result = %d", clientResponse->result);

    return OC_STACK_KEEP_TRANSACTION;
}

//-----------------------------------------------------------------------------
// Entity handler
//-----------------------------------------------------------------------------
OCEntityHandlerResult entityHandler(OCEntityHandlerFlag /*flag*/,
        OCEntityHandlerRequest * /*entityHandlerRequest*/,
        void* /*callbackParam*/)
{
    OC_LOG(INFO, TAG, "Entering entityHandler");

    return OC_EH_OK;
}

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
void InitStack(OCMode mode)
{
    OC_LOG(INFO, TAG, "Entering InitStack");

    EXPECT_EQ(OC_STACK_OK, OCInit(NULL, 0, mode));
    OC_LOG(INFO, TAG, "Leaving InitStack");
}

uint8_t InitNumExpectedResources()
{
#ifdef WITH_PRESENCE
    // When presence is enabled, it is a resource and so is (currently) included
    // in the returned resource count returned by the OCGetNumberOfResources API.
    return 1;
#else
    return 0;
#endif
}

uint8_t InitResourceIndex()
{
#ifdef WITH_PRESENCE
    // When presence is enabled, it is a resource and so is (currently) included
    // in the returned resource count returned by the OCGetNumberOfResources API.
    // The index of the presence resource is 0, so the first user app resource index
    // is 1.
    return 1;
#else
    return 0;
#endif
}
//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------

TEST(StackInit, StackInitNullAddr)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackInit, StackInitNullPort)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 0, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackInit, StackInitNullAddrAndPort)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit(0, 0, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackInit, StackInitInvalidMode)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_ERROR, OCInit(0, 0, (OCMode)10));
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

TEST(StackStart, StackStartSuccessClient)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, StackStartSuccessServer)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, StackStartSuccessClientServer)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, StackStartSuccessiveInits)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.2", 5683, OC_SERVER));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, SetPlatformInfoValid)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));

    OCPlatformInfo info =
    {
        gDeviceUUID,
        gManufacturerName,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    EXPECT_EQ(OC_STACK_OK, OCSetPlatformInfo(info));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, SetPlatformInfoWithNoPlatformID)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));

    OCPlatformInfo info =
     {
         0,
         gDeviceUUID,
         0, 0, 0, 0, 0, 0, 0, 0, 0
     };

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSetPlatformInfo(info));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, SetPlatformInfoWithNoManufacturerName)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));

    OCPlatformInfo info =
    {
        gDeviceUUID,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSetPlatformInfo(info));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, SetPlatformInfoWithZeroLengthManufacturerName)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));

    OCPlatformInfo info =
    {
        gDeviceUUID,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    info.manufacturerName = (char *) "";

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSetPlatformInfo(info));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, SetPlatformInfoWithTooLongManufacName)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));

    OCPlatformInfo info =
    {
        gDeviceUUID,
        gTooLongManufacturerName,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSetPlatformInfo(info));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStart, SetPlatformInfoWithTooLongManufacURL)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_SERVER));
    OCPlatformInfo info =
    {
        gDeviceUUID,
        gManufacturerName,
        gManufacturerUrl,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSetPlatformInfo(info));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackDiscovery, DISABLED_DoResourceDeviceDiscovery)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OCCallbackData cbData;
    OCDoHandle handle;

    OC_LOG(INFO, TAG, "Starting DoResourceDeviceDiscovery test ");
    InitStack(OC_CLIENT);

    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    strcpy(szQueryUri, OC_RSRVD_WELL_KNOWN_URI);
    cbData.cb = asyncDoResourcesCallback;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    EXPECT_EQ(OC_STACK_OK, OCDoResource(&handle,
                                        OC_REST_GET,
                                        szQueryUri,
                                        0,
                                        0,
                                        CT_ADAPTER_IP,
                                        OC_LOW_QOS,
                                        &cbData,
                                        NULL,
                                        0));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackStop, StackStopWithoutInit)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

TEST(StackStop, StackStopRepeated)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    EXPECT_EQ(OC_STACK_OK, OCInit("127.0.0.1", 5683, OC_CLIENT));
    EXPECT_EQ(OC_STACK_OK, OCStop());
    EXPECT_EQ(OC_STACK_ERROR, OCStop());
}

TEST(StackResource, DISABLED_UpdateResourceNullURI)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OCCallbackData cbData;
    OCDoHandle handle;

    OC_LOG(INFO, TAG, "Starting UpdateResourceNullURI test");
    InitStack(OC_CLIENT);

    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };
    strcpy(szQueryUri, OC_RSRVD_WELL_KNOWN_URI);
    cbData.cb = asyncDoResourcesCallback;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    EXPECT_EQ(OC_STACK_OK, OCDoResource(&handle,
                                        OC_REST_GET,
                                        szQueryUri,
                                        0,
                                        0,
                                        CT_ADAPTER_IP,
                                        OC_LOW_QOS,
                                        &cbData,
                                        NULL,
                                        0));
    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceBadParams)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceBadParams test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(NULL, //&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            NULL, //"core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    // Property bitmask out of range
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            128));// invalid bitmask for OCResourceProperty

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceBadUri)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceBadUri test");
    InitStack(OC_SERVER);

    const char *uri65 = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL";

    OCResourceHandle handle;

    EXPECT_EQ(OC_STACK_INVALID_URI, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            NULL, //"/a/led",
                                            0,
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_URI, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "", //"/a/led",
                                            0,
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_URI, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            uri65, //"/a/led",
                                            0,
                                            0,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceSuccess)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceSuccess test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceSuccessWithResourcePolicyPropNone)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceSuccessWithResourcePolicyPropNone test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    // the resource is non-discoverable & non-observable by the client.
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_RES_PROP_NONE));// the resource is non-discoverable &
                                                // non-observable by the client.
    const char* url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceWithClientStackMode)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceSuccess test");
    InitStack(OC_CLIENT);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceFailDuplicateUri)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceFailDuplicateUri test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceMultipleResources)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceMultipleResources test");
    InitStack(OC_SERVER);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    const char *url = OCGetResourceUri(handle1);
    EXPECT_STREQ("/a/led1", url);

    url = OCGetResourceUri(handle2);
    EXPECT_STREQ("/a/led2", url);

    url = OCGetResourceUri(handle3);
    EXPECT_STREQ("/a/led3", url);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceBadResoureType)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceBadResoureType test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle,
                                            NULL, //"core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCCreateResource(&handle2,
                                            "",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, CreateResourceGoodResourceType)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting CreateResourceGoodResourceType test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceTypeName)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting ResourceTypeName test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);
    const char *resourceTypeName = OCGetResourceTypeName(handle, 0);
    EXPECT_STREQ("core.led", resourceTypeName);

    // try getting resource type names with an invalid index
    resourceTypeName = OCGetResourceTypeName(handle, 1);
    EXPECT_STREQ(NULL, resourceTypeName);
    // try getting resource type names with an invalid index
    resourceTypeName = OCGetResourceTypeName(handle, 10);
    EXPECT_STREQ(NULL, resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceTypeAttrRepresentation)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting ResourceTypeAttrRepresentation test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceTypeInterface)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting ResourceTypeInterface test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    // try getting resource interface names with an invalid index
    resourceInterfaceName = OCGetResourceInterfaceName(handle, 1);
    EXPECT_STREQ(NULL, resourceInterfaceName);
    // try getting resource interface names with an invalid index
    resourceInterfaceName = OCGetResourceInterfaceName(handle, 10);
    EXPECT_STREQ(NULL, resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceDefaultInterfaceAlwaysFirst)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);

    OC_LOG(INFO, TAG, "Starting ResourceDefaultInterfaceAlwaysFirst test");

    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle,
                                        OC_RSRVD_INTERFACE_DEFAULT));
    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(2, numResourceInterfaces);

    const char *interfaceName_1 = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ(OC_RSRVD_INTERFACE_DEFAULT, interfaceName_1);

    const char*interfaceName_2 = OCGetResourceInterfaceName(handle, 1);
    EXPECT_STREQ("core.rw", interfaceName_2);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceDuplicateDefaultInterfaces)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);

    OC_LOG(INFO, TAG, "Starting ResourceDuplicateDefaultInterfaces test");

    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle,
                                        OC_RSRVD_INTERFACE_DEFAULT));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle,
                                        OC_RSRVD_INTERFACE_DEFAULT));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(2, numResourceInterfaces);

    const char *interfaceName_1 = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ(OC_RSRVD_INTERFACE_DEFAULT, interfaceName_1);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceDuplicateNonDefaultInterfaces)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);

    OC_LOG(INFO, TAG, "Starting ResourceDuplicateInterfaces test");

    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle,
                                        "core.rw"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle,
                                        "core.rw"));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, ResourceTypeInterfaceMethods)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting ResourceTypeInterfaceMethods test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, GetResourceProperties)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting GetResourceProperties test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_ACTIVE|OC_DISCOVERABLE|OC_OBSERVABLE, OCGetResourceProperties(handle));
    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, StackTestResourceDiscoverOneResourceBad)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting StackTestResourceDiscoverOneResourceBad test");
    InitStack(OC_SERVER);
    uint8_t numResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a1/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a1/led", url);

    //EXPECT_EQ(OC_STACK_INVALID_URI, OCHandleServerRequest(&res, uri, query, req, rsp));
    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle));
    uint8_t numExpectedResources = 0;

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));
    EXPECT_EQ(numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, StackTestResourceDiscoverOneResource)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting StackTestResourceDiscoverOneResource test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    const char *url = OCGetResourceUri(handle);
    EXPECT_STREQ("/a/led", url);

    //EXPECT_EQ(OC_STACK_OK, OCHandleServerRequest(&res, uri, query, req, rsp));
    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResource, StackTestResourceDiscoverManyResources)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting StackTestResourceDiscoverManyResources test");
    InitStack(OC_SERVER);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE));
    const char *url = OCGetResourceUri(handle1);
    EXPECT_STREQ("/a/led1", url);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    url = OCGetResourceUri(handle2);
    EXPECT_STREQ("/a/led2", url);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle2, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle2, "core.colorled"));

    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    url = OCGetResourceUri(handle3);
    EXPECT_STREQ("/a/led3", url);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle3, OC_RSRVD_INTERFACE_LL));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle3, OC_RSRVD_INTERFACE_BATCH));

    OCResourceHandle handle4;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle4,
                                            "core.led",
                                            "core.rw",
                                            "/a/led4",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE));
    url = OCGetResourceUri(handle4);
    EXPECT_STREQ("/a/led4", url);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle4, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle4, OC_RSRVD_INTERFACE_LL));
    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle4, OC_RSRVD_INTERFACE_BATCH));

    //EXPECT_EQ(OC_STACK_OK, OCHandleServerRequest(&res, uri, query, req, rsp));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindResourceTypeNameBad)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceTypeNameBad test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);
    const char *resourceTypeName = OCGetResourceTypeName(handle, 0);
    EXPECT_STREQ("core.led", resourceTypeName);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceTypeToResource(handle, NULL));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindResourceTypeNameGood)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceTypeNameGood test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);
    const char *resourceTypeName = OCGetResourceTypeName(handle, 0);
    EXPECT_STREQ("core.led", resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(2, numResourceTypes);
    resourceTypeName = OCGetResourceTypeName(handle, 1);
    EXPECT_STREQ("core.brightled", resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.reallybrightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(3, numResourceTypes);
    resourceTypeName = OCGetResourceTypeName(handle, 2);
    EXPECT_STREQ("core.reallybrightled", resourceTypeName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindResourceTypeAttribRepGood)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceTypeAttribRepGood test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceTypes;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(1, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.brightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(2, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceTypeToResource(handle, "core.reallybrightled"));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceTypes(handle, &numResourceTypes));
    EXPECT_EQ(3, numResourceTypes);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}


TEST(StackBind, BindResourceInterfaceNameBad)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceInterfaceNameBad test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceInterfaceToResource(handle, NULL));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindResourceInterfaceNameGood)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceInterfaceNameGood test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle, "core.r"));

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(2, numResourceInterfaces);
    resourceInterfaceName = OCGetResourceInterfaceName(handle, 1);
    EXPECT_STREQ("core.r", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindResourceInterfaceMethodsBad)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceInterfaceMethodsBad test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceInterfaceToResource(handle, 0));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindResourceInterfaceMethodsGood)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindResourceInterfaceMethodsGood test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCBindResourceInterfaceToResource(handle, "core.r"));

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle, &numResourceInterfaces));
    EXPECT_EQ(2, numResourceInterfaces);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindContainedResourceBad)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindContainedResourceBad test");
    InitStack(OC_SERVER);

    OCResourceHandle containerHandle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&containerHandle,
                                            "core.led",
                                            "core.rw",
                                            "/a/kitchen",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResource(containerHandle, containerHandle));

    EXPECT_EQ(OC_STACK_ERROR, OCBindResource((OCResourceHandle) 0, handle0));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindContainedResourceGood)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindContainedResourceGood test");
    InitStack(OC_SERVER);

    uint8_t numResources = 0;
    uint8_t numExpectedResources = 0;

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));

    OCResourceHandle containerHandle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&containerHandle,
                                            "core.led",
                                            "core.rw",
                                            "/a/kitchen",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle4;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle4,
                                            "core.led",
                                            "core.rw",
                                            "/a/led4",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle5;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle5,
                                            "core.led",
                                            "core.rw",
                                            "/a/led5",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);


    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle0));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle1));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle2));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle3));
    EXPECT_EQ(OC_STACK_OK, OCBindResource(containerHandle, handle4));
    EXPECT_EQ(OC_STACK_ERROR, OCBindResource(containerHandle, handle5));

    EXPECT_EQ(handle0, OCGetResourceHandleFromCollection(containerHandle, 0));
    EXPECT_EQ(handle1, OCGetResourceHandleFromCollection(containerHandle, 1));
    EXPECT_EQ(handle2, OCGetResourceHandleFromCollection(containerHandle, 2));
    EXPECT_EQ(handle3, OCGetResourceHandleFromCollection(containerHandle, 3));
    EXPECT_EQ(handle4, OCGetResourceHandleFromCollection(containerHandle, 4));

    EXPECT_EQ(NULL, OCGetResourceHandleFromCollection(containerHandle, 5));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}


TEST(StackBind, BindEntityHandlerBad)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindEntityHandlerBad test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCBindResourceHandler(NULL, NULL, NULL));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackBind, BindEntityHandlerGood)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting BindEntityHandlerGood test");
    InitStack(OC_SERVER);

    OCResourceHandle handle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle,
                                            "core.led",
                                            "core.rw",
                                            "/a/led",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));

    OCEntityHandler myHandler = entityHandler;

    EXPECT_EQ(OC_STACK_OK, OCBindResourceHandler(handle, myHandler, NULL));

    EXPECT_EQ(myHandler, OCGetResourceHandler(handle));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResourceAccess, GetResourceByIndex)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting GetResourceByIndex test");
    InitStack(OC_SERVER);

    uint8_t numResources = 0;
    uint8_t numExpectedResources = 0;
    uint8_t resourceIndex = 0;
    uint8_t prevResources = 0;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));
    prevResources = numExpectedResources;
    OCResourceHandle containerHandle;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&containerHandle,
                                            "core.led",
                                            "core.rw",
                                            "/a/kitchen",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle3;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle3,
                                            "core.led",
                                            "core.rw",
                                            "/a/led3",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle4;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle4,
                                            "core.led",
                                            "core.rw",
                                            "/a/led4",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle5;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle5,
                                            "core.led",
                                            "core.rw",
                                            "/a/led5",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);
    resourceIndex += prevResources;
    EXPECT_EQ(containerHandle, OCGetResourceHandle(resourceIndex));
    EXPECT_EQ(handle0, OCGetResourceHandle(++resourceIndex));
    EXPECT_EQ(handle1, OCGetResourceHandle(++resourceIndex));
    EXPECT_EQ(handle2, OCGetResourceHandle(++resourceIndex));
    EXPECT_EQ(handle3, OCGetResourceHandle(++resourceIndex));
    EXPECT_EQ(handle4, OCGetResourceHandle(++resourceIndex));
    EXPECT_EQ(handle5, OCGetResourceHandle(++resourceIndex));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResourceAccess, DeleteHeadResource)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting DeleteHeadResource test");
    InitStack(OC_SERVER);

    uint8_t numResources = 0;
    uint8_t numExpectedResources = 0;

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle0));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(--numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResourceAccess, DeleteHeadResource2)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting DeleteHeadResource2 test");
    InitStack(OC_SERVER);

    uint8_t numResources = 0;
    uint8_t numExpectedResources = 0;

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));
    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle0));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(--numExpectedResources, numResources);

    EXPECT_EQ(handle1, OCGetResourceHandle(numResources - 1));

    EXPECT_EQ(OC_STACK_OK, OCStop());
}


TEST(StackResourceAccess, DeleteLastResource)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting DeleteLastResource test");
    InitStack(OC_SERVER);

    uint8_t numResources = 0;
    uint8_t numExpectedResources = 0;

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));

    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle1));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(--numExpectedResources, numResources);

    EXPECT_EQ(handle0, OCGetResourceHandle(numResources - 1));

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(StackResourceAccess, DeleteMiddleResource)
{
    itst::DeadmanTimer killSwitch(SHORT_TEST_TIMEOUT);
    OC_LOG(INFO, TAG, "Starting DeleteMiddleResource test");
    InitStack(OC_SERVER);

    uint8_t numResources = 0;
    uint8_t numExpectedResources = 0;
    uint8_t resourceIndex = InitResourceIndex();

    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numExpectedResources));
    resourceIndex = numExpectedResources;
    OCResourceHandle handle0;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle0,
                                            "core.led",
                                            "core.rw",
                                            "/a/led0",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle1;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle1,
                                            "core.led",
                                            "core.rw",
                                            "/a/led1",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    OCResourceHandle handle2;
    EXPECT_EQ(OC_STACK_OK, OCCreateResource(&handle2,
                                            "core.led",
                                            "core.rw",
                                            "/a/led2",
                                            0,
                                            NULL,
                                            OC_DISCOVERABLE|OC_OBSERVABLE));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(++numExpectedResources, numResources);

    EXPECT_EQ(OC_STACK_OK, OCDeleteResource(handle1));
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResources(&numResources));
    EXPECT_EQ(--numExpectedResources, numResources);

    EXPECT_EQ(handle0, OCGetResourceHandle(resourceIndex));
    EXPECT_EQ(handle2, OCGetResourceHandle(++resourceIndex));

    // Make sure the resource elements are still correct
    uint8_t numResourceInterfaces;
    EXPECT_EQ(OC_STACK_OK, OCGetNumberOfResourceInterfaces(handle2, &numResourceInterfaces));
    EXPECT_EQ(1, numResourceInterfaces);
    const char *resourceInterfaceName = OCGetResourceInterfaceName(handle2, 0);
    EXPECT_STREQ("core.rw", resourceInterfaceName);

    EXPECT_EQ(OC_STACK_OK, OCStop());
}

TEST(PODTests, OCHeaderOption)
{
    EXPECT_TRUE(std::is_pod<OCHeaderOption>::value);
}

TEST(PODTests, OCCallbackData)
{
    EXPECT_TRUE(std::is_pod<OCHeaderOption>::value);
}
