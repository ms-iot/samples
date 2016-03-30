/**
 * ***************************************************************
 * <p/>
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 * <p/>
 * <p/>
 * <p/>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * <p/>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p/>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * <p/>
 * ****************************************************************
 */

package org.iotivity.service.easysetup;

import java.io.IOException;

import org.iotivity.service.easysetup.mediator.EasySetupService;
import org.iotivity.service.easysetup.mediator.EasySetupStatus;
import org.iotivity.service.easysetup.mediator.EnrolleeDevice;
import org.iotivity.service.easysetup.mediator.IpOnBoardingConnection;
import org.iotivity.service.easysetup.mediator.EnrolleeDeviceFactory;
import org.iotivity.service.easysetup.mediator.WiFiOnBoardingConfig;
import org.iotivity.service.easysetup.mediator.WiFiProvConfig;

import android.app.Activity;
import android.net.wifi.WifiConfiguration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {


    /*Status to update the UI */
    public static final int SUCCESS = 0;
    public static final int FAILED = 1;
    public static final int STATE_CHANGED = 2;

    EditText mSsidText;
    EditText mPassText;

    EditText mEnrolleeSsidText;
    EditText mmEnrolleePasswordPassText;


    TextView mDeviceIpTextView;
    TextView mDeviceMacTextView;


    TextView mResultTextView;
    ProgressBar mProgressbar;
    Button mStartButton;
    Button mStopButton;
    Handler mHandler = new ThreadHandler();

    /**
     * Objects to be instantiated by the programmer
     */
    WiFiProvConfig mWiFiProvConfig;
    WiFiOnBoardingConfig mWiFiOnBoardingConfig;
    EasySetupService mEasySetupService;
    EnrolleeDeviceFactory mDeviceFactory;
    EnrolleeDevice mDevice;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        /* Initialize widgets to get user input for target network's SSID & password*/
        mSsidText = (EditText) findViewById(R.id.ssid);
        mPassText = (EditText) findViewById(R.id.password);
        mEnrolleeSsidText = (EditText) findViewById(R.id.enrolleeSsid);
        mmEnrolleePasswordPassText = (EditText) findViewById(R.id.enrolleePass);
        mDeviceIpTextView = (TextView) findViewById(R.id.ipAddr);
        mDeviceMacTextView = (TextView) findViewById(R.id.hardAddr);

        mResultTextView = (TextView) findViewById(R.id.status);
        mProgressbar = (ProgressBar) findViewById(R.id.progressBar);




       /* Create Easy Setup Service instance*/
        mEasySetupService = EasySetupService.getInstance(getApplicationContext(),
                new EasySetupStatus() {

                    @Override
                    public void onFinished(final EnrolleeDevice enrolledevice) {
                        Log.i("MainActivity", "onFinished() is received " + enrolledevice
                                .isSetupSuccessful());
                        if (enrolledevice.isSetupSuccessful()) {
                            mHandler.sendEmptyMessage(SUCCESS);
                        } else {
                            mHandler.sendEmptyMessage(FAILED);
                        }
                    }

                    @Override
                    public void onProgress(EnrolleeDevice enrolleeDevice) {
                        Log.i("MainActivity", "onProgress() is received ");
                        mHandler.sendEmptyMessage(STATE_CHANGED);
                    }

                });

        /* Create EnrolleeDevice Factory instance*/
        mDeviceFactory = EnrolleeDeviceFactory.newInstance(getApplicationContext());

        /* Create a device using Factory instance*/
        mDevice = mDeviceFactory.newEnrolleeDevice(getOnBoardingWifiConfig(),
                getEnrollerWifiConfig());

        addListenerForStartAP();
        addListenerForStopAP();
    }

    public WiFiProvConfig getEnrollerWifiConfig() {
        /* Provide the credentials for the Mediator Soft AP to be connected by Enrollee*/
        mWiFiProvConfig = new WiFiProvConfig("hub2.4G", "11112222");
        mEnrolleeSsidText.setText("hub2.4G");
        mmEnrolleePasswordPassText.setText("11112222");

        return mWiFiProvConfig;
    }

    public WiFiOnBoardingConfig getOnBoardingWifiConfig() {
        mWiFiOnBoardingConfig = new WiFiOnBoardingConfig();

        /* Provide the target credentials to be provisioned to the Enrollee by Mediator*/
        mWiFiOnBoardingConfig.setSSId("EasySetup123");
        mWiFiOnBoardingConfig.setSharedKey("EasySetup123");
        mWiFiOnBoardingConfig.setAuthAlgo(WifiConfiguration.AuthAlgorithm.OPEN);
        mWiFiOnBoardingConfig.setKms(WifiConfiguration.KeyMgmt.WPA_PSK);

        // Updating the UI with default credentials
        mSsidText.setText("EasySetup123");
        mPassText.setText("EasySetup123");

        return mWiFiOnBoardingConfig;
    }


    public void onDestroy() {
        super.onDestroy();
        /*Reset the Easy setup process*/
        if (mEasySetupService != null) {
            mEasySetupService.finish();
        }
    }

    public void addListenerForStartAP() {
        mStartButton = (Button) findViewById(R.id.startSetup);

        mStartButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                try {

                    mProgressbar.setVisibility(View.VISIBLE);
                    mProgressbar.setIndeterminate(true);
                    mStartButton.setEnabled(false);
                    mResultTextView.setText(R.string.running);

                    //Reset Device information
                    mDeviceIpTextView.setText(R.string.not_available);
                    mDeviceMacTextView.setText(R.string.not_available);


                    String ssid = mSsidText.getText().toString();
                    String password = mPassText.getText().toString();

                    String enrolleeSsid = mEnrolleeSsidText.getText().toString();
                    String enrolleePassword = mmEnrolleePasswordPassText.getText().toString();

                    mWiFiProvConfig = new WiFiProvConfig(enrolleeSsid, enrolleePassword);

                    mWiFiOnBoardingConfig.setSSId(ssid);
                    mWiFiOnBoardingConfig.setSharedKey(password);


                    mEasySetupService.startSetup(mDevice);

                    mStopButton.setEnabled(true);


                } catch (IOException e) {
                    e.printStackTrace();
                }

            }
        });
    }

    public void addListenerForStopAP() {
        mStopButton = (Button) findViewById(R.id.stopSetup);

        mStopButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                mStartButton.setEnabled(true);
                mStopButton.setEnabled(false);
                mResultTextView.setText(R.string.stopped);
                mProgressbar.setIndeterminate(false);
                mProgressbar.setVisibility(View.INVISIBLE);
                mEasySetupService.stopSetup(mDevice);
            }
        });
    }


    class ThreadHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case SUCCESS: {

                    mProgressbar.setIndeterminate(false);
                    mStopButton.setEnabled(false);
                    mStartButton.setEnabled(true);
                    mProgressbar.setVisibility(View.INVISIBLE);
                    String resultMsg = "Device configured successfully";
                    mResultTextView.setText(R.string.success);

                    /* Update device information on the Ui */
                    IpOnBoardingConnection connection = (IpOnBoardingConnection) mDevice
                            .getConnection();
                    mDeviceIpTextView.setText(connection.getIp());
                    mDeviceMacTextView.setText(connection.getHardwareAddress());

                    Toast.makeText(getApplicationContext(), resultMsg, Toast.LENGTH_SHORT).show();
                    break;
                }
                case FAILED: {

                    mProgressbar.setIndeterminate(false);
                    mStopButton.setEnabled(false);
                    mStartButton.setEnabled(true);
                    mProgressbar.setVisibility(View.INVISIBLE);
                    String resultMsg = "Device configuration failed";
                    mResultTextView.setText(R.string.failed);
                    Toast.makeText(getApplicationContext(), resultMsg, Toast.LENGTH_SHORT).show();
                    break;
                }

                case STATE_CHANGED: {
                    String resultMsg = "Device state changed";
                    Toast.makeText(getApplicationContext(), resultMsg, Toast.LENGTH_SHORT).show();
                    break;
                }

            }


        }
    }

}
