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

package org.iotivity.service.sample.server;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

/**
 * Starting Activity of the application responsible for configuring the
 * OcPlatform and redirecting to ServerBuilder or ResourceContainer activity as
 * per user's selection
 */
public class MainActivity extends Activity {

    private static final String LOG_TAG = MainActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (!isWifiConnected()) {
            showWifiUnavailableDialog();
            return;
        }

        configurePlatform();
    }

    public void onSimpleServerBtnClick(View v) {
        startActivity(new Intent(this, SimpleServerActivity.class));
    }

    public void onCustomServerBtnClick(View v) {
        startActivity(new Intent(this, CustomServerActivity.class));
    }

    private void showWifiUnavailableDialog() {
        new AlertDialog.Builder(this).setTitle("Error")
                .setMessage(
                        "WiFi is not enabled/connected! Please connect the WiFi and start application again...")
                .setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                }).create().show();
    }

    private boolean isWifiConnected() {
        ConnectivityManager connManager = (ConnectivityManager) getSystemService(
                CONNECTIVITY_SERVICE);
        return connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI)
                .isConnected();
    }

    private void configurePlatform() {
        OcPlatform.Configure(new PlatformConfig(getApplicationContext(),
                ServiceType.IN_PROC, ModeType.CLIENT_SERVER, "0.0.0.0", 0,
                QualityOfService.LOW));
        Log.i(LOG_TAG, "Configuration done successfully");
    }
}