/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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
 */

package org.oic.simulator;

/**
 * This class contains remote device platform information and provide APIs
 * access it.
 */
public class PlatformInfo {
    private String mPlatformId;
    private String m_manufacturerName;
    private String m_manufacturerUrl;
    private String m_modelNumber;
    private String m_dateOfManufacture;
    private String m_platformVersion;
    private String m_operationSystemVersion;
    private String m_hardwareVersion;
    private String m_firmwareVersion;
    private String m_supportUrl;
    private String m_systemTime;

    /**
     * This method is for getting platform id.
     *
     * @return Platform id.
     */
    public String getPlatformID() {
        return mPlatformId;
    }

    /**
     * This method is for setting platform id.
     *
     * @param mPlatformId
     *            Platform identifier.
     */
    public void setPlatformID(String mPlatformId) {
        this.mPlatformId = mPlatformId;
    }

    /**
     * This method is for getting manufacturer name.
     *
     * @return Manufacturer name.
     */
    public String getManufacturerName() {
        return m_manufacturerName;
    }

    /**
     * This method is for setting manufacturer name.
     *
     * @param m_manufacturerName
     *            Manufacturer name.
     */
    public void setManufacturerName(String m_manufacturerName) {
        this.m_manufacturerName = m_manufacturerName;
    }

    /**
     * This method is for getting manufacturer URL.
     *
     * @return Manufacturer URL.
     */
    public String getManufacturerUrl() {
        return m_manufacturerUrl;
    }

    /**
     * This method is for setting manufacturer URL.
     *
     * @param m_manufacturerUrl
     *            Manufacturer URL.
     */
    public void setManufacturerUrl(String m_manufacturerUrl) {
        this.m_manufacturerUrl = m_manufacturerUrl;
    }

    /**
     * This method is for getting model number.
     *
     * @return Model number.
     */
    public String getModelNumber() {
        return m_modelNumber;
    }

    /**
     * This method is for setting model number.
     *
     * @param m_modelNumber
     *            Model number.
     */
    public void setModelNumber(String m_modelNumber) {
        this.m_modelNumber = m_modelNumber;
    }

    /**
     * This method is for getting date of manufacture.
     *
     * @return Date of manufacture.
     */
    public String getDateOfManufacture() {
        return m_dateOfManufacture;
    }

    /**
     * This method is for setting date of manufacture.
     *
     * @param m_dateOfManufacture
     *            Date of manufacture.
     */
    public void setDateOfManufacture(String m_dateOfManufacture) {
        this.m_dateOfManufacture = m_dateOfManufacture;
    }

    /**
     * This method is for getting platform version.
     *
     * @return Platform version.
     */
    public String getPlatformVersion() {
        return m_platformVersion;
    }

    /**
     * This method is for setting platform version.
     *
     * @param m_platformVersion
     *            Platform version.
     */
    public void setPlatformVersion(String m_platformVersion) {
        this.m_platformVersion = m_platformVersion;
    }

    /**
     * This method is for getting operating system version.
     *
     * @return Operation system version.
     */
    public String getOperationSystemVersion() {
        return m_operationSystemVersion;
    }

    /**
     * This method is for setting operating system version.
     *
     * @param m_operationSystemVersion
     *            Operation system version.
     */
    public void setOperationSystemVersion(String m_operationSystemVersion) {
        this.m_operationSystemVersion = m_operationSystemVersion;
    }

    /**
     * This method is for getting hardware version.
     *
     * @return Hardware version.
     */
    public String getHardwareVersion() {
        return m_hardwareVersion;
    }

    /**
     * This method is for setting hardware version.
     *
     * @param m_hardwareVersion
     *            Hardware version.
     */
    public void setHardwareVersion(String m_hardwareVersion) {
        this.m_hardwareVersion = m_hardwareVersion;
    }

    /**
     * This method is for getting firmware version.
     *
     * @return Firmware version.
     */
    public String getFirmwareVersion() {
        return m_firmwareVersion;
    }

    /**
     * This method is for setting firmware version.
     *
     * @param m_firmwareVersion
     *            Firmware version.
     */
    public void setFirmwareVersion(String m_firmwareVersion) {
        this.m_firmwareVersion = m_firmwareVersion;
    }

    /**
     * This method is for getting support link URL.
     *
     * @return URL of support link.
     */
    public String getSupportUrl() {
        return m_supportUrl;
    }

    /**
     * This method is for setting support link URL.
     *
     * @param m_supportUrl
     *            URL of support link.
     */
    public void setSupportUrl(String m_supportUrl) {
        this.m_supportUrl = m_supportUrl;
    }

    /**
     * This method is for getting system time.
     *
     * @return System time.
     */
    public String getSystemTime() {
        return m_systemTime;
    }

    /**
     * This method is for setting system time.
     *
     * @param m_systemTime
     *            System time.
     */
    public void setSystemTime(String m_systemTime) {
        this.m_systemTime = m_systemTime;
    }
}
