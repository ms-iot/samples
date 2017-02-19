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

import org.oic.simulator.clientcontroller.IFindResourceListener;
import org.oic.simulator.clientcontroller.SimulatorRemoteResource;

/**
 * This class implements methods for receiving notification when
 * resources are discovered in network.
 */
public class FindResourceListener implements IFindResourceListener
{

    private CountDownLatch lockObject;
    private SimulatorRemoteResourceObject simulatorRemoteResource;

    public FindResourceListener(CountDownLatch lockObject, SimulatorRemoteResourceObject simulatorRemoteResource)
    {
        this.lockObject = lockObject;
        this.simulatorRemoteResource = simulatorRemoteResource;
    }

    @Override
    public void onResourceCallback(SimulatorRemoteResource resource)
    {
        simulatorRemoteResource.setSimulatorRemoteResource(resource);

        lockObject.countDown();
    }
}
