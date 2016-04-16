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

#include <time.h>

#include "BrokerTypes.h"
#include "ResourceBroker.h"

namespace OIC
{
    namespace Service
    {
        ResourceBroker * ResourceBroker::s_instance = NULL;
        std::mutex ResourceBroker::s_mutexForCreation;
        std::unique_ptr<PresenceList>  ResourceBroker::s_presenceList(nullptr);
        std::unique_ptr<BrokerIDMap> ResourceBroker::s_brokerIDMap(nullptr);

        ResourceBroker::~ResourceBroker()
        {
            if(s_presenceList != nullptr)
            {
                OC_LOG_V(DEBUG, BROKER_TAG, "clear the ResourcePresenceList.");
                s_presenceList->erase(s_presenceList->begin(), s_presenceList->end());
                s_presenceList->clear();
            }
            if(s_brokerIDMap != nullptr)
            {
                OC_LOG_V(DEBUG, BROKER_TAG, "clear the brokerIDMap.");
                s_brokerIDMap->erase(s_brokerIDMap->begin(), s_brokerIDMap->end());
                s_brokerIDMap->clear();
            }
        }

        ResourceBroker * ResourceBroker::getInstance()
        {
            if (!s_instance)
            {
                s_mutexForCreation.lock();
                if (!s_instance)
                {
                    s_instance = new ResourceBroker();
                    s_instance->initializeResourceBroker();
                }
                s_mutexForCreation.unlock();
            }
            return s_instance;
        }

        BrokerID ResourceBroker::hostResource(PrimitiveResourcePtr pResource, BrokerCB cb)
        {
            OC_LOG_V(DEBUG, BROKER_TAG, "hostResource().");
            if(pResource == nullptr || cb == nullptr || cb == NULL)
            {
                throw InvalidParameterException("[hostResource] input parameter(PrimitiveResource or BrokerCB) is Invalid");
            }

            BrokerID retID = generateBrokerID();

            ResourcePresencePtr presenceItem = findResourcePresence(pResource);
            if(presenceItem == nullptr)
            {
                OC_LOG_V(DEBUG, BROKER_TAG, "Not found any Handled Resource.");
                OC_LOG_V(DEBUG, BROKER_TAG, "Create New Resource Presence Handler.");

                try
                {
                    OC_LOG_V(DEBUG, BROKER_TAG, "create the ResourcePresence.");
                    presenceItem.reset(new ResourcePresence());
                    presenceItem->initializeResourcePresence(pResource);
                }catch(RCSPlatformException &e)
                {
                    throw FailedSubscribePresenceException(e.getReasonCode());
                }
                if(s_presenceList != nullptr)
                {
                    OC_LOG_V(DEBUG, BROKER_TAG, "push the ResourcePresence in presenceList.");
                    s_presenceList->push_back(presenceItem);
                }
            }
            OC_LOG_V(DEBUG, BROKER_TAG, "add the BrokerRequester in ResourcePresence.");
            presenceItem->addBrokerRequester(retID, cb);

            BrokerCBResourcePair pair(presenceItem, cb);
            s_brokerIDMap->insert(std::pair<BrokerID, BrokerCBResourcePair>
                (retID, BrokerCBResourcePair(presenceItem, cb)));

            return retID;
        }

        void ResourceBroker::cancelHostResource(BrokerID brokerId)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"cancelHostResource().");
            if(brokerId == 0)
            {
                // input parameter is wrong.
                // hostResource never return value 0;
                OC_LOG_V(DEBUG,BROKER_TAG,"brokerId is zero.");
                throw InvalidParameterException("[cancelHostResource] brokerId is invalid.");
            }

            BrokerIDMap::iterator it = s_brokerIDMap->find(brokerId);
            if(it == s_brokerIDMap->end())
            {
                // not found requested brokerId in BrokerMap;
                OC_LOG_V(DEBUG,BROKER_TAG,"brokerId is not found in brokerIDMap.");
                throw InvalidParameterException("[cancelHostResource] brokerId is not found in brokerIDMap.");
            }
            else
            {
                ResourcePresencePtr presenceItem = it->second.pResource;
                presenceItem->removeBrokerRequester(brokerId);
                s_brokerIDMap->erase(brokerId);

                if(presenceItem->isEmptyRequester())
                {
                    OC_LOG_V(DEBUG,BROKER_TAG,"remove resourcePresence in presenceList because it is not including any requester info.");
                    s_presenceList->remove(presenceItem);
                }
            }
        }

        BROKER_STATE ResourceBroker::getResourceState(BrokerID brokerId)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"getResourceState().");
            if(brokerId == 0)
            {
                OC_LOG_V(DEBUG,BROKER_TAG,"brokerId is zero.");
                throw InvalidParameterException("[getResourceState] input BrokerID is Invalid");
            }

            BROKER_STATE retState = BROKER_STATE::NONE;

            BrokerIDMap::iterator it = s_brokerIDMap->find(brokerId);
            if(it == s_brokerIDMap->end())
            {
                // not found requested brokerId in BrokerMap;
                OC_LOG_V(DEBUG,BROKER_TAG,"brokerId is not found in brokerIDMap.");
                throw InvalidParameterException("[getResourceState] input BrokerID is unknown ID");
            }
            else
            {
                ResourcePresencePtr foundResource = it->second.pResource;
                retState = foundResource->getResourceState();
            }

            return retState;
        }

        BROKER_STATE ResourceBroker::getResourceState(PrimitiveResourcePtr pResource)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"getResourceState().");
            if(pResource == nullptr)
            {
                throw InvalidParameterException("[getResourceState] input PrimitiveResource is Invalid");
            }

            BROKER_STATE retState = BROKER_STATE::NONE;

            ResourcePresencePtr foundResource = findResourcePresence(pResource);
            if(foundResource != nullptr)
            {
                retState = foundResource->getResourceState();
            }

            return retState;
        }

        void ResourceBroker::initializeResourceBroker()
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"initializeResourceBroker().");
            if(s_presenceList == nullptr)
            {
                OC_LOG_V(DEBUG,BROKER_TAG,"create the presenceList.");
                s_presenceList = std::unique_ptr<PresenceList>(new PresenceList);
            }
            if(s_brokerIDMap == nullptr)
            {
                OC_LOG_V(DEBUG,BROKER_TAG,"create the brokerIDMap.");
                s_brokerIDMap = std::unique_ptr<BrokerIDMap>(new BrokerIDMap);
            }
        }

        ResourcePresencePtr ResourceBroker::findResourcePresence(PrimitiveResourcePtr pResource)
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"findResourcePresence().");
            ResourcePresencePtr retResource(nullptr);

            if(s_presenceList->empty() != true)
            {
                for(auto & it : * s_presenceList)
                {
                    PrimitiveResourcePtr temp = it->getPrimitiveResource();
                    if(temp == pResource)
                    {
                        retResource = it;
                        break;
                    }
                }
            }

            return retResource;
        }

        BrokerID ResourceBroker::generateBrokerID()
        {
            OC_LOG_V(DEBUG,BROKER_TAG,"generateBrokerID().");
            BrokerID retID = 0;
            srand(time(NULL));

            while(1)
            {
                if(retID != 0 && s_brokerIDMap->find(retID) == s_brokerIDMap->end())
                {
                    break;
                }
                retID = (unsigned int)rand();
            }

            return retID;
        }
    } // namespace Service
} // namespace OIC
