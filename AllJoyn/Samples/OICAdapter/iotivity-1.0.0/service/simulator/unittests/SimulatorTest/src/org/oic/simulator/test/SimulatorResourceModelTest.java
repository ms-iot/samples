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

package org.oic.simulator.test;

import java.util.Map;

import junit.framework.TestCase;

import org.oic.simulator.ResourceAttribute;
import org.oic.simulator.SimulatorResourceModel;

/**
 * This class tests the functionality of Simulator Resource Model
 * class APIs.
 */
public class SimulatorResourceModelTest extends TestCase
{

    private SimulatorResourceModel simulatorResourceModel;

    private static final String KEY = "test";

    static
    {
        System.loadLibrary("SimulatorManager");
        System.loadLibrary("RamlParser");
        System.loadLibrary("oc");
        System.loadLibrary("oc_logger");
        System.loadLibrary("octbstack");
    }

    @Override
    protected void setUp() throws Exception
    {
        super.setUp();

        simulatorResourceModel = new SimulatorResourceModel();
    }

    @Override
    protected void tearDown() throws Exception
    {
        super.tearDown();

        simulatorResourceModel = null;
    }

    public void testAddAttributeInt_P01()
    {
        int val = 100;

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeInt(KEY, val);
            result = result && Integer.parseInt(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeInt_N01()
    {
        int val = -10;

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeInt(KEY, val);
            result = result && Integer.parseInt(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeInt_N02()
    {
        int val = 666666;

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeInt(KEY, val);
            result = result && Integer.parseInt(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeDouble_P01()
    {
        double val = 10.11;

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeDouble(KEY, val);
            result = result && Double.parseDouble(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeDouble_N01()
    {
        double val = -11.12;

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeDouble(KEY, val);
            result = result && Double.parseDouble(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeDouble_N02()
    {
        double val = 0.0044444444444;

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeDouble(KEY, val);
            result = result && Double.parseDouble(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeString_P01()
    {
        String val = "asdf";

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeString(KEY, val);
            result = result && simulatorResourceModel.getAttribute(KEY).getValue().toString().equals(val);
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeString_N01()
    {
        String val = "";

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeString(KEY, val);
            result = result && simulatorResourceModel.getAttribute(KEY).getValue().toString().equals(val);
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeString_N03()
    {
        String val = "@#$$&^*^(*^&";

        boolean result = true;
        try
        {
            simulatorResourceModel.addAttributeString(KEY, val);
            result = result && simulatorResourceModel.getAttribute(KEY).getValue().toString().equals(val);
        }
        catch(Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testAddAttributeBoolean_P01()
    {
        boolean result = true;

        boolean val = true;

        try
        {
            simulatorResourceModel.addAttributeBoolean(KEY, val);

            result = result && ((Boolean.parseBoolean(simulatorResourceModel.getAttribute(KEY).getValue() + "")));

            val = false;

            simulatorResourceModel.addAttributeBoolean(KEY, val);

            result = result && !((Boolean.parseBoolean(simulatorResourceModel.getAttribute(KEY).getValue() + "")));
        }
        catch (Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testSize_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceModel.addAttributeInt("test1", 1234);

            result = result && (simulatorResourceModel.size() == 1);

            simulatorResourceModel.addAttributeString("test2", "asdf");
            simulatorResourceModel.addAttributeBoolean("test3", true);
            simulatorResourceModel.addAttributeDouble("test4", 22.435234);

            result = result && (simulatorResourceModel.size() == 4);
        }
        catch(Exception e)
        {
            result = false;
        }
        assertTrue(result);
    }

    public void testGetAttributes_P01()
    {
        boolean result = true;

        try
        {
            simulatorResourceModel.addAttributeInt("test1", 1234);
            simulatorResourceModel.addAttributeString("test2", "asdf");
            simulatorResourceModel.addAttributeBoolean("test3", true);
            simulatorResourceModel.addAttributeDouble("test4", 22.435234);

            Map<String, ResourceAttribute> attributes = simulatorResourceModel.getAttributes();

            result = result && (((Integer)attributes.get("test1").getValue()) == 1234) &&
                     (((String)attributes.get("test2").getValue()).equals("asdf")) &&
                     ((Boolean.parseBoolean(attributes.get("test3").getValue() + "")==true)) &&
                     (((Double)attributes.get("test4").getValue()) == 22.435234);
        }
        catch(Exception e)
        {
            result = false;
        }
        assertTrue(result);
    }

    public void testGetAttribute_P01()
    {
        int val = 100;

        boolean result = true;

        try
        {
            simulatorResourceModel.addAttributeInt(KEY, val);

            result = result && Integer.parseInt(simulatorResourceModel.getAttribute(KEY).getValue().toString()) == val;
        }
        catch(Exception e)
        {
            result = false;
        }
        assertTrue(result);
    }
}
