//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

/**
 * @file
 *
 * This file contains the declaration of classes and its members related to
 * GroupManager.
 */

#ifndef __OC_GROUPMANAGER__
#define __OC_GROUPMANAGER__

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ActionSet.h>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

namespace OIC
{
typedef std::function< void(std::vector< std::shared_ptr< OCResource > >) > CandidateCallback;
typedef std::function< void(std::string, OCStackResult) > CollectionPresenceCallback;

typedef std::function< void(const HeaderOptions&, const OCRepresentation&, const int) > GetCallback;
typedef std::function< void(const HeaderOptions&, const OCRepresentation&, const int) > PostCallback;
typedef std::function< void(const HeaderOptions&, const OCRepresentation&, const int) > PutCallback;

/**
 * @class GroupManager
 * @brief
 * This APIs provide functions for application to find appropriate devices (i.e. things) in network,
 * create a group of the devices, check a presence of member devices in the group, and actuate a
 * group action in a more convenient way.
 */
class GroupManager
{
public:
    /**
     * Constructor for GroupManager. Constructs a new GroupManager
     */
    GroupManager(void);

    /**
     * Virtual destructor
     */
    ~GroupManager(void);

    /**
     * API for candidate resources discovery.
     * Callback only call when all resource types found.
     *
     * @param resourceTypes required resource types(called "candidate")
     * @param callback callback with OCResource vector.
     * @param waitsec time to wait to finish finding resources
     *
     * @return OCStackResult return value of this API. Returns OC_STACK_OK if success.
     *
     * NOTE: OCStackResult is defined in ocstack.h.
     */
    OCStackResult findCandidateResources(std::vector< std::string > resourceTypes,
            CandidateCallback callback, int waitsec = -1);

    /**
     * API for Collection member's state subscribe.
     *
     * NOTE: NOT IMPLEMENT YET
     */
    OCStackResult subscribeCollectionPresence(std::shared_ptr< OCResource > resource,
            CollectionPresenceCallback);

    OCStackResult bindResourceToGroup(OCResourceHandle& childHandle,
            std::shared_ptr< OCResource > resource, OCResourceHandle& collectionHandle);

private:

    void onFoundResource(std::shared_ptr< OCResource > resource, int waitsec);
    void findPreparedRequest(std::map< std::vector< std::string >, CandidateCallback > &request);
    void lazyCallback(int second);

    void onGetForPresence(const HeaderOptions& headerOptions, const OCRepresentation& rep,
            const int eCode, CollectionPresenceCallback callback);
    void checkCollectionRepresentation(const OCRepresentation& rep,
            CollectionPresenceCallback callback);
    void collectionPresenceHandler(OCStackResult result, const unsigned int nonce,
            const std::string& hostAddress, std::string host, std::string uri);

    /**
     *   API for Collection(Group) action.
     */

public:
    /**
     * API for extracting an action set string from the ActionSet class instance
     *
     * @param newActionSet pointer of ActionSet class instance
     *
     * @return std::string return value of this API.
     *                     It returns an action set String.
     * @note OCStackResult is defined in ocstack.h.
     */
    std::string getStringFromActionSet(const ActionSet *newActionSet);

    /**
     * API for extracting ActionSet class instance from an action set string.
     *
     * @param desc description of an action set string
     *
     * @return ActionSet* return value of this API.
     *                      It returns pointer of ActionSet.
     */
    ActionSet* getActionSetfromString(std::string desc);

    /**
     * API for adding an action set.
     * Callback is called when the response of PUT operation arrives.
     *
     * @param resource resource pointer of the group resource
     * @param newActionSet pointer of ActionSet class instance
     * @param cb callback for PUT operation.
     *
     * @return Returns ::OC_STACK_OK if success.
     *
     * @note OCStackResult is defined in ocstack.h.
     */
    OCStackResult addActionSet(std::shared_ptr< OCResource > resource,
            const ActionSet* newActionSet, PutCallback cb);

    /**
     * API for executing an existing action set.
     * Callback is called when the response of  POST operation arrives.
     *
     * @param resource resource pointer of the group resource
     * @param actionsetName the action set name for executing the action set
     * @param cb callback for POST operation.
     *
     * @return Returns ::OC_STACK_OK if success.
     * @note OCStackResult is defined in ocstack.h.
     */
    OCStackResult executeActionSet(std::shared_ptr< OCResource > resource,
            std::string actionsetName, PostCallback cb);

    /**
     * API for executing an existing action set.
     * Callback is called when the response of  POST operation arrives.
     *
     * @param resource resource pointer of the group resource
     * @param actionsetName the action set name for executing the action set
     * @param delay waiting time for until the action set run.
     * @param cb callback for POST operation.
     *
     * @return Returns ::OC_STACK_OK if success.
     * @note OCStackResult is defined in ocstack.h.
     */
    OCStackResult executeActionSet(std::shared_ptr< OCResource > resource,
            std::string actionsetName, long int delay, PostCallback cb);

    /**
     * API for canceling an existing action set.
     * Callback is called when the response of POST operation arrives.
     *
     * @param resource resource pointer of the group resource
     * @param actionsetName the action set name for executing the action set
     * @param cb callback for POST operation.
     *
     * @return Returns ::OC_STACK_OK if success.
     * @note OCStackResult is defined in ocstack.h.
     */
    OCStackResult cancelActionSet(std::shared_ptr< OCResource > resource,
            std::string actionsetName, PostCallback cb);

    /**
     * API for reading an existing action set.
     * Callback is called when the response of GET operation arrives.
     *
     * @param resource resource pointer of the group resource
     * @param actionsetName the action set name for reading the action set
     * @param cb callback for GET operation.
     *
     * @return Returns ::OC_STACK_OK if success.
     * @note OCStackResult is defined in ocstack.h.
     */
    OCStackResult getActionSet(std::shared_ptr< OCResource > resource, std::string actionsetName,
            PostCallback cb);

    /**
     * API for removing an existing action set.
     * Callback is called when the response of  POST operation arrives.
     *
     * @param resource resource pointer of the group resource
     * @param actionsetName the action set name for removing the action set
     * @param cb callback for POST operation.
     *
     * @return Returns ::OC_STACK_OK if success.
     * @note OCStackResult is defined in ocstack.h.
     */
    OCStackResult deleteActionSet(std::shared_ptr< OCResource > resource, std::string actionsetName,
            PostCallback cb);
};
}
#endif  /* __OC_GROUPMANAGER__*/
