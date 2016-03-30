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

import android.content.Context;

/**
 * Refrigerator
 * <p/>
 * Refrigerator class has different objects (resources) which are instantiated when a
 * Refrigerator object is created. Operations are performed on each of the individual resources.
 */
public class Refrigerator {
    public static final String LEFT_SIDE = "left";
    public static final String RIGHT_SIDE = "right";
    public static final String RANDOM_SIDE = "random";

    private FridgeResource mFridge;
    private LightResource mLight;
    private DoorResource mLeftDoor;
    private DoorResource mRightDoor;
    private DoorResource mRandomDoor;

    /**
     * constructor - creates resources of light and doors
     *
     * @param context needed by individual resources to be able to send broadcast
     *                messages to be displayed on the user screen
     */
    Refrigerator(Context context) {
        mFridge = new FridgeResource(context);
        mLight = new LightResource(context);
        mLeftDoor = new DoorResource(LEFT_SIDE, context);
        mRightDoor = new DoorResource(RIGHT_SIDE, context);
        mRandomDoor = new DoorResource(RANDOM_SIDE, context);

        mLight.bindTo(mFridge.getHandle());
        mLeftDoor.bindTo(mFridge.getHandle());
        mRightDoor.bindTo(mFridge.getHandle());
        mRandomDoor.bindTo(mFridge.getHandle());
    }
}