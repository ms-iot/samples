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

import android.content.Context;

/**
 * Data structure to provide the configuration.
 */
public class PlatformConfig {

    private Context mContext;
    private ServiceType mServiceType;
    private ModeType mModeType;
    private String mIpAddress;
    private int mPort;
    private QualityOfService mQualityOfService;
    private String mSvrDbPath; //TODO: Instead of SVRDB file, it should be Persistent Storage.
                              //this is only for 0.9.2

    /**
     * @param context          app context
     * @param serviceType      indicate IN_PROC or OUT_OF_PROC
     * @param modeType         indicate whether we want to do server, client or both
     * @param ipAddress        ip address of server
     *                         if you specify 0.0.0.0 : it listens on any interface
     * @param port             port of server
     *                         if you specifiy 0 : next available random port is used
     *                         if you specify 5683 : client discovery can work even if they don't
     *                         specify port
     * @param qualityOfService quality of service
     * @param dbPath           Persistant storage file for SVR Database.
     */
    public PlatformConfig(Context context,
                          ServiceType serviceType,
                          ModeType modeType,
                          String ipAddress,
                          int port,
                          QualityOfService qualityOfService,
                          String dbPath) {
        this.mContext = context;
        this.mServiceType = serviceType;
        this.mModeType = modeType;
        this.mIpAddress = ipAddress;
        this.mPort = port;
        this.mQualityOfService = qualityOfService;
        this.mSvrDbPath = dbPath;
    }

    /**
     * @param context          app context
     * @param serviceType      indicate IN_PROC or OUT_OF_PROC
     * @param modeType         indicate whether we want to do server, client or both
     * @param ipAddress        ip address of server
     *                         if you specify 0.0.0.0 : it listens on any interface
     * @param port             port of server
     *                         if you specifiy 0 : next available random port is used
     *                         if you specify 5683 : client discovery can work even if they don't
     *                         specify port
     * @param qualityOfService quality of service
     */
    //Avoid breaking building java samples due to persistent storage SVR DB changes.
    public PlatformConfig(Context context,
                          ServiceType serviceType,
                          ModeType modeType,
                          String ipAddress,
                          int port,
                          QualityOfService qualityOfService) {
        this(context,serviceType,modeType,ipAddress,port,qualityOfService, "");
    }

    public Context getContext() {
        return mContext;
    }

    public ServiceType getServiceType() {
        return mServiceType;
    }

    public ModeType getModeType() {
        return mModeType;
    }

    public String getIpAddress() {
        return mIpAddress;
    }

    public int getPort() {
        return mPort;
    }

    public QualityOfService getQualityOfService() {
        return mQualityOfService;
    }

    public String getSvrDbPath() {
        return mSvrDbPath;
    }
}
