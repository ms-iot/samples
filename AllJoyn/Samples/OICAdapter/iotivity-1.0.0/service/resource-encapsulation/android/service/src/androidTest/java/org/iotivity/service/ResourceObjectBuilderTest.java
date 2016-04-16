package org.iotivity.service;

import org.iotivity.service.server.RcsResourceObject;

import android.test.InstrumentationTestCase;

public class ResourceObjectBuilderTest extends InstrumentationTestCase {
    private static final String RESOURCEURI       = "/a/TemperatureSensor";
    private static final String RESOURCETYPE      = "oic.r.type";
    private static final String RESOURCEINTERFACE = "oic.if.baseline";

    private RcsResourceObject mObject;

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();

        mObject.destroy();
        mObject = null;
    }

    public void testResourceServerHasPropertiesSetByBuilder()
            throws RcsException {
        mObject = new RcsResourceObject.Builder(RESOURCEURI, RESOURCETYPE,
                RESOURCEINTERFACE).setDiscoverable(false).setObservable(true)
                        .build();

        assertTrue(mObject.isObservable());
    }

    public void testResourceServerHasAttrsSetByBuilder() throws RcsException {
        RcsResourceAttributes attrs = new RcsResourceAttributes();
        attrs.put("key", new RcsValue(100));

        mObject = new RcsResourceObject.Builder(RESOURCEURI, RESOURCETYPE,
                RESOURCEINTERFACE).setAttributes(attrs).build();

        assertEquals(new RcsValue(100), mObject.getAttributeValue("key"));
    }
}
