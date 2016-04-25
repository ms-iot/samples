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

/**
 * Interface for receiving the verification status via callback. An
 * IVerificationListener can be registered via the resource startVerification
 * call. Event listeners are notified asynchronously.
 */
public interface IVerificationListener {
    /**
     * Called when the verification request is accepted and started.
     *
     * @param uId
     *            Unique Id of the resource.
     * @param id
     *            Verification Id.
     */
    public void onVerificationStarted(String uId, int id);

    /**
     * Called when the verification is stopped before its completion.
     *
     * @param uId
     *            Unique Id of the resource.
     * @param id
     *            Verification Id.
     */
    public void onVerificationAborted(String uId, int id);

    /**
     * Called when the verification is done.
     *
     * @param uId
     *            Unique Id of the resource.
     * @param id
     *            Verification Id.
     */
    public void onVerificationCompleted(String uId, int id);
}
