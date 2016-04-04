/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

package org.iotivity.base;

public class ObservationInfo {

    private ObserveAction mObserveAction;
    private byte mOcObservationId;

    private ObservationInfo(int observationAction, byte observationId) {
        this.mObserveAction = ObserveAction.get(observationAction);
        this.mOcObservationId = observationId;
    }

    public ObservationInfo(ObserveAction observeAction, byte observationId) {
        this.mObserveAction = observeAction;
        this.mOcObservationId = observationId;
    }

    public ObserveAction getObserveAction() {
        return mObserveAction;
    }

    public void setObserveAction(ObserveAction observeAction) {
        this.mObserveAction = observeAction;
    }

    public byte getOcObservationId() {
        return mOcObservationId;
    }

    public void setOcObservationId(byte ocObservationId) {
        this.mOcObservationId = ocObservationId;
    }
}
