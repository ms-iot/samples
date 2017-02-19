//******************************************************************
//
// Copyright 2014 Intel Corporation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

package org.iotivity.guiclient;

import android.os.Handler;

import java.util.List;

/**
 * Applications wishing to use the OcWorker object must implement this interface to
 * receive notification of OcWorker's ResourceFound and ResourceChanged events.
 *
 * @see org.iotivity.guiclient.OcWorker
 */
public interface OcWorkerListener {

    /**
     * Called whenever a new Resource is discovered.
     *
     * Note that the calling thread for this callback is not a UI thread.  OcWorkerListeners
     * with UI functionality should post a message to their own UI thread, or similar action.
     *
     * @param resourceInfo
     */
    public void onResourceFound(final OcResourceInfo resourceInfo);

    /**
     * Called whenever a previously-discovered Resource changes, e.g. as a result of Put,
     * or Observe callbacks.
     *
     * Note that the calling thread for this callback is not a UI thread.  OcWorkerListeners
     * with UI functionality should post a message to their own UI thread, or similar action.
     *
     * @param resourceInfo
     */
    public void onResourceChanged(final OcResourceInfo resourceInfo);

}
