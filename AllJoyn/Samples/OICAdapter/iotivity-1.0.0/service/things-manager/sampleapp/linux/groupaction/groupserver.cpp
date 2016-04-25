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

#include <OCPlatform.h>
#include <OCApi.h>

#include <functional>
#include <pthread.h>
#include <iostream>

#include "timer.h"

#include <GroupManager.h>

using namespace std;
using namespace OC;
using namespace OIC;
namespace PH = std::placeholders;

bool isReady = false;

OCResourceHandle resourceHandle;
std::vector<OCResourceHandle> resourceHandleVector;

shared_ptr<OCResource> g_resource;
vector<string> lights;

GroupManager *groupMgr = new GroupManager();

void onGet(const HeaderOptions& opt, const OCRepresentation &rep,
        const int eCode);

void onPut(const HeaderOptions& headerOptions, const OCRepresentation& rep,
        const int eCode);

void onPost(const HeaderOptions& headerOptions, const OCRepresentation& rep,
        const int eCode);

void onObserve(const HeaderOptions headerOptions, const OCRepresentation& rep,
        const int& eCode, const int& sequenceNumber);

void allBulbOn();
void allBulbOff();

shared_ptr<OCResource> g_light;

void foundResources(
        std::vector<std::shared_ptr<OC::OCResource> > listOfResource)
{

    for (auto rsrc = listOfResource.begin(); rsrc != listOfResource.end();
            ++rsrc)
    {
        std::string resourceURI = (*rsrc)->uri();
        std::string hostAddress = (*rsrc)->host();

        if (resourceURI == "/a/light")
        {
            cout << "\tResource URI : " << resourceURI << endl;
            cout << "\tResource Host : " << hostAddress << endl;

            OCResourceHandle foundResourceHandle;
            OCStackResult result = OCPlatform::registerResource(
                    foundResourceHandle, (*rsrc));
            cout << "\tresource registed!" << endl;
            if (result == OC_STACK_OK)
            {
                OCPlatform::bindResource(resourceHandle, foundResourceHandle);
                resourceHandleVector.push_back(foundResourceHandle);
            }
            else
            {
                cout << "\tresource Error!" << endl;
            }

            lights.push_back((hostAddress + resourceURI));

            g_light = (*rsrc);
        }
    }

    isReady = true;
}

