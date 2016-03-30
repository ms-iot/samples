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

import org.oic.simulator.clientcontroller.IVerificationListener;

/**
 * This class implements listeners for getting verification
 * status callbacks.
 */
public class VerifyListener implements IVerificationListener
{

    private CountDownLatch lockObject;
    private VerifyListenerObject verifyListenerObject;

    public VerifyListener(CountDownLatch lockObject, VerifyListenerObject verifyListenerObject)
    {
        this.lockObject = lockObject;
        this.verifyListenerObject = verifyListenerObject;
    }

    @Override
    public void onVerificationStarted(String uId, int id)
    {
        verifyListenerObject.setId(id);
        verifyListenerObject.setuId(uId);
        verifyListenerObject.setWhichOne("started");

        lockObject.countDown();
    }

    @Override
    public void onVerificationAborted(String uId, int id)
    {
        verifyListenerObject.setId(id);
        verifyListenerObject.setuId(uId);
        verifyListenerObject.setWhichOne("aborted");

        lockObject.countDown();
    }

    @Override
    public void onVerificationCompleted(String uId, int id)
    {
        verifyListenerObject.setId(id);
        verifyListenerObject.setuId(uId);
        verifyListenerObject.setWhichOne("completed");

        lockObject.countDown();
    }
}
