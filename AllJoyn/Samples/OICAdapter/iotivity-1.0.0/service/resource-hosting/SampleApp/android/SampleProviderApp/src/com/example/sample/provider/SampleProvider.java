//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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
package com.example.sample.provider;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ScrollView;
import android.widget.TextView;

public class SampleProvider extends Activity implements OnClickListener,
        IMessageLogger {
    private final static String   TAG              = "NMProvider : SampleProvider";
    private TextView              mLogTextView;
    private TextView              mTempValue;
    private TextView              mHumValue;
    private TemperatureResource   mySensor;
    private boolean               isExecutePresence;
    private ScrollView            sv_sclLog;
    private MessageReceiver       mMessageReceiver = new MessageReceiver();
    private Handler               mHandler;
    private static String         message;
    private static SampleProvider sampleProviderObj;
    private String                temp;
    private String                hum;

    /*
     * To initialize UI Function Setting To execute initOICStack for running
     * find resource
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {

        sampleProviderObj = this;
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sampleprovider_layout);
        registerReceiver(mMessageReceiver, new IntentFilter(
                "com.example.sample.provider.SampleProvider"));

        mLogTextView = (TextView) findViewById(R.id.txtLog);
        mTempValue = (TextView) findViewById(R.id.temperatureValue);
        mHumValue = (TextView) findViewById(R.id.humidityValue);

        sv_sclLog = (ScrollView) findViewById(R.id.sclLog);
        sv_sclLog.fullScroll(View.FOCUS_DOWN);
        findViewById(R.id.btnTemperatureUP).setOnClickListener(this);
        findViewById(R.id.btnTemperatureDown).setOnClickListener(this);
        findViewById(R.id.btnHumidityUP).setOnClickListener(this);
        findViewById(R.id.btnHumidityDown).setOnClickListener(this);
        findViewById(R.id.btnLogClear).setOnClickListener(this);

        isExecutePresence = false;
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case 0:
                        String[] tempHum = message.split(":");
                        mTempValue.setText(tempHum[0]);
                        mHumValue.setText(tempHum[1]);
                }
            }
        };
        setmHandler(mHandler);
    }

    private void initOICStack() {
        // create platform config
        PlatformConfig cfg = new PlatformConfig(this, ServiceType.IN_PROC,
                ModeType.SERVER, "0.0.0.0", // bind to all available interfaces
                0, QualityOfService.LOW);

        OcPlatform.Configure(cfg);

        try {
            OcPlatform.startPresence(30);
            isExecutePresence = true;
        } catch (OcException e) {
            e.printStackTrace();
        }

        mySensor = new TemperatureResource(this);
        // create and register a resource
        mySensor.createResource();
    }

    /*
     * To execute initOICStack for running find resource
     */
    @Override
    protected void onStart() {
        super.onStart();
        initOICStack();

    }

    /*
     * To execute initOICStack for running find resource
     */
    @Override
    protected void onRestart() {
        super.onRestart();
        initOICStack();

    }

    /*
     * To terminate presence process and unregister messageHandler(to get
     * message from JNI Function)
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();

        try {
            mySensor.destroyResource();

            if (isExecutePresence == true) {
                OcPlatform.stopPresence();
                isExecutePresence = false;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
        LocalBroadcastManager.getInstance(this).unregisterReceiver(
                mMessageReceiver);
    }

    public class MessageReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String message = intent
                    .getStringExtra(StringConstants.MESSAGE);
            logMessage(message);

        }
    }

    public void logMessage(final String text) {
        runOnUiThread(new Runnable() {
            public void run() {
                final Message msg = new Message();
                msg.obj = text;
                mLogTextView.append("\n");
                mLogTextView.append(text);
            }
        });
        Log.i(TAG, text);
    }

    /*
     * To terminate presence process and unregister messagehandler(to get
     * message from JNI Function)
     */
    @Override
    protected void onStop() {
        super.onStop();
        try {
            mySensor.destroyResource();

            if (isExecutePresence == true) {
                OcPlatform.stopPresence();
                isExecutePresence = false;
            }

        } catch (OcException e) {

            e.printStackTrace();
        }
        LocalBroadcastManager.getInstance(this).unregisterReceiver(
                mMessageReceiver);
    }

    @Override
    public void onClick(View v) {
        int getId = v.getId();

        switch (getId) {
            case R.id.btnTemperatureUP:
                logMessage(TAG + "Click temerature up btn");
                mySensor.mtemp++;
                mySensor.notifyObserver();
                break;
            case R.id.btnTemperatureDown:
                logMessage(TAG + "Click temerature down btn");
                mySensor.mtemp--;
                mySensor.notifyObserver();
                break;
            case R.id.btnHumidityUP:
                logMessage(TAG + "Click Humidity up btn");
                mySensor.mhumidity++;
                mySensor.notifyObserver();
                break;
            case R.id.btnHumidityDown:
                logMessage(TAG + "Click Humidity down btn");
                mySensor.mhumidity--;
                mySensor.notifyObserver();
                break;
            case R.id.btnLogClear:
                mLogTextView.setText("");
                Log.i(TAG, "Log message cleared");
                break;
        }

        mTempValue.setText(String.valueOf(mySensor.getTemp()));
        mHumValue.setText(String.valueOf(mySensor.getHumidity()));

    }

    /*
     * To handle event about back button
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            try {
                mySensor.destroyResource();

                if (isExecutePresence == true) {
                    OcPlatform.stopPresence();
                    isExecutePresence = false;
                }
            } catch (OcException e) {

                e.printStackTrace();
            }
            LocalBroadcastManager.getInstance(this).unregisterReceiver(
                    mMessageReceiver);
        }
        return super.onKeyDown(keyCode, event);
    }

    public Handler getmHandler() {
        return mHandler;
    }

    public void setmHandler(Handler mHandler) {
        this.mHandler = mHandler;
    }

    public static SampleProvider getSampleProviderObject() {
        return sampleProviderObj;
    }

    public static void setmessage(String msg) {
        message = msg;
    }
}