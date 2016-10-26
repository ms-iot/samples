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
package com.example.resourcehostingsampleapp;

import java.lang.reflect.Method;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import org.iotivity.ResourceHosting.ResourceHosting;
import org.iotivity.base.ModeType;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;
import android.widget.Toast;

/**
 * To execute resource hosting function for android sample application .
 *
 * @author Copyright 2015 Samsung Electronics All Rights Reserved.
 * @see className class : ResourceHosting</br>
 *
 */

public class ResourceHostingSampleApp extends Activity implements
        OnClickListener {

    private String          TAG  = "ResourceHosting";
    private String          mIpAddress;
    private TextView        mLogTextView;
    private String          mLog = "";
    private ResourceHosting resourceHosting;

    /**
     * To initialize UI Function Setting.
     *
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : onCreate</br>
     */
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mLogTextView = (TextView) findViewById(R.id.txtLog);
        findViewById(R.id.btnStartHosting).setOnClickListener(this);
        findViewById(R.id.btnStopHosting).setOnClickListener(this);
        findViewById(R.id.btLogClear).setOnClickListener(this);

        PlatformConfig platformConfigObj;
        resourceHosting = new ResourceHosting();
        platformConfigObj = new PlatformConfig(this, ServiceType.IN_PROC,
                ModeType.CLIENT_SERVER, "0.0.0.0", 0, QualityOfService.LOW);

        Log.i(TAG, "Before Calling Configure of ocPlatform");
        OcPlatform.Configure(platformConfigObj);
        Log.i(TAG, "Configuration done Successfully");
    }

    /**
     * To execute initOICStack for running Resource hosting.
     *
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : onStart</br>
     */
    @Override
    protected void onStart() {
        super.onStart();
        initOICStack();
    }

    /**
     * To terminate Resource hosting.
     *
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : onStop</br>
     */
    @Override
    protected void onStop() {
        super.onStop();
        int result;
        result = resourceHosting.ResourceHostingTerminate();
        Log.d(TAG, "ResourceHostingTerminate : " + result);
    }

    protected void onResume() {
        super.onResume();
    }

    /**
     * To execute initOICStack for running Resource hosting.
     *
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : onRestart</br>
     */
    @Override
    protected void onRestart() {
        super.onRestart();
        initOICStack();
    }

    /**
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : onDestroy</br>
     */
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * get IpAddress and execute resourceHostingInit() method.
     *
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : initOICStack</br>
     */
    private void initOICStack() {
        try {
            mIpAddress = getIpAddress();
            int result = 0;
            result = resourceHosting.ResourceHostingInit(mIpAddress);
            Log.d(TAG, "ResourceHostingInit : " + result);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : getIpAddress</br>
     */
    private String getIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface
                    .getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = (NetworkInterface) en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf
                        .getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = (InetAddress) enumIpAddr
                            .nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        if (inetAddress instanceof Inet4Address)
                            return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * @see Class class :
     *      com_example_resourcehostingsampleapp_ResourceHosting</br>
     * @see Method method : onClick</br>
     * @param v
     *            view to choice
     */
    public void onClick(View v) {
        int getId = v.getId();

        switch (getId) {
            case R.id.btnStartHosting:
                try {
                    int result;
                    result = resourceHosting.OICCoordinatorStart();
                    Log.d(TAG, "OICCoordinatorStart : " + result);
                } catch (Exception e) {
                    Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT)
                            .show();
                }
                break;
            case R.id.btnStopHosting:
                int result;
                result = resourceHosting.OICCoordinatorStop();
                Log.d(TAG, "OICCoordinatorStop : " + result);
                break;
            case R.id.btLogClear:
            default:
                break;
        }
    }
}
