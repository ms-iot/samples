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

package org.iotivity.service.easysetup.mediator;

/**
 * This is an abstract class represents the device being provisioned into the network. The
 * device being enrolled or provisioned into the network is called Enrollee.
 * Application has to extend this class and provide implementation of abstract methods according
 * to the ON-BOARDING & PROVISION connectivity i.e. WiFi etc.
 */

public abstract class EnrolleeDevice {

    protected EnrolleeState mState;
    private EnrolleeSetupError mError;

    protected OnBoardingConnection mConnection;
    protected final ProvisioningConfig mProvConfig;
    protected final OnBoardingConfig mOnBoardingConfig;

    protected OnBoardingCallback mOnBoardingCallback;
    protected ProvisioningCallback mProvisioningCallback;

    /**
     * @param onBoardingConfig Contains details about the connectivity to be established between
     *                         the Enrollee device & Mediator device in order to perform
     *                         on-boarding
     * @param provConfig       Contains details about the network to which Enrollee device is
     *                         going to connect.
     */
    protected EnrolleeDevice(OnBoardingConfig onBoardingConfig, ProvisioningConfig provConfig) {
        mProvConfig = provConfig;
        mOnBoardingConfig = onBoardingConfig;
    }

    /**
     * Application has to implement it according to the on boarding connectivity the device is
     * having.
     * This method will be called back during the easy setup process.
     */
    protected abstract void startOnBoardingProcess();

    /**
     * This method is called back during the easy setup process if Application cancels the setup.
     * Easy setup service checks the state of device and calls this function accordingly.
     * Application has to provide implementation for this method to cancel the on boarding step.
     */
    protected abstract void stopOnBoardingProcess();

    /**
     * Application has to implement it according to the type of the network device is going to
     * connect or provisioned.
     * This method will be called back once on-boarding of the device is successful.
     *
     * @param conn Contains detail about the network established between the Enrollee device &
     *             Mediator device. Its implementation vary according to the connectivity type.
     */
    protected abstract void startProvisioningProcess(OnBoardingConnection conn);

    /**
     * Once on boarding is successful concrete Enrollee class would call this method and set the
     * Connection.
     *
     * @param conn Connectivity between Enrollee device & Mediator device.
     */
    protected void setConnection(OnBoardingConnection conn) {
        mConnection = conn;
    }

    /**
     * This method returns the OnBoardingConnection object depending on the connection type
     *
     * @return onBoardingConnection object
     */
    public OnBoardingConnection getConnection() {
        return mConnection;
    }


    /**
     * This method is called back by Easy setup service if on boarding needs to be done.
     *
     * @param onBoardingCallback This is called back once the on boarding is completed.
     */
    void startOnBoarding(OnBoardingCallback onBoardingCallback) {
        mOnBoardingCallback = onBoardingCallback;
        startOnBoardingProcess();
    }

    /**
     * This method is called back by Easy setup service once on boarding is successful
     *
     * @param provisioningCallback This is called back once the provisioning process is completed
     */
    void startProvisioning(ProvisioningCallback provisioningCallback) {
        mProvisioningCallback = provisioningCallback;
        startProvisioningProcess(mConnection);
    }

    /**
     * This method is used to check easy setup status
     *
     * @return true if successful or false
     */

    public boolean isSetupSuccessful() {
        return (mState == EnrolleeState.DEVICE_PROVISIONED_STATE) ? true : false;
    }

    /**
     * sets error occured during easy setup process
     */
    protected void setError(EnrolleeSetupError error) {
        mError = error;
    }

    /**
     * Returns error occured during easy setup process
     *
     * @return True EnrolleeSetupError object
     */
    public EnrolleeSetupError getError() {
        return mError;
    }

    /**
     * Gives the state of the device being enrolled during the easy setup process.
     *
     * @return Returns EnrolleeState object
     */
    public EnrolleeState getState() {
        return mState;
    }

    /**
     * This method is used to know if the device is on boarded or not
     *
     * @return True if on-boarded successfully or False
     */

    protected boolean onBoarded() {
        return (mState == EnrolleeState.DEVICE_PROVISIONING_STATE) ? true : false;
    }


}
