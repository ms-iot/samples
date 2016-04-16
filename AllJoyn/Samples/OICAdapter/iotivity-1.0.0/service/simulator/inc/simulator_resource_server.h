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

/**
 * @file   simulator_resource_server.h
 *
 * @brief   This file contains a class which represents a simulator resource that provides a set
 *             of functions for operating a resource and performing automation on attribute values.
 */

#ifndef SIMULATOR_RESOURCE_SERVER_H_
#define SIMULATOR_RESOURCE_SERVER_H_

#include "simulator_server_types.h"
#include "simulator_resource_model.h"
#include "simulator_exceptions.h"

enum class ObservationStatus : unsigned char
{
    OBSERVE_REGISTER,
    OBSERVE_UNREGISTER
};

typedef struct
{
    uint8_t id;
    std::string address;
    uint16_t port;
} ObserverInfo;

/**
 * @class   SimulatorResourceServer
 * @brief   This class provides a set of functions for operating and automating a resource.
 */
class SimulatorResourceServer
{
    public:
        /**
         * Callback method for receiving notifications when resource model gets changed.
         *
         * @param uri - Resource URI
         * @param resModel - Resource model
         */
        typedef std::function<void (const std::string &uri, const SimulatorResourceModel &resModel)>
        ResourceModelChangedCB;

        /**
         * Callback method for receiving notifications when observer is registered/unregistered
         * with resource.
         *
         * @param uri - Resource URI
         * @param state - OBSERVE_REGISTER if observer is registered, otherwise OBSERVE_UNREGISTER.
         * @param observerInfo - Information about observer.
         */
        typedef std::function<void (const std::string &uri, ObservationStatus state, const ObserverInfo &observerInfo)>
        ObserverCB;

        SimulatorResourceServer();

        virtual ~SimulatorResourceServer() {};

        /**
         * API to get the resource URI.
         *
         * @return Resource URI
         */
        std::string getURI() const;

        /**
         * API to get the resource URI.
         *
         * @return Resource Type
         */
        std::string getResourceType() const;

        /**
         * API to get the interface type of the resource.
         *
         * @return Interface type of the resource
         */
        std::string getInterfaceType() const;

        /**
         * API to get the name of the resource.
         *
         * @return Resource name
         */
        std::string getName() const;

        /**
         * API to add a new attribute to the resource model.
         *
         * @param attribute - Attribute to be add to model.
         */
        void addAttribute(SimulatorResourceModel::Attribute &attribute);

        /**
         * API to set the value range of an attribute.
         * This method is intended to be used for attributes whose values are numbers only.
         *
         * @param attrName - Name of the attribute
         * @param min - Minimum value of the range
         * @param max - Maximum value of the range
         */
        void setRange(const std::string &attrName, const int min, const int max);

        /**
         * API to set the allowed values of an attribute.
         *
         * @param attrName - Name of the attribute
         * @param values - Allowed values
         */
        template <typename T>
        void setAllowedValues(const std::string &attrName, const std::vector<T> &values)
        {
            m_resModel.setAllowedValues(attrName, values);
        }

        /**
         * API to set the update interval time for automation.
         *
         * @param attrName - Name of the attribute
         * @param interval - Interval time in miliseconds for attribute value update automation
         */
        void setUpdateInterval(const std::string &attrName, int interval);

        /**
         * API to update the value of an attribute.
         *
         * @param attrName - Name of the attribute
         * @param value - Value of the attribute
         */
        template <typename T>
        void updateAttributeValue(const std::string &attrName, const T &value)
        {
            m_resModel.updateAttribute(attrName, value);

            // Notify all the subscribers
            notifyAll();
        }

        /**
         * API to update the attribute's value by taking the index of the value
         * in the allowed values range.
         *
         * @param attrName - Name of the attribute
         * @param allowedValueIndex - Index of the value in the allowed values range
         */
        void updateFromAllowedValues(const std::string &attrName, unsigned int index);

        /**
          * API to remove an attribute from the resource model.
          *
          * @param attName - Name of the attribute to be removed
          */
        void removeAttribute(const std::string &attName);

        /**
         * API to get the object of SimulatorResourceModel.
         * Attributes of the resource are accessed using this object.
         *
         * @return Resource model of the resource.
         */
        SimulatorResourceModel getModel() const;

        /**
         * API to get the observable state of resource.
         *
         * @return bool - true if resource is observable, otherwise false.
         */
        virtual bool isObservable() const = 0;

        /**
         * API to start the attribute value automation for all attributes.
         * Once started, values for the attributes will be selected randomly from their allowed range
         * and the updated values will be notified to all the observers of the resource.
         *
         * @param type - Automation type.
         * @param callback - Callback to get notifiy when update automation is finished.
         * @param id - Identifier for automation.
         *
         * @return ID representing update automation session.
         * NOTE: API throws @InvalidArgsException, @SimulatorException exceptions.
         */
        virtual int startUpdateAutomation(AutomationType type,
                                          updateCompleteCallback callback) = 0;

        /**
         * This method is used to start the attribute value automation for a specific attribute.
         * Once started, values for the attribute will be selected randomly from its allowed range
         * and the updated value will be notified to all the observers of the resource.
         *
         * @param attrName - Name of the attribute to be automated.
         * @param type - Automation type.
         * @param callback - Callback to get notifiy when update automation is finished.
         * @param id - Identifier for automation.
         *
         * @return ID representing update automation session.
         * NOTE: API throws @InvalidArgsException, @SimulatorException exceptions.
         */
        virtual int startUpdateAutomation(const std::string &attrName, AutomationType type,
                                          updateCompleteCallback callback) = 0;

        /**
         * API to get the Ids of all ongoing resource update automation .
         *
         * @return vector of resource automation ids.
         */
        virtual std::vector<int> getResourceAutomationIds() = 0;

        /**
         * API to get the Ids of all ongoing attribute update automation .
         *
         * @return vector of attribute automation ids.
         */
        virtual std::vector<int> getAttributeAutomationIds() = 0;

        /**
        * API to stop the resource/attribute automation.
        *
        * @param id - Identifier for automation.
        */
        virtual void stopUpdateAutomation(const int id) = 0;

        /**
         * API to set the callback for receiving the notifications when the
         * resource model changes.
         *
         * @param callback - Callback to be set for receiving the notifications.
         */
        virtual void setModelChangeCallback(ResourceModelChangedCB callback) = 0;

        /**
         * API to set the callback for receiving the notifications when
         * observer is registered or unregistered with resource.
         *
         * @param callback - Callback to be set for receiving the notifications.
         */
        virtual void setObserverCallback(ObserverCB callback) = 0;

        /**
         * API to get observers which are registered with resource.
         *
         * @return vector of ObserverInfo.
         */
        virtual std::vector<ObserverInfo> getObserversList() = 0;

        /**
         * API to notify current resource model to specific observer.
         *
         * NOTE: API throws @SimulatorException exception.
         */
        virtual void notify(uint8_t id) = 0;

        /**
         * API to notify all registered observers.
         *
         * NOTE: API throws @SimulatorException exception.
         */
        virtual void notifyAll() = 0;

    protected:
        std::string m_name;
        std::string m_uri;
        std::string m_resourceType;
        std::string m_interfaceType;
        SimulatorResourceModel m_resModel;
};

typedef std::shared_ptr<SimulatorResourceServer> SimulatorResourceServerSP;

#endif
