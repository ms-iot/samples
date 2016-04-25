package org.iotivity.service;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;

import org.iotivity.service.client.RcsRemoteResourceObject.OnRemoteAttributesReceivedListener;
import org.iotivity.service.server.RcsGetResponse;
import org.iotivity.service.server.RcsRequest;
import org.iotivity.service.server.RcsResourceObject.AutoNotifyPolicy;
import org.iotivity.service.server.RcsResourceObject.GetRequestHandler;
import org.iotivity.service.server.RcsResourceObject.OnAttributeUpdatedListener;
import org.iotivity.service.server.RcsResourceObject.SetRequestHandler;
import org.iotivity.service.server.RcsResourceObject.SetRequestHandlerPolicy;
import org.iotivity.service.server.RcsSetResponse;
import org.mockito.Mockito;

public class ResourceObjectTest extends TestBase {
    private static final String NEW_KEY = "new" + KEY;

    private OnRemoteAttributesReceivedListener mOnRemoteAttributesReceivedListener;
    private SetRequestHandler                  mSetRequestHandler;
    private GetRequestHandler                  mGetRequestHandler;
    private OnAttributeUpdatedListener         mOnAttributeUpdatedListener;

    private void setSetRequestHandlerReturn(RcsSetResponse setResponse) {
        doReturn(setResponse).when(mSetRequestHandler).onSetRequested(
                any(RcsRequest.class), any(RcsResourceAttributes.class));
    }

    private void setGetRequestHandlerReturn(RcsGetResponse getResponse) {
        doReturn(getResponse).when(mGetRequestHandler).onGetRequested(
                any(RcsRequest.class), any(RcsResourceAttributes.class));
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mOnRemoteAttributesReceivedListener = Mockito
                .mock(OnRemoteAttributesReceivedListener.class);

        mGetRequestHandler = Mockito.mock(GetRequestHandler.class);
        mSetRequestHandler = Mockito.mock(SetRequestHandler.class);
        mOnAttributeUpdatedListener = Mockito
                .mock(OnAttributeUpdatedListener.class);
    }

    public void testDefalutAutoNotifyPolicyIsUpdated() throws RcsException {
        assertEquals(AutoNotifyPolicy.UPDATED, mServer.getAutoNotifyPolicy());
    }

    public void testAutoNotifyPolicyCanBeSet() throws RcsException {
        mServer.setAutoNotifyPolicy(AutoNotifyPolicy.NEVER);
        assertEquals(AutoNotifyPolicy.NEVER, mServer.getAutoNotifyPolicy());

        mServer.setAutoNotifyPolicy(AutoNotifyPolicy.UPDATED);
        assertEquals(AutoNotifyPolicy.UPDATED, mServer.getAutoNotifyPolicy());

        mServer.setAutoNotifyPolicy(AutoNotifyPolicy.ALWAYS);
        assertEquals(AutoNotifyPolicy.ALWAYS, mServer.getAutoNotifyPolicy());
    }

    public void testDefalutSetRequestHandlerPolicyIsNever()
            throws RcsException {
        assertEquals(SetRequestHandlerPolicy.NEVER,
                mServer.getSetRequestHandlerPolicy());
    }

    public void testSetRequestHandlerPolicyCanBeSet() throws RcsException {
        mServer.setSetRequestHandlerPolicy(SetRequestHandlerPolicy.ACCEPT);
        assertEquals(SetRequestHandlerPolicy.ACCEPT,
                mServer.getSetRequestHandlerPolicy());

        mServer.setSetRequestHandlerPolicy(SetRequestHandlerPolicy.NEVER);
        assertEquals(SetRequestHandlerPolicy.NEVER,
                mServer.getSetRequestHandlerPolicy());
    }

    public void testGetRequestHandlerCalledIfReceived() throws RcsException {
        setGetRequestHandlerReturn(RcsGetResponse.defaultAction());

        mServer.setGetRequestHandler(mGetRequestHandler);

        mClient.getRemoteAttributes(mOnRemoteAttributesReceivedListener);

        verify(mGetRequestHandler, timeout(1000)).onGetRequested(
                any(RcsRequest.class), any(RcsResourceAttributes.class));
    }

    public void testGetRequestHandlerCanReturnCustomAttrsAsResponse()
            throws RcsException {
        RcsResourceAttributes newAttrs = new RcsResourceAttributes();
        newAttrs.put(NEW_KEY, new RcsValue(RAW_VALUE + 1));

        setGetRequestHandlerReturn(RcsGetResponse.create(newAttrs));

        mServer.setGetRequestHandler(mGetRequestHandler);
        mClient.getRemoteAttributes(mOnRemoteAttributesReceivedListener);

        verify(mGetRequestHandler, timeout(1000)).onGetRequested(
                any(RcsRequest.class), any(RcsResourceAttributes.class));

        verify(mOnRemoteAttributesReceivedListener, timeout(1000))
                .onAttributesReceived(eq(newAttrs), anyInt());
    }

