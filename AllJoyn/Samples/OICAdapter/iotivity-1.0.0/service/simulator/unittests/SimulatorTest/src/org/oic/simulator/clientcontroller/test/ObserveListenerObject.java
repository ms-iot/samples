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

import org.oic.simulator.SimulatorResourceModel;

/**
 * This class implements methods for setting/getting UID,
 * resource representation and sequence number for observe.
 */
public class ObserveListenerObject
{
    private String uId;
    private SimulatorResourceModel representation;
    private int sequenceNumber;
    private Throwable ex;

    public void setuId(String uId)
    {
        this.uId = uId;
    }

    public String getuId()
    {
        return uId;
    }

    public void setRepresentation(SimulatorResourceModel representation)
    {
        this.representation = representation;
    }

    public SimulatorResourceModel getRepresentation()
    {
        return representation;
    }

    public void setSequenceNumber(int sequenceNumber)
    {
        this.sequenceNumber = sequenceNumber;
    }

    public int getSequenceNumber()
    {
        return sequenceNumber;
    }

    public void setEx(Throwable ex)
    {
        this.ex = ex;
    }

    public Throwable getEx()
    {
        return ex;
    }
}
