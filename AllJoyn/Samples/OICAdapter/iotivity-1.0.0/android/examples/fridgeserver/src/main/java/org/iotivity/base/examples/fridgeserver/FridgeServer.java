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

package org.iotivity.base.examples.fridgeserver;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.TextView;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

/**
 * FridgeServer
 * <p/>
 * This is the main fridgeServer class. This instantiates Refrigerator object
 * which has different resources (DeviceResource, LightResource, DoorResources).
 */
public class FridgeServer extends Activity {
    private Refrigerator refrigerator;

    /**
     * configure OIC platform and call findResource
     */
    private void startFridgeServer() {
        //create platform config
        PlatformConfig cfg = new PlatformConfig(
                this, //context
                ServiceType.IN_PROC,
                ModeType.SERVER,
                "0.0.0.0", // bind to all available interfaces
                0,
                QualityOfService.LOW);
        logMessage("Configuring platform");
        OcPlatform.Configure(cfg);
        logMessage("Creating refrigerator resources");
        refrigerator = new Refrigerator(this);
        logMessage("-----------------------------------------------------");
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    private static String TAG = "FridgeServer: ";
    public static final String MESSAGE = "message";
    public static final String INTENT = "org.iotivity.base.examples.fridgeserver";
    private TextView mConsoleTextView;
    private ScrollView mScrollView;
    private MessageReceiver mMessageReceiver = new MessageReceiver();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fridge_server);
        registerReceiver(mMessageReceiver, new IntentFilter(INTENT));

        mConsoleTextView = (TextView) findViewById(R.id.consoleTextView);
        mConsoleTextView.setMovementMethod(new ScrollingMovementMethod());
        mScrollView = (ScrollView) findViewById(R.id.scrollView);
        mScrollView.fullScroll(View.FOCUS_DOWN);
        final Button button = (Button) findViewById(R.id.button);

        if (null == savedInstanceState) {
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    button.setEnabled(false);
                    new Thread(new Runnable() {
                        public void run() {
                            startFridgeServer();
                        }
                    }).start();
                }
            });
        } else {
            String consoleOutput = savedInstanceState.getString("consoleOutputString");
            mConsoleTextView.setText(consoleOutput);
        }
    }

    public class MessageReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String message = intent.getStringExtra(MESSAGE);
            logMessage(message);
        }
    }

    public void logMessage(final String text) {
        runOnUiThread(new Runnable() {
            public void run() {
                final Message msg = new Message();
                msg.obj = text;
                mConsoleTextView.append("\n");
                mConsoleTextView.append(text);
            }
        });
        Log.i(TAG, text);
    }
}
