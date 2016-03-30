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

#if defined(__linux__)
#include <unistd.h>
#endif

#include "RCSResourceContainer.h"
#include "RCSBundleInfo.h"
#include "oc_logger.hpp"
#include <iostream>

using namespace std;
using namespace OIC::Service;
using OC::oc_log_stream;

#define MAX_PATH 2048

/* Annother way to create a context: */
auto info_logger = []() -> boost::iostreams::stream<OC::oc_log_stream> &
{
    static OC::oc_log_stream ols(oc_make_ostream_logger);
    static boost::iostreams::stream<OC::oc_log_stream> os(ols);

    return os;
};

void getCurrentPath(std::string *pPath)
{
    char buffer[MAX_PATH];

#if defined(__linux__)
    char *strPath = NULL;
    int length = readlink("/proc/self/exe", buffer, MAX_PATH - 1);

    if (length != -1)
    {
        buffer[length] = '\0';
        strPath = strrchr(buffer, '/');

        if (strPath != NULL)
            *strPath = '\0';
    }
#endif
    pPath->append(buffer);
}

int main()
{
    info_logger()->set_module("ContainerTest");
    info_logger()->set_level(OC_LOG_INFO);

    info_logger() << "Starting container test." << std::flush;

    std::string strConfigPath;
    getCurrentPath(&strConfigPath);
    strConfigPath.append("/examples/ResourceContainerConfig.xml");

    RCSResourceContainer *container = RCSResourceContainer::getInstance();
    container->startContainer(strConfigPath);

    /*so bundle add test*/
    cout << "++++++++++++++++++++++++++" << endl;
    cout << "+++ Test for SO Bundle +++" << endl;
    cout << "++++++++++++++++++++++++++" << endl;
    cout << "Press enter to add SO bundle " << endl;
    getchar();
    std::map<string, string> bundleParams;
    container->addBundle("oic.bundle.hueSample", "", "libHueBundle.so", "huesample", bundleParams);

    std::list<unique_ptr<RCSBundleInfo>> bundles = container->listBundles();
    std::list<unique_ptr<RCSBundleInfo>>::iterator bundleIt;

    cout << "\t>>> bundle list size : " << bundles.size() << endl;
    for (bundleIt = bundles.begin(); bundleIt != bundles.end(); bundleIt++)
    {
        cout << "\t>>> bundle Id : " << (*bundleIt)->getID().c_str() << endl;
    }

    cout << "\nPress enter to start SO bundle " << endl;
    getchar();
    container->startBundle("oic.bundle.hueSample");

    std::map<string, string> resourceParams;
    cout << "Press enter to add SO bundle resource " << endl;
    getchar();
    resourceParams["resourceType"] = "oic.r.light";
    resourceParams["address"] = "http://192.168.0.2/api/newdeveloper/lights/1";
    container->addResourceConfig("oic.bundle.hueSample", "", resourceParams);
    container->addResourceConfig("oic.bundle.hueSample", "", resourceParams);

    std::list<string> resources = container->listBundleResources("oic.bundle.hueSample");
    std::list<string>::iterator resourceIt;
    cout << "\t>>> resource list size : " << resources.size() << endl;
    for (resourceIt = resources.begin(); resourceIt != resources.end(); resourceIt++)
    {
        cout << "\t>>> resource uri : " << (*resourceIt).c_str() << endl;
    }

    cout << "\nPress enter to remove SO bundle resource " << endl;
    getchar();
    container->removeResourceConfig("oic.bundle.hueSample", "/hue/light/1");

    resources = container->listBundleResources("oic.bundle.hueSample");
    cout << "\t>>> resource list size : " << resources.size() << endl;
    for (resourceIt = resources.begin(); resourceIt != resources.end(); resourceIt++)
    {
        cout << "\t>>> resource uri : " << (*resourceIt).c_str() << endl;
    }

    cout << "\nPress enter to stop SO Bundle " << endl;
    getchar();
    container->stopBundle("oic.bundle.hueSample");

    cout << "Press enter to test remove SO Bundle " << endl;
    getchar();
    container->removeBundle("oic.bundle.hueSample");

    bundles = container->listBundles();
    cout << "\t>>> bundle list size : " << bundles.size() << endl;
    for (bundleIt = bundles.begin(); bundleIt != bundles.end(); bundleIt++)
    {
        cout << "\t>>> bundle Id : " << (*bundleIt)->getID().c_str() << endl;
    }

#if(JAVA_SUPPORT)
    /*java bundle add test*/
    cout << "\n++++++++++++++++++++++++++++" << endl;
    cout << "+++ Test for JAVA Bundle +++" << endl;
    cout << "++++++++++++++++++++++++++++" << endl;
    cout << "Press enter to add java bundle " << endl;
    getchar();
    bundleParams["libraryPath"] = ".";
    std::string activator = "org.iotivity.bundle.hue.HueBundleActivator";
    container->addBundle("oic.bundle.hueJavaSample2", "/hueJava",
                 "../../../../../../service/resource-container/" \
                 "examples/HueJavaSampleBundle/hue/target/hue-0.1-jar-with-dependencies.jar",
                 activator,
                 bundleParams);

    bundles = container->listBundles();
    cout << "\t>>> bundle list size : " << bundles.size() << endl;
    for (bundleIt = bundles.begin(); bundleIt != bundles.end(); bundleIt++)
    {
        cout << "\t>>> bundle Id : " << (*bundleIt)->getID().c_str() << endl;
    }

    cout << "\nPress enter to start java bundle " << endl;
    getchar();
    container->startBundle("oic.bundle.hueJavaSample2");

    cout << "Press enter to stop java Bundle " << endl;
    getchar();
    container->stopBundle("oic.bundle.hueJavaSample2");

    cout << "Press enter to test remove java Bundle " << endl;
    getchar();
    container->removeBundle("oic.bundle.hueJavaSample2");

    bundles = container->listBundles();
    cout << "\t>>> bundle list size : " << bundles.size() << endl;
    for (bundleIt = bundles.begin(); bundleIt != bundles.end(); bundleIt++)
    {
        cout << "\t>>> bundle Id : " << (*bundleIt)->getID().c_str() << endl;
    }
#endif

    cout << "\nPress enter to stop container " << endl;
    getchar();
    container->stopContainer();

    cout << "Container stopped. Bye" << endl;
}
