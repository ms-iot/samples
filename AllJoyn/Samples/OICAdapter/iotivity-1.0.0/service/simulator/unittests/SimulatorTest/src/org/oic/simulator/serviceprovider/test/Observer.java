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

import java.util.concurrent.CountDownLatch;

import org.oic.simulator.serviceprovider.IObserver;
import org.oic.simulator.serviceprovider.ObserverInfo;

/**
 * This class implements methods for receiving observer
 * change callbacks.
 */
public class Observer implements IObserver
{
    private CountDownLatch lockObject;
    private ObserverObject observerObject;

    public Observer(CountDownLatch lockObject, ObserverObject observerObject)
    {
        this.lockObject = lockObject;
        this.observerObject = observerObject;
    }

    @Override
    public void onObserverChanged(String resourceURI, int state, ObserverInfo observer)
    {
        observerObject.setState(state);
        observerObject.setResourceURI(resourceURI);
        observerObject.setObserver(observer);

        lockObject.countDown();
    }
}
