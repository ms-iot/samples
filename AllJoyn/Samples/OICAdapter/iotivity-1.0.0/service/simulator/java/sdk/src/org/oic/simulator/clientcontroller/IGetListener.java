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

package org.oic.simulator.clientcontroller;

import org.oic.simulator.SimulatorResourceModel;

/**
 * Interface for receiving response for GET request. An IGetListener can be
 * registered via the resource get call. Event listeners are notified
 * asynchronously.
 */
public interface IGetListener {
    /**
     * This method will be called when response from the remote resource for GET
     * request arrives.
     *
     * @param uId
     *            Unique Id of the resource.
     * @param representation
     *            {@link SimulatorResourceModel}.
     */
    public void onGetCompleted(String uId, SimulatorResourceModel representation);

    /**
     * Called when there is an error in GET request.
     *
     * @param ex
     *            Error information.
     */
    public void onGetFailed(Throwable ex);
}
