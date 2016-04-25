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

package org.oic.simulator.clientcontroller.test;

import java.util.HashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import junit.framework.TestCase;

import org.oic.simulator.SimulatorException;
import org.oic.simulator.SimulatorManager;
import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.clientcontroller.SimulatorObserveType;
import org.oic.simulator.clientcontroller.SimulatorRemoteResource;
import org.oic.simulator.clientcontroller.SimulatorVerificationType;
import org.oic.simulator.serviceprovider.SimulatorResourceServer;
import org.oic.simulator.test.FindResourceListener;
import org.oic.simulator.test.ResourceModelChangeListener;
import org.oic.simulator.test.ResourceModelObject;
import org.oic.simulator.test.SimulatorRemoteResourceObject;

/**
 * This class tests the functionality of Simulator Remote Resource
 * class APIs.
 */
public class SimulatorRemoteResourceTest extends TestCase
{
    private static final String CONFIG_PATH = "./ramls/simple-light.raml";
    //  private static final String RESOURCE_TYPE = "oic.light";

    private CountDownLatch lockObject;
    private ResourceModelObject resourceModelObject;
    private ResourceModelChangeListener resourceModelChangeListener;

    private SimulatorRemoteResourceObject simulatorRemoteResourceObject;
    private SimulatorRemoteResource simulatorRemoteResource;

    private SimulatorResourceServer simulatorResourceServer;

    private FindResourceListener findResourceListener;

    static
    {
        System.loadLibrary("SimulatorManager");
        System.loadLibrary("RamlParser");
        System.loadLibrary("oc");
        System.loadLibrary("oc_logger");
        System.loadLibrary("octbstack");
    }

    protected void setUp() throws Exception
    {
        super.setUp();

        lockObject = new CountDownLatch(1);
        resourceModelObject = new ResourceModelObject();
        resourceModelChangeListener = new ResourceModelChangeListener(resourceModelObject);

        simulatorResourceServer = SimulatorManager.createResource(CONFIG_PATH, resourceModelChangeListener);

        simulatorRemoteResourceObject = new SimulatorRemoteResourceObject();

        findResourceListener = new FindResourceListener(lockObject, simulatorRemoteResourceObject);

        SimulatorManager.findResource(findResourceListener);

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        simulatorRemoteResource = simulatorRemoteResourceObject.getSimulatorRemoteResource();
    }

    protected void tearDown() throws Exception
    {
        super.tearDown();

        SimulatorManager.deleteResource(simulatorResourceServer);

        lockObject = null;
        resourceModelObject = null;
        resourceModelChangeListener = null;

        simulatorRemoteResourceObject = null;
        findResourceListener = null;

        simulatorRemoteResource = null;
    }

    public void testGetUri_P01()
    {
        assertNotNull(simulatorRemoteResource.getUri());
    }

    public void testGetIsObservable_P01()
    {
        assertTrue(simulatorRemoteResource.getIsObservable());
    }

    public void testGetConnectivityType_P01()
    {
        assertNotNull(simulatorRemoteResource.getConnectivityType());
    }

    public void testGetResourceTypes_P01()
    {
        assertTrue(simulatorRemoteResource.getResourceTypes() != null && simulatorRemoteResource.getResourceTypes().size() > 0);
    }

    public void testGetResourceInterfaces_P01()
    {
        assertTrue(simulatorRemoteResource.getResourceInterfaces() != null && simulatorRemoteResource.getResourceInterfaces().size() > 0);
    }

    public void testGetId_P01()
    {
        assertNotNull(simulatorRemoteResource.getId());
    }

