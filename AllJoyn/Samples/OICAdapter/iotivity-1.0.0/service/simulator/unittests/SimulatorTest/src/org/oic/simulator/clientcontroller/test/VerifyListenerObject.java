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

/**
 * This class tests the functionality of getting/setting
 * verification ID.
 */
public class VerifyListenerObject
{

    private String uId;
    private int id;
    private String whichOne;

    public void setuId(String uId)
    {
        this.uId = uId;
    }

    public String getuId()
    {
        return uId;
    }

    public void setId(int id)
    {
        this.id = id;
    }

    public int getId()
    {
        return id;
    }

    public void setWhichOne(String whichOne)
    {
        this.whichOne = whichOne;
    }

    public String getWhichOne()
    {
        return whichOne;
    }
}
