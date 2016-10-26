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

#ifndef BUNDLEINFOINTERNAL_H_
#define BUNDLEINFOINTERNAL_H_

#include <string>
#include "RCSBundleInfo.h"
#include "ResourceContainerBundleAPI.h"

#if (JAVA_SUPPORT)
#include "jni.h"
#endif

using namespace std;
using namespace OIC::Service;

namespace OIC
{
    namespace Service
    {
        typedef void activator_t(ResourceContainerBundleAPI *, std::string bundleId);
        typedef void deactivator_t(void);
        typedef void resourceCreator_t(resourceInfo resourceInfo);
        typedef void resourceDestroyer_t(BundleResource::Ptr pBundleResource);

        class BundleInfoInternal: public RCSBundleInfo
        {
            public:
                BundleInfoInternal();
                BundleInfoInternal(RCSBundleInfo *info);
                virtual ~BundleInfoInternal();
                void setID(const std::string &id);
                const std::string &getID();
                void setPath(const std::string &path);
                const std::string &getPath();
                void setVersion(const std::string &version);
                const std::string &getVersion();

                void setLoaded(bool loaded);
                bool isLoaded();
                void setActivated(bool activated);
                bool isActivated();

                virtual void setLibraryPath(const std::string &libpath);
                virtual const std::string &getLibraryPath();

                void setActivatorName(const std::string &activatorName);
                const std::string &getActivatorName();

                void setBundleActivator(activator_t *);
                activator_t *getBundleActivator();

                void setBundleDeactivator(deactivator_t *);
                deactivator_t *getBundleDeactivator();

                void setResourceCreator(resourceCreator_t *);
                resourceCreator_t *getResourceCreator();

                void setResourceDestroyer(resourceDestroyer_t *);
                resourceDestroyer_t *getResourceDestroyer();

                void setBundleInfo(RCSBundleInfo *bundleInfo);

                void setBundleHandle(void *);
                void *getBundleHandle();

                void setJavaBundle(bool javaBundle);
                bool getJavaBundle();

#if (JAVA_SUPPORT)
                void setJavaBundleActivatorMethod(jmethodID activator);
                jmethodID getJavaBundleActivatorMethod();
                void setJavaBundleDeactivatorMethod(jmethodID deactivator);
                jmethodID getJavaBundleDeactivatorMethod();

                void setJavaBundleActivatorObject(jobject);
                jobject getJavaBundleActivatorObject();
#endif

            private:
                bool m_loaded, m_activated, m_java_bundle;
                int m_id;
                activator_t *m_activator;
                deactivator_t *m_deactivator;
                resourceCreator_t *m_resourceCreator;
                resourceDestroyer_t *m_resourceDestroyer;
                void *m_bundleHandle;
                string m_activator_name;
                string m_library_path;
#if (JAVA_SUPPORT)
                jmethodID m_java_activator, m_java_deactivator;
                jobject m_java_activator_object;
#endif

        };
    }
}

#endif /* BUNDLEINFOINTERNAL_H_ */
