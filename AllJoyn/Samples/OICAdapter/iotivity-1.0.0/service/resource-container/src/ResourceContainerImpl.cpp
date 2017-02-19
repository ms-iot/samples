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

#include "ResourceContainerImpl.h"

#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <algorithm>

#include "BundleActivator.h"
#include "SoftSensorResource.h"
#include "InternalTypes.h"

using namespace OIC::Service;
using namespace std;

namespace OIC
{
    namespace Service
    {
        ResourceContainerImpl::ResourceContainerImpl()
        {
            m_config = nullptr;
        }

        ResourceContainerImpl::~ResourceContainerImpl()
        {
            m_config = nullptr;
        }

        bool has_suffix(const std::string &str, const std::string &suffix)
        {
            return str.size() >= suffix.size()
                   && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        void ResourceContainerImpl::startContainer(const std::string &configFile)
        {
            OC_LOG(INFO, CONTAINER_TAG, "Starting resource container.");
#if (JAVA_SUPPORT)
            OC_LOG(INFO, CONTAINER_TAG, "Resource container has Java support.");
#else
            OC_LOG(INFO, CONTAINER_TAG, "Resource container without Java support.");
#endif


            activationLock.lock();
            if (!configFile.empty())
            {
                m_config = new Configuration(configFile);

                if (m_config->isLoaded())
                {
                    configInfo bundles;
                    m_config->getConfiguredBundles(&bundles);

                    for (unsigned int i = 0; i < bundles.size(); i++)
                    {
                        BundleInfoInternal *bundleInfo = new BundleInfoInternal();
                        bundleInfo->setPath(bundles[i][BUNDLE_PATH]);
                        bundleInfo->setVersion(bundles[i][BUNDLE_VERSION]);
                        bundleInfo->setID(bundles[i][BUNDLE_ID]);
                        if (!bundles[i][BUNDLE_ACTIVATOR].empty())
                        {
                            string activatorName = bundles[i][BUNDLE_ACTIVATOR];
                            std::replace(activatorName.begin(), activatorName.end(), '.', '/');
                            ((BundleInfoInternal *) bundleInfo)->setActivatorName(activatorName);
                            ((BundleInfoInternal *) bundleInfo)->setLibraryPath(
                                bundles[i][BUNDLE_LIBRARY_PATH]);
                        }

                        OC_LOG_V(INFO, CONTAINER_TAG, "Init Bundle:(%s)",
                                 std::string(bundles[i][BUNDLE_ID] + ";" +
                                             bundles[i][BUNDLE_PATH]).c_str());

                        registerBundle(bundleInfo);
                        activateBundle(bundleInfo);
                    }
                }
                else
                {
                    OC_LOG_V(ERROR, CONTAINER_TAG, "Container started with invalid configfile path.");
                }
            }
            else
            {
                OC_LOG_V(INFO, CONTAINER_TAG, "No configuration file for the container provided.");
            }

            map<std::string, boost::thread >::iterator activatorIterator;

            for (activatorIterator = m_activators.begin(); activatorIterator != m_activators.end();
                 activatorIterator++)
            {
                activatorIterator->second.timed_join(
                    boost::posix_time::seconds(BUNDLE_ACTIVATION_WAIT_SEC));
                // wait for bundles to be activated
            }
            activationLock.unlock();
        }

        void ResourceContainerImpl::stopContainer()
        {
            OC_LOG(INFO, CONTAINER_TAG, "Stopping resource container.");

            for (std::map< std::string, BundleInfoInternal * >::iterator it = m_bundles.begin();
                 it != m_bundles.end(); ++it)
            {
                BundleInfoInternal *bundleInfo = it->second;
                deactivateBundle(bundleInfo);
                unregisterBundle(bundleInfo);
            }

            if (!m_mapServers.empty())
            {
                map< std::string, RCSResourceObject::Ptr >::iterator itor = m_mapServers.begin();

                while (itor != m_mapServers.end())
                {
                    (itor++)->second.reset();
                }

                m_mapResources.clear();
                m_mapBundleResources.clear();
            }

            if (m_config)
                delete m_config;
        }

        void ResourceContainerImpl::activateBundle(RCSBundleInfo *bundleInfo)
        {
            activationLock.lock();
            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) bundleInfo;

            if (bundleInfoInternal->isLoaded())
            {
                activateBundle(bundleInfo->getID());
            }
            activationLock.unlock();
        }

