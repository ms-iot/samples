/* *****************************************************************
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

/**
 * @file
 * This file contains the declaration of ThingsMaintenanceCallbacks class.
 */

#ifndef JNI_THINGS_MAINTENANCE_CALLBACKS_H_
#define JNI_THINGS_MAINTENANCE_CALLBACKS_H_

#include <string>

#include "ThingsMaintenance.h"

/**
 * This class provides a set of callback functions for things maintenance.
 */
class ThingsMaintenanceCallbacks
{

    public:
        ThingsMaintenanceCallbacks() {}
        virtual ~ThingsMaintenanceCallbacks() {}

        /**
         * This callback method is called when a response for the reboot request
         * just arrives.
         *
         * @param headerOptions - It comprises of optionID and optionData as members.
         * @param rep           - Configuration parameters are carried as a pair of attribute key and value
         *                        in a form of OCRepresentation instance.
         * @param eCode         - error code.
         */
        static void onRebootResponse(const OC::HeaderOptions &headerOptions,
                                     const OC::OCRepresentation &rep,
                                     const int eCode);

        /**
         * This callback method is called when a response for the factoryReset request
         * just arrives.
         *
         * @param headerOptions - It comprises of optionID and optionData as members.
         * @param rep           - Configuration parameters are carried as a pair of attribute key and value
         *                        in a form of OCRepresentation instance.
         * @param eCode         - error code.
         */
        static void onFactoryResetResponse(const OC::HeaderOptions &headerOptions,
                                           const OC::OCRepresentation &rep,
                                           const int eCode);

        /**
         * This method invokes the Callback function with particular name and signature.
         *
         * @param headerOptions - It comprises of optionID and optionData as members.
         * @param rep           - Configuration parameters are carried as a pair of attribute key and value
         *                        in a form of OCRepresentation instance.
         * @param eCode         - error code.
         * @param callbackName  - callbackName to be invoked.
         * @param signature     - Signature of the callback method to be called.
         */
        static void invokeCallback(const OC::HeaderOptions &headerOptions,
                                   const OC::OCRepresentation &rep, const int eCode,
                                   const char *callbackName, const char *signature);
};
#endif //JNI_THINGS_MAINTENANCE_CALLBACKS_H_
