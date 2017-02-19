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
import org.oic.simulator.IAutomation;

/**
 * This class implements methods for receiving notifications on
 * completion of automation.
 */
public class AutomationListener implements IAutomation
{

    private CountDownLatch lockObject;
    private AutomationObject automationObject;

    public AutomationListener(CountDownLatch lockObject, AutomationObject automationObject)
    {
        this.lockObject = lockObject;
        this.automationObject = automationObject;
    }

    @Override
    public void onAutomationComplete(String resourceURI, int automationId)
    {
        automationObject.setAutomationId(automationId);
        automationObject.setResourceURI(resourceURI);

        lockObject.countDown();
    }
}
