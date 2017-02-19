package org.iotivity.service;

import static org.iotivity.service.client.RcsRemoteResourceObject.CacheState;
import static org.iotivity.service.client.RcsRemoteResourceObject.OnCacheUpdatedListener;
import static org.iotivity.service.client.RcsRemoteResourceObject.OnRemoteAttributesReceivedListener;
import static org.iotivity.service.client.RcsRemoteResourceObject.OnStateChangedListener;
import static org.iotivity.service.client.RcsRemoteResourceObject.ResourceState;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;

import org.mockito.Mockito;

public class RemoteResourceObjectTest extends TestBase {

    private RcsResourceAttributes createAttrs() {
        RcsResourceAttributes attrs = new RcsResourceAttributes();
        attrs.put("key", new RcsValue(3));
        attrs.put("b", new RcsValue(new RcsResourceAttributes()));
        attrs.put("myKey", new RcsValue("string-value"));

        return attrs;
    }

    public void testGetRemoteAttributesGetsAttributesOfServer()
            throws RcsException {
        OnRemoteAttributesReceivedListener listener = Mockito
                .mock(OnRemoteAttributesReceivedListener.class);

        setServerAttrbutes(createAttrs());

        mClient.getRemoteAttributes(listener);

        verify(listener, timeout(1000)).onAttributesReceived(eq(createAttrs()),
                anyInt());
    }

    public void testGetRemoteAttributesSetsAttributesOfServer()
            throws RcsException {
        OnRemoteAttributesReceivedListener listener = Mockito
                .mock(OnRemoteAttributesReceivedListener.class);

        mServer.setAttribute("key", new RcsValue(0));

        RcsResourceAttributes attrs = new RcsResourceAttributes();
        attrs.put("key", new RcsValue(3));

        mClient.setRemoteAttributes(attrs, listener);

        verify(listener, timeout(1000)).onAttributesReceived(
                any(RcsResourceAttributes.class), anyInt());

        assertEquals(3, mServer.getAttributeValue("key").asInt());
    }

    public void testMonitoringIsNotStartedByDefault() throws RcsException {
        assertFalse(mClient.isMonitoring());
    }

    public void testIsMonitoringReturnsTrueAfterStartMonitoring()
            throws RcsException {
        OnStateChangedListener listener = Mockito
                .mock(OnStateChangedListener.class);
        mClient.startMonitoring(listener);

        assertTrue(mClient.isMonitoring());
    }

    public void testStartMonitoringThrowsIfTryingToStartAgain()
            throws RcsException {
        OnStateChangedListener listener = Mockito
                .mock(OnStateChangedListener.class);
        mClient.startMonitoring(listener);

        try {
            mClient.startMonitoring(listener);
            fail("No exception thrown");
        } catch (RcsIllegalStateException e) {
        }
    }

    public void testDefaultResourceStateIsNone() throws RcsException {
        assertEquals(ResourceState.NONE, mClient.getState());
    }

    public void testCachingIsNotStartedByDefault() throws RcsException {
        assertFalse(mClient.isCaching());
    }

    public void testIsCachingReturnsTrueAfterStartCaching()
            throws RcsException {
        mClient.startCaching();

        assertTrue(mClient.isCaching());
    }

    public void testStartCachingThrowsIfTryingToStartAgain()
            throws RcsException {
        mClient.startCaching();

        try {
            mClient.startCaching();
            fail("No exception thrown");
        } catch (RcsIllegalStateException e) {
        }
    }

    public void testDefaultCacheStateIsNone() throws RcsException {
        assertEquals(CacheState.NONE, mClient.getCacheState());
    }

    public void testCacheStateIsUnreadyAfterStartCaching() throws RcsException {
        mClient.startCaching();

        assertEquals(CacheState.UNREADY, mClient.getCacheState());
    }

    public void testCacheStateIsReadyAfterCacheUpdated() throws RcsException {
        OnCacheUpdatedListener listener = Mockito
                .mock(OnCacheUpdatedListener.class);
        mClient.startCaching(listener);

        verify(listener, timeout(1000))
                .onCacheUpdated(any(RcsResourceAttributes.class));

        assertEquals(CacheState.READY, mClient.getCacheState());
    }

    public void testIsCachedAvailableReturnsTrueWhenCacheIsReady()
            throws RcsException {
        OnCacheUpdatedListener listener = Mockito
                .mock(OnCacheUpdatedListener.class);
        mClient.startCaching(listener);

        verify(listener, timeout(1000))
                .onCacheUpdated(any(RcsResourceAttributes.class));

        assertTrue(mClient.isCachedAvailable());
    }

    public void testGetCachedAttributesThrowsIfCachingIsNotStarted()
            throws RcsException {
        try {
            mClient.getCachedAttributes();
            fail("No exception thrown");
        } catch (RcsIllegalStateException e) {
        }
    }

    public void testCachedAttributesHasSameAttributesWithServer()
            throws RcsException {
        setServerAttrbutes(createAttrs());

        OnCacheUpdatedListener listener = Mockito
                .mock(OnCacheUpdatedListener.class);
        mClient.startCaching(listener);

        verify(listener, timeout(1000))
                .onCacheUpdated(any(RcsResourceAttributes.class));

        assertEquals(createAttrs(), mClient.getCachedAttributes());
    }

    public void testGetCachedAttributeThrowsIfCachingIsNotStarted()
            throws RcsException {
        try {
            mClient.getCachedAttribute("some_key");
            fail("No exception thrown");
        } catch (RcsIllegalStateException e) {
        }
    }

    public void testGetCachedAttributeReturnsNullIfKeyIsInvalid()
            throws RcsException {
        OnCacheUpdatedListener listener = Mockito
                .mock(OnCacheUpdatedListener.class);
        mClient.startCaching(listener);

        verify(listener, timeout(1000))
                .onCacheUpdated(any(RcsResourceAttributes.class));

        assertNull(mClient.getCachedAttribute("some_key"));
    }

    public void testHasSameUriWithServer() throws RcsException {
        assertEquals(RESOURCEURI, mClient.getUri());
    }

    public void testHasSameTypeWithServer() throws RcsException {
        assertEquals(RESOURCETYPE, mClient.getTypes()[0]);
    }

    public void testHasSameInterfaceWithServer() throws RcsException {
        assertEquals(RESOURCEINTERFACE, mClient.getInterfaces()[0]);
    }

    public void testThrowsIfObjectIsDestroyed() throws RcsException {
        mClient.destroy();

        try {
            mClient.getUri();
            fail("No exception thrown");
        } catch (RcsDestroyedObjectException e) {
        }
    }
}