        void ResourceContainerImpl::deactivateBundle(RCSBundleInfo *bundleInfo)
        {
            if (((BundleInfoInternal *) bundleInfo)->isActivated())
            {
                deactivateBundle(bundleInfo->getID());
            }
        }

        void ResourceContainerImpl::activateBundle(const std::string &id)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "Activating bundle: (%s)",
                     std::string(m_bundles[id]->getID()).c_str());
            activationLock.lock();
            auto f = std::bind(&ResourceContainerImpl::activateBundleThread, this,
                               id);
            boost::thread activator(f);
            activator.timed_join(boost::posix_time::seconds(BUNDLE_SET_GET_WAIT_SEC));
            activationLock.unlock();
            OC_LOG_V(INFO, CONTAINER_TAG, "Bundle activated: (%s)",
                     std::string(m_bundles[id]->getID()).c_str());
        }

        void ResourceContainerImpl::deactivateBundle(const std::string &id)
        {
            if (m_bundles[id]->getJavaBundle())
            {
#if(JAVA_SUPPORT)
                deactivateJavaBundle(id);
#endif
            }
            else
            {
                deactivateSoBundle(id);
            }
        }

        // loads the bundle
        void ResourceContainerImpl::registerBundle(RCSBundleInfo *bundleInfo)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "Registering bundle: (%s)",
                     std::string(bundleInfo->getPath()).c_str());

            if (has_suffix(bundleInfo->getPath(), ".jar"))
            {
#if(JAVA_SUPPORT)
                ((BundleInfoInternal *) bundleInfo)->setJavaBundle(true);
                registerJavaBundle(bundleInfo);
#endif
            }
            else
            {
                ((BundleInfoInternal *) bundleInfo)->setJavaBundle(false);
                registerSoBundle(bundleInfo);
            }
        }

        void ResourceContainerImpl::unregisterBundle(RCSBundleInfo *bundleInfo)
        {
            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) bundleInfo;
            if (bundleInfoInternal->isLoaded() && !bundleInfoInternal->isActivated())
            {
                if (!bundleInfoInternal->getJavaBundle())
                {
                    unregisterBundleSo(bundleInfo->getID());
                }
                else
                {
#if(JAVA_SUPPORT)
                    unregisterBundleJava(bundleInfo->getID());
#endif
                }
            }
        }

        void ResourceContainerImpl::unregisterBundleSo(const std::string &id)
        {
            void *bundleHandle = m_bundles[id]->getBundleHandle();

            OC_LOG_V(INFO, CONTAINER_TAG, "Unregister bundle: (%s)",
                     std::string(m_bundles[id]->getID()).c_str());

            const char *error;
            dlclose(bundleHandle);

            if ((error = dlerror()) != NULL)
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Error (%s)", error);
            }
            else
            {
                delete m_bundles[id];
                m_bundles.erase(id);
            }
        }

        void ResourceContainerImpl::registerResource(BundleResource::Ptr resource)
        {
            string strUri = resource->m_uri;
            string strResourceType = resource->m_resourceType;
            RCSResourceObject::Ptr server = nullptr;

            OC_LOG_V(INFO, CONTAINER_TAG, "Registration of resource (%s)" ,
                     std::string(strUri + ", " + strResourceType).c_str());

            registrationLock.lock();
            if (m_mapResources.find(strUri) == m_mapResources.end())
            {
                server = buildResourceObject(strUri, strResourceType);

                if (server != nullptr)
                {
                    m_mapServers[strUri] = server;
                    m_mapResources[strUri] = resource;
                    m_mapBundleResources[resource->m_bundleId].push_back(strUri);

                    server->setGetRequestHandler(
                        std::bind(&ResourceContainerImpl::getRequestHandler, this,
                                  std::placeholders::_1, std::placeholders::_2));

                    server->setSetRequestHandler(
                        std::bind(&ResourceContainerImpl::setRequestHandler, this,
                                  std::placeholders::_1, std::placeholders::_2));

                    OC_LOG_V(INFO, CONTAINER_TAG, "Registration finished (%s)",
                             std::string(strUri + ", " +
                                         strResourceType).c_str());

                    if (m_config->isHasInput(resource->m_bundleId))
                    {
                        discoverInputResource(strUri);
                    }

                    // to get notified if bundle resource attributes are updated
                    resource->registerObserver((NotificationReceiver *) this);
                }
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "resource with (%s)",
                         std::string(strUri + " already exists.").c_str());
            }
            registrationLock.unlock();
        }

        void ResourceContainerImpl::unregisterResource(BundleResource::Ptr resource)
        {
            string strUri = resource->m_uri;
            string strResourceType = resource->m_resourceType;

            OC_LOG_V(INFO, CONTAINER_TAG, "Unregistration of resource (%s)",
                     std::string(resource->m_uri + ", " +
                                 resource->m_resourceType).c_str());

            if (m_config->isHasInput(resource->m_bundleId))
            {
                undiscoverInputResource(strUri);
            }

            if (m_mapServers.find(strUri) != m_mapServers.end())
            {
                m_mapServers[strUri].reset();

                m_mapResources.erase(m_mapResources.find(strUri));
                m_mapBundleResources[resource->m_bundleId].remove(strUri);
            }
        }

        void ResourceContainerImpl::getBundleConfiguration(const std::string &bundleId,
                configInfo *configOutput)
        {
            if (m_config)
            {
                m_config->getBundleConfiguration(bundleId, (configInfo *) configOutput);
            }
        }

        void ResourceContainerImpl::getResourceConfiguration(const std::string &bundleId,
                std::vector< resourceInfo > *configOutput)
        {
            if (m_config)
            {
                m_config->getResourceConfiguration(bundleId, configOutput);
            }
        }

        RCSGetResponse ResourceContainerImpl::getRequestHandler(const RCSRequest &request,
                const RCSResourceAttributes &)
        {
            RCSResourceAttributes attr;
            std::string strResourceUri = request.getResourceUri();

            if (m_mapServers.find(strResourceUri) != m_mapServers.end()
                && m_mapResources.find(strResourceUri) != m_mapResources.end())
            {
                if (m_mapResources[strResourceUri])
                {
                    auto getFunction = [this, &attr, &strResourceUri]()
                    {
                        attr = m_mapResources[strResourceUri]->handleGetAttributesRequest();
                    };
                    boost::thread getThread(getFunction);
                    getThread.timed_join(boost::posix_time::seconds(BUNDLE_SET_GET_WAIT_SEC));

                }
            }

            return RCSGetResponse::create(std::move(attr), 200);
        }

        RCSSetResponse ResourceContainerImpl::setRequestHandler(const RCSRequest &request,
                const RCSResourceAttributes &attributes)
        {
            RCSResourceAttributes attr;
            std::list<std::string> lstAttributes;
            std::string strResourceUri = request.getResourceUri();

            if (m_mapServers.find(strResourceUri) != m_mapServers.end()
                && m_mapResources.find(strResourceUri) != m_mapResources.end())
            {
                if (m_mapResources[strResourceUri])
                {
                    auto setFunction = [this, &lstAttributes, &strResourceUri, &attributes, &attr]()
                    {
                        lstAttributes = m_mapResources[strResourceUri]->getAttributeNames();

                        for (RCSResourceAttributes::const_iterator itor = attributes.begin();
                             itor != attributes.end(); itor++)
                        {
                            if (std::find(lstAttributes.begin(), lstAttributes.end(), itor->key())
                                != lstAttributes.end())
                            {
                                attr[itor->key()] = itor->value();
                            }
                        }

                        m_mapResources[strResourceUri]->handleSetAttributesRequest(attr);
                    };
                    boost::thread setThread(setFunction);
                    setThread.timed_join(boost::posix_time::seconds(BUNDLE_SET_GET_WAIT_SEC));
                }
            }

            return RCSSetResponse::create(std::move(attr), 200);
        }

        void ResourceContainerImpl::onNotificationReceived(const std::string &strResourceUri)
        {
            OC_LOG_V(INFO, CONTAINER_TAG,
                     "notification from (%s)", std::string(strResourceUri + ".").c_str());

            if (m_mapServers.find(strResourceUri) != m_mapServers.end())
            {
                m_mapServers[strResourceUri]->notify();
            }
        }

        ResourceContainerImpl *ResourceContainerImpl::getImplInstance()
        {
            static ResourceContainerImpl m_instance;
            return &m_instance;
        }

        RCSResourceObject::Ptr ResourceContainerImpl::buildResourceObject(const std::string &strUri,
                const std::string &strResourceType)
        {
            return RCSResourceObject::Builder(strUri, strResourceType,
                                              "oic.if.baseline").setObservable(
                       true).setDiscoverable(true).build();
        }

        void ResourceContainerImpl::startBundle(const std::string &bundleId)
        {
            if (m_bundles.find(bundleId) != m_bundles.end())
            {
                if (!m_bundles[bundleId]->isActivated())
                    activateBundle(m_bundles[bundleId]);
                else
                {
                    OC_LOG(ERROR, CONTAINER_TAG, "Bundle already started");
                }
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Bundle with ID \'(%s)",
                         std::string(bundleId + "\' is not registered.").c_str());
            }
        }

        void ResourceContainerImpl::stopBundle(const std::string &bundleId)
        {
            if (m_bundles.find(bundleId) != m_bundles.end())
            {
                if (m_bundles[bundleId]->isActivated())
                    deactivateBundle(m_bundles[bundleId]);
                else
                {
                    OC_LOG(ERROR, CONTAINER_TAG, "Bundle not activated");
                }
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Bundle with ID \'(%s)",
                         std::string(bundleId + "\' is not registered.").c_str());
            }
        }

        void ResourceContainerImpl::addBundle(const std::string &bundleId,
                                              const std::string &bundleUri, const std::string &bundlePath,
                                              const std::string &activator, std::map< string, string > params)
        {
            (void) bundleUri;

            if (m_bundles.find(bundleId) != m_bundles.end())
                OC_LOG(ERROR, CONTAINER_TAG, "BundleId already exist");

            else
            {
                BundleInfoInternal *bundleInfo = new BundleInfoInternal();
                bundleInfo->setID(bundleId);
                bundleInfo->setPath(bundlePath);
                bundleInfo->setActivatorName(activator);
                if (params.find("libraryPath") != params.end())
                {
                    string activatorName = activator; // modify activator for Java bundle
                    std::replace(activatorName.begin(), activatorName.end(), '.', '/');
                    ((BundleInfoInternal *) bundleInfo)->setActivatorName(activatorName);
                    ((BundleInfoInternal *)bundleInfo)->setLibraryPath(params[BUNDLE_LIBRARY_PATH]);
                }

                OC_LOG_V(INFO, CONTAINER_TAG, "Add Bundle: (%s)",
                         std::string(bundleInfo->getID() + "; " +
                                     bundleInfo->getPath()).c_str());

                registerBundle(bundleInfo);
            }
        }

        void ResourceContainerImpl::removeBundle(const std::string &bundleId)
        {
            if (m_bundles.find(bundleId) != m_bundles.end())
            {
                BundleInfoInternal *bundleInfo = m_bundles[bundleId];
                if (bundleInfo->isActivated())
                    deactivateBundle(bundleInfo);

                if (bundleInfo->isLoaded())
                    unregisterBundle(bundleInfo);
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Bundle with ID \'(%s)",
                         std::string(bundleId + "\' is not registered.").c_str());
            }
        }

        std::list<std::unique_ptr<RCSBundleInfo>> ResourceContainerImpl::listBundles()
        {
            std::list<std::unique_ptr<RCSBundleInfo> > ret;
            for (std::map< std::string, BundleInfoInternal * >::iterator it = m_bundles.begin();
                 it != m_bundles.end(); ++it)
            {
                {
                    std::unique_ptr<BundleInfoInternal> bundleInfo(new BundleInfoInternal);
                    (bundleInfo)->setBundleInfo(it->second);
                    ret.push_back(std::move(bundleInfo));
                }
            }
            return ret;
        }

        void ResourceContainerImpl::addResourceConfig(const std::string &bundleId,
                const std::string &resourceUri, std::map< string, string > params)
        {
            if (m_bundles.find(bundleId) != m_bundles.end())
            {
                if (!m_bundles[bundleId]->getJavaBundle())
                {
                    resourceInfo newResourceInfo;
                    newResourceInfo.uri = resourceUri;

                    if (params.find(OUTPUT_RESOURCE_NAME) != params.end())
                        newResourceInfo.name = params[OUTPUT_RESOURCE_NAME];
                    if (params.find(OUTPUT_RESOURCE_TYPE) != params.end())
                        newResourceInfo.resourceType = params[OUTPUT_RESOURCE_TYPE];
                    if (params.find(OUTPUT_RESOURCE_ADDR) != params.end())
                        newResourceInfo.address = params[OUTPUT_RESOURCE_ADDR];

                    addSoBundleResource(bundleId, newResourceInfo);
                }
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Bundle with ID \'(%s)",
                         std::string(bundleId + "\' is not registered.").c_str());
            }
        }

        void ResourceContainerImpl::removeResourceConfig(const std::string &bundleId,
                const std::string &resourceUri)
        {
            if (m_bundles.find(bundleId) != m_bundles.end())
            {
                if (!m_bundles[bundleId]->getJavaBundle())
                {
                    removeSoBundleResource(bundleId, resourceUri);
                }
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Bundle with ID \'(%s)",
                         std::string(bundleId + "\' is not registered.").c_str());
            }
        }

        std::list< string > ResourceContainerImpl::listBundleResources(const std::string &bundleId)
        {
            std::list < string > ret;

            if (m_mapBundleResources.find(bundleId) != m_mapBundleResources.end())
            {
                ret = m_mapBundleResources[bundleId];
            }

            return ret;

        }

        void ResourceContainerImpl::registerSoBundle(RCSBundleInfo *bundleInfo)
        {
            const char *error;

            activator_t *bundleActivator = NULL;
            deactivator_t *bundleDeactivator = NULL;
            resourceCreator_t *resourceCreator = NULL;
            resourceDestroyer_t *resourceDestroyer = NULL;
            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) bundleInfo;
            void *bundleHandle = NULL;
            bundleHandle = dlopen(bundleInfo->getPath().c_str(), RTLD_LAZY);

            if (bundleHandle != NULL)
            {
                bundleActivator =
                    (activator_t *) dlsym(bundleHandle,
                                          ("" + bundleInfoInternal->getActivatorName()
                                           + "_externalActivateBundle").c_str());
                bundleDeactivator =
                    (deactivator_t *) dlsym(bundleHandle,
                                            ("" + bundleInfoInternal->getActivatorName()
                                             + "_externalDeactivateBundle").c_str());
                resourceCreator =
                    (resourceCreator_t *) dlsym(bundleHandle,
                                                ("" + bundleInfoInternal->getActivatorName()
                                                 + "_externalCreateResource").c_str());
                resourceDestroyer =
                    (resourceDestroyer_t *) dlsym(bundleHandle,
                                                  ("" + bundleInfoInternal->getActivatorName()
                                                   + "_externalDestroyResource").c_str());


                if ((error = dlerror()) != NULL)
                {
                    OC_LOG_V(ERROR, CONTAINER_TAG, "Error : (%s)", error);
                }
                else
                {
                    ((BundleInfoInternal *) bundleInfo)->setBundleActivator(bundleActivator);
                    ((BundleInfoInternal *) bundleInfo)->setBundleDeactivator(bundleDeactivator);
                    ((BundleInfoInternal *) bundleInfo)->setResourceCreator(resourceCreator);
                    ((BundleInfoInternal *) bundleInfo)->setResourceDestroyer(resourceDestroyer);
                    ((BundleInfoInternal *) bundleInfo)->setLoaded(true);
                    ((BundleInfoInternal *) bundleInfo)->setBundleHandle(bundleHandle);

                    m_bundles[bundleInfo->getID()] = ((BundleInfoInternal *) bundleInfo);
                }
            }
            else
            {
                if ((error = dlerror()) != NULL)
                {
                    OC_LOG_V(ERROR, CONTAINER_TAG, "Error : (%s)", error);
                }
            }
        }

        void ResourceContainerImpl::activateSoBundle(const std::string &bundleId)
        {
            activator_t *bundleActivator = m_bundles[bundleId]->getBundleActivator();

            if (bundleActivator != NULL)
            {
                bundleActivator(this, m_bundles[bundleId]->getID());
                m_bundles[bundleId]->setActivated(true);
            }
            else
            {
                //Unload module and return error
                OC_LOG(ERROR, CONTAINER_TAG, "Activation unsuccessful.");
            }

            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) m_bundles[bundleId];
            bundleInfoInternal->setActivated(true);

        }

        void ResourceContainerImpl::undiscoverInputResource(const std::string &outputResourceUri)
        {
            auto foundDiscoverResource = m_mapDiscoverResourceUnits.find(outputResourceUri);
            if (foundDiscoverResource != m_mapDiscoverResourceUnits.end())
            {
                m_mapDiscoverResourceUnits.erase(foundDiscoverResource);
            }
        }

        void ResourceContainerImpl::discoverInputResource(const std::string &outputResourceUri)
        {
            auto foundOutputResource = m_mapResources.find(outputResourceUri);
            auto resourceProperty = foundOutputResource->second->m_mapResourceProperty;

            try
            {
                resourceProperty.at(INPUT_RESOURCE);
            }
            catch (std::out_of_range &e)
            {
                return;
            }

            for (auto iter : resourceProperty)
            {
                if (iter.first.compare(INPUT_RESOURCE) == 0)
                {
                    for (auto it : iter.second)
                    {
                        auto makeValue = [&](const std::string & reference) mutable -> std::string
                        {
                            std::string retStr = "";
                            try
                            {
                                retStr = it.at(reference);
                            }
                            catch (std::out_of_range &e)
                            {
                                return "";
                            }
                            return retStr;
                        };
                        std::string uri = makeValue(INPUT_RESOURCE_URI);
                        std::string type = makeValue(INPUT_RESOURCE_TYPE);
                        std::string attributeName = makeValue(INPUT_RESOURCE_ATTRIBUTENAME);

                        DiscoverResourceUnit::Ptr newDiscoverUnit = std::make_shared
                                < DiscoverResourceUnit > (outputResourceUri);
                        newDiscoverUnit->startDiscover(
                            DiscoverResourceUnit::DiscoverResourceInfo(uri, type,
                                    attributeName),
                            std::bind(&SoftSensorResource::onUpdatedInputResource,
                                      std::static_pointer_cast< SoftSensorResource > (foundOutputResource->second),
                                      std::placeholders::_1, std::placeholders::_2));

                        auto foundDiscoverResource = m_mapDiscoverResourceUnits.find(
                                                         outputResourceUri);
                        if (foundDiscoverResource != m_mapDiscoverResourceUnits.end())
                        {
                            foundDiscoverResource->second.push_back(newDiscoverUnit);
                        }
                        else
                        {
                            m_mapDiscoverResourceUnits.insert(
                                std::make_pair(outputResourceUri,
                                               std::list< DiscoverResourceUnit::Ptr >
                            { newDiscoverUnit }));
                        }
                    }
                }
            }
        }

        void ResourceContainerImpl::deactivateSoBundle(const std::string &id)
        {
            deactivator_t *bundleDeactivator = m_bundles[id]->getBundleDeactivator();

            OC_LOG_V(INFO, CONTAINER_TAG, "De-activating bundle: (%s)", std::string(
                         m_bundles[id]->getID()).c_str());

            if (bundleDeactivator != NULL)
            {
                bundleDeactivator();
                m_bundles[id]->setActivated(false);
            }
            else
            {
                //Unload module and return error
                OC_LOG(ERROR, CONTAINER_TAG, "De-activation unsuccessful.");
            }
        }

        void ResourceContainerImpl::addSoBundleResource(const std::string &bundleId,
                resourceInfo newResourceInfo)
        {
            resourceCreator_t *resourceCreator;

            resourceCreator = m_bundles[bundleId]->getResourceCreator();

            if (resourceCreator != NULL)
            {
                resourceCreator(newResourceInfo);
            }
            else
            {
                OC_LOG(ERROR, CONTAINER_TAG, "addResource unsuccessful.");
            }
        }

        void ResourceContainerImpl::removeSoBundleResource(const std::string &bundleId,
                const std::string &resourceUri)
        {
            if (m_mapResources.find(resourceUri) != m_mapResources.end())
            {
                resourceDestroyer_t *resourceDestroyer =
                    m_bundles[bundleId]->getResourceDestroyer();

                if (resourceDestroyer != NULL)
                {
                    resourceDestroyer(m_mapResources[resourceUri]);
                }
                else
                {
                    OC_LOG(ERROR, CONTAINER_TAG, "removeResource unsuccessful.");
                }
            }
        }

        void ResourceContainerImpl::activateBundleThread(const std::string &id)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "Activating bundle: (%s)",
                     std::string(m_bundles[id]->getID()).c_str());

            if (m_bundles[id]->getJavaBundle())
            {
#if(JAVA_SUPPORT)
                activateJavaBundle(id);
#endif
            }
            else
            {
                activateSoBundle (id);
            }

            OC_LOG_V(INFO, CONTAINER_TAG, "Bundle activated: (%s)",
                     std::string(m_bundles[id]->getID()).c_str());
        }

