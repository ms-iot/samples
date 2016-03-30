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

import org.oic.simulator.serviceprovider.ObserverInfo;

/**
 * This class provides methods to set/get observers and state
 * information.
 */
public class ObserverObject
{
    private String resourceURI;
    private int state;
    private ObserverInfo observer;

    public void setResourceURI(String resourceURI)
    {
        this.resourceURI = resourceURI;
    }

    public String getResourceURI()
    {
        return resourceURI;
    }

    public void setState(int state)
    {
        this.state = state;
    }

    public int getState()
    {
        return state;
    }

    public void setObserver(ObserverInfo observer)
    {
        this.observer = observer;
    }

    public ObserverInfo getObserver()
    {
        return observer;
    }
}
