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

#ifndef RESOURCECONTAINERIMPL_H_
#define RESOURCECONTAINERIMPL_H_

#include "RCSResourceContainer.h"
#include "ResourceContainerBundleAPI.h"
#include "BundleInfoInternal.h"

#include "RCSRequest.h"
#include "RCSResponse.h"
#include "RCSResourceObject.h"

#include "DiscoverResourceUnit.h"

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#if(JAVA_SUPPORT)
#include <jni.h>
#endif

#include <map>

#define BUNDLE_ACTIVATION_WAIT_SEC 10
#define BUNDLE_SET_GET_WAIT_SEC 10

using namespace OIC::Service;

namespace OIC
{
    namespace Service
    {

        class ResourceContainerImpl: public RCSResourceContainer, public ResourceContainerBundleAPI
        {
            public:
                // methods from ResourceContainer
                void startContainer(const std::string &configFile);
                void stopContainer();
                void activateBundle(RCSBundleInfo *bundleInfo);
                void deactivateBundle(RCSBundleInfo *bundleInfo);
                void activateBundle(const std::string &bundleId);
                void deactivateBundle(const std::string &bundleId);
                void registerBundle(RCSBundleInfo *bundleinfo);
                void unregisterBundle(RCSBundleInfo *bundleinfo);
                void unregisterBundleSo(const std::string &id);

                // methods from ResourceContainerBundleAPI
                void registerResource(BundleResource::Ptr resource);
                void unregisterResource(BundleResource::Ptr resource);

                void getBundleConfiguration(const std::string &bundleId, configInfo *configOutput);
                void getResourceConfiguration(const std::string &bundleId,
                                              std::vector< resourceInfo > *configOutput);

                RCSGetResponse getRequestHandler(const RCSRequest &request,
                                                 const RCSResourceAttributes &attributes);
                RCSSetResponse setRequestHandler(const RCSRequest &request,
                                                 const RCSResourceAttributes &attributes);

                void onNotificationReceived(const std::string &strResourceUri);

                static ResourceContainerImpl *getImplInstance();
                static RCSResourceObject::Ptr buildResourceObject(const std::string &strUri,
                        const std::string &strResourceType);

                void startBundle(const std::string &bundleId);
                void stopBundle(const std::string &bundleId);

                void addBundle(const std::string &bundleId, const std::string &bundleUri,
                               const std::string &bundlePath, const std::string &activator,
                               std::map< string, string > params);
                void removeBundle(const std::string &bundleId);

                std::list<std::unique_ptr<RCSBundleInfo>> listBundles();

                void addResourceConfig(const std::string &bundleId, const std::string &resourceUri,
                                       std::map< string, string > params);
                void removeResourceConfig(const std::string &bundleId, const std::string &resourceUri);

                std::list< string > listBundleResources(const std::string &bundleId);

#if(JAVA_SUPPORT)
                JavaVM *getJavaVM(string bundleId);
                void unregisterBundleJava(string id);
#endif

            private:
                map< std::string, BundleInfoInternal * > m_bundles; // <bundleID, bundleInfo>
                map< std::string, RCSResourceObject::Ptr > m_mapServers; //<uri, serverPtr>
                map< std::string, BundleResource::Ptr > m_mapResources; //<uri, resourcePtr>
                map< std::string, list< string > > m_mapBundleResources; //<bundleID, vector<uri>>
                map< std::string, list< DiscoverResourceUnit::Ptr > > m_mapDiscoverResourceUnits;
                //<uri, DiscoverUnit>
                string m_configFile;
                Configuration *m_config;
                // holds for a bundle the threads for bundle activation
                map< std::string, boost::thread > m_activators;
                // used for synchronize the resource registration of multiple bundles
                std::mutex registrationLock;
                // used to synchronize the startup of the container with other operation
                // such as individual bundle activation
                std::recursive_mutex activationLock;

                ResourceContainerImpl();
                virtual ~ResourceContainerImpl();

                ResourceContainerImpl(const ResourceContainerImpl &) = delete;
                ResourceContainerImpl(ResourceContainerImpl &&) = delete;
                ResourceContainerImpl &operator=(const ResourceContainerImpl &) const = delete;
                ResourceContainerImpl &operator=(ResourceContainerImpl &&) const = delete;

                void activateSoBundle(const std::string &bundleId);
                void deactivateSoBundle(const std::string &bundleId);
                void addSoBundleResource(const std::string &bundleId, resourceInfo newResourceInfo);
                void removeSoBundleResource(const std::string &bundleId,
                                            const std::string &resourceUri);
                void registerSoBundle(RCSBundleInfo *bundleInfo);
                void discoverInputResource(const std::string &outputResourceUri);
                void undiscoverInputResource(const std::string &outputResourceUri);
                void activateBundleThread(const std::string &bundleId);

#if(JAVA_SUPPORT)
                map<string, JavaVM *> m_bundleVM;

                void registerJavaBundle(RCSBundleInfo *bundleInfo);
                void activateJavaBundle(string bundleId);
                void deactivateJavaBundle(string bundleId);

#endif

        };
    }
}
#endif
