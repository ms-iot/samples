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
 */

#include <algorithm>
#include <thread>
#include <unistd.h>
#include <string.h>

#include "GroupManager.h"

#define PLAIN_DELIMITER "\""
#define ACTION_DELIMITER "*"
#define DESC_DELIMITER "|"
#define ATTR_DELIMITER "="

using namespace OC;
using namespace OIC;

std::map< std::vector< std::string >, CandidateCallback > candidateRequest;
std::map< std::vector< std::string >, CandidateCallback > candidateRequestForTimer;
std::map< std::string, std::map< std::string, std::shared_ptr< OCResource > > > rtForResourceList;
std::vector< std::string > allFoundResourceTypes;
std::mutex callbackLock;


template< typename T >
bool IsSubset(std::vector< T > full, std::vector< T > sub)
{
    std::sort(full.begin(), full.end());
    std::sort(sub.begin(), sub.end());
    return std::includes(full.begin(), full.end(), sub.begin(), sub.end());
}
std::vector< std::string > &str_split(const std::string &s, char delim,
        std::vector< std::string > &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector< std::string > str_split(const std::string &s, char delim)
{
    std::vector< std::string > elems;
    str_split(s, delim, elems);
    return elems;
}

void GroupManager::onFoundResource(std::shared_ptr< OCResource > resource, int waitsec)
{

    std::string resourceURI;
    std::string hostAddress;
    try
    {
        // Do some operations with resource object.
        if (resource)
        {

            std::cout << "DISCOVERED Resource:" << std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;

            hostAddress.append(resourceURI);

            for (auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;

                if (std::find(allFoundResourceTypes.begin(), allFoundResourceTypes.end(),
                        resourceTypes) == allFoundResourceTypes.end())
                {
                    allFoundResourceTypes.push_back(resourceTypes);
                }

                rtForResourceList[resourceTypes][hostAddress] = resource;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for (auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            if (waitsec == -1)
            {
                findPreparedRequest(candidateRequest);
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch (std::exception& e)
    {
        //log(e.what());
    }
}

GroupManager::GroupManager(void)
{
    ;
}

/**
 * Virtual destructor
 */
GroupManager::~GroupManager(void)
{
    candidateRequest.clear();
    candidateRequestForTimer.clear();
    rtForResourceList.clear();
    allFoundResourceTypes.clear();
}

void GroupManager::findPreparedRequest(
        std::map< std::vector< std::string >, CandidateCallback > &request)
{
    std::lock_guard<std::mutex> lock(callbackLock);
    std::vector< std::shared_ptr< OCResource > > resources;

    for (auto it = request.begin(); it != request.end();)
    {

        if (IsSubset(allFoundResourceTypes, it->first))
        {
            for (unsigned int i = 0; i < it->first.size(); ++i)
            {

                for (auto secondIt = rtForResourceList[it->first.at(i)].begin();
                        secondIt != rtForResourceList[it->first.at(i)].end(); ++secondIt)
                {
                    //insert resource related to request
                    resources.push_back(secondIt->second);
                }
            }

            it->second(resources);

            //TODO : decide policy - callback only once
            request.erase(it++);
        }
        else
        {
            ++it;
        }

    }

}

void GroupManager::lazyCallback(int second)
{
    sleep(second);
    findPreparedRequest(candidateRequestForTimer);

}

OCStackResult GroupManager::findCandidateResources(
        std::vector< std::string > resourceTypes,
        CandidateCallback callback, int waitsec)
{
    if (resourceTypes.size() < 1)
    {
        return OC_STACK_ERROR;
    }
    if(callback == NULL)
    {
        return OC_STACK_ERROR;
    }

    std::sort(resourceTypes.begin(), resourceTypes.end());
    resourceTypes.erase(std::unique(resourceTypes.begin(), resourceTypes.end()),
            resourceTypes.end());

    if (waitsec >= 0)
    {
        candidateRequestForTimer.insert(std::make_pair(resourceTypes, callback));
    }
    else
    {
        candidateRequest.insert(std::make_pair(resourceTypes, callback));
    }

    for (unsigned int i = 0; i < resourceTypes.size(); ++i)
    {
        // std::cout << "resourceTypes : " << resourceTypes.at(i) << std::endl;

        std::string query = OC_RSRVD_WELL_KNOWN_URI;
        query.append("?rt=");
        query.append(resourceTypes.at(i));

        OCPlatform::findResource("",
                query,
                CT_DEFAULT,
                std::function < void(std::shared_ptr < OCResource > resource)
                        > (std::bind(&GroupManager::onFoundResource, this, std::placeholders::_1,
                                waitsec)));
    }

    if (waitsec >= 0)
    {
        std::thread exec(
                std::function< void(int second) >(
                        std::bind(&GroupManager::lazyCallback, this, std::placeholders::_1)),
                waitsec);
        exec.detach();
    }

    return OC_STACK_OK;
}


OCStackResult GroupManager::bindResourceToGroup(OCResourceHandle& childHandle, std::shared_ptr< OCResource > resource, OCResourceHandle& collectionHandle)
{

    OCStackResult result = OCPlatform::registerResource(childHandle, resource);

    cout << "\tresource registed!" << endl;

    if(result == OC_STACK_OK)
    {
        OCPlatform::bindResource(collectionHandle, childHandle);
    }
    else
    {
        cout << "\tresource Error!" << endl;
    }

    return result;
 }



/*
 Presence Check
 */

std::map< std::string, CollectionPresenceCallback > presenceCallbacks;

// Callback to presence
void GroupManager::collectionPresenceHandler(OCStackResult result, const unsigned int nonce,
        const std::string& /*hostAddress*/, std::string host, std::string uri)
{
    std::cout << "uri : " << uri << std::endl;
    std::cout << "host : " << host << std::endl;
    std::cout << "result : " << result << std::endl;
    switch (result)
    {
        case OC_STACK_OK:
            std::cout << "Nonce# " << nonce << std::endl;
            break;
        case OC_STACK_PRESENCE_STOPPED:
            std::cout << "Presence Stopped\n";
            break;
        case OC_STACK_PRESENCE_DO_NOT_HANDLE:
            std::cout << "Presence do not handle\n";
            break;
        case OC_STACK_PRESENCE_TIMEOUT:
            std::cout << "Presence TIMEOUT\n";
            break;
        default:
            std::cout << "Error\n";
            break;
    }

    if (presenceCallbacks.find(uri) != presenceCallbacks.end())
    {
        (presenceCallbacks.find(uri)->second)(uri, result);
    }
}

void GroupManager::checkCollectionRepresentation(const OCRepresentation& rep,
        CollectionPresenceCallback callback)
{
    std::cout << "\tResource URI: " << rep.getUri() << std::endl;

    std::vector< OCRepresentation > children = rep.getChildren();
    if(children.size() == 0 )
    {
        callback("", OC_STACK_ERROR);
        return;
    }

    for (auto oit = children.begin(); oit != children.end(); ++oit)
    {
        if(oit->getUri().find("coap://") == std::string::npos)
        {
            std::cout << "The resource with a URI " << oit->getUri() << " is not a remote resource."
                << " Thus, we ignore to send a request for presence check to the resource.\n";

            continue;
        }

        std::vector< std::string > hostAddressVector = str_split(oit->getUri(), '/');
        std::string hostAddress = "";
        for (unsigned int i = 0; i < hostAddressVector.size(); ++i)
        {
            if (i < 3)
            {
                hostAddress.append(hostAddressVector.at(i));
                if (i != 2)
                {
                    hostAddress.append("/");
                }
            }
        }

        std::vector< std::string > resourceTypes = oit->getResourceTypes();
        // for (unsigned int i = 0; i < resourceTypes.size(); ++i)
        // {
        //     std::cout << "\t\t\tresourcetype :" << resourceTypes.at(i) << std::endl;
        // }

        // std::string resourceType = "core.";
        // resourceType.append(str_split(oit->getUri(), '/').at(4));
        // std::cout << "\t\tconvertRT : " << resourceType << std::endl;
        // std::cout << "\t\tresource type front : " << resourceTypes.front() << endl;
        // std::cout << "\t\thost : " << hostAddress << std::endl;
        OCPlatform::OCPresenceHandle presenceHandle;
        OCStackResult result = OC_STACK_ERROR;

        try
        {
            result = OCPlatform::subscribePresence(presenceHandle, hostAddress,
                    // resourceType,
                    resourceTypes.front(),
                    CT_DEFAULT,
                    std::function<
                            void(OCStackResult result, const unsigned int nonce,
                                    const std::string& hostAddress) >(
                            std::bind(&GroupManager::collectionPresenceHandler, this,
                                    std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3, hostAddress, oit->getUri())));
        }catch(OCException& e)
        {
            std::cout<< "Exception in subscribePresence: "<< e.what() << std::endl;
        }

        if (result == OC_STACK_OK)
        {
            presenceCallbacks.insert(std::make_pair(oit->getUri(), callback));
        }
        else
        {
            callback("", OC_STACK_ERROR);
        }
    }
}

void GroupManager::onGetForPresence(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode, CollectionPresenceCallback callback)
{
    if (eCode == OC_STACK_OK)
    {
        std::cout << "GET request was successful" << std::endl;
        std::cout << "Resource URI: " << rep.getUri() << std::endl;

        checkCollectionRepresentation(rep, callback);

    }
    else
    {
        std::cout << "onGET Response error: " << eCode << std::endl;
        callback("", OC_STACK_ERROR);
    }
}

OCStackResult GroupManager::subscribeCollectionPresence(
        std::shared_ptr< OCResource > collectionResource, CollectionPresenceCallback callback)
{
    if(callback == NULL || collectionResource == NULL)
    {
        return OC_STACK_ERROR;
    }

    OCStackResult result = OC_STACK_OK;
    //callback("core.room",OC_STACK_OK);

    QueryParamsMap queryParam;

    //parameter 1 = resourceType
    collectionResource->get("", DEFAULT_INTERFACE, queryParam,
            std::function<
                    void(const HeaderOptions& headerOptions, const OCRepresentation& rep,
                            const int eCode) >(
                    std::bind(&GroupManager::onGetForPresence, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3, callback)));

    return result;
}

/*
 Group Action
 */

std::string GroupManager::getStringFromActionSet(const ActionSet *newActionSet)
{
    std::string message = "";

    if(newActionSet == NULL)
        return message;

    message = newActionSet->actionsetName;
    message.append("*");
    message.append(newActionSet->toString());
    message.append("*");
    for (auto iterAction = newActionSet->listOfAction.begin();
            iterAction != newActionSet->listOfAction.end(); iterAction++)
    {
        message.append("uri=");
        message.append((*iterAction)->target);
        message.append("|");

        for (auto iterCapa = (*iterAction)->listOfCapability.begin();
                iterCapa != (*iterAction)->listOfCapability.end(); iterCapa++)
        {
            message.append((*iterCapa)->capability);
            message.append("=");
            message.append((*iterCapa)->status);

            if (iterCapa + 1 != (*iterAction)->listOfCapability.end())
                message.append("|");
        }

        if (iterAction + 1 != newActionSet->listOfAction.end())
        {
            message.append("*");
        }
    }

    return message;
}

#define DELETE(p) { \
    delete p; \
    p = NULL; \
}

#define DELETEARRAY(p) { \
    delete[] p; \
    p = NULL; \
}

ActionSet* GroupManager::getActionSetfromString(std::string description)
{
    char *token = NULL;
    char *plainText = NULL;
    char *plainPtr = NULL;
    char *attr = NULL, *desc = NULL;

    Action *action = NULL;
    Capability *capa = NULL;
    ActionSet *actionset = new ActionSet();

    if(actionset == NULL)
    {
        goto exit;
    }

    if(description.empty())
    {
        goto exit;
    }
    else if(description.at(0) == '*')
    {
        goto exit;
    }

    plainText = new char[(description.length() + 1)];
    strcpy(plainText, description.c_str());

    token = strtok_r(plainText, ACTION_DELIMITER, &plainPtr);

    if (token != NULL)
    {
        actionset->actionsetName = std::string(token);

        if((actionset->actionsetName).empty())
            goto exit;

        token = strtok_r(NULL, ACTION_DELIMITER, &plainPtr);
    }
    else
    {
        goto exit;
    }

    if (token != NULL)
    {
        sscanf(token, "%ld %d", &actionset->mDelay, (int*)&actionset->type);

        token = strtok_r(NULL, ACTION_DELIMITER, &plainPtr);
    }
    else
    {
        goto exit;
    }

    while (token)
    {
        char *descPtr = NULL;
        desc = new char[(strlen(token) + 1)];

        if (desc != NULL)
        {
            strcpy(desc, token);
            token = strtok_r(desc, DESC_DELIMITER, &descPtr);

            while (token != NULL)
            {
                char *attrPtr = NULL;
                attr = new char[(strlen(token) + 1)];

                strcpy(attr, token);

                token = strtok_r(attr, ATTR_DELIMITER, &attrPtr);
                while (token != NULL)
                {
                    if ( (action == NULL) && strcmp(token, "uri") == 0)    //consider only first "uri" as uri, other as attribute.
                    {
                        token = strtok_r(NULL, ATTR_DELIMITER, &attrPtr);
                        if(token == NULL)
                        {
                            goto exit;
                        }

                        action = new Action();
                        if (action != NULL)
                        {
                            action->target = std::string(token);
                        }
                        else
                        {
                            goto exit;
                        }
                    }
                    else
                    {
                        capa = new Capability();
                        capa->capability = std::string(token);
                        token = strtok_r(NULL, ATTR_DELIMITER, &attrPtr);

                        if( token == NULL )
                            goto exit;

                        capa->status = std::string(token);

                        if (action != NULL)
                            action->listOfCapability.push_back(capa);
                        else
                            goto exit;
                    }

                    token = strtok_r(NULL, ATTR_DELIMITER, &attrPtr);
                }
                DELETEARRAY(attr);
                token = strtok_r(NULL, DESC_DELIMITER, &descPtr);
            }

            if( action != NULL )
            {
                actionset->listOfAction.push_back(action);
                action = NULL;
            }
            else
                goto exit;
            //delete action;
        }
        else
        {
            goto exit;

        }

        DELETEARRAY(desc);

        token = strtok_r(NULL, ACTION_DELIMITER, &plainPtr);
    }

    DELETEARRAY(plainText);
    return actionset;

exit:
    DELETE(action);
    DELETE(capa)
    DELETE(actionset)
    DELETEARRAY(attr);
    DELETEARRAY(plainText);
    DELETEARRAY(desc);
    return NULL;
}

OCStackResult GroupManager::addActionSet(std::shared_ptr< OCResource > resource,
        const ActionSet* newActionSet, PutCallback cb)
{
    // BUILD message of ActionSet which it is included delimiter.
    if ((resource != NULL) && (newActionSet != NULL))
    {
        if(newActionSet->mDelay < 0)
        {
            return OC_STACK_INVALID_PARAM;
        }

        std::string message = getStringFromActionSet(newActionSet);

        OCRepresentation rep;
        rep.setValue("ActionSet", message);
        return resource->put(resource->getResourceTypes().front(), GROUP_INTERFACE, rep,
                QueryParamsMap(), cb);
    }
    else
    {
        return OC_STACK_ERROR;
    }
}

OCStackResult GroupManager::executeActionSet(std::shared_ptr< OCResource > resource,
        std::string actionsetName, PostCallback cb)
{
    if (resource != NULL)
    {
        OCRepresentation rep;

        rep.setValue("DoAction", actionsetName);
        return resource->post(resource->getResourceTypes().front(), GROUP_INTERFACE, rep,
                QueryParamsMap(), cb);
    }
    else
    {
        return OC_STACK_ERROR;
    }
}

OCStackResult GroupManager::executeActionSet(std::shared_ptr< OCResource > resource,
        std::string actionsetName, long int delay, PostCallback cb)
{
    if(delay <= 0 )
    {
        return OC_STACK_INVALID_PARAM;
    }
    if (resource != NULL)
    {
        std::string value = actionsetName;
        value.append("*");
        value.append(std::to_string(delay));

        OCRepresentation rep;
        rep.setValue("DoScheduledAction", value);
        return resource->post(resource->getResourceTypes().front(), GROUP_INTERFACE, rep,
                QueryParamsMap(), cb);
    }
    else
    {
        return OC_STACK_ERROR;
    }
}

OCStackResult GroupManager::cancelActionSet(std::shared_ptr< OCResource > resource,
        std::string actionsetName, PostCallback cb)
{
    if (resource != NULL)
    {
        OCRepresentation rep;

        rep.setValue("CancelAction", actionsetName);
        return resource->post(resource->getResourceTypes().front(), GROUP_INTERFACE, rep,
                QueryParamsMap(), cb);
    }
    else
    {
        return OC_STACK_ERROR;
    }
}

OCStackResult GroupManager::getActionSet(std::shared_ptr< OCResource > resource,
        std::string actionsetName, PostCallback cb)
{
    if (resource != NULL)
    {
        OCRepresentation rep;

        rep.setValue("GetActionSet", actionsetName);

        return resource->post(resource->getResourceTypes().front(), GROUP_INTERFACE, rep,
                QueryParamsMap(), cb);
    }
    else
    {
        return OC_STACK_ERROR;
    }
}

OCStackResult GroupManager::deleteActionSet(std::shared_ptr< OCResource > resource,
        std::string actionsetName, PutCallback cb)
{
    if (resource != NULL)
    {
        OCRepresentation rep;

        rep.setValue("DelActionSet", actionsetName);

        return resource->put(resource->getResourceTypes().front(), GROUP_INTERFACE, rep,
                QueryParamsMap(), cb);
    }
    else
    {
        return OC_STACK_ERROR;
    }
}
