/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.oic.simulator.serviceprovider.test;

import java.util.Vector;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import junit.framework.TestCase;

import org.oic.simulator.InvalidArgsException;
import org.oic.simulator.ResourceAttribute;
import org.oic.simulator.SimulatorException;
import org.oic.simulator.SimulatorManager;
import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.serviceprovider.AutomationType;
import org.oic.simulator.serviceprovider.SimulatorResourceServer;
import org.oic.simulator.test.ResourceModelChangeListener;
import org.oic.simulator.test.ResourceModelObject;

/**
 * This class tests the functionality of Simulator Resource Server
 * class APIs.
 */
public class SimlatorResourceServerTest extends TestCase
{

    private static final String CONFIG_PATH = "./ramls/simple-light.raml";

    private static final String KEY = "testkey";

    private CountDownLatch lockObject;
    private ResourceModelObject resourceModelObject;
    private ResourceModelChangeListener resourceModelChangeListener;

    private SimulatorResourceServer simulatorResourceServer;

    static
    {
        System.loadLibrary("SimulatorManager");
        System.loadLibrary("RamlParser");
        System.loadLibrary("YamlParser");
        System.loadLibrary("oc");
        System.loadLibrary("oc_logger");
        System.loadLibrary("octbstack");
    }

    protected void setUp() throws Exception
    {
        super.setUp();
        lockObject= new CountDownLatch(1);
        resourceModelObject = new ResourceModelObject();
        resourceModelChangeListener = new ResourceModelChangeListener(resourceModelObject);
        simulatorResourceServer = SimulatorManager.createResource(CONFIG_PATH, resourceModelChangeListener);
    }

    protected void tearDown() throws Exception
    {
        super.tearDown();

        SimulatorManager.deleteResource(simulatorResourceServer);

        lockObject = null;
        resourceModelObject = null;
        resourceModelChangeListener = null;

        simulatorResourceServer = null;
    }

    public void testGetURI_P01()
    {
        assertNotNull(simulatorResourceServer.getURI());
    }

    public void testGetResourceType_P01()
    {
        assertNotNull(simulatorResourceServer.getResourceType());
    }

    public void testGetInterfaceType_P01()
    {
        assertNotNull(simulatorResourceServer.getInterfaceType());
    }

    public void testGetModel_P01()
    {
        boolean result = false;

        try
        {
            if(simulatorResourceServer.getModel() != null && simulatorResourceServer.getModel().size() > 0)
                result = true;
        }
        catch (InvalidArgsException e)
        {
            e.printStackTrace();
        }
        catch (SimulatorException e)
        {
            e.printStackTrace();
        }

        assertTrue(result);
    }

