//
// Copyright 2014 Intel Corporation.
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

package org.iotivity.guiclient;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcConnectivityType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcResource;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;

import static org.iotivity.guiclient.OcProtocolStrings.COAP_CORE;
import static org.iotivity.guiclient.OcProtocolStrings.CORE_EDISON_RESOURCES;
import static org.iotivity.guiclient.OcProtocolStrings.CORE_LIGHT;
import static org.iotivity.guiclient.OcProtocolStrings.INTERFACE_QUERY;
import static org.iotivity.guiclient.OcProtocolStrings.RESOURCE_TYPE_QUERY;
import static org.iotivity.guiclient.OcWorker.OCW_IN_MSG.DO_CLEAR_RESOURCES;
import static org.iotivity.guiclient.OcWorker.OCW_IN_MSG.DO_DISCOVER_RESOURCES;
import static org.iotivity.guiclient.OcWorker.OCW_IN_MSG.DO_GET_RESOURCE;
import static org.iotivity.guiclient.OcWorker.OCW_IN_MSG.DO_PUT_RESOURCE;
import static org.iotivity.guiclient.OcWorker.OCW_IN_MSG.fromInt;

/**
 * OcWorker
 *
 * A class designed to encapsulate the OIC API functionality.  OcWorker has its own
 * thread which is used to call all OIC APIs.  To get results back from OcWorker,
 * implement the interface OcWorkerListener, and call registerListener().
 *
 * @see org.iotivity.guiclient.OcWorkerListener
 *
 * Created by nathanhs on 12/22/15.
 */
public class OcWorker extends Thread
        implements OcPlatform.OnResourceFoundListener, OcResourceInfo.OcResourceInfoListener  {
    /**
     * Hardcoded TAG... if project never uses proguard then
     * MyOcClient.class.getName() is the better way.
     */
    private static final String TAG = "OcWorker";
    private Context mContext;

    private static final boolean LOCAL_LOGV = true; // set to false to compile out verbose logging

    /**
     * NOTE: DO NOT assign non-default values to these enums!
     * The correctness of "fromInt()" depends on the enum values not being
     * overridden.  For example DO_TEST = 100, RESULT_TEST = 101.. would BREAK the
     * fromInt() function.  There are designs which can account for arbitrary enum
     * values, but they are less desirable for other reasons and since we do not
     * need to use any specific value, this design is the best for this case.
     */

    /**
     * These "IN" message types are for posting API-generated request actions
     * to our OcWorker queue.
     */
    public enum OCW_IN_MSG {
        DO_TEST,  // developer testing only
        DO_DISCOVER_RESOURCES,
        DO_CLEAR_RESOURCES,
        DO_GET_RESOURCE,
        DO_PUT_RESOURCE,
        DO_OBSERVE_RESOURCE,
        DO_STOP_OBSERVING_RESOURCE;

        private static final OCW_IN_MSG[] values = values();

        public static OCW_IN_MSG fromInt(int i) {
            return values[i];
        }
    }

    /**
     * These events are for internally putting work on our thread's looper
     * queue, usually in a callback where we don't want to do work on the
     * callback thread.
     */
    private enum OC_EVENT {
        OIC_RESOURCE_FOUND,
        OIC_RESOURCE_CHANGED;

        private static final OC_EVENT[] values = OC_EVENT.values();

        public static OC_EVENT fromInt(int i) {
            return values[i];
        }
    }

    private Handler mDoMsgHandler;

    private Handler mOcEventHandler;

    /**
     * The OcResourceInfo List
     */
    private ArrayList<OcResourceInfo> mOcResourceInfoList;

    /**
     * The types of OIC Resources included in "FindResource" calls by this object.
     */
    private final String[] mOcFindQueries = {
            COAP_CORE + RESOURCE_TYPE_QUERY + CORE_LIGHT,
            COAP_CORE + INTERFACE_QUERY + CORE_EDISON_RESOURCES
    };

    private List<OcWorkerListener> mListeners;

    public OcWorker(Context context) {
        if (LOCAL_LOGV) Log.v(TAG, "OcWorker() constructor");

        mContext = context;
        this.mListeners = new ArrayList<>();
    }

    /**
     * Set up our Handler and Looper, then initialize the OIC platform and
     * start processing messages as they arrive.
     */
    public void run() {
        if (LOCAL_LOGV) Log.v(TAG, "run()");

        Looper.prepare();
        this.initHandlers(); // set up our message handler
        this.ocInit(); // init the OIC layer including calling ConfigurePlatform
        Looper.loop();
    }

    /**
     * Registers a listener for OcWorker events.
     *
     * @see org.iotivity.guiclient.OcWorkerListener
     */
    public void registerListener(OcWorkerListener listener) {
        if (LOCAL_LOGV) Log.v(TAG, "registerListener()");

        if(null != this.mListeners) {
            this.mListeners.add(listener);
        } else {
            Log.e(TAG, "registerListener(): null mListeners list; not adding listener!");
            Log.e(TAG, "OcWorker.run() must be called before using public methods.");
        }
    }

    /**
     * The Resource discovery external API
     */
    public void doDiscoverResources() {
        if (LOCAL_LOGV) Log.v(TAG, "doDiscoverResources()");

        if(null != this.mDoMsgHandler) {
            this.mDoMsgHandler.obtainMessage(
                    DO_DISCOVER_RESOURCES.ordinal()).sendToTarget();
        } else {
            Log.e(TAG, "doDiscoverResources(): null mDoMsgHandler; not discovering resources!");
            Log.e(TAG, "OcWorker.run() must be called before using public methods.");
        }
    }

    /**
     * The GetResource external API
     */
    public void doGetResource(OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "doGetResource()");

        if(null != this.mDoMsgHandler) {
            this.mDoMsgHandler.obtainMessage(
                    DO_GET_RESOURCE.ordinal(), resourceInfo).sendToTarget();
        } else {
            Log.e(TAG, "doPutResource(): null mDoMsgHandler; not putting resource!");
            Log.e(TAG, "OcWorker.run() must be called before using public methods.");
        }
    }

    /**
     * The PutResource external API
     */
    public void doPutResource(OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "doPutResource()");

        if(null != this.mDoMsgHandler) {
            this.mDoMsgHandler.obtainMessage(
                    DO_PUT_RESOURCE.ordinal(), resourceInfo).sendToTarget();
        } else {
            Log.e(TAG, "doPutResource(): null mDoMsgHandler; not putting resource!");
            Log.e(TAG, "OcWorker.run() must be called before using public methods.");
        }
    }

    /**
     * The Clear Resources external API
     */
    public void doClearResources() {
        if (LOCAL_LOGV) Log.v(TAG, "doClearResources()");

        if(null != this.mDoMsgHandler) {
            this.mDoMsgHandler.obtainMessage(
                    DO_CLEAR_RESOURCES.ordinal()).sendToTarget();
        } else {
            Log.e(TAG, "doClearResources(): null mDoMsgHandler; not clearing resources!");
            Log.e(TAG, "OcWorker.run() must be called before using public methods.");
        }
    }

    /**
     * Set up handlers
     */
    private void initHandlers() {
        if (LOCAL_LOGV) Log.v(TAG, "initHandler()");

        this.mDoMsgHandler = new Handler() {
            public void handleMessage(Message msg) {
                Log.d(TAG, String.format("mDoMsgHandler.handleMessage(%s)", msg.toString()));
                // process incoming messages here
                OCW_IN_MSG type = fromInt(msg.what);
                switch(type) {
                    case DO_TEST:
                        break;
                    case DO_DISCOVER_RESOURCES:
                        discoverResources();
                        break;
                    case DO_CLEAR_RESOURCES:
                        clearResourceInfoList();
                        break;
                    case DO_GET_RESOURCE:
                        getResourceAttributes((OcResourceInfo)msg.obj);
                        break;
                    case DO_PUT_RESOURCE:
                        putResourceAttributes((OcResourceInfo)msg.obj);
                        break;
                    case DO_OBSERVE_RESOURCE:
                        break;
                    case DO_STOP_OBSERVING_RESOURCE:
                        break;
                    default:
                        Log.e(TAG, "unknown msg.what in handler");
                        break;
                }
            }
        };

        this.mOcEventHandler = new Handler() {
            public void handleMessage(Message msg) {
                Log.d(TAG, String.format("mOcEventHandler.handleMessage(%s)", msg.toString()));
                // process incoming messages here
                OC_EVENT type = OC_EVENT.fromInt(msg.what);
                switch(type) {
                    case OIC_RESOURCE_FOUND:
                        handleNewResourceFound((OcResource)msg.obj);
                        break;
                    case OIC_RESOURCE_CHANGED:
                        handleResourceInfoChange((OcResourceInfo)msg.obj);
                        break;
                }
            }
        };
    }

    /**
     * Get the attributes on resourceInfo.
     *
     * @param resourceInfo
     */
    private void getResourceAttributes(OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "getResourceAttributes()");

        // find the matching resource in our resourceList
        OcResourceInfo existingResource = this.selectResourceInfoByHostAndUri(
                resourceInfo.getHost() + resourceInfo.getUri());
        if(null != existingResource) {
            existingResource.doOcGet();
        } else {
            Log.e(TAG, "getResourceAttributes(): could not find target resource.");
        }
        // Done.  Later, the onGet listener in the OcResourceInfo object will notify us of a change
        // via our onResourceChanged() method
    }

    /**
     * For each attribute in the resourceInfo.mAttributes, put the attribute value to the
     * Resource.
     *
     * @param resourceInfo
     */
    private void putResourceAttributes(OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "putResourceAttributes()");

        // find the matching resource in our resourceList
        OcResourceInfo existingResource = this.selectResourceInfoByHostAndUri(
                resourceInfo.getHost() + resourceInfo.getUri());
        if(null != existingResource) {
            // for each attribute in resourceInfo, put that attribute to the resource
            for(OcAttributeInfo attribute : resourceInfo.getAttributes()) {
                if(false == attribute.isReadOnly()) {
                    existingResource.doOcPut(attribute);
                }
            }
        } else {
            Log.e(TAG, "putResourceAttributes(): could not find target resource.");
        }
        // Done. later, the onPut listener in the OcResourceInfo object will notify us of a change
        // via our onResourceChanged() method
    }

    /**
     * Because this callback is called on a JNI layer thread, don't do work here.
     * Instead, create a "found resource" message and send to OcWorker's message queue,
     * Our looper/handler then calls handleNewResource on our own worker thread.
     *
     * Also note that this method must be thread safe because it can be called by
     * multiple concurrent native threads.
     *
     * @param resource
     */
    private Object onResourceFoundLock = new Object(); // not strictly necessary with this impl.,
                                                       // but clears up Log message readability.
    @Override
    public void onResourceFound(OcResource resource) {
        synchronized (onResourceFoundLock) {
            if (LOCAL_LOGV) Log.v(TAG, "onResourceFound()");
            if (LOCAL_LOGV) Log.v(TAG, "host: " + resource.getHost());
            if (LOCAL_LOGV) Log.v(TAG, "uri: " + resource.getUri());
            if (LOCAL_LOGV) Log.v(TAG, "is observable: " + resource.isObservable());

            this.mOcEventHandler.obtainMessage(OC_EVENT.OIC_RESOURCE_FOUND.ordinal(),
                    resource).sendToTarget();
        }
    }

    /**
     * Handles the internal NEW_RESOURCE_FOUND event, typically engueued on "onResourceFound".
     * Creates a new OcResourceInfo object to wrap the new OcResource and store other info.
     *
     * @param resource the OcResource object
     */
    private void handleNewResourceFound(OcResource resource) {
        if (LOCAL_LOGV) Log.v(TAG, String.format("handleNewResourceFound(%s)",
                resource.toString()));

        OcResourceInfo ri =
                this.selectResourceInfoByHostAndUri(resource.getHost() + resource.getUri());

        // before notifying listeners, update our own internal OcResourceInfo list
        if(null != mOcResourceInfoList) {
            // check for pre-existing duplicate before adding
            if(null == ri) {
                if (LOCAL_LOGV) Log.v(TAG, "handleNewResourceFound(): ri is new; adding.");
                // if not found, create new info object
                ri = new OcResourceInfo(resource, this);
                // register as a listener to the newly created OcResourceInfo
                ri.registerListener(this);
                // kick off a get to fill in attributes
                ri.doOcGet();
                // add the info object to our list
                mOcResourceInfoList.add(ri);
            }
        }
        // notify listeners
        for(OcWorkerListener l : this.mListeners) {
            l.onResourceFound(ri);
        }
    }

    /**
     * The "listener" callback from the OcResourceInfo class.
     * Called by the OcResourceInfo object using the native callback thread.
     * We use this callback to post an event to our queue so that the work
     * is serialized with other incoming events, and executed on our worker thread.
     *
     * Also note that this method must be thread safe because it could be called by
     * one of many OcResourceInfo objects on separate native threads.
     *
     * @param resourceInfo
     */
    private Object onResourceInfoChangedLock = new Object();
    @Override
    public void onResourceInfoChanged(OcResourceInfo resourceInfo) {

        synchronized (onResourceInfoChangedLock) {
            if (LOCAL_LOGV) Log.v(TAG, String.format("resourceInfoChanged(%s)",
                    resourceInfo.toString()));

            // this is a result of a callback (i.e. onGetCompleted, onPut, onObserve)
            // so we post a message to our queue to transfer the work to our own thread
            this.mOcEventHandler.obtainMessage(OC_EVENT.OIC_RESOURCE_CHANGED.ordinal(),
                    resourceInfo).sendToTarget();
        }
    }

    /**
     * Handle our internal event that is enqueued when a resource is found.
     *
     * @param resourceInfo
     */
    private void handleResourceInfoChange(OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "handleResourceInfoChange()");

        // notify listeners
        for(OcWorkerListener l : this.mListeners) {
            l.onResourceChanged(resourceInfo);
        }
    }

    /**
     * Complete OIC-related initialization, including configuring the platform
     */
    private void ocInit() {
        if (LOCAL_LOGV) Log.v(TAG, "ocInit()");

        // OIC initialization
        mOcResourceInfoList = new ArrayList<>();

        this.configurePlatform();
    }

    /**
     * Configures the OIC platform.
     */
    private void configurePlatform() {
        if (LOCAL_LOGV) Log.v(TAG, "configurePlatform()");

        PlatformConfig cfg = new PlatformConfig(
                mContext,
                ServiceType.IN_PROC,
                ModeType.CLIENT_SERVER,
                "0.0.0.0", // bind to all available interfaces
                0,
                QualityOfService.LOW);

        Log.d(TAG, "configurePlatform(): calling OcPlatform.Configure()");
        OcPlatform.Configure(cfg);
    }

    /**
     * Search mOcResourceInfo list for a resource whose Host and Uri concatenated
     * matches the param passed, and return it.
     *
     * @param resourceHostAndUri
     * @return OcResourceInfo with Host and Uri matching resourceHostAndUri, or null if
     * no such OcResourceInfo exists in mOcResourceInfoList
     */
    private OcResourceInfo selectResourceInfoByHostAndUri(String resourceHostAndUri) {
        if (LOCAL_LOGV) Log.v(TAG, "selectResourceByHostAndUri()");

        boolean found = false;
        OcResourceInfo retVal = null;

        for(OcResourceInfo ri : mOcResourceInfoList) {
            if(!found) {
                String s = ri.getHost() + ri.getUri();
                if (resourceHostAndUri.equalsIgnoreCase(s)) {
                    retVal = ri;
                    found = true;
                }
            }
        }
        if(!found) {
            Log.v(TAG, "selectResourceByHostAndUri(): no resource found matching HostAndUri "
                    + resourceHostAndUri);
        }

        return retVal;
    }

    /**
     * Finds OIC Resources matching known patterns.
     *
     * @see org.iotivity.guiclient.OcProtocolStrings
     */
    private void discoverResources() {
        if (LOCAL_LOGV) Log.v(TAG, "discoverResources()");

        try {
            for (String s : mOcFindQueries) {
                Log.d(TAG, String.format("discoverResources(): Calling OcPlatform.findResource(%s)", s));
                OcPlatform.findResource("",
                        OcPlatform.WELL_KNOWN_QUERY + "?rt=" + s,
                        EnumSet.of(OcConnectivityType.CT_DEFAULT),
                        this);
            }
        } catch (OcException e) {
            e.printStackTrace();
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * Clear the ResourceInfoList
     */
    private void clearResourceInfoList() {
        if (LOCAL_LOGV) Log.v(TAG, "clearResourceInfoList()");

        this.mOcResourceInfoList.clear();
    }
}
