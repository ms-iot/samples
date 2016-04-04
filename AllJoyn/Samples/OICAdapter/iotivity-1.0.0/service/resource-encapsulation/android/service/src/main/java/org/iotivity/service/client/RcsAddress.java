/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
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
 *
 ******************************************************************/

package org.iotivity.service.client;

/**
 * This is to specify a target address to discover.
 * 
 * @see RcsDiscoveryManager
 */
public final class RcsAddress {
    private final String mAddress;

    private RcsAddress(String addr) {
        mAddress = addr;
    }

    /**
     * Factory method for multicast.
     *
     */
    public static RcsAddress multicast() {
        return new RcsAddress(null);
    }

    /**
     * Factory method for unicast.
     *
     * @param address
     *            A physical address for the target.
     *
     * @throws NullPointerException
     *             If address is null.
     */
    public static RcsAddress unicast(String address) {
        if (address == null) throw new NullPointerException("address is null.");
        return new RcsAddress(address);
    }

    String getAddress() {
        return mAddress;
    }
}