    public void testAddAttributeInteger_P01()
    {
        try
        {
            simulatorResourceServer.addAttributeInteger(KEY, 2);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        assertEquals(Integer.parseInt(getValue(KEY) + ""), 2);
    }

    public void testAddAttributeDouble_P01()
    {
        try
        {
            simulatorResourceServer.addAttributeDouble(KEY, 4.0);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        assertEquals(Double.parseDouble(getValue(KEY) + ""), 4.0);
    }

    public void testAddAttributeBoolean_P01()
    {
        try
        {
            simulatorResourceServer.addAttributeBoolean(KEY, true);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        assertEquals(Boolean.parseBoolean(getValue(KEY) + ""), true);
    }

    public void testaddAttributeString_P01()
    {
        try
        {
            simulatorResourceServer.addAttributeString(KEY, "test");
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        assertEquals(getValue(KEY) + "", "test");
    }

    public void testUpdateAttributeInteger_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeInteger(KEY, 10);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        try
        {
            simulatorResourceServer.updateAttributeInteger(KEY, 12);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }
        result = result && Integer.parseInt(getValue(KEY) + "") == 12;

        assertTrue(result);
    }

    public void testUpdateAttributeDouble_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeDouble(KEY, 22.0);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Double.parseDouble(getValue(KEY) + "") == 22.0;

        try
        {
            simulatorResourceServer.updateAttributeDouble(KEY, 25.3);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Double.parseDouble(getValue(KEY) + "") == 25.3;

        assertTrue(result);
    }

    public void testUpdateAttributeBoolean_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeBoolean(KEY, true);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Boolean.parseBoolean(getValue(KEY) + "")==true;

        try
        {
            simulatorResourceServer.updateAttributeBoolean(KEY, false);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && !Boolean.parseBoolean(getValue(KEY) + "");

        assertTrue(result);
    }

    public void testupdateAttributeString_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeString(KEY, "old");
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && getValue(KEY).equals("old");

        try
        {
            simulatorResourceServer.updateAttributeString(KEY, "new");
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && getValue(KEY).equals("new");

        assertTrue(result);
    }

    public void testSetRange_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeInteger(KEY, 10);
            simulatorResourceServer.setRange(KEY, 1, 12);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        try
        {
            simulatorResourceServer.updateAttributeInteger(KEY, 3);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 3;

        assertTrue(result);
    }

    /**
     * Try to set the value out of range
     */
    public void testSetRange_N01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeInteger(KEY, 10);
            simulatorResourceServer.setRange(KEY, 1, 12);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        try
        {
            simulatorResourceServer.updateAttributeInteger(KEY, 13);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        assertTrue(result);
    }

    public void testSetAllowedValuesInteger_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeInteger(KEY, 10);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        Vector<Integer> values = new Vector<Integer>();
        values.add(1);
        values.add(10);
        values.add(20);
        values.add(50);

        try
        {
            simulatorResourceServer.setAllowedValuesInteger(KEY, values);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        try
        {
            simulatorResourceServer.updateAttributeInteger(KEY, 20);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 20;

        assertTrue(result);
    }

    /**
     * Try setting with out of range
     */
    public void testSetAllowedValuesInteger_N01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeInteger(KEY, 10);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        Vector<Integer> values = new Vector<Integer>();
        values.add(1);
        values.add(10);
        values.add(20);
        values.add(50);

        try
        {
            simulatorResourceServer.setAllowedValuesInteger(KEY, values);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        try
        {
            simulatorResourceServer.updateAttributeInteger(KEY, 2);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        result = result && Integer.parseInt(getValue(KEY) + "") == 10;

        assertTrue(result);
    }

    /**
     * Try setting values multiple times
     */
    public void testSetAllowedValuesDouble_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeDouble(KEY, 11.5);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        Vector<Double> values = new Vector<Double>();
        values.add(11.5);
        values.add(10.5);
        values.add(20.5);
        values.add(50.5);

        try
        {
            simulatorResourceServer.setAllowedValuesDouble(KEY, values);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Double.parseDouble(getValue(KEY) + "") == 11.5;

        try
        {
            simulatorResourceServer.updateAttributeDouble(KEY, 10.5);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Double.parseDouble(getValue(KEY) + "") == 10.5;

        assertTrue(result);
    }

    /**
     * Try setting with out of range
     */
    public void testSetAllowedValuesDouble_N01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeDouble(KEY, 10.5);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        Vector<Double> values = new Vector<Double>();
        values.add(11.5);
        values.add(10.5);
        values.add(20.5);
        values.add(50.5);

        try
        {
            simulatorResourceServer.setAllowedValuesDouble(KEY, values);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && Double.parseDouble(getValue(KEY) + "") == 10.5;

        try
        {
            simulatorResourceServer.updateAttributeDouble(KEY, 2.2);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        result = result && Double.parseDouble(getValue(KEY) + "") == 10.5;

        assertTrue(result);
    }

    public void testsetAllowedValuesString_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeString(KEY, "mon");
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        Vector<String> values = new Vector<String>();
        values.add("mon");
        values.add("tue");
        values.add("wed");

        try
        {
            simulatorResourceServer.setAllowedValuesString(KEY, values);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && getValue(KEY).equals("mon");

        try
        {
            simulatorResourceServer.updateAttributeString(KEY, "tue");
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && getValue(KEY).equals("tue");

        assertTrue(result);
    }

    /**
     * Set the value that is not in allowed values
     */
    public void testsetAllowedValuesString_N01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeString(KEY, "mon");
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        Vector<String> values = new Vector<String>();
        values.add("mon");
        values.add("tue");
        values.add("wed");

        try
        {
            simulatorResourceServer.setAllowedValuesString(KEY, values);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && getValue(KEY).equals("mon");

        try
        {
            simulatorResourceServer.updateAttributeString(KEY, "thu");
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        result = result && getValue(KEY).equals("mon");

        assertTrue(result);
    }

    public void testRemoveAttribute_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceServer.addAttributeString(KEY, "fri");
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        result = result && getValue(KEY).equals("fri");

        try
        {
            simulatorResourceServer.removeAttribute(KEY);
            result = result && !simulatorResourceServer.getModel().getAttributes().containsKey(KEY);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        assertTrue(result);
    }

    /**
     * Try removing the attribute thats not present
     */
    public void testRemoveAttribute_N01()
    {
        boolean result = false;

        try
        {
            simulatorResourceServer.removeAttribute("something");
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }

    /**
     * Try removing when null is passed
     */
    public void testRemoveAttribute_N02()
    {
        boolean result = false;

        try
        {
            simulatorResourceServer.removeAttribute(null);
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }

    /**
     * Try removing when attribute is empty
     */
    public void testRemoveAttribute_N03()
    {
        boolean result = false;

        try
        {
            simulatorResourceServer.removeAttribute("");
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }

    public void testStartResourceAutomation_P01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        AutomationObject automationObject = new AutomationObject();
        AutomationListener automationListener = new AutomationListener(lockObject, automationObject);
        int id = 0;
        try
        {
            id = simulatorResourceServer.startResourceAutomation(AutomationType.NORMAL, automationListener);
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        try
        {
            lockObject.await(10,TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }

        assertTrue(result && automationObject.getResourceURI() != null && automationObject.getAutomationId() != -1 && id != -1);

        try
        {
            simulatorResourceServer.stopAutomation(id);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * null listener
     */
    public void testStartResourceAutomation_N01()
    {
        boolean result = true;
        int id = 0;
        try
        {
            id = simulatorResourceServer.startResourceAutomation(AutomationType.NORMAL, null);
        }
        catch (Exception e)
        {
            result = false;
        }
        assertTrue(result && id == -1);
    }

    public void testStartAttributeAutomation_P01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        AutomationObject automationObject = new AutomationObject();
        AutomationListener automationListener = new AutomationListener(lockObject, automationObject);
        int id = 0;
        try
        {
            id = simulatorResourceServer.startAttributeAutomation(simulatorResourceServer.getModel().getAttributes().get(0).getName(),
                    AutomationType.NORMAL, automationListener);
        }
        catch (Exception e)
        {
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result && automationObject.getResourceURI() != null && automationObject.getAutomationId() != -1 && id != -1);

        try
        {
            simulatorResourceServer.stopAutomation(id);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * null listener
     */
    public void testStartAttributeAutomation_N01()
    {
        boolean result = false;
        int id = 0;
        try
        {
            id = simulatorResourceServer.startAttributeAutomation(simulatorResourceServer.getModel().getAttributes().get(0).getName(),
                    AutomationType.NORMAL, null);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(result && id != -1);
    }

    public void testStopAutomation_P01()
    {
        boolean result = true;

        lockObject = new CountDownLatch(1);

        AutomationObject automationObject = new AutomationObject();
        AutomationListener automationListener = new AutomationListener(lockObject, automationObject);

        int id = 0;
        try
        {
            id = simulatorResourceServer.startResourceAutomation(AutomationType.NORMAL, automationListener);
        }
        catch(Exception e)
        {
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        result = result && automationObject.getResourceURI() != null && automationObject.getAutomationId() != -1 && id != -1;

        try
        {
            simulatorResourceServer.stopAutomation(id);
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    /**
     * checking for crash
     * random id
     */
    public void testStopAutomation_N01()
    {
        boolean result = false;
        try
        {
            simulatorResourceServer.stopAutomation(144353544);
        }
        catch (Exception e)
        {
            result = true;
            e.printStackTrace();
        }
        assertTrue(result);
    }

    private Object getValue(Object key)
    {
        SimulatorResourceModel simulatorResourceModel = null;
        try
        {
            simulatorResourceModel = simulatorResourceServer.getModel();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        ResourceAttribute resourceAttribute = null;
        try
        {
            resourceAttribute = simulatorResourceModel.getAttributes().get(key);
        }
        catch (SimulatorException e)
        {
            e.printStackTrace();
        }
        return resourceAttribute.getValue();
    }
}
