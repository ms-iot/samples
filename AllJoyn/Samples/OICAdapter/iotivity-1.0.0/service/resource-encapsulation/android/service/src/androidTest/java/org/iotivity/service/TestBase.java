package org.iotivity.service;

import org.iotivity.service.client.RcsAddress;
import org.iotivity.service.client.RcsDiscoveryManager;
import org.iotivity.service.client.RcsDiscoveryManager.OnResourceDiscoveredListener;
import org.iotivity.service.client.RcsRemoteResourceObject;
import org.iotivity.service.server.RcsResourceObject;

import android.test.InstrumentationTestCase;

public abstract class TestBase extends InstrumentationTestCase {
    protected static final String RESOURCEURI       = "/a/TemperatureSensor";
    protected static final String RESOURCETYPE      = "oic.r.type";
    protected static final String RESOURCEINTERFACE = "oic.if.baseline";

    protected static final String   KEY       = "key";
    protected static final RcsValue VALUE     = new RcsValue(100);
    protected static final int      RAW_VALUE = 100;

    private final Object mCond = new Object();

    protected RcsResourceObject       mServer;
    protected RcsRemoteResourceObject mClient;

    private OnResourceDiscoveredListener mOnResourceDiscoveredListener = new OnResourceDiscoveredListener() {
        @Override
        public void onResourceDiscovered(
                RcsRemoteResourceObject RcsRemoteResourceObject) {
            if (mClient != null)
                return;

            mClient = RcsRemoteResourceObject;
            synchronized (mCond) {
                mCond.notify();
            }
        }
    };

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mServer = new RcsResourceObject.Builder(RESOURCEURI, RESOURCETYPE,
                RESOURCEINTERFACE).build();
        mServer.setAttribute(KEY, VALUE);

        WaitUntilDiscovered();

        assertNotNull(mClient);
    }

    private void WaitUntilDiscovered() throws RcsException {
        while (mClient == null) {
            try {
                RcsDiscoveryManager.DiscoveryTask discoveryTask = RcsDiscoveryManager
                        .getInstance().discoverResourceByType(
                                RcsAddress.multicast(), "/oic/res",
                                RESOURCETYPE, mOnResourceDiscoveredListener);

                synchronized (mCond) {
                    mCond.wait(100);
                }

                discoveryTask.cancel();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    protected void setServerAttrbutes(RcsResourceAttributes attrs)
            throws RcsException {
        RcsResourceObject.AttributesLock lock = mServer.getAttributesLock();
        lock.lock().putAll(attrs);
        lock.apply();
        lock.unlock();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();

        mServer.destroy();
        mClient.destroy();
    }
}