void foundResource(std::shared_ptr<OCResource> resource)
{
    std::string resourceURI;
    std::string hostAddress;

    cout << "FOUND RESOURCE" << endl;

    try
    {
        if (resource)
        {
            resourceURI = resource->uri();
            hostAddress = resource->host();
            if (resourceURI == "/core/a/collection")
            {
                g_resource = resource;

                // g_resource->get("", DEFAULT_INTERFACE, QueryParamsMap(), onGet);

                cout << "FOUND " << resourceURI << endl;
                // printf("\tHOST :: %s\n", resource->host().c_str());
            }
            else if (resourceURI == "/core/bookmark")
            {
                resource->observe(ObserveType::Observe, QueryParamsMap(),
                        &onObserve);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

void onGet(const HeaderOptions& /*opt*/, const OCRepresentation &/*rep*/,
        const int eCode)
{
    cout << "\nonGet" << endl;
    if (eCode == OC_STACK_OK)
        cout << "\tResult is OK." << endl;
    else
        cout << "\tInvalid parameter." << endl;
}

void onPut(const HeaderOptions& /*opt*/, const OCRepresentation &/*rep*/,
        const int eCode)
{
    cout << "\nonPut" << endl;
    if (eCode == OC_STACK_OK)
        cout << "\tResult is OK." << endl;
    else
        cout << "\tInvalid parameter." << endl;
}

void onPost(const HeaderOptions& /*opt*/, const OCRepresentation &rep,
        const int /*eCode*/)
{
    printf("\nonPost\n");

    if (rep.hasAttribute("ActionSet"))
    {
        std::string plainText;

        if (rep.getValue("ActionSet", plainText))
        {
            ActionSet *actionset = groupMgr->getActionSetfromString(plainText);
            if (actionset != NULL)
            {
                cout << endl << "\tACTIONSET NAME :: "
                        << actionset->actionsetName << endl;
                for (auto actIter = actionset->listOfAction.begin();
                        actIter != actionset->listOfAction.end(); ++actIter)
                {
                    cout << "\t\tTARGET :: " << (*actIter)->target << endl;

                    for (auto capaIter = (*actIter)->listOfCapability.begin();
                            capaIter != (*actIter)->listOfCapability.end();
                            ++capaIter)
                    {
                        cout << "\t\t\t" << (*capaIter)->capability << " :: "
                                << (*capaIter)->status << endl;
                    }
                }
            }
            delete actionset;
        }

        // printf( "\tPlain Text :: %s\n", plainText.c_str() );
    }
    else if (rep.hasAttribute("DoAction"))
    {
        std::string plainText;
        if (rep.getValue("DoAction", plainText))
        {
            cout << "\t" << plainText << endl;
        }
    }
    else
    {

    }
}

void allBulbOff()
{
    OCRepresentation rep;

    rep.setValue("DoAction", std::string("AllBulbOff"));

    if (g_resource)
    {
        g_resource->post("a.collection", GROUP_INTERFACE, rep, QueryParamsMap(),
                &onPost);
    }
}

void allBulbOn()
{
    OCRepresentation rep;

    rep.setValue("DoAction", std::string("AllBulbOn"));

    if (g_resource)
    {
        OCStackResult res = g_resource->post("a.collection", GROUP_INTERFACE,
            rep, QueryParamsMap(), &onPost);

        if( res != OC_STACK_OK )
            cout << "failed" << endl;
    }
}

void Scheduled_AllbulbOff()
{
    groupMgr->executeActionSet(g_resource, "AllBulbOffScheduledCall", &onPost);
}
void Scheduled_AllbulbOffEx()
{
    groupMgr->executeActionSet(g_resource, "AllBulbOffScheduledCall", 10, &onPost);
}
void CancelScheduled_AllBulbOff()
{
    groupMgr->cancelActionSet(g_resource, "AllBulbOffScheduledCall", &onPost);
}
void Recursive_allBulbOn()
{
    groupMgr->executeActionSet(g_resource, "AllBulbOnRecursiveCall", &onPost);
}
void Recursive_allBulbOnEx()
{
    groupMgr->executeActionSet(g_resource, "AllBulbOnRecursiveCall", 10, &onPost);
}

void CancelRecursive_allBulbOn()
{

    groupMgr->cancelActionSet(g_resource, "AllBulbOnRecursiveCall", &onPost);
}

void onObserve(const HeaderOptions /*headerOptions*/, const OCRepresentation& rep,
        const int& eCode, const int& sequenceNumber)
{
    if (eCode == OC_STACK_OK)
    {
        int level;

        std::cout << "OBSERVE RESULT:" << std::endl;
        std::cout << "\tSequenceNumber: " << sequenceNumber << endl;

        if (rep.getValue("level", level))
        {
            if (level == 0)
            {
                allBulbOn();
            }
            else
            {
                allBulbOff();
            }
        }
        std::cout << "\tlevel: " << level << std::endl;
    }
    else
    {
        std::cout << "onObserve Response error: " << eCode << std::endl;
        std::exit(-1);
    }
}

void createActionSet_AllBulbOff()
{
    string actionsetDesc;
    ActionSet *allBulbOff = new ActionSet();
    allBulbOff->actionsetName = "AllBulbOff";

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbOff->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        groupMgr->addActionSet(g_resource, allBulbOff, onPut);
    }

    delete allBulbOff;
}

void createActionSet_AllBulbOn()
{
    string actionsetDesc;
    ActionSet *allBulbOff = new ActionSet();
    allBulbOff->actionsetName = "AllBulbOn";

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "on";

        action->listOfCapability.push_back(capa);
        allBulbOff->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        groupMgr->addActionSet(g_resource, allBulbOff, onPut);
    }

    delete allBulbOff;
}

void createScheduledActionSet_AllBulbOff()
{
    string actionsetDesc;
    ActionSet *allBulbOff = new ActionSet();
    allBulbOff->type = OIC::ACTIONSET_TYPE::SCHEDULED;
    allBulbOff->actionsetName = "AllBulbOffScheduledCall";

    printf("ENTER(YYYY-MM-DD hh:mm:ss) :: ");
    int res = scanf("%d-%d-%d %d:%d:%d", &allBulbOff->mTime.tm_year,
            &allBulbOff->mTime.tm_mon, &allBulbOff->mTime.tm_mday,
            &allBulbOff->mTime.tm_hour, &allBulbOff->mTime.tm_min,
            &allBulbOff->mTime.tm_sec);
    if( res < 0 )
    {
        printf("Invalid Input. try again.");
        delete allBulbOff;
        return;
    }

    allBulbOff->setDelay(allBulbOff->getSecondsFromAbsoluteTime());
    printf("DELAY :: %ld\n", allBulbOff->getSecondsFromAbsoluteTime());

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "off";

        action->listOfCapability.push_back(capa);
        allBulbOff->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        groupMgr->addActionSet(g_resource, allBulbOff, onPut);
    }

    delete allBulbOff;
}

void createRecursiveActionSet_AllBulbOn()
{
    string actionsetDesc;
    ActionSet *allBulbOn = new ActionSet();
    allBulbOn->type = OIC::ACTIONSET_TYPE::RECURSIVE;

    allBulbOn->actionsetName = "AllBulbOnRecursiveCall";
    allBulbOn->mTime.tm_year = 0;
    allBulbOn->mTime.tm_mon = 0;
    allBulbOn->mTime.tm_mday = 0;
    allBulbOn->mTime.tm_hour = 0;
    allBulbOn->mTime.tm_min = 0;
    allBulbOn->mTime.tm_sec = 5;

    allBulbOn->setDelay(allBulbOn->getSecAbsTime());

    for (auto iter = lights.begin(); iter != lights.end(); ++iter)
    {
        Action *action = new Action();
        action->target = (*iter);

        Capability *capa = new Capability();
        capa->capability = "power";
        capa->status = "on";

        action->listOfCapability.push_back(capa);
        allBulbOn->listOfAction.push_back(action);
    }
    if (g_resource)
    {
        groupMgr->addActionSet(g_resource, allBulbOn, onPut);
    }

    delete allBulbOn;
}

int main()
{
    PlatformConfig config
    { OC::ServiceType::InProc, ModeType::Both, "0.0.0.0", 0,
            OC::QualityOfService::LowQos };

    try
    {
        string resourceURI = "/core/a/collection";
        string resourceTypeName = "a.collection";
        string resourceInterface = BATCH_INTERFACE;
        OCPlatform::Configure(config);

        // Find lights for group creation.
        vector<string> types;
        types.push_back("core.light");
        groupMgr->findCandidateResources(types, &foundResources, 5);

        OCStackResult res = OCPlatform::registerResource(resourceHandle, resourceURI,
                resourceTypeName, resourceInterface, NULL, OC_DISCOVERABLE);

        if( res != OC_STACK_OK )
        {
            cout << "Resource registeration failed." << endl;
            return 0;
        }

        cout << "registerResource is called." << endl;

        OCPlatform::bindInterfaceToResource(resourceHandle, GROUP_INTERFACE);
        OCPlatform::bindInterfaceToResource(resourceHandle, DEFAULT_INTERFACE);

        bool isRun = true;

        while (isRun)
        {
            while (isReady)
            {
                int n;

                cout << endl;
                cout << "1 :: CREATE ACTIONSET" << endl;
                cout << "2 :: EXECUTE ACTIONSET(ALLBULBON)" << endl;
                cout << "3 :: EXECUTE ACTIONSET(ALLBULBOFF)" << endl;
                cout << "4 :: CREATE ACTIONSET(R_ALLBULBON)" << endl;
                cout << "\t41 :: EXECUTE ACTIONSET 42 :: CANCEL ACTIONSET" << endl;
                cout << "5 :: CREATE ACTIONSET(S_ALLBULBON)" << endl;
                cout << "\t51 :: EXECUTE ACTIONSET 52 :: CANCEL ACTIONSET" << endl;
                cout << "6 :: GET ACTIONSET" << endl;
                cout << "7 :: DELETE ACTIONSET" << endl;
                cout << "8 :: QUIT" << endl;
                cout << "9 :: FIND GROUP" << endl;
                cout << "0 :: FIND BOOKMARK TO OBSERVE"
                        << endl;

                //fflush(stdin);
                cin >> n;

                if (n == 9)
                {
                    std::string query = OC_RSRVD_WELL_KNOWN_URI;
                    query.append("?rt=");
                    query.append(resourceTypeName);

                    OCPlatform::findResource("",
                            query,
                            CT_DEFAULT,
                            &foundResource);

                    // OCPlatform::findResource("",
                    //         query,
                    //         OC_WIFI,
                    //         &foundResource);
                }
                else if (n == 0)
                {
                    std::string query = OC_RSRVD_WELL_KNOWN_URI;
                    query.append("?rt=");
                    query.append("core.bookmark");

                    OCPlatform::findResource("",
                            query,
                            CT_DEFAULT,
                            &foundResource);
                    // OCPlatform::findResource("",
                    //         query,
                    //         OC_WIFI,
                    //         &foundResource);
                }
                else if (n == 1)
                {
                    createActionSet_AllBulbOff();
                    createActionSet_AllBulbOn();
                }
                else if (n == 2)
                {
                    allBulbOn();
                }
                else if (n == 3)
                {
                    allBulbOff();
                }
                else if (n == 4)
                {
                    createRecursiveActionSet_AllBulbOn();
                }
                else if (n == 41)
                {
                    Recursive_allBulbOn();
                }
                else if (n == 42)
                {
                    CancelRecursive_allBulbOn();
                }
                // Exampel of
                else if (n == 43)
                {
                    Recursive_allBulbOnEx();
                }
                else if (n == 5)
                {
                    createScheduledActionSet_AllBulbOff();
                }
                else if (n == 51)
                {
                    Scheduled_AllbulbOff();
                }
                else if (n == 52)
                {
                    CancelScheduled_AllBulbOff();
                }
                else if (n == 53)
                {
                    Scheduled_AllbulbOffEx();
                }
                else if (n == 6)
                {
                    groupMgr->getActionSet(g_resource, "AllBulbOff", onPost);
                }
                else if (n == 7)
                {
                    groupMgr->deleteActionSet(g_resource, "AllBulbOff", onPut);
                }
                else if (n == 8)
                {
                    isRun = false;
                    break;
                }
                else if(n == 100)
                {

                    OCRepresentation rep;

                    rep.setValue("power", std::string("on"));

                    g_light->put(rep, QueryParamsMap(), &onPut);

                }
            }
        }
        usleep(500*1000);
    }
    catch (OCException& e)
    {
        cout << "ERROR :: " << e.reason() << endl;
    }

    return 0;
}
