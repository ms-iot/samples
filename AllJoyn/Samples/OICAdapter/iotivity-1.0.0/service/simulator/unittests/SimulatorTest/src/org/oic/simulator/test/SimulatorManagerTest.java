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

import java.util.concurrent.CountDownLatch;
import junit.framework.TestCase;

import org.oic.simulator.DeviceInfo;
import org.oic.simulator.PlatformInfo;
import org.oic.simulator.SimulatorManager;
import org.oic.simulator.serviceprovider.SimulatorResourceServer;

/**
 * This class tests the functionality of Simulator Manager
 * class APIs.
 */
public class SimulatorManagerTest extends TestCase
{

    private static final String CONFIG_PATH = "./ramls/simple-light.raml";
    private static final String RESOURCE_TYPE = "oic.r.light";

    private CountDownLatch lockObject;
    private ResourceModelObject resourceModelObject;
    private ResourceModelChangeListener resourceModelChangeListener;

    private DeviceInfo info;
    private PlatformInfo platformInfo;

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

        lockObject = new CountDownLatch(1);
    }

    @Override
    protected void tearDown() throws Exception
    {
        super.tearDown();

        resourceModelObject = null;
        resourceModelChangeListener = null;
        lockObject = null;
    }

    private SimulatorResourceServer createResource()
    {
        resourceModelObject = new ResourceModelObject();
        resourceModelChangeListener = new ResourceModelChangeListener(resourceModelObject);

        SimulatorResourceServer simulatorResourceServer = null;
        try
        {
            simulatorResourceServer = SimulatorManager.createResource(CONFIG_PATH, resourceModelChangeListener);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return simulatorResourceServer;
    }

    private SimulatorResourceServer[] createResources(int n)
    {
        resourceModelObject = new ResourceModelObject();
        resourceModelChangeListener = new ResourceModelChangeListener(resourceModelObject);

        SimulatorResourceServer[] simulatorResourceServers = null;
        try
        {
            simulatorResourceServers = SimulatorManager.createResource(CONFIG_PATH, n, resourceModelChangeListener);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return simulatorResourceServers;
    }

    private void deleteResource(SimulatorResourceServer sim)
    {
        try
        {
            SimulatorManager.deleteResource(sim);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void testCreateResource_P01()
    {
        SimulatorResourceServer simulatorResourceServer = createResource();

        assertNotNull(simulatorResourceServer);

        deleteResource(simulatorResourceServer);
    }

    /**
     * When config path is empty
     */
    public void testCreateResource_N01()
    {
        String configPath = "";
        boolean result = false;

        resourceModelObject = new ResourceModelObject();
        resourceModelChangeListener = new ResourceModelChangeListener(resourceModelObject);

        SimulatorResourceServer simulatorResourceServer = null;
        try
        {
            simulatorResourceServer = SimulatorManager.createResource(configPath, resourceModelChangeListener);
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(simulatorResourceServer == null && result);
    }

    /**
     * When listener is not set. Passed null
     */
    public void testCreateResource_N02()
    {
        boolean result = false;
        SimulatorResourceServer simulatorResourceServer = null;
        try
        {
            simulatorResourceServer = SimulatorManager.createResource(CONFIG_PATH, null);
        }
        catch (Exception e)
        {
            result = true;
        }
        assertTrue(simulatorResourceServer == null && result);
    }

    /**
     * When listener and config path are set to null
     */
    public void testCreateResource_N03()
    {
        boolean result = false;
        SimulatorResourceServer simulatorResourceServer = null;
        try
        {
            simulatorResourceServer = SimulatorManager.createResource(null, null);
        }
        catch (Exception e)
        {
            result = true;
        }
        assertTrue(simulatorResourceServer == null && result);
    }

    public void testCreateResourceCount_P01()
    {
        int count = 5;

        SimulatorResourceServer[] simulatorResourceServers = createResources(count);

        assertTrue(simulatorResourceServers != null && simulatorResourceServers.length == 5);

        for(SimulatorResourceServer srs : simulatorResourceServers)
            deleteResource(srs);
    }

    /**
     * When config path is empty
     */
    public void testCreateResourceCount_N01()
    {
        int count = 5;
        String configPath = "";
        boolean result = false;

        resourceModelObject = new ResourceModelObject();
        resourceModelChangeListener = new ResourceModelChangeListener(resourceModelObject);

        SimulatorResourceServer[] simulatorResourceServers = null;
        try
        {
            simulatorResourceServers = SimulatorManager.createResource(configPath, count, resourceModelChangeListener);
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(simulatorResourceServers == null && result);
    }

    /**
     * When listener is not set
     */
    public void testCreateResourceCount_N02()
    {
        int count = 5;
        boolean result = false;

        SimulatorResourceServer[] simulatorResourceServers = null;

        try
        {
            simulatorResourceServers = SimulatorManager.createResource(CONFIG_PATH, count, null);
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(simulatorResourceServers == null && result);
    }

    /**
     * When configPath and listener are set to null
     */
    public void testCreateResourceCount_N03()
    {
        int count = 5;
        boolean result = false;

        SimulatorResourceServer[] simulatorResourceServers = null;
        try
        {
            simulatorResourceServers = SimulatorManager.createResource(null, count, null);
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(simulatorResourceServers == null && result);
    }

    /**
     * When count is set to 0
     */
    public void testCreateResourceCount_N04()
    {
        int count = 0;

        SimulatorResourceServer[] simulatorResourceServers = createResources(count);

        assertTrue(simulatorResourceServers == null);
    }

    public void testDeleteResource_P01()
    {
        boolean result = true;

        SimulatorResourceServer simRes = createResource();

        try
        {
            SimulatorManager.deleteResource(simRes);
        }
        catch (Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testDeleteResource_P02()
    {
        boolean result = true;

        SimulatorResourceServer[] simResoruces = createResources(4);

        try
        {
            SimulatorManager.deleteResource(simResoruces[0]);
        }
        catch (Exception e)
        {
            result = false;
        }

        for(SimulatorResourceServer simResServer : simResoruces)
            deleteResource(simResServer);

        assertTrue(result);
    }

    public void testDeleteResources_P01()
    {
        boolean result = true;

        createResources(4);

        try
        {
            SimulatorManager.deleteResources(RESOURCE_TYPE);
        }
        catch (Exception e)
        {
            result = false;
        }

        assertTrue(result);
    }

    public void testFindResouce_P01()
    {
        boolean result = true;

        SimulatorResourceServer simulatorResourceServer = createResource();

        SimulatorRemoteResourceObject simulatorRemoteResource = new SimulatorRemoteResourceObject();

        FindResourceListener findResourceListener = new FindResourceListener(lockObject, simulatorRemoteResource);

        try
        {
            SimulatorManager.findResource(findResourceListener);
        }
        catch (Exception e)
        {
            result = false;
        }

        assertTrue(result);

        deleteResource(simulatorResourceServer);
    }

    /**
     * Pass null to listener
     */
    public void testFindResouce_N01()
    {
        boolean result = true;

        SimulatorResourceServer simulatorResourceServer = createResource();

        try
        {
            SimulatorManager.findResource(null);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(result);

        deleteResource(simulatorResourceServer);
    }

    /**
     *  checking for crash
     */
    public void testSetDeviceInfo_P01()
    {
        SimulatorManager.setDeviceInfo("test");
    }

    /**
     *  checking for crash
     *  Pass empty
     */
    public void testSetDeviceInfo_N01()
    {
        SimulatorManager.setDeviceInfo("");
    }

    /**
     * Checking for crash
     */
    public void testSetPlatformInfo_P01()
    {
        PlatformInfo platformInfo = new PlatformInfo();
        platformInfo.setDateOfManufacture("asdf");
        platformInfo.setFirmwareVersion("asdf");
        platformInfo.setHardwareVersion("asdf");
        platformInfo.setManufacturerName("asdfdfg");
        platformInfo.setManufacturerUrl("asdffdg");
        platformInfo.setModelNumber("fddfg");
        platformInfo.setOperationSystemVersion("sadfg");
        platformInfo.setPlatformID("asdf");
        platformInfo.setPlatformVersion("asdfgfdg");
        platformInfo.setSupportUrl("adfgg");
        platformInfo.setSystemTime("adsfgfg");

        SimulatorManager.setPlatformInfo(platformInfo);
    }
}
