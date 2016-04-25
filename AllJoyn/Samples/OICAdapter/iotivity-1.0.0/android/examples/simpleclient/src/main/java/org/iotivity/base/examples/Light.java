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
package org.iotivity.base.examples;

import org.iotivity.base.OcException;
import org.iotivity.base.OcRepresentation;

/**
 * Light
 * <p/>
 * This class is used by SimpleClient to create an object representation of a remote light resource
 * and update the values depending on the server response
 */
public class Light {
    public static final String NAME_KEY =  "name";
    public static final String STATE_KEY = "state";
    public static final String POWER_KEY = "power";

    private String mName;
    private boolean mState;
    private int mPower;

    public Light() {
        mName = "";
        mState = false;
        mPower = 0;
    }

    public void setOcRepresentation(OcRepresentation rep) throws OcException {
        mName = rep.getValue(NAME_KEY);
        mState = rep.getValue(Light.STATE_KEY);
        mPower = rep.getValue(Light.POWER_KEY);
    }

    public OcRepresentation getOcRepresentation() throws OcException {
        OcRepresentation rep = new OcRepresentation();
        rep.setValue(NAME_KEY, mName);
        rep.setValue(STATE_KEY, mState);
        rep.setValue(POWER_KEY, mPower);
        return rep;
    }

    public String getName() {
        return mName;
    }

    public void setName(String name) {
        this.mName = mName;
    }

    public boolean getState() {
        return mState;
    }

    public void setState(boolean state) {
        this.mState = state;
    }

    public int getPower() {
        return mPower;
    }

    public void setPower(int power) {
        this.mPower = power;
    }

    @Override
    public String toString() {
        return "\t" + NAME_KEY + ": " + mName +
                "\n\t" + STATE_KEY + ": " + mState +
                "\n\t" + POWER_KEY + ": " + mPower;
    }
}
