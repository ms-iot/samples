/******************************************************************
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 * <p>
 * <p>
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************/

package org.iotivity.service.sample.client;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import org.iotivity.service.RcsException;
import org.iotivity.service.client.RcsAddress;
import org.iotivity.service.client.RcsDiscoveryManager;
import org.iotivity.service.client.RcsRemoteResourceObject;

/**
 * It contains the discover resource API for Discovering Container Resource
 */
public class ContainerClientActivity extends Activity implements
        RcsDiscoveryManager.OnResourceDiscoveredListener {
    private final String LOG_TAG = "[SampleClient] "
                                         + this.getClass().getSimpleName();

    private TextView     mLogView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_container_client);
        mLogView = (TextView) findViewById(R.id.log);
    }

    public void onDiscoverResourceClick(View v) {
        try {
            RcsDiscoveryManager.getInstance().discoverResourceByType(
                    RcsAddress.multicast(), "oic.r.sensor",
                    ContainerClientActivity.this);
            mLogView.setText("");
        } catch (RcsException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onResourceDiscovered(
            final RcsRemoteResourceObject discoveredResource) {
        Log.i(LOG_TAG, "onResourceDiscovered");

        runOnUiThread(new Runnable() {
            public void run() {
                try {
                    mLogView.setText(Utils.resourceInfo(discoveredResource));
                } catch (RcsException e) {
                    Utils.showError(ContainerClientActivity.this, LOG_TAG, e);
                }
            }
        });
    }
}