    public void testSetRequestHandlerCalledIfReceived() throws RcsException {
        setSetRequestHandlerReturn(RcsSetResponse.defaultAction());

        mServer.setSetRequestHandler(mSetRequestHandler);

        mClient.setRemoteAttributes(new RcsResourceAttributes(),
                mOnRemoteAttributesReceivedListener);

        verify(mSetRequestHandler, timeout(1000)).onSetRequested(
                any(RcsRequest.class), any(RcsResourceAttributes.class));
    }

    public void testIgnoreSetRequestIfSetRequestHandlerReturnsIgnore()
            throws RcsException {
        setSetRequestHandlerReturn(RcsSetResponse.ignore());

        mServer.setSetRequestHandler(mSetRequestHandler);

        RcsResourceAttributes newAttrs = new RcsResourceAttributes();
        newAttrs.put(KEY, new RcsValue(RAW_VALUE + 1));

        mClient.setRemoteAttributes(newAttrs,
                mOnRemoteAttributesReceivedListener);

        verify(mSetRequestHandler, timeout(1000))
                .onSetRequested(any(RcsRequest.class), eq(newAttrs));

        assertEquals(RAW_VALUE, mServer.getAttributeValue(KEY).asInt());
    }

    public void testAcceptSetRequestIfSetRequestHandlerReturnsAccept()
            throws RcsException {
        setSetRequestHandlerReturn(RcsSetResponse.accept());

        mServer.setSetRequestHandlerPolicy(SetRequestHandlerPolicy.NEVER);
        mServer.setSetRequestHandler(mSetRequestHandler);

        RcsResourceAttributes newAttrs = new RcsResourceAttributes();
        newAttrs.put(NEW_KEY, new RcsValue(RAW_VALUE + 1));

        mClient.setRemoteAttributes(newAttrs,
                mOnRemoteAttributesReceivedListener);

        verify(mSetRequestHandler, timeout(1000))
                .onSetRequested(any(RcsRequest.class), eq(newAttrs));

        assertEquals(RAW_VALUE + 1, mServer.getAttributeValue(NEW_KEY).asInt());
    }

    public void testSetRequestHandlerCanReturnCustomAttrsAsResponse()
            throws RcsException {
        final RcsResourceAttributes newAttrs = new RcsResourceAttributes();
        newAttrs.put(NEW_KEY, new RcsValue(RAW_VALUE + 1));

        setSetRequestHandlerReturn(RcsSetResponse.create(newAttrs));

        mServer.setSetRequestHandler(mSetRequestHandler);
        mClient.setRemoteAttributes(new RcsResourceAttributes(),
                mOnRemoteAttributesReceivedListener);

        verify(mSetRequestHandler, timeout(1000)).onSetRequested(
                any(RcsRequest.class), any(RcsResourceAttributes.class));

        verify(mOnRemoteAttributesReceivedListener, timeout(1000))
                .onAttributesReceived(eq(newAttrs), anyInt());
    }

    public void testOnAttributeUpdatedListenerIsCalledIfValueUpdated()
            throws RcsException {
        final RcsValue newValue = new RcsValue(RAW_VALUE + 1);
        final RcsResourceAttributes newAttrs = new RcsResourceAttributes();
        newAttrs.put(KEY, newValue);

        mServer.addAttributeUpdatedListener(KEY, mOnAttributeUpdatedListener);
        mClient.setRemoteAttributes(newAttrs,
                mOnRemoteAttributesReceivedListener);

        verify(mOnAttributeUpdatedListener, timeout(1000))
                .onAttributeUpdated(eq(VALUE), eq(newValue));
    }

    public void testOnAttributeUpdatedListenerIsNotCalledIRemoved()
            throws RcsException {
        final RcsValue newValue = new RcsValue(RAW_VALUE + 1);
        final RcsResourceAttributes newAttrs = new RcsResourceAttributes();
        newAttrs.put(KEY, newValue);

        mServer.addAttributeUpdatedListener(KEY, mOnAttributeUpdatedListener);
        mServer.removeAttribute(KEY);
        mClient.setRemoteAttributes(newAttrs,
                mOnRemoteAttributesReceivedListener);

        verify(mOnAttributeUpdatedListener, never())
                .onAttributeUpdated(eq(VALUE), eq(newValue));
    }

    public void testThrowIfObjectIsDestroyed() throws RcsException {
        mServer.destroy();
        try {
            mServer.removeAttribute("");
            fail("No exception thrown");
        } catch (RcsDestroyedObjectException e) {
        }

    }
}
