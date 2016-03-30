/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

package org.iotivity.base;

import android.test.InstrumentationTestCase;
import android.util.Log;

import java.util.Date;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class SmokeTest extends InstrumentationTestCase {
    private static final String TAG = "SmokeTest";
    private Random rnd = new Random();

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        PlatformConfig cfg = new PlatformConfig(
                getInstrumentation().getContext(),
                ServiceType.IN_PROC,
                ModeType.CLIENT_SERVER,
                "0.0.0.0",
                0,
                QualityOfService.LOW);

        OcPlatform.Configure(cfg);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testResourceRegisterUnregister() throws InterruptedException {
        final String resourceType = "unit.test.resource" +
                new Date().getTime();
        final CountDownLatch signal = new CountDownLatch(1);

        OcPlatform.EntityHandler entityHandler = new OcPlatform.EntityHandler() {
            @Override
            public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                return EntityHandlerResult.OK;
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        Log.i(TAG, "Host: " + resource.getHost());
                        Log.i(TAG, "Server ID: " + resource.getServerId());
                        Log.i(TAG, "Connectivity Types: ");
                        for (OcConnectivityType connectivityType : resource.getConnectivityTypeSet()) {
                            Log.i(TAG, " " + connectivityType);
                        }
                        signal.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    entityHandler,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testStartStopListenForPresence() throws InterruptedException {
        final String resourceType = "unit.test.resource" +
                new Date().getTime();
        final CountDownLatch signal = new CountDownLatch(1);

        OcPlatform.EntityHandler entityHandler = new OcPlatform.EntityHandler() {
            @Override
            public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                return EntityHandlerResult.OK;
            }
        };

        final OcPlatform.OnPresenceListener presenceListener = new OcPlatform.OnPresenceListener() {
            @Override
            public void onPresence(OcPresenceStatus ocPresenceStatus, int nonce, String hostAddress) {
                Log.i(TAG, "onPresence status " + ocPresenceStatus.toString() + " nonce " + nonce);
                signal.countDown();
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        try {
                            //client
                            OcPresenceHandle presenceHandle = OcPlatform.subscribePresence(
                                    resource.getHost(),
                                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                                    presenceListener
                            );

                            //wait for onPresence event
                            assertTrue(signal.await(60, TimeUnit.SECONDS));

                            //client
                            OcPlatform.unsubscribePresence(presenceHandle);
                        } catch (OcException e) {
                            assertTrue(false);
                        } catch (InterruptedException e) {
                            assertTrue(false);
                        }
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    entityHandler,
                    EnumSet.of(ResourceProperty.DISCOVERABLE, ResourceProperty.OBSERVABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //server
            OcPlatform.startPresence(OcPlatform.DEFAULT_PRESENCE_TTL);

            //wait for onPresence event
            assertTrue(signal.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.stopPresence();

            //client
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testHandleGetRequest() throws InterruptedException {
        final String someKey = "SomeKey";
        final String someValue = "SomeValue";
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal1 = new CountDownLatch(1);
        final CountDownLatch signal2 = new CountDownLatch(1);
        final List<OcResource> ocResourceList = new LinkedList<OcResource>();

        //client
        final OcResource.OnGetListener onGetListener = new OcResource.OnGetListener() {
            @Override
            public void onGetCompleted(List<OcHeaderOption> headerOptionList, OcRepresentation ocRepresentation) {
                if (!headerOptionList.isEmpty()) {
                    for (OcHeaderOption headerOption : headerOptionList) {
                        Log.i(TAG, "Header option " +
                                headerOption.getOptionId() +
                                " : " +
                                headerOption.getOptionData());
                    }
                }
                try {
                    Log.i(TAG, "Power: " + ocRepresentation.getValue("power"));
                } catch (OcException e) {
                    Log.e(TAG, e.toString());
                    assertTrue(false);
                }
                signal2.countDown();
            }

            @Override
            public void onGetFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                    if (ErrorCode.NO_RESOURCE != errCode) {
                        Log.e(TAG, ocEx.getMessage());
                        assertTrue(false);
                    }
                } else {
                    Log.e(TAG, ex.getMessage());
                    assertTrue(false);
                }
            }
        };

        //client
        final OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        Map<String, String> queryParamsMap = new HashMap<String, String>();
                        queryParamsMap.put(someKey, someValue);

                        ocResourceList.add(resource);
                        try {
                            resource.get(queryParamsMap, onGetListener);
//                            TODO there is a bug in the stack that prevents the usage of the following APIs
//                            resource.get(resourceType, OcPlatform.DEFAULT_INTERFACE, queryParamsMap,
//                                    onGetListener);
//
//                            resource.get(queryParamsMap, onGetListener, QualityOfService.LOW);
//
//                            resource.get(resourceType, OcPlatform.DEFAULT_INTERFACE,queryParamsMap,
//                                    onGetListener, QualityOfService.LOW);
                        } catch (OcException e) {
                            Log.e(TAG, e.toString());
                            assertTrue(false);
                        }
                        signal1.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    new OcPlatform.EntityHandler() {
                        @Override
                        public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                            EnumSet<RequestHandlerFlag> handlerFlagSet =
                                    ocResourceRequest.getRequestHandlerFlagSet();

                            RequestType requestType = ocResourceRequest.getRequestType();

                            if (handlerFlagSet.contains(RequestHandlerFlag.INIT)) {
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.REQUEST)) {
                                OcResourceResponse ocResourceResponse = new OcResourceResponse();
                                ocResourceResponse.setRequestHandle(
                                        ocResourceRequest.getRequestHandle());
                                ocResourceResponse.setResourceHandle(
                                        ocResourceRequest.getResourceHandle());

                                switch (requestType) {
                                    case GET:
                                        Map<String, String> queryParams =
                                                ocResourceRequest.getQueryParameters();

                                        if (!(queryParams.containsKey(someKey) &&
                                                someValue.equals(queryParams.get(someKey)))) {
                                            assertTrue(false);
                                        }

                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(EntityHandlerResult.OK);
                                        ocResourceResponse.setResourceRepresentation(
                                                getRepresentation(74));
                                        break;
                                }

                                try {
                                    OcPlatform.sendResponse(ocResourceResponse);
                                } catch (OcException e) {
                                    Log.e(TAG, e.getMessage());
                                    return EntityHandlerResult.ERROR;
                                }
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.OBSERVER)) {
                            }
                            return EntityHandlerResult.OK;
                        }
                    },
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource(null,
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal1.await(60, TimeUnit.SECONDS));

            //wait for onGetCompleted event
            assertTrue(signal2.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testHandlePutRequest() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal1 = new CountDownLatch(1);
        final CountDownLatch signal2 = new CountDownLatch(3);
        final List<OcResource> ocResourceList = new LinkedList<OcResource>();

        final OcResource.OnPutListener onPutListener = new OcResource.OnPutListener() {
            @Override
            public void onPutCompleted(List<OcHeaderOption> headerOptionList, OcRepresentation ocRepresentation) {
                if (!headerOptionList.isEmpty()) {
                    for (OcHeaderOption headerOption : headerOptionList) {
                        Log.i(TAG, "Header option " +
                                headerOption.getOptionId() +
                                " : " +
                                headerOption.getOptionData());
                    }
                }
                try {
                    Log.i(TAG, "onPutCompleted Power: " + ocRepresentation.getValue("power"));
                } catch (OcException e) {
                    Log.e(TAG, e.getMessage());
                    assertTrue(false);
                }
                Log.i(TAG, "onPutCompleted Uri: " + ocRepresentation.getUri());
                signal2.countDown();
            }

            @Override
            public void onPutFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                    if (ErrorCode.NO_RESOURCE != errCode) {
                        Log.e(TAG, ocEx.getMessage());
                        assertTrue(false);
                    }
                } else {
                    Log.e(TAG, ex.getMessage());
                    assertTrue(false);
                }
            }
        };

        final OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        ocResourceList.add(resource);
                        try {
                            resource.put(
                                    getRepresentation(),
                                    new HashMap<String, String>(),
                                    onPutListener);

                            resource.put(
                                    getRepresentation(),
                                    new HashMap<String, String>(),
                                    onPutListener);

                            resource.put(
                                    getRepresentation(),
                                    new HashMap<String, String>(),
                                    new OcResource.OnPutListener() {
                                        @Override
                                        public void onPutCompleted(List<OcHeaderOption> headerOptionList, OcRepresentation ocRepresentation) {
                                            signal2.countDown();
                                        }

                                        @Override
                                        public void onPutFailed(Throwable ex) {
                                            if (ex instanceof OcException) {
                                                OcException ocEx = (OcException) ex;
                                                ErrorCode errCode = ocEx.getErrorCode();
                                                if (ErrorCode.NO_RESOURCE != errCode) {
                                                    Log.e(TAG, ocEx.getMessage());
                                                    assertTrue(false);
                                                }
                                            } else {
                                                Log.e(TAG, ex.getMessage());
                                                assertTrue(false);
                                            }
                                        }
                                    });

                        } catch (OcException e) {
                            assertTrue(false);
                        }
                        signal1.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    new OcPlatform.EntityHandler() {
                        @Override
                        public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                            EnumSet<RequestHandlerFlag> handlerFlagSet = ocResourceRequest.getRequestHandlerFlagSet();
                            RequestType requestType = ocResourceRequest.getRequestType();

                            if (handlerFlagSet.contains(RequestHandlerFlag.INIT)) {
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.REQUEST)) {
                                OcResourceResponse ocResourceResponse = new OcResourceResponse();
                                ocResourceResponse.setRequestHandle(
                                        ocResourceRequest.getRequestHandle());
                                ocResourceResponse.setResourceHandle(
                                        ocResourceRequest.getResourceHandle());

                                switch (requestType) {
                                    case GET:
                                        assertTrue(false);
                                        break;
                                    case PUT:
                                        OcRepresentation rep = ocResourceRequest.getResourceRepresentation();
                                        try {
                                            Log.i(TAG, "Put res. power: " + rep.getValue("power"));
                                        } catch (OcException e) {
                                            Log.e(TAG, e.toString());
                                            assertTrue(false);
                                        }
                                        Log.i(TAG, "URI: " + rep.getUri());

                                        ocResourceResponse.setResponseResult(EntityHandlerResult.OK);
                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResourceRepresentation(rep);
                                        break;
                                    case POST:
                                        assertTrue(false);
                                        break;
                                    case DELETE:
                                        break;
                                }

                                try {
                                    OcPlatform.sendResponse(ocResourceResponse);
                                } catch (OcException e) {
                                    Log.e(TAG, e.getMessage());
                                    return EntityHandlerResult.ERROR;
                                }
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.OBSERVER)) {
                            }
                            return EntityHandlerResult.OK;
                        }
                    },
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal1.await(60, TimeUnit.SECONDS));

            //wait for onGetCompleted event
            assertTrue(signal2.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testHandlePostRequest() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal1 = new CountDownLatch(1);
        final CountDownLatch signal2 = new CountDownLatch(3);
        final List<OcResource> ocResourceList = new LinkedList<OcResource>();

        final OcResource.OnPostListener onPostListener = new OcResource.OnPostListener() {
            @Override
            public void onPostCompleted(List<OcHeaderOption> headerOptionList, OcRepresentation ocRepresentation) {
                if (!headerOptionList.isEmpty()) {
                    for (OcHeaderOption headerOption : headerOptionList) {
                        Log.i(TAG, "Header option " +
                                headerOption.getOptionId() +
                                " : " +
                                headerOption.getOptionData());
                    }
                }
                try {
                    Log.i(TAG, "onPostCompleted Power: " + ocRepresentation.getValue("power"));
                } catch (OcException e) {
                    Log.e(TAG, e.toString());
                    assertTrue(false);
                }
                Log.i(TAG, "onPostCompleted Uri: " + ocRepresentation.getUri());
                signal2.countDown();
            }

            @Override
            public void onPostFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                    if (ErrorCode.NO_RESOURCE != errCode) {
                        Log.e(TAG, ocEx.getMessage());
                        assertTrue(false);
                    }
                } else {
                    Log.e(TAG, ex.getMessage());
                    assertTrue(false);
                }
            }
        };

        final OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        ocResourceList.add(resource);
                        try {
                            resource.post(
                                    getRepresentation(),
                                    new HashMap<String, String>(),
                                    onPostListener);

                            resource.post(
                                    getRepresentation(),
                                    new HashMap<String, String>(),
                                    onPostListener);

                            resource.post(
                                    getRepresentation(),
                                    new HashMap<String, String>(),
                                    new OcResource.OnPostListener() {
                                        @Override
                                        public void onPostCompleted(List<OcHeaderOption> headerOptionList, OcRepresentation ocRepresentation) {
                                            signal2.countDown();
                                        }

                                        @Override
                                        public void onPostFailed(Throwable ex) {
                                            if (ex instanceof OcException) {
                                                OcException ocEx = (OcException) ex;
                                                ErrorCode errCode = ocEx.getErrorCode();
                                                if (ErrorCode.NO_RESOURCE != errCode) {
                                                    Log.e(TAG, ocEx.getMessage());
                                                    assertTrue(false);
                                                }
                                            } else {
                                                Log.e(TAG, ex.getMessage());
                                                assertTrue(false);
                                            }
                                        }
                                    });

                        } catch (OcException e) {
                            assertTrue(false);
                        }
                        signal1.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    new OcPlatform.EntityHandler() {
                        @Override
                        public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                            EnumSet<RequestHandlerFlag> handlerFlagSet = ocResourceRequest.getRequestHandlerFlagSet();
                            RequestType requestType = ocResourceRequest.getRequestType();

                            if (handlerFlagSet.contains(RequestHandlerFlag.INIT)) {
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.REQUEST)) {
                                OcResourceResponse ocResourceResponse = new OcResourceResponse();
                                ocResourceResponse.setRequestHandle(
                                        ocResourceRequest.getRequestHandle());
                                ocResourceResponse.setResourceHandle(
                                        ocResourceRequest.getResourceHandle());

                                switch (requestType) {
                                    case GET:
                                        assertTrue(false);
                                        break;
                                    case PUT:
                                        assertTrue(false);
                                        break;
                                    case POST:
                                        OcRepresentation rep = ocResourceRequest.getResourceRepresentation();
                                        try {
                                            Log.i(TAG, "Post res. power: " + rep.getValue("power"));
                                        } catch (OcException e) {
                                            Log.e(TAG, e.toString());
                                            assertTrue(false);
                                        }
                                        Log.i(TAG, "URI: " + rep.getUri());

                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(EntityHandlerResult.OK);
                                        ocResourceResponse.setResourceRepresentation(
                                                getRepresentation(44));

                                        break;
                                    case DELETE:
                                        assertTrue(false);
                                        break;
                                }

                                try {
                                    OcPlatform.sendResponse(ocResourceResponse);
                                } catch (OcException e) {
                                    Log.e(TAG, e.getMessage());
                                    return EntityHandlerResult.ERROR;
                                }
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.OBSERVER)) {
                            }
                            return EntityHandlerResult.OK;
                        }
                    },
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal1.await(60, TimeUnit.SECONDS));

            //wait for onPostCompleted event
            assertTrue(signal2.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }

    }

    public void testHandleDeleteRequest() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal1 = new CountDownLatch(1);
        final CountDownLatch signal2 = new CountDownLatch(1);
        final List<OcResource> ocResourceList = new LinkedList<OcResource>();

        final OcResource.OnDeleteListener onDeleteListener = new OcResource.OnDeleteListener() {
            @Override
            public void onDeleteCompleted(List<OcHeaderOption> headerOptionList) {
                signal2.countDown();
            }

            @Override
            public void onDeleteFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                    if (ErrorCode.NO_RESOURCE != errCode) {
                        Log.e(TAG, ocEx.getMessage());
                        assertTrue(false);
                    }
                } else {
                    Log.e(TAG, ex.getMessage());
                    assertTrue(false);
                }
            }
        };

        final OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        ocResourceList.add(resource);
                        try {
                            resource.deleteResource(onDeleteListener);
                        } catch (OcException e) {
                            assertTrue(false);
                        }
                        signal1.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    new OcPlatform.EntityHandler() {
                        @Override
                        public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                            EnumSet<RequestHandlerFlag> handlerFlagSet =
                                    ocResourceRequest.getRequestHandlerFlagSet();
                            RequestType requestType = ocResourceRequest.getRequestType();

                            if (handlerFlagSet.contains(RequestHandlerFlag.INIT)) {
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.REQUEST)) {
                                OcResourceResponse ocResourceResponse = new OcResourceResponse();
                                ocResourceResponse.setRequestHandle(
                                        ocResourceRequest.getRequestHandle());
                                ocResourceResponse.setResourceHandle(
                                        ocResourceRequest.getResourceHandle());

                                switch (requestType) {
                                    case DELETE:
                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(
                                                EntityHandlerResult.RESOURCE_DELETED);
                                        break;
                                }

                                try {
                                    OcPlatform.sendResponse(ocResourceResponse);
                                } catch (OcException e) {
                                    Log.e(TAG, e.getMessage());
                                    return EntityHandlerResult.ERROR;
                                }
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.OBSERVER)) {
                            }
                            return EntityHandlerResult.OK;
                        }
                    },
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal1.await(60, TimeUnit.SECONDS));

            //wait for onDeleteCompleted event
            assertTrue(signal2.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testHandleGetPutPostDeleteFailures() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal1 = new CountDownLatch(1);
        final CountDownLatch signal2 = new CountDownLatch(1);
        final CountDownLatch signal3 = new CountDownLatch(1);
        final CountDownLatch signal4 = new CountDownLatch(1);
        final CountDownLatch signal5 = new CountDownLatch(1);

        final List<OcResource> ocResourceList = new LinkedList<OcResource>();
        final OcResource.OnGetListener onGetListener = new OcResource.OnGetListener() {
            @Override
            public void onGetCompleted(List<OcHeaderOption> headerOptionList,
                                       OcRepresentation ocRepresentation) {
                assertTrue(false);
            }

            @Override
            public void onGetFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                }
                Log.i(TAG, ex.toString());
                signal2.countDown();
            }
        };

        final OcResource.OnPutListener onPutListener = new OcResource.OnPutListener() {
            @Override
            public void onPutCompleted(List<OcHeaderOption> headerOptionList,
                                       OcRepresentation ocRepresentation) {
                assertTrue(false);
            }

            @Override
            public void onPutFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                }
                Log.i(TAG, ex.toString());
                signal3.countDown();
            }
        };
        final OcResource.OnPostListener onPostListener = new OcResource.OnPostListener() {
            @Override
            public void onPostCompleted(List<OcHeaderOption> headerOptionList,
                                        OcRepresentation ocRepresentation) {
                assertTrue(false);
            }

            @Override
            public void onPostFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                }
                Log.i(TAG, ex.toString());
                signal4.countDown();
            }
        };

        final OcResource.OnDeleteListener onDeleteListener = new OcResource.OnDeleteListener() {
            @Override
            public void onDeleteCompleted(List<OcHeaderOption> headerOptionList) {
                assertTrue(false);
            }

            @Override
            public void onDeleteFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                }
                Log.i(TAG, ex.toString());
                signal5.countDown();
            }
        };

        final OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        ocResourceList.add(resource);
                        try {
                            resource.get(new HashMap<String, String>(), onGetListener);
                            resource.put(new OcRepresentation(), new HashMap<String, String>(),
                                    onPutListener);
                            resource.post(new OcRepresentation(), new HashMap<String, String>(),
                                    onPostListener);
                            resource.deleteResource(onDeleteListener);
                        } catch (OcException e) {
                            assertTrue(false);
                        }
                        signal1.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    new OcPlatform.EntityHandler() {
                        @Override
                        public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                            EnumSet<RequestHandlerFlag> handlerFlagSet =
                                    ocResourceRequest.getRequestHandlerFlagSet();
                            RequestType requestType = ocResourceRequest.getRequestType();

                            if (handlerFlagSet.contains(RequestHandlerFlag.INIT)) {
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.REQUEST)) {
                                OcResourceResponse ocResourceResponse = new OcResourceResponse();
                                ocResourceResponse.setRequestHandle(
                                        ocResourceRequest.getRequestHandle());
                                ocResourceResponse.setResourceHandle(
                                        ocResourceRequest.getResourceHandle());

                                switch (requestType) {
                                    case GET:
                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(
                                                EntityHandlerResult.ERROR);
                                        break;
                                    case PUT:
                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(
                                                EntityHandlerResult.ERROR);
                                        break;
                                    case POST:
                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(
                                                EntityHandlerResult.ERROR);
                                        break;
                                    case DELETE:
                                        ocResourceResponse.setErrorCode(200);
                                        ocResourceResponse.setResponseResult(
                                                EntityHandlerResult.ERROR);
                                        break;
                                }
                                try {
                                    OcPlatform.sendResponse(ocResourceResponse);
                                } catch (OcException e) {
                                    Log.e(TAG, e.getMessage());
                                    return EntityHandlerResult.ERROR;
                                }
                            }
                            if (handlerFlagSet.contains(RequestHandlerFlag.OBSERVER)) {
                            }
                            return EntityHandlerResult.OK;
                        }
                    },
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal1.await(60, TimeUnit.SECONDS));
            //wait for onGetCompleted event
            assertTrue(signal2.await(60, TimeUnit.SECONDS));
            //wait for onPutCompleted event
            assertTrue(signal3.await(60, TimeUnit.SECONDS));
            //wait for onPostCompleted event
            assertTrue(signal4.await(60, TimeUnit.SECONDS));
            //wait for onDeleteCompleted event
            assertTrue(signal5.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testPlatformInfo() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal = new CountDownLatch(1);

        OcPlatform.OnPlatformFoundListener platformFoundListener = new OcPlatform.OnPlatformFoundListener() {
            @Override
            public void onPlatformFound(OcRepresentation ocRepresentation) {
                Log.i(TAG, "Platform Info Received: ");
                Log.i(TAG, "URI: " + ocRepresentation.getUri());
                signal.countDown();
            }
        };

        OcPlatformInfo platformInfo = null;

        platformInfo = new OcPlatformInfo("myPlatformID", "myManuName", "myManuUrl");

        platformInfo.setModelNumber("myModelNumber");
        platformInfo.setDateOfManufacture("myDateOfManufacture");
        platformInfo.setPlatformVersion("myPlatformVersion");
        platformInfo.setOperatingSystemVersion("myOperatingSystemVersion");
        platformInfo.setHardwareVersion("myHardwareVersion");
        platformInfo.setFirmwareVersion("myFirmwareVersion");
        platformInfo.setSupportUrl("mySupportUrl");
        platformInfo.setSystemTime("mySystemTime");

        try {
            //server
            OcPlatform.registerPlatformInfo(platformInfo);

            //client
            OcPlatform.getPlatformInfo(
                    "",
                    OcPlatform.WELL_KNOWN_PLATFORM_QUERY,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    platformFoundListener);

            //wait for onPlatformFound event
            assertTrue(signal.await(60, TimeUnit.SECONDS));
        } catch (OcException e) {
            Log.e(TAG, e.getMessage() + platformInfo.toString());
            assertTrue(false);
        }
    }

    public void testRegisterDeviceInfoGetDeviceInfo() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal = new CountDownLatch(1);

        OcPlatform.OnDeviceFoundListener deviceFoundListener = new OcPlatform.OnDeviceFoundListener() {
            @Override
            public void onDeviceFound(OcRepresentation ocRepresentation) {
                try {
                    Log.i(TAG, "Device Name: " + ocRepresentation.getValue("n"));
                } catch (OcException e) {
                    Log.e(TAG, e.toString());
                    assertTrue(false);
                }
                boolean hasDeviceNameAtr = ocRepresentation.hasAttribute("n");
                assertTrue(hasDeviceNameAtr);
                boolean hasNonExistingAtr = ocRepresentation.hasAttribute("NonExisting");
                assertFalse(hasNonExistingAtr);
                Log.i(TAG, "URI: " + ocRepresentation.getUri());
                signal.countDown();
            }
        };

        OcDeviceInfo devInfo = new OcDeviceInfo("myDeviceName");

        try {
            //server
            OcPlatform.registerDeviceInfo(devInfo);
            //client
            OcPlatform.getDeviceInfo(
                    "",
                    OcPlatform.WELL_KNOWN_DEVICE_QUERY,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    deviceFoundListener);

            //wait for onDeviceFound event
            assertTrue(signal.await(60, TimeUnit.SECONDS));

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testBindUnbindResources() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal1 = new CountDownLatch(3);
        final CountDownLatch signal2 = new CountDownLatch(2);
        final CountDownLatch signal3 = new CountDownLatch(1);
        final CountDownLatch signal4 = new CountDownLatch(3);
        final CountDownLatch signal5 = new CountDownLatch(3);
        final CountDownLatch signal6 = new CountDownLatch(1);

        OcPlatform.EntityHandler entityHandler = new OcPlatform.EntityHandler() {
            @Override
            public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                return EntityHandlerResult.OK;
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener1 = new OcPlatform.OnResourceFoundListener() {
            @Override
            public void onResourceFound(OcResource resource) {
                signal1.countDown();
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener2 = new OcPlatform.OnResourceFoundListener() {
            @Override
            public void onResourceFound(OcResource resource) {
                signal2.countDown();
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener3 = new OcPlatform.OnResourceFoundListener() {
            @Override
            public void onResourceFound(OcResource resource) {
                signal3.countDown();
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener4 = new OcPlatform.OnResourceFoundListener() {
            @Override
            public void onResourceFound(OcResource resource) {
                signal4.countDown();
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener5 = new OcPlatform.OnResourceFoundListener() {
            @Override
            public void onResourceFound(OcResource resource) {
                signal5.countDown();
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener6 = new OcPlatform.OnResourceFoundListener() {
            @Override
            public void onResourceFound(OcResource resource) {
                signal6.countDown();
            }
        };

        try {

            //server
            OcResourceHandle resourceHandleCollection = OcPlatform.registerResource(
                    "/a/unittest1",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    entityHandler,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            OcResourceHandle resourceHandle1 = OcPlatform.registerResource(
                    "/a/unittest2",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    entityHandler,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            OcResourceHandle resourceHandle2 = OcPlatform.registerResource(
                    "/a/unittest3",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    entityHandler,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener1);

            //wait for onResourceFound event to find 3 registered resources
            assertTrue(signal1.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.bindResource(resourceHandleCollection, resourceHandle1);
            OcPlatform.bindResource(resourceHandleCollection, resourceHandle2);

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener3);

            //wait for onResourceFound event to find 1 collection resources
            assertTrue(signal3.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unbindResource(resourceHandleCollection, resourceHandle1);

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener2);

            //wait for onResourceFound event to find 2 resources
            assertTrue(signal2.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unbindResource(resourceHandleCollection, resourceHandle2);

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener4);

            //wait for onResourceFound event to find 3 registered resources
            assertTrue(signal4.await(60, TimeUnit.SECONDS));

            //Bind/unbind a list of resource handles
            List<OcResourceHandle> resourceHandleList = new LinkedList<OcResourceHandle>();
            resourceHandleList.add(resourceHandle1);
            resourceHandleList.add(resourceHandle2);
            OcPlatform.bindResources(resourceHandleCollection, resourceHandleList);

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener6);

            //wait for onResourceFound event to find 1 collection resources
            assertTrue(signal6.await(60, TimeUnit.SECONDS));

            OcPlatform.unbindResources(resourceHandleCollection, resourceHandleList);

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener5);

            //wait for onResourceFound event to find 1 collection resources
            assertTrue(signal5.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandleCollection);
            OcPlatform.unregisterResource(resourceHandle1);
            OcPlatform.unregisterResource(resourceHandle2);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testResourceMethods() throws InterruptedException {
        final String resourceType1 = "unit.test.resource" + new Date().getTime();
        final String resourceType2 = "unit.test.resource" + new Date().getTime();

        final CountDownLatch signal = new CountDownLatch(2);
        final List<OcResource> resourceList = new LinkedList<>();

        OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public synchronized void onResourceFound(OcResource resource) {
                        resourceList.add(resource);
                        Log.i(TAG, "Host: " + resource.getHost());
                        Log.i(TAG, "Uri: " + resource.getUri());
                        Log.i(TAG, "Observable: " + resource.isObservable());

                        assertFalse(resource.getResourceTypes().isEmpty());
                        for (String resourceType : resource.getResourceTypes()) {
                            Log.i(TAG, "Type: " + resourceType);
                        }

                        assertFalse(resource.getResourceTypes().isEmpty());
                        for (String resourceInterface : resource.getResourceInterfaces()) {
                            Log.i(TAG, "Interface: " + resourceInterface);
                        }

                        List<OcHeaderOption> headerOptionList = new LinkedList<OcHeaderOption>();
                        headerOptionList.add(new OcHeaderOption(2885, "OptionData1"));
                        headerOptionList.add(new OcHeaderOption(2886, "OptionData2"));

                        resource.setHeaderOptions(headerOptionList);
                        resource.unsetHeaderOptions();

                        OcResourceIdentifier resourceIdentifier = resource.getUniqueIdentifier();
                        OcResourceIdentifier resourceIdentifier2 = resource.getUniqueIdentifier();
                        assertTrue(resourceIdentifier.equals(resourceIdentifier2));

                        signal.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle1 = OcPlatform.registerResource(
                    "/a/unittest1",
                    resourceType1,
                    OcPlatform.DEFAULT_INTERFACE,
                    null,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            OcResourceHandle resourceHandle2 = OcPlatform.registerResource(
                    "/a/unittest2",
                    resourceType2,
                    OcPlatform.DEFAULT_INTERFACE,
                    null,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType1,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType2,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal.await(60, TimeUnit.SECONDS));

            assertTrue(2 == resourceList.size());

            OcResource res0 = resourceList.get(0);
            OcResource res1 = resourceList.get(1);
            assertFalse(res0.getUniqueIdentifier().equals(res1.getUniqueIdentifier()));
            assertTrue(res0.getUniqueIdentifier().equals(res0.getUniqueIdentifier()));

            //server
            OcPlatform.unregisterResource(resourceHandle1);
            OcPlatform.unregisterResource(resourceHandle2);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testCreateResourceProxy() throws InterruptedException {
        final String resourceType = "unit.test.resource" + new Date().getTime();
        final CountDownLatch signal = new CountDownLatch(1);

        OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {

                        try {
                            //client: construct resource proxy
                            OcResource resourceProxy = OcPlatform.constructResourceObject(
                                    resource.getHost(),
                                    resource.getUri(),
                                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                                    resource.isObservable(),
                                    resource.getResourceTypes(),
                                    resource.getResourceInterfaces());

                            //client: register resource proxy
                            OcResourceHandle resourceProxyHandle =
                                    OcPlatform.registerResource(resourceProxy);

                            OcPlatform.unregisterResource(resourceProxyHandle);
                        } catch (OcException e) {
                            assertTrue(false);
                        }
                        signal.countDown();
                    }
                };

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    null,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }

        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    null,
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    public void testObserveClientServer() throws InterruptedException {
        final int NUM_OBSERVES = 20;
        final Timer timer = new Timer();
        final List<OcResource> resourceList = new LinkedList<OcResource>();
        final List<OcResourceHandle> resourceHandleList = new LinkedList<OcResourceHandle>();
        final CountDownLatch signal1 = new CountDownLatch(1);
        final CountDownLatch signal2 = new CountDownLatch(NUM_OBSERVES);
        final CountDownLatch signal3 = new CountDownLatch(1);

        final String resourceType = "unit.test.resource" + new Date().getTime();

        final OcResource.OnObserveListener observeListener = new OcResource.OnObserveListener() {
            @Override
            public void onObserveCompleted(
                    List<OcHeaderOption> headerOptionList,
                    OcRepresentation ocRepresentation,
                    int sequenceNumber) {

                try {
                    Log.i(TAG, "Observe #" + sequenceNumber + " power: " +
                            ocRepresentation.getValue("power"));
                } catch (OcException e) {
                    Log.e(TAG, e.toString());
                    assertTrue(false);
                }
                signal2.countDown();
            }

            @Override
            public void onObserveFailed(Throwable ex) {
                if (ex instanceof OcException) {
                    OcException ocEx = (OcException) ex;
                    ErrorCode errCode = ocEx.getErrorCode();
                }
                Log.e(TAG, ex.toString());
                assertTrue(false);
            }
        };

        final List<Byte> observationIdList = new LinkedList<Byte>();
        OcPlatform.EntityHandler entityHandler = new OcPlatform.EntityHandler() {
            @Override
            public EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
                EnumSet<RequestHandlerFlag> handlerFlagSet = ocResourceRequest.getRequestHandlerFlagSet();
                RequestType requestType = ocResourceRequest.getRequestType();

                if (handlerFlagSet.contains(RequestHandlerFlag.INIT)) {

                }
                if (handlerFlagSet.contains(RequestHandlerFlag.REQUEST)) {
                    OcResourceResponse ocResourceResponse = new OcResourceResponse();
                    ocResourceResponse.setRequestHandle(ocResourceRequest.getRequestHandle());
                    ocResourceResponse.setResourceHandle(ocResourceRequest.getResourceHandle());

                    switch (requestType) {
                        case GET:
                            ocResourceResponse.setErrorCode(200);
                            ocResourceResponse.setResponseResult(EntityHandlerResult.OK);
                            ocResourceResponse.setResourceRepresentation(
                                    getRepresentation(
                                            rnd.nextInt(100)));
                            try {
                                OcPlatform.sendResponse(ocResourceResponse);
                            } catch (OcException e) {
                                Log.e(TAG, e.getMessage());
                                return EntityHandlerResult.ERROR;
                            }
                            break;
                    }
                }

                if (handlerFlagSet.contains(RequestHandlerFlag.OBSERVER)) {
                    ObservationInfo observationInfo = ocResourceRequest.getObservationInfo();

                    switch (observationInfo.getObserveAction()) {
                        case REGISTER:
                            synchronized (observationIdList) {
                                observationIdList.add(observationInfo.getOcObservationId());
                                timer.schedule(new TimerTask() {
                                    int numNotified = 1;

                                    @Override
                                    public void run() {
                                        if (0 < resourceHandleList.size()) {
                                            synchronized (observationIdList) {
                                                if (numNotified > NUM_OBSERVES) {
                                                    timer.cancel();
                                                    timer.purge();
                                                    signal3.countDown();
                                                } else {
                                                    try {
                                                        OcPlatform.notifyAllObservers(
                                                                resourceHandleList.get(0));
                                                    } catch (OcException e) {
                                                        if (ErrorCode.NO_OBSERVERS == e.getErrorCode()) {
                                                            timer.cancel();
                                                            timer.purge();
                                                            signal3.countDown();
                                                        }
                                                        Log.e(TAG, e.getMessage());
                                                    }
                                                    numNotified++;
                                                }
                                            }
                                        }

                                    }
                                }, 0, 100);
                            }
                            break;
                        case UNREGISTER:
                            //TODO unregister isn't implemented in C++ API, yet
                            synchronized (observationIdList) {
                                timer.cancel();
                                break;
                            }
                    }
                }
                return EntityHandlerResult.OK;
            }
        };

        OcPlatform.OnResourceFoundListener resourceFoundListener =
                new OcPlatform.OnResourceFoundListener() {
                    @Override
                    public void onResourceFound(OcResource resource) {
                        resourceList.add(resource);
                        if (resource.isObservable()) {
                            try {
                                resource.observe(
                                        ObserveType.OBSERVE,
                                        new HashMap<String, String>(),
                                        observeListener);

                                signal1.countDown();
                            } catch (OcException e) {
                                Log.e(TAG, e.getMessage());
                                assertTrue(false);
                            }
                        }
                    }
                };
        try {
            //server
            OcResourceHandle resourceHandle = OcPlatform.registerResource(
                    "/a/unittest",
                    resourceType,
                    OcPlatform.DEFAULT_INTERFACE,
                    entityHandler,
                    EnumSet.of(ResourceProperty.DISCOVERABLE, ResourceProperty.OBSERVABLE)
            );

            resourceHandleList.add(resourceHandle);

            //client
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + resourceType,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    resourceFoundListener);

            //wait for onResourceFound event
            assertTrue(signal1.await(60, TimeUnit.SECONDS));

            //wait for OnObserveListener event to observe 20 values
            assertTrue(signal2.await(60, TimeUnit.SECONDS));

            if (resourceList.size() > 0) {
                OcResource resource = resourceList.get(0);
                if (resource.isObservable()) {
                    resource.cancelObserve();
                }
            }

            //wait for server to finish
            assertTrue(signal3.await(60, TimeUnit.SECONDS));

            //server
            OcPlatform.unregisterResource(resourceHandle);

        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
            assertTrue(false);
        }
    }

    private OcRepresentation getRepresentation(int value) {
        OcRepresentation rep = new OcRepresentation();
        try {
            rep.setValue("power", value);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            assertTrue(false);
        }
        return rep;
    }

    private OcRepresentation getRepresentation() {
        return getRepresentation(74);
    }
}