#if(JAVA_SUPPORT)
        JavaVM *ResourceContainerImpl::getJavaVM(string bundleId)
        {
            return m_bundleVM[bundleId];
        }

        void ResourceContainerImpl::registerJavaBundle(RCSBundleInfo *bundleInfo)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "Registering Java bundle (%s)",
                     std::string(bundleInfo->getID()).c_str());
            JavaVM *jvm;
            JNIEnv *env;
            JavaVMInitArgs vm_args;
            JavaVMOption options[3];

            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) bundleInfo;

            if (FILE *file = fopen(bundleInfo->getPath().c_str(), "r"))
            {
                fclose(file);

                OC_LOG_V(INFO, CONTAINER_TAG, "Resource bundle (%s)",
                         std::string(bundleInfo->getPath() +
                                     " available.").c_str());
            }
            else
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Resource bundle (%s)",
                         std::string(bundleInfo->getPath() + " not available.").c_str());

                return;
            }

            char optionString[] = "-Djava.compiler=NONE";
            options[0].optionString = optionString;
            char classpath[1000];
            strcpy(classpath, "-Djava.class.path=");
            strcat(classpath, bundleInfo->getPath().c_str());

            OC_LOG(INFO, CONTAINER_TAG,
                   std::string("Configured classpath: ").append(classpath).c_str());

            options[1].optionString = classpath;

            char libraryPath[1000];
            strcpy(libraryPath, "-Djava.library.path=");
            strcat(libraryPath, bundleInfo->getLibraryPath().c_str());
            options[2].optionString = libraryPath;

            OC_LOG(INFO, CONTAINER_TAG,
                   std::string("Configured library path: ").append(libraryPath).c_str());

            vm_args.version = JNI_VERSION_1_4;
            vm_args.options = options;
            vm_args.nOptions = 3;
            vm_args.ignoreUnrecognized = 1;

            int res;
            res = JNI_CreateJavaVM(&jvm, (void **) &env, &vm_args);

            if (res < 0)
            {
                OC_LOG(ERROR, CONTAINER_TAG, "cannot create JavaVM.");
                return;
            }
            else
            {
                OC_LOG(INFO, CONTAINER_TAG, "JVM successfully created.");
            }

            m_bundleVM.insert(std::pair< string, JavaVM * >(bundleInfo->getID(), jvm));

            const char *className = bundleInfoInternal->getActivatorName().c_str();

            OC_LOG_V(INFO, CONTAINER_TAG, "Looking up class: (%s)", std::string(
                         bundleInfoInternal->getActivatorName() + "|").c_str());

            jclass bundleActivatorClass = env->FindClass(className);

            if (bundleActivatorClass == NULL)
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Cannot register bundle (%s)",
                         std::string( bundleInfoInternal->getID()
                                      + " bundle activator(" + bundleInfoInternal->getActivatorName()
                                      + ") not found ").c_str());
                return;
            }

            jmethodID activateMethod = env->GetMethodID(bundleActivatorClass, "activateBundle",
                                       "()V");

            if (activateMethod == NULL)
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Cannot register bundle (%s)",
                         std::string( bundleInfoInternal->getID()
                                      + " activate bundle method not found ").c_str());
                return;
            }
            bundleInfoInternal->setJavaBundleActivatorMethod(activateMethod);

            jmethodID deactivateMethod = env->GetMethodID(bundleActivatorClass, "deactivateBundle",
                                         "()V");

            if (deactivateMethod == NULL)
            {
                OC_LOG_V(ERROR, CONTAINER_TAG, "Cannot register bundle (%s)",
                         std::string( bundleInfoInternal->getID()
                                      + " deactivate bundle method not found ").c_str());
                return;
            }

            bundleInfoInternal->setJavaBundleDeactivatorMethod(deactivateMethod);

            jmethodID constructor;

            constructor = env->GetMethodID(bundleActivatorClass, "<init>", "(Ljava/lang/String;)V");

            jstring bundleID = env->NewStringUTF(bundleInfoInternal->getID().c_str());

            jobject bundleActivator = env->NewObject(bundleActivatorClass, constructor, bundleID);

            bundleInfoInternal->setJavaBundleActivatorObject(bundleActivator);

            bundleInfoInternal->setLoaded(true);

            m_bundles[bundleInfo->getID()] = ((BundleInfoInternal *)bundleInfo);


            OC_LOG(INFO, CONTAINER_TAG, "Bundle registered");
        }

        void ResourceContainerImpl::activateJavaBundle(string bundleId)
        {
            OC_LOG(INFO, CONTAINER_TAG, "Activating java bundle");

            JavaVM *vm = getJavaVM(bundleId);
            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) m_bundles[bundleId];
            JNIEnv *env;
            int envStat = vm->GetEnv((void **) &env, JNI_VERSION_1_4);

            if (envStat == JNI_EDETACHED)
            {
                if (vm->AttachCurrentThread((void **) &env, NULL) != 0)
                {
                    OC_LOG(ERROR, CONTAINER_TAG, "Failed to attach ");
                }
            }
            else if (envStat == JNI_EVERSION)
            {
                OC_LOG(ERROR, CONTAINER_TAG, "Env: version not supported ");
            }

            env->CallVoidMethod(bundleInfoInternal->getJavaBundleActivatorObject(),
                                bundleInfoInternal->getJavaBundleActivatorMethod());

            m_bundles[bundleId]->setActivated(true);
        }

        void ResourceContainerImpl::deactivateJavaBundle(string bundleId)
        {
            OC_LOG(INFO, CONTAINER_TAG, "Deactivating java bundle");

            JavaVM *vm = getJavaVM(bundleId);
            BundleInfoInternal *bundleInfoInternal = (BundleInfoInternal *) m_bundles[bundleId];
            JNIEnv *env;
            int envStat = vm->GetEnv((void **) &env, JNI_VERSION_1_4);

            if (envStat == JNI_EDETACHED)
            {
                if (vm->AttachCurrentThread((void **) &env, NULL) != 0)
                {
                    OC_LOG(ERROR, CONTAINER_TAG, "Failed to attach ");
                }
            }
            else if (envStat == JNI_EVERSION)
            {
                OC_LOG(ERROR, CONTAINER_TAG, "Env: version not supported ");
            }

            env->CallVoidMethod(bundleInfoInternal->getJavaBundleActivatorObject(),
                                bundleInfoInternal->getJavaBundleDeactivatorMethod());

            m_bundles[bundleId]->setActivated(false);
        }

        void ResourceContainerImpl::unregisterBundleJava(string id)
        {
            OC_LOG_V(INFO, CONTAINER_TAG, "Unregister Java bundle: (%s)", std::string(
                         m_bundles[id]->getID()).c_str());

            OC_LOG(INFO, CONTAINER_TAG, "Destroying JVM");

            m_bundleVM[id]->DestroyJavaVM();

            delete m_bundles[id];
            m_bundles.erase(id);
        }
#endif
    }
}