    public void testStartObserve_P01()
    {
        boolean result = true;
        HashMap<String, String> queryParamMap = new HashMap<String, String>();

        lockObject = new CountDownLatch(1);

        ObserveListenerObject observeListenerObject = new ObserveListenerObject();
        ObserveListener observeListener = new ObserveListener(lockObject, observeListenerObject);

        try
        {
            simulatorRemoteResource.startObserve(SimulatorObserveType.OBSERVE, queryParamMap, observeListener);
            simulatorResourceServer.addAttributeString("test", "test");
        }
        catch (Exception e1)
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

        assertTrue(observeListenerObject.getRepresentation() != null && result);

        try
        {
            simulatorRemoteResource.stopObserve();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        observeListenerObject = null;
        observeListener = null;
    }

    public void testStopObserve_P01()
    {
        boolean result = true;

        HashMap<String, String> queryParamMap = new HashMap<String, String>();
        lockObject = new CountDownLatch(1);
        ObserveListenerObject observeListenerObject = new ObserveListenerObject();
        ObserveListener observeListener = new ObserveListener(lockObject, observeListenerObject);

        try
        {
            simulatorRemoteResource.startObserve(SimulatorObserveType.OBSERVE, queryParamMap, observeListener);
            simulatorResourceServer.addAttributeString("test", "test");
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        result = result && observeListenerObject.getRepresentation() != null;

        try
        {
            simulatorRemoteResource.stopObserve();
        }
        catch (Exception e)
        {
            result = false;
            e.printStackTrace();
        }

        assertTrue(result);

        observeListenerObject = null;
        observeListener = null;
    }

    public void testGetQueryParamGetListener_P01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        HashMap<String, String> queryParamMap = new HashMap<String, String>();

        ListenerObject getListenerObject = new ListenerObject();
        GetListener getListener = new GetListener(lockObject, getListenerObject);

        try
        {
            simulatorRemoteResource.get(queryParamMap, getListener);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        if(getListenerObject.getEx() == null)
        {
            try
            {
                result = result && getListenerObject.getRepresentation() != null && getListenerObject.getRepresentation().size() > 0;
            }
            catch (SimulatorException e)
            {
                result = false;
                e.printStackTrace();
            }
        }
        else
            result = false;

        assertTrue(result);
    }

    public void testGetStringMapOfStringStringIGetListener_P01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        HashMap<String, String> queryParamMap = new HashMap<String, String>();

        String resourceInterface = "oic.if.baseline";

        ListenerObject getListenerObject = new ListenerObject();
        GetListener getListener = new GetListener(lockObject, getListenerObject);

        try
        {
            simulatorRemoteResource.get(resourceInterface, queryParamMap, getListener);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        if(getListenerObject.getEx() == null)
        {
            try
            {
                result = result && getListenerObject.getRepresentation() != null && getListenerObject.getRepresentation().size() > 0;
            }
            catch (SimulatorException e)
            {
                result = false;
                e.printStackTrace();
            }
        }
        else
            result = false;

        assertTrue(result);
    }

    public void testPut_P01()
    {
        boolean result = true;
        SimulatorResourceModel model = new SimulatorResourceModel();

        lockObject = new CountDownLatch(1);

        ListenerObject listenerObject = null;

        try
        {
            listenerObject = new ListenerObject();
            PutListener putListener = new PutListener(lockObject, listenerObject);

            model.addAttributeInt("intensity", 5);
            model.addAttributeString("power", "off");

            simulatorRemoteResource.put(model, null, putListener);
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

        assertTrue(result && listenerObject != null && listenerObject.getRepresentation() != null && listenerObject.getuId() != null);
    }

    public void testPost_P01()
    {
        boolean result = true;
        ListenerObject listenerObject = null;
        lockObject = new CountDownLatch(1);

        SimulatorResourceModel model = new SimulatorResourceModel();
        try
        {
            model.addAttributeInt("intensity", 8);
            //model.addAttributeString("power", "off");

            listenerObject = new ListenerObject();
            PostListener postListener = new PostListener(lockObject, listenerObject);

            simulatorRemoteResource.post(model, null, postListener);
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

        assertTrue(result && listenerObject != null && listenerObject.getRepresentation() != null && listenerObject.getuId() != null);
    }

    public void testGet_P01()
    {
        boolean result = true;
        ListenerObject listenerObject = null;
        lockObject = new CountDownLatch(1);

        try
        {
            listenerObject = new ListenerObject();
            GetListener onGetListener = new GetListener(lockObject, listenerObject);

            String resInterface = simulatorRemoteResource.getResourceInterfaces().get(0);

            if(resInterface == null)
                simulatorRemoteResource.get(resInterface, null, onGetListener);
            else
                result = false;
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

        assertTrue(result && listenerObject != null && listenerObject.getRepresentation() != null && listenerObject.getuId() != null);
    }

    /**
     * null resInterface
     */
    public void testGet_N01()
    {
        boolean result = false;
        ListenerObject listenerObject = null;
        lockObject = new CountDownLatch(1);

        try
        {
            listenerObject = new ListenerObject();
            GetListener onGetListener = new GetListener(lockObject, listenerObject);

            simulatorRemoteResource.get(null, null, onGetListener);
            result = false;
        }
        catch(Exception e)
        {
            result = true;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result);
    }

    /**
     * null listener
     */
    public void testGet_N02()
    {
        boolean result = false;
        try
        {
            String resInterface = simulatorRemoteResource.getResourceInterfaces().get(0);

            if(resInterface == null)
                simulatorRemoteResource.get(resInterface, null, null);
            else
                result = false;
        }
        catch(Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }

    /**
     * all params as null
     */
    public void testGet_N03()
    {
        boolean result = false;
        try
        {
            simulatorRemoteResource.get(null, null, null);
            result = false;
        }
        catch(Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }

    public void testGetWithoutResInterface_P01()
    {
        boolean result = true;
        ListenerObject listenerObject = null;
        lockObject = new CountDownLatch(1);

        try
        {
            listenerObject = new ListenerObject();
            GetListener onGetListener = new GetListener(lockObject, listenerObject);

            simulatorRemoteResource.get(null, onGetListener);
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

        assertTrue(result && listenerObject != null && listenerObject.getRepresentation() != null && listenerObject.getuId() != null);
    }

    /**
     * null listener
     */
    public void testGetWithoutResInterface_N01()
    {
        boolean result = false;
        try
        {
            simulatorRemoteResource.get(null, null);
            result = false;
        }
        catch(Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }

    public void testSetConfigInfo_P01()
    {
        boolean result = true;
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e2)
        {
            e2.printStackTrace();
            result = false;
        }

        lockObject = new CountDownLatch(1);
        VerifyListenerObject verifyListenerObject = new VerifyListenerObject();
        VerifyListener verifyListener = new VerifyListener(lockObject, verifyListenerObject);

        try
        {
            simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_POST, verifyListener);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.MILLISECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result && verifyListenerObject.getWhichOne().equals("started")&&
                   verifyListenerObject.getuId() != null &&
                   verifyListenerObject.getId() != -1);
    }

    /**
     * Passing empty
     */
    public void testSetConfigInfo_N01()
    {
        boolean result = true;
        try
        {
            simulatorRemoteResource.setConfigInfo("");
            result = false;
        }
        catch (Exception e2)
        {
            result = true;
        }

        assertTrue(result);
    }

    public void testStartVerification_P01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        VerifyListenerObject verifyListenerObject = new VerifyListenerObject();
        VerifyListener verifyListener = new VerifyListener(lockObject, verifyListenerObject);
        try
        {
            result =  result && simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_POST, verifyListener) != -1;
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.MILLISECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result && verifyListenerObject.getWhichOne().equals("started") &&
                   verifyListenerObject.getuId() != null &&
                   verifyListenerObject.getId() != -1);
    }

    public void testStartVerification_P02()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        VerifyListenerObject verifyListenerObject = new VerifyListenerObject();
        VerifyListener verifyListener = new VerifyListener(lockObject, verifyListenerObject);
        try
        {
            result =  result && simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_PUT, verifyListener) != -1;
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.MILLISECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result && verifyListenerObject.getWhichOne().equals("started") &&
                   verifyListenerObject.getuId() != null &&
                   verifyListenerObject.getId() != -1);
    }

    public void testStartVerification_P03()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        VerifyListenerObject verifyListenerObject = new VerifyListenerObject();
        VerifyListener verifyListener = new VerifyListener(lockObject, verifyListenerObject);
        try
        {
            result =  result && simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_GET, verifyListener) != -1;
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.MILLISECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result && verifyListenerObject.getWhichOne().equals("started") &&
                   verifyListenerObject.getuId() != null &&
                   verifyListenerObject.getId() != -1);
    }

    /**
     * setting listener to null
     */
    public void testStartVerification_N01()
    {
        boolean result = true;
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            result = false;
        }

        try
        {
            result = result && (simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_POST, null) == -1);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }
        assertTrue(result);
    }

    public void testStopVerification_P01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(2);
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        VerifyListenerObject verifyListenerObject = new VerifyListenerObject();
        VerifyListener verifyListener = new VerifyListener(lockObject, verifyListenerObject);
        try
        {
            result =  result && simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_POST, verifyListener) != -1;
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.MILLISECONDS);
        }
        catch (InterruptedException e)
        {
        }

        result = result && verifyListenerObject.getWhichOne().equals("started") &&
                 verifyListenerObject.getuId() != null &&
                 verifyListenerObject.getId() != -1;

        try
        {
            simulatorRemoteResource.stopVerification(verifyListenerObject.getId());
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {

            lockObject.await(100, TimeUnit.MILLISECONDS);
        }
        catch (InterruptedException e)
        {
        }

        assertTrue(result && verifyListenerObject.getWhichOne().equals("aborted") &&
                   verifyListenerObject.getuId() != null &&
                   verifyListenerObject.getId() != -1);
    }

    /**
     * Random id. This is just to check the crash
     */
    public void testStopVerification_N01()
    {
        boolean result = true;
        lockObject = new CountDownLatch(1);
        try
        {
            simulatorRemoteResource.setConfigInfo(CONFIG_PATH);
        }
        catch (Exception e2)
        {
            result = false;
            e2.printStackTrace();
        }

        VerifyListenerObject verifyListenerObject = new VerifyListenerObject();
        VerifyListener verifyListener = new VerifyListener(lockObject, verifyListenerObject);
        try
        {
            result =  result && simulatorRemoteResource.startVerification(SimulatorVerificationType.RQ_TYPE_POST, verifyListener) != -1;
        }
        catch (Exception e1)
        {
            e1.printStackTrace();
            result = false;
        }

        try
        {
            lockObject.await(10, TimeUnit.SECONDS);
        }
        catch (InterruptedException e)
        {
        }

        result = result && verifyListenerObject.getWhichOne().equals("started") &&
                 verifyListenerObject.getuId() != null &&
                 verifyListenerObject.getId() != -1;

        try
        {
            simulatorRemoteResource.stopVerification(123435);
            result = false;
        }
        catch (Exception e)
        {
            result = true;
        }

        assertTrue(result);
    }
}
