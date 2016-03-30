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

/**
 * OcResourceIdentifier represents the identity information for a server. This
 * object combined with the OcResource's URI property uniquely identify an
 * OcResource on or across networks.
 */
public class OcResourceIdentifier {
    private OcResourceIdentifier(long nativeHandle) {
        this.mNativeHandle = nativeHandle;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();

        dispose();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof OcResourceIdentifier)) {
            return false;
        }
        OcResourceIdentifier other = (OcResourceIdentifier) obj;
        return equalsN(other);
    }

    @Override
    public int hashCode() {
        //return the same hash code for every object to force dictionary objects to use equals() in
        //key comparisons, since IoTivity wants to treat OcResourceIdentifier as a blob
        return 0;
    }

    private native boolean equalsN(OcResourceIdentifier other);

    private native void dispose();

    private long mNativeHandle;
}
