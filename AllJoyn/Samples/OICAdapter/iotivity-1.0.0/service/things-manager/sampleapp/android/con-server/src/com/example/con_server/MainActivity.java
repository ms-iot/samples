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
package com.example.con_server;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

/*
 * Starting Activity of the application responsible for
 * configuring the OcPlatform and for handling user's selection on UI.
 */
public class MainActivity extends Activity {

    private final String         LOG_TAG = "[CON-SERVER]"
                                                 + this.getClass()
                                                         .getSimpleName();
    private Handler              mHandler;
    private static MainActivity  mainActivityObj;
    private ConfigurationServer  conServerObj;
    private static String        message;
    private EditText             editText;
    public static ProgressDialog dialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mainActivityObj = this;
        Button doBootStrap = (Button) findViewById(R.id.button1);
        final Button createConfig = (Button) findViewById(R.id.button2);
        createConfig.setEnabled(false);
        editText = (EditText) findViewById(R.id.EditText);
        conServerObj = new ConfigurationServer();

        // handler for updating the UI i.e. MessageLog (TextBox)
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case 0:
                        editText.setText(message);
                        Log.i(LOG_TAG, message);
                }
            }
        };
        setmHandler(mHandler);

        // listener for doBootStrap Button
        doBootStrap.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                conServerObj.DoBootStrap();
                createConfig.setEnabled(true);
            }
        });

        // listener for createConfiguration Resource Button
        createConfig.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                conServerObj.CreateConfigurationResource();
                createConfig.setEnabled(false);
            }
        });

        // calling the method to check the Wi-fi connectivity and configuring
        // the OcPlatform
        configurePlatform();
    }

    private void configurePlatform() {

        // Check the wi-fi connectivity
        ConnectivityManager connmanager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
        NetworkInfo wifi = connmanager
                .getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (false == wifi.isConnected()) {
            // WiFi is not connected close the application
            AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(this);
            dialogBuilder.setTitle("Error");
            dialogBuilder
                    .setMessage("WiFi is not enabled/connected! Please connect the WiFi and start application again...");
            dialogBuilder.setCancelable(false);
            dialogBuilder.setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // Closing the application
                            mainActivityObj.finish();
                        }
                    });

            AlertDialog dialog = dialogBuilder.create();
            dialog.show();
            Log.i(LOG_TAG,
                    "WiFi is not enabled/connected! Please connect the WiFi and start application again...");
            return;
        }

        // If wifi is connected calling the configure method for configuring the
        // ocPlatform
        PlatformConfig cfg = new PlatformConfig(getApplicationContext(),
                ServiceType.IN_PROC, ModeType.CLIENT_SERVER, "0.0.0.0", 0,
                QualityOfService.LOW);
        OcPlatform.Configure(cfg);
        Log.i(LOG_TAG, "Configuration done Successfully");
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();

        // deleting all the resources that we have created.
        if (null != conServerObj)
            conServerObj.deleteResources();
    }

    @SuppressWarnings("unused")
    private Handler handler = new Handler() {
                                @Override
                                public void handleMessage(Message msg) {
                                    dialog.dismiss();
                                }
                            };

    // Function called when receive a reboot Request
    public static void reboot() throws InterruptedException {

        dialog = new ProgressDialog(mainActivityObj);
        dialog.setMessage("Rebooting..");
        dialog.setTitle("Please wait ...");
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setProgress(0);
        dialog.setMax(100);
        dialog.show();
        // Log.i(LOG_TAG, "Rebooting.. Please wait ...");
        Thread thread = new Thread() {
            @Override
            public void run() {
                try {
                    sleep(5000);
                    dialog.dismiss();

                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        };
        thread.start();
    }

    // Functions required for updating the UI

    public Handler getmHandler() {
        return mHandler;
    }

    public void setmHandler(Handler mHandler) {
        this.mHandler = mHandler;
    }

    public static MainActivity getMainActivityObject() {
        return mainActivityObj;
    }

    public static void setmessage(String msg) {
        message = msg;
    }
}
