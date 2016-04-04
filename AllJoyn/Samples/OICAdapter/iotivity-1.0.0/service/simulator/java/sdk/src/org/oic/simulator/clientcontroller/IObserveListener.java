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
 * Provides interface for getting notification when resource model of an
 * observed resource gets changed. An IObserveListener can be registered via the
 * SimulatorRemoteResource observe call. Event listeners are notified
 * asynchronously.
 */
public interface IObserveListener {
    /**
     * This method will be called when there is a change in the resource model
     * of the remote resource.
     *
     * @param uId
     *            Unique Id of the resource.
     * @param representation
     *            {@link SimulatorResourceModel}.
     * @param sequenceNumber
     *            Sequential number for ordering the model change notifications.
     */
    public void onObserveCompleted(String uId,
            SimulatorResourceModel representation, int sequenceNumber);

    /**
     * Called when there is an error in observe request.
     *
     * @param ex
     *            Error information.
     */
    public void onObserveFailed(Throwable ex);
}
