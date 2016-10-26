/**
 * ***************************************************************
 * <p>
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
 * <p>
 * ****************************************************************
 */

package org.iotivity.service.easysetup.core.ip;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.util.ArrayList;

import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.util.Log;

import org.iotivity.service.easysetup.core.EnrolleeInfo;
import org.iotivity.service.easysetup.core.EnrolleeOnBoardingInfo;
import org.iotivity.service.easysetup.core.IOnBoardingStatus;

/**
 * WiFiSoftAPManager provides wrapper methods for accessing Android Soft Access Point functionality.
 * This is a convenient class, providing Android "WiFiManager" kind of equivalent class for Soft AP.
 * <p>
 * Note: Android doesn't provide public APIs for Soft Access Point feature access.
 * This class provides only reference implementation to use the Soft AP and it uses Java reflection
 * for access Soft Access point features.
 * </p>
 */
public class WiFiSoftAPManager {
    private static final String TAG = WiFiSoftAPManager.class.getName();
    private final WifiManager mWifiManager;
    private Context context;
    static ArrayList<EnrolleeOnBoardingInfo> appNotification =
            new ArrayList<EnrolleeOnBoardingInfo>();
    IOnBoardingStatus finishListener = null;

    public enum WIFI_AP_STATE {
        WIFI_AP_STATE_DISABLING,
        WIFI_AP_STATE_DISABLED,
        WIFI_AP_STATE_ENABLING,
        WIFI_AP_STATE_ENABLED,
        WIFI_AP_STATE_FAILED
    }

    public WiFiSoftAPManager(Context context) {
        this.context = context;
        mWifiManager = (WifiManager) this.context
                .getSystemService(Context.WIFI_SERVICE);
    }

    /*
    * Utility API to check the validity of the MAC Address read from the ARP cache
    */
    private boolean CheckIfValidMacAddress(String macAddr) {
        if (macAddr.matches("..:..:..:..:..:..")) {
            return true;
        } else {
            return false;
        }
    }

    /*
    * The API is used for checking the device entry in the list maintained for the device
    * notifications.
    * If device entry is not found in the list, app is notified.
    * If the device entry is found in the device, as application is already notified it will
    * continue
    */
    private boolean CheckForDeviceEntryAndNotify(String ipAddr,
                                                 String macAddr, boolean isReachable) {
        final EnrolleeInfo result = new EnrolleeInfo();
        boolean deviceAddedToList = false;

        if (appNotification.size() > 0) {
            for (EnrolleeOnBoardingInfo ipDeviceOnBoardingNotification : appNotification) {
                EnrolleeOnBoardingInfo ipEnrolleeDevice = (EnrolleeOnBoardingInfo)
                        ipDeviceOnBoardingNotification;
                boolean macAddressComparison = ipEnrolleeDevice.getHWAddr().equalsIgnoreCase(
                        macAddr) ? true : false;

                if (macAddressComparison) {
                    deviceAddedToList = true;

                    if (ipDeviceOnBoardingNotification
                            .isAdditionNotified()
                            && isReachable) {
                        continue;
                    } else if (ipDeviceOnBoardingNotification
                            .isRemovalNotified()
                            && !isReachable) {
                        continue;
                    } else {
                        result.setIpAddr(ipAddr);
                        result.setHWAddr(macAddr);
                        result.setReachable(isReachable);

                        appNotification
                                .remove(ipDeviceOnBoardingNotification);
                        if (isReachable) {
                            appNotification
                                    .add(new EnrolleeOnBoardingInfo(ipAddr, macAddr, "",
                                            isReachable,
                                            false, true));
                        } else {
                            //This case will happen during the transition from connected to
                            // disconneted. This case need not be notified to application.
                            // Notifying this state will cause failure of OnBoarding
                        }
                        NotifyApplication(result);
                        return true;
                    }
                }
            }
            if (!deviceAddedToList) {
                if (isReachable) {
                    appNotification
                            .add(new EnrolleeOnBoardingInfo(ipAddr, macAddr, "", isReachable, false,
                                    true));

                    result.setIpAddr(ipAddr);
                    result.setHWAddr(macAddr);
                    result.setReachable(isReachable);

                    NotifyApplication(result);
                } else {
                    //This case will happen for the first time device is listed, but reachability
                    // is false. This case need not be notified to application. Notifying this
                    // state will cause failure of OnBoarding
                }
                return true;
            }
        } else {
            if (isReachable) {
                appNotification
                        .add(new EnrolleeOnBoardingInfo(ipAddr, macAddr, "", isReachable, false,
                                true));
                result.setIpAddr(ipAddr);
                result.setHWAddr(macAddr);
                result.setReachable(isReachable);

                NotifyApplication(result);
            } else {
                //This case will happen for the first time device is listed,  but
                // reachability is false. This case need not be notified to
                // application. Notifying this state will cause failure of OnBoarding
            }

            return true;
        }
        return false;
    }

