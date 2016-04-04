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

package org.oic.simulator.serviceprovider;

import org.oic.simulator.SimulatorResourceModel;

/**
 * Interface for receiving notifications whenever there is a change in the
 * resource model.
 */
public interface IResourceModelChangedListener {
    /**
     * This callback method will be called to notify about the changes in the
     * resource model.
     *
     * @param resourceURI
     *            URI of resource.
     * @param resourceModel
     *            {@link SimulatorResourceModel} of the resource.
     */
    public void onResourceModelChanged(String resourceURI,
            SimulatorResourceModel resourceModel);
}