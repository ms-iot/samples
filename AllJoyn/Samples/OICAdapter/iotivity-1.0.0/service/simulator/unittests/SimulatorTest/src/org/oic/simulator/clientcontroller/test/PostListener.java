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

import java.util.concurrent.CountDownLatch;

import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.clientcontroller.IPostListener;

/**
 * This class implements methods for receiving notification when
 * response is received for POST request.
 */
public class PostListener implements IPostListener
{

    private CountDownLatch lockObject;
    private ListenerObject postListenerObject;

    public PostListener(CountDownLatch lockObject, ListenerObject postListenerObject)
    {
        this.lockObject = lockObject;
        this.postListenerObject = postListenerObject;
    }

    @Override
    public void onPostCompleted(String uId, SimulatorResourceModel representation)
    {
        if (null != postListenerObject) {
            postListenerObject.setuId(uId);
            postListenerObject.setRepresentation(representation);
        }

        lockObject.countDown();
    }

    @Override
    public void onPostFailed(Throwable ex)
    {
        if (null != postListenerObject)
            postListenerObject.setEx(ex);

        lockObject.countDown();
    }
}