    /**
     * Start WiFi Soft AccessPoint mode with the specified configuration.
     * If the Soft AP is already running, this API call will update the new configuration.
     * <p>
     * Note: Starting Wi-Fi Soft Access Point will disable the Wi-Fi normal operation.
     * </p>
     *
     * @param wifiConfig SSID, security and channel details as part of WifiConfiguration
     * @return {@code true} if the operation succeeds, {@code false} otherwise
     */
    public boolean setWifiApEnabled(WifiConfiguration wifiConfig,
                                    boolean enabled) {
        try {
            appNotification.clear();
            // Stopping Wi-Fi mode
            if (enabled) {
                mWifiManager.setWifiEnabled(false);
            }

            Method method = mWifiManager.getClass().getMethod(
                    "setWifiApEnabled", WifiConfiguration.class, boolean.class);
            return (Boolean) method.invoke(mWifiManager, wifiConfig, enabled);
        } catch (Exception e) {
            Log.e(this.getClass().toString(), "", e);
            return false;
        }
    }

    /**
     * Gets a list of the Soft AP clients connected to the Wi-Fi Soft Access point
     *
     * @param finishListener   Interface called when the scan method finishes
     * @param reachableTimeout Reachable Timeout in miliseconds
     */
    public synchronized void getClientList(IOnBoardingStatus finishListener, final int
            reachableTimeout) {
        this.finishListener = finishListener;
        //Clearing the scan list for providing u
        appNotification.clear();
        Runnable runnable = new Runnable() {
            public void run() {
                Log.i(TAG, "Scanning enrolling device in the network");

                BufferedReader bufferedReader = null;

                try {
                    // Note : This is a reference implementation for getting the list of Enrollee's
                    // connected to the Soft AP.
                    // There is no Android API for getting list of connected devices to the Soft AP.
                    // The connected device information is fetched from Arp cache for Soft AP and
                    // it is maintained in the file "/proc/net/arp"
                    // This holds an ASCII readable dump of the kernel ARP  table  used
                    // for  address resolutions.  It will show both dynamically learned
                    // and preprogrammed ARP entries.  The format is:
                    // IP address     HW type   Flags     HW address          Mask   Device
                    // 192.168.0.50   0x1       0x2       00:50:BF:25:68:F3   *      eth0
                    // 192.168.0.250  0x1       0xc       00:00:00:00:00:00   *      eth0
                    bufferedReader = new BufferedReader(new InputStreamReader(
                            new FileInputStream("/proc/net/arp"), "UTF-8"));
                    String line;
                    while ((line = bufferedReader.readLine()) != null) {
                        //ARP entries are splitted using Regex for getting the IP and MAC Address
                        // info
                        String[] arpEntry = line.split(" +");

                        if ((arpEntry != null) && (arpEntry.length >= 4)) {
                            String ipAddr = arpEntry[0];
                            String macAddr = arpEntry[3];


                            // Checking if the string is matching MAC Address is matching the
                            // standard MAC address format.
                            // If the entry is not matching with MAC address format,
                            // it will continue
                            if (CheckIfValidMacAddress(macAddr)) {
                                boolean isReachable = InetAddress.getByName(
                                        ipAddr).isReachable(
                                        reachableTimeout);

                                Log.i("exec statement", ipAddr);
                                Log.i("Return Value", " " + isReachable);

                                // Checking if the app notification entries are available in the
                                // list for the current device
                                // API returns true is there is a notification to the application.
                                // API returns false if there is no entry or if device is
                                // already notified
                                if (CheckForDeviceEntryAndNotify(ipAddr, macAddr, isReachable)) {
                                    break;
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    Log.e(this.getClass().toString(), e.toString());
                } finally {
                    try {
                        bufferedReader.close();
                    } catch (IOException e) {
                        Log.e(this.getClass().toString(), e.getMessage());
                    }
                }
            }
        };

        Thread mythread = new Thread(runnable);
        mythread.start();
    }

    void NotifyApplication(final EnrolleeInfo result) {
        // Get a handler that can be used to post to the main thread
/*
        Handler mainHandler = new Handler(context.getMainLooper());
        Runnable myRunnable = new Runnable() {
            @Override
            public void run() {
                finishListener.deviceOnBoardingStatus(result);
            }
        };
        mainHandler.post(myRunnable);
*/
        Log.i(TAG, "Scanning is finished with result, IP : " + result.getIpAddr() + "Notifying " +
                "to Application");
        finishListener.deviceOnBoardingStatus(result);

    }
}
