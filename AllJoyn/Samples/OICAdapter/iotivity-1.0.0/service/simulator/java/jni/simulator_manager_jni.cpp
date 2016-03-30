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

#include "simulator_manager_jni.h"
#include "simulator_resource_server_jni.h"
#include "simulator_common_jni.h"
#include "simulator_manager.h"
#include "simulator_remote_resource_jni.h"
#include "simulator_resource_model_jni.h"
#include "simulator_device_info_jni.h"
#include "simulator_platform_info_jni.h"
#include "simulator_resource_jni_util.h"
#include "simulator_jni_utils.h"

SimulatorClassRefs gSimulatorClassRefs;
std::mutex gEnvMutex;
JavaVM *gvm;

JNIEnv *getEnv()
{
    std::unique_lock<std::mutex> lock(gEnvMutex);
    if (nullptr == gvm)
        return NULL;

    JNIEnv *env = NULL;
    jint ret = gvm->GetEnv((void **)&env, JNI_VERSION_1_6);
    switch (ret)
    {
        case JNI_OK:
            return env;
        case JNI_EDETACHED:
            if (0 == gvm->AttachCurrentThread((void **)&env, NULL))
                return env;
    }

    return NULL;
}

void releaseEnv()
{
    std::unique_lock<std::mutex> lock(gEnvMutex);
    if (nullptr == gvm)
        return;
    gvm->DetachCurrentThread();
}

class JNILogger : public ILogger
{
    public:
        void setJavaLogger(JNIEnv *env, jobject logger)
        {
            m_logger = env->NewWeakGlobalRef(logger);
        }

        void write(std::string time, ILogger::Level level, std::string message)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject logger = env->NewLocalRef(m_logger);
            if (!logger)
            {
                releaseEnv();
                return;
            }

            jclass loggerCls = env->GetObjectClass(logger);
            if (!loggerCls)
            {
                releaseEnv();
                return;
            }

            jmethodID writeMId = env->GetMethodID(loggerCls, "write",
                                                  "(Ljava/lang/String;ILjava/lang/String;)V");
            if (!writeMId)
            {
                releaseEnv();
                return;
            }

            jstring msg = env->NewStringUTF(message.c_str());
            jstring timeStr = env->NewStringUTF(time.c_str());
            env->CallVoidMethod(logger, writeMId, timeStr, static_cast<jint>(level), msg);
            env->DeleteLocalRef(msg);
            env->DeleteLocalRef(timeStr);
            releaseEnv();
        }

    private:
        jweak m_logger;
};


jobject SimulatorRemoteResourceToJava(JNIEnv *env, jlong resource)
{
    jmethodID constructor = env->GetMethodID(gSimulatorClassRefs.classSimulatorRemoteResource, "<init>",
                            "(J)V");
    if (NULL == constructor)
    {
        return NULL;
    }

    jobject resourceObj = (jobject) env->NewObject(gSimulatorClassRefs.classSimulatorRemoteResource,
                          constructor, resource);
    if (NULL == resourceObj)
    {
        return NULL;
    }

    return resourceObj;
}

class JNIFoundResourceListener
{
    public:
        void setJavaFoundResourceListener(JNIEnv *env, jobject listener)
        {
            m_listener = env->NewWeakGlobalRef(listener);
        }

        void onFoundResource(std::shared_ptr<SimulatorRemoteResource> resource)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject foundResourceListener = env->NewLocalRef(m_listener);
            if (!foundResourceListener)
            {
                releaseEnv();
                return;
            }

            jclass foundResourceCls = env->GetObjectClass(foundResourceListener);
            if (!foundResourceCls)
            {
                releaseEnv();
                return;
            }

            jmethodID foundResourceMId = env->GetMethodID(foundResourceCls, "onResourceCallback",
                                         "(Lorg/oic/simulator/clientcontroller/SimulatorRemoteResource;)V");
            if (!foundResourceMId)
            {
                releaseEnv();
                return;
            }

            JniSimulatorRemoteResource *jniSimulatorResource = new JniSimulatorRemoteResource(resource);
            if (!jniSimulatorResource)
            {
                releaseEnv();
                return;
            }

            jobject simulatorResource = SimulatorRemoteResourceToJava(env,
                                        reinterpret_cast<jlong>(jniSimulatorResource));

            jfieldID fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mUri",
                                               "Ljava/lang/String;");
            jstring jUri = env->NewStringUTF(resource->getURI().c_str());
            env->SetObjectField(simulatorResource, fieldID, jUri);

            fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mConnType", "I");
            jint jConnType = resource->getConnectivityType();
            env->SetIntField(simulatorResource, fieldID, jConnType);

            fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mHost",
                                      "Ljava/lang/String;");
            jstring jHost = env->NewStringUTF(resource->getHost().c_str());
            env->SetObjectField(simulatorResource, fieldID, jHost);

            fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mId",
                                      "Ljava/lang/String;");
            jstring jUid = env->NewStringUTF(resource->getID().c_str());
            env->SetObjectField(simulatorResource, fieldID, jUid);

            fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mResTypes",
                                      "Ljava/util/LinkedList;");
            std::vector<std::string> resourceTypes = resource->getResourceTypes();
            jobject jResTypes = convertStringVectorToJavaList(env, resourceTypes);
            env->SetObjectField(simulatorResource, fieldID, jResTypes);

            fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mResInterfaces",
                                      "Ljava/util/LinkedList;");
            std::vector<std::string> interfaceTypes = resource->getResourceInterfaces();
            jobject jResInterfaces = convertStringVectorToJavaList(env, interfaceTypes);
            env->SetObjectField(simulatorResource, fieldID, jResInterfaces);

            fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorRemoteResource, "mIsObservable", "Z");
            env->SetBooleanField(simulatorResource, fieldID, resource->isObservable());

            env->CallVoidMethod(foundResourceListener, foundResourceMId, simulatorResource);
            if ((env)->ExceptionCheck())
            {
                delete jniSimulatorResource;
                releaseEnv();
                return;
            }

            releaseEnv();
        }

    private:
        jweak m_listener;

};

void onResourceModelChange(jweak jlistenerRef, const std::string &uri,
                           const SimulatorResourceModel &resModel)
{
    JNIEnv *env = getEnv();
    if (nullptr == env)
        return;

    jobject modelChangeListener = env->NewLocalRef(jlistenerRef);
    if (!modelChangeListener)
    {
        releaseEnv();
        return;
    }

    jclass modelChangeCls = env->GetObjectClass(modelChangeListener);
    if (!modelChangeCls)
    {
        releaseEnv();
        return;
    }

    jmethodID foundModelChangeMId = env->GetMethodID(modelChangeCls, "onResourceModelChanged",
                                    "(Ljava/lang/String;Lorg/oic/simulator/SimulatorResourceModel;)V");
    if (!foundModelChangeMId)
    {
        releaseEnv();
        return;
    }

    JSimulatorResourceModel *jniModel = new JSimulatorResourceModel(resModel);
    if (!jniModel)
    {
        releaseEnv();
        return;
    }

    jobject jModel = JSimulatorResourceModel::toJava(env, reinterpret_cast<jlong>(jniModel));
    jstring jUri = env->NewStringUTF(uri.c_str());
    env->CallVoidMethod(modelChangeListener, foundModelChangeMId, jUri, jModel);
    if ((env)->ExceptionCheck())
    {
        delete jniModel;
        releaseEnv();
        return;
    }

    env->DeleteLocalRef(jUri);

    releaseEnv();
}


JNIEXPORT jobject JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_createResource
(JNIEnv *env, jclass object, jstring configPath, jobject listener)
{
    if (!configPath)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM,
                                  "Configuration file path is empty!");
        return nullptr;
    }

    if (!listener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK,
                                  "Resource model change callback not set!");
        return nullptr;
    }

    jweak jlistenerRef = env->NewWeakGlobalRef(listener);
    SimulatorResourceServer::ResourceModelChangedCB callback =  [jlistenerRef](const std::string & uri,
            const SimulatorResourceModel & resModel)
    {
        onResourceModelChange(jlistenerRef, uri, resModel);
    };

    const char *configPathCStr = env->GetStringUTFChars(configPath, NULL);
    SimulatorResourceServerSP resource = NULL;
    try
    {
        resource = SimulatorManager::getInstance()->createResource(
                       configPathCStr, callback);
        if (nullptr == resource)
        {
            if (configPathCStr)
                env->ReleaseStringUTFChars(configPath, configPathCStr);
            return NULL;
        }
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return nullptr;
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
        return nullptr;
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return nullptr;
    }

    JniSimulatorResource *jniSimResource = new JniSimulatorResource(resource);
    jobject jSimulatorResource = JniSimulatorResource::toJava(env,
                                 reinterpret_cast<jlong>(jniSimResource));

    jniSimResource->setResourceInfo(env, jSimulatorResource);

    if (configPathCStr)
        env->ReleaseStringUTFChars(configPath, configPathCStr);
    return jSimulatorResource;
}

JNIEXPORT jobjectArray JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_createResources
(JNIEnv *env, jclass object, jstring configPath, jint count, jobject listener)
{
    if (!configPath)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM,
                                  "Configuration file path is empty!");
        return nullptr;
    }

    if (!listener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK,
                                  "Resource model change callback not set!");
        return nullptr;
    }

    jweak jlistenerRef = env->NewWeakGlobalRef(listener);
    SimulatorResourceServer::ResourceModelChangedCB callback =  [jlistenerRef](const std::string & uri,
            const SimulatorResourceModel & resModel)
    {
        onResourceModelChange(jlistenerRef, uri, resModel);
    };

    const char *configPathCStr = env->GetStringUTFChars(configPath, NULL);
    std::vector<SimulatorResourceServerSP> resources;
    try
    {
        resources = SimulatorManager::getInstance()->createResource(configPathCStr, count, callback);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return nullptr;
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
        return nullptr;
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return nullptr;
    }

    // Construct the object array and send it java layer
    jobjectArray resourceArray = env->NewObjectArray(resources.size(),
                                 gSimulatorClassRefs.classSimulatorResource, NULL);
    if (resourceArray)
    {
        for (size_t i = 0; i < resources.size(); i++)
        {
            JniSimulatorResource *jniSimResource = new JniSimulatorResource(resources[i]);
            jobject jSimulatorResource = JniSimulatorResource::toJava(env,
                                         reinterpret_cast<jlong>(jniSimResource));
            jniSimResource->setResourceInfo(env, jSimulatorResource);
            env->SetObjectArrayElement(resourceArray, i, jSimulatorResource);
        }
    }

    if (configPathCStr)
        env->ReleaseStringUTFChars(configPath, configPathCStr);
    return resourceArray;
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_deleteResource
(JNIEnv *env, jclass object, jobject jResource)
{
    if (!jResource)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM,
                                  "No resource has been passed!");
        return;
    }

    SimulatorResourceServerSP resource =
        JniSimulatorResource::getJniSimulatorResourceSP(env, jResource);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT,
                                "Simulator resource not found!");
        return;
    }

    try
    {
        SimulatorManager::getInstance()->deleteResource(resource);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_deleteResources
(JNIEnv *env, jclass object, jstring resourceType)
{
    std::string type;
    const char *typeCStr = NULL;
    if (resourceType)
    {
        typeCStr = env->GetStringUTFChars(resourceType, NULL);
        type = typeCStr;
    }

    try
    {
        SimulatorManager::getInstance()->deleteResource(type);
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }

    if (typeCStr)
        env->ReleaseStringUTFChars(resourceType, typeCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_findResource
(JNIEnv *env, jobject object, jstring jResourceType, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    const char *typeCStr = NULL;
    std::string resourceType;
    if (jResourceType)
    {
        typeCStr = env->GetStringUTFChars(jResourceType, NULL);
        resourceType = typeCStr;
    }

    JNIFoundResourceListener *resourceListener = new JNIFoundResourceListener();
    resourceListener->setJavaFoundResourceListener(env, jListener);

    try
    {
        if (!jResourceType)
        {
            SimulatorManager::getInstance()->findResource(
                std::bind(&JNIFoundResourceListener::onFoundResource,
                          resourceListener, std::placeholders::_1));
        }
        else
        {
            SimulatorManager::getInstance()->findResource(resourceType,
                    std::bind(&JNIFoundResourceListener::onFoundResource,
                              resourceListener, std::placeholders::_1));
        }

    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return;
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
        return;
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }

    if (typeCStr)
        env->ReleaseStringUTFChars(jResourceType, typeCStr);
}


JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_setLogger
(JNIEnv *env, jclass object, jobject logger)
{
    static std::shared_ptr<JNILogger> target(new JNILogger());
    target->setJavaLogger(env, logger);
    SimulatorManager::getInstance()->setLogger(target);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_setDeviceInfo
(JNIEnv *env, jobject interfaceObject, jstring deviceInfo)
{
    if (!deviceInfo)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid device info!");
        return;
    }

    const char *deviceName = env->GetStringUTFChars(deviceInfo, NULL);

    try
    {
        SimulatorManager::getInstance()->setDeviceInfo(deviceName);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }

    env->ReleaseStringUTFChars(deviceInfo, deviceName);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_getDeviceInfo
(JNIEnv *env, jobject interfaceObject, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    JniDeviceInfoListener *deviceInfoListener = new JniDeviceInfoListener(env, jListener);
    DeviceInfoCallback callback = std::bind([deviceInfoListener](DeviceInfo & deviceInfo)
    {
        deviceInfoListener->onDeviceInfoReceived(deviceInfo);
        delete deviceInfoListener;
    }, std::placeholders::_1);

    try
    {
        SimulatorManager::getInstance()->getDeviceInfo(callback);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_setPlatformInfo
(JNIEnv *env, jobject interfaceObject, jobject jPlatformInfo)
{
    if (!jPlatformInfo)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid platform info!");
        return;
    }

    JPlatformInfo jniPlatformInfo(env);
    try
    {
        PlatformInfo platformInfo = jniPlatformInfo.toCPP(jPlatformInfo);
        SimulatorManager::getInstance()->setPlatformInfo(platformInfo);
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorManagerNativeInterface_getPlatformInfo
(JNIEnv *env, jobject interfaceObject, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    JniPlatformInfoListener *platformInfoListener = new JniPlatformInfoListener(env, jListener);
    PlatformInfoCallback callback = std::bind([platformInfoListener](PlatformInfo & platformInfo)
    {
        platformInfoListener->onPlatformInfoReceived(platformInfo);
        delete platformInfoListener;
    }, std::placeholders::_1);

    try
    {
        SimulatorManager::getInstance()->getPlatformInfo(callback);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }
}

static bool getClassRef(JNIEnv *env, const char *className, jclass &classRef)
{
    jclass localClassRef = nullptr;
    localClassRef = env->FindClass(className);
    if (!localClassRef)
        return false;

    classRef = (jclass)env->NewGlobalRef(localClassRef);
    env->DeleteLocalRef(localClassRef);
    return true;
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    if (!vm)
    {
        return JNI_ERR;
    }

    JNIEnv *env = NULL;
    if (JNI_OK != vm->GetEnv((void **) &env, JNI_VERSION_1_6))
    {
        return JNI_ERR;
    }

    // Get the class references
    if (false == getClassRef(env, "java/lang/Object", gSimulatorClassRefs.classObject))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/lang/Integer", gSimulatorClassRefs.classInteger))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/lang/Double", gSimulatorClassRefs.classDouble))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/lang/String", gSimulatorClassRefs.classString))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/HashMap", gSimulatorClassRefs.classHashMap))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/Vector", gSimulatorClassRefs.classVector))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/Map", gSimulatorClassRefs.classMap))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/Map$Entry", gSimulatorClassRefs.classMapEntry))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/Set", gSimulatorClassRefs.classSet))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/Iterator", gSimulatorClassRefs.classIterator))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "java/util/LinkedList", gSimulatorClassRefs.classLinkedList))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/serviceprovider/SimulatorResourceServer",
                             gSimulatorClassRefs.classSimulatorResource))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/SimulatorResourceModel",
                             gSimulatorClassRefs.classSimulatorResourceModel))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/ResourceAttribute",
                             gSimulatorClassRefs.classResourceAttribute))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/clientcontroller/SimulatorRemoteResource",
                             gSimulatorClassRefs.classSimulatorRemoteResource))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/serviceprovider/ObserverInfo",
                             gSimulatorClassRefs.classObserverInfo))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/DeviceInfo",
                             gSimulatorClassRefs.classDeviceInfo))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/PlatformInfo",
                             gSimulatorClassRefs.classPlatformInfo))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/SimulatorException",
                             gSimulatorClassRefs.classSimulatorException))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/InvalidArgsException",
                             gSimulatorClassRefs.classInvalidArgsException))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/NoSupportException",
                             gSimulatorClassRefs.classNoSupportException))
    {
        return JNI_ERR;
    }

    if (false == getClassRef(env, "org/oic/simulator/OperationInProgressException",
                             gSimulatorClassRefs.classOperationInProgressException))
    {
        return JNI_ERR;
    }

    // Get the reference to methods
    gSimulatorClassRefs.classIntegerCtor = env->GetMethodID(gSimulatorClassRefs.classInteger, "<init>",
                                           "(I)V");
    if (!gSimulatorClassRefs.classIntegerCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classDoubleCtor = env->GetMethodID(gSimulatorClassRefs.classDouble, "<init>",
                                          "(D)V");
    if (!gSimulatorClassRefs.classDoubleCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classHashMapCtor = env->GetMethodID(gSimulatorClassRefs.classHashMap, "<init>",
                                           "()V");
    if (!gSimulatorClassRefs.classHashMapCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classHashMapPut = env->GetMethodID(gSimulatorClassRefs.classHashMap, "put",
                                          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (!gSimulatorClassRefs.classHashMapPut)
        return JNI_ERR;

    gSimulatorClassRefs.classVectorCtor = env->GetMethodID(gSimulatorClassRefs.classVector, "<init>",
                                          "()V");
    if (!gSimulatorClassRefs.classVectorCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classVectorAddElement = env->GetMethodID(gSimulatorClassRefs.classVector,
            "addElement",
            "(Ljava/lang/Object;)V");
    if (!gSimulatorClassRefs.classVectorAddElement)
        return JNI_ERR;

    gSimulatorClassRefs.classMapEntrySet = env->GetMethodID(
            gSimulatorClassRefs.classMap, "entrySet", "()Ljava/util/Set;");
    if (!gSimulatorClassRefs.classMapEntrySet)
        return JNI_ERR;

    gSimulatorClassRefs.classMapGetKey = env->GetMethodID(
            gSimulatorClassRefs.classMapEntry, "getKey", "()Ljava/lang/Object;");
    if (!gSimulatorClassRefs.classMapGetKey)
        return JNI_ERR;

    gSimulatorClassRefs.classMapGetValue = env->GetMethodID(
            gSimulatorClassRefs.classMapEntry, "getValue", "()Ljava/lang/Object;");
    if (!gSimulatorClassRefs.classMapGetValue)
        return JNI_ERR;

    gSimulatorClassRefs.classIteratorId = env->GetMethodID(
            gSimulatorClassRefs.classSet, "iterator", "()Ljava/util/Iterator;");
    if (!gSimulatorClassRefs.classIteratorId)
        return JNI_ERR;

    gSimulatorClassRefs.classHasNextId = env->GetMethodID(
            gSimulatorClassRefs.classIterator, "hasNext", "()Z");
    if (!gSimulatorClassRefs.classHasNextId)
        return JNI_ERR;

    gSimulatorClassRefs.classNextId = env->GetMethodID(
                                          gSimulatorClassRefs.classIterator, "next", "()Ljava/lang/Object;");
    if (!gSimulatorClassRefs.classNextId)
        return JNI_ERR;

    gSimulatorClassRefs.classLinkedListCtor = env->GetMethodID(gSimulatorClassRefs.classLinkedList,
            "<init>", "()V");
    if (!gSimulatorClassRefs.classLinkedListCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classLinkedListAddObject = env->GetMethodID(gSimulatorClassRefs.classLinkedList,
            "add", "(Ljava/lang/Object;)Z");
    if (!gSimulatorClassRefs.classLinkedListAddObject)
        return JNI_ERR;

    gSimulatorClassRefs.classSimulatorResourceCtor = env->GetMethodID(
                gSimulatorClassRefs.classSimulatorResource, "<init>", "(J)V");
    if (!gSimulatorClassRefs.classSimulatorResourceCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classSimulatorResourceModelCtor = env->GetMethodID(
                gSimulatorClassRefs.classSimulatorResourceModel, "<init>", "(J)V");
    if (!gSimulatorClassRefs.classSimulatorResourceModelCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classResourceAttributeCtor = env->GetMethodID(
                gSimulatorClassRefs.classResourceAttribute, "<init>", "()V");
    if (!gSimulatorClassRefs.classResourceAttributeCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classResourceAttributeSetRange = env->GetMethodID(
                gSimulatorClassRefs.classResourceAttribute, "setRange", "(II)V");
    if (!gSimulatorClassRefs.classResourceAttributeSetRange)
        return JNI_ERR;

    gSimulatorClassRefs.classSimulatorResourceModelId = env->GetMethodID(
                gSimulatorClassRefs.classSimulatorResourceModel, "<init>", "(J)V");
    if (!gSimulatorClassRefs.classSimulatorResourceModelId)
        return JNI_ERR;

    gSimulatorClassRefs.classObserverInfoCtor = env->GetMethodID(
                gSimulatorClassRefs.classObserverInfo, "<init>",
                "(ILjava/lang/String;I)V");
    if (!gSimulatorClassRefs.classObserverInfoCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classSimulatorExceptionCtor = env->GetMethodID(
                gSimulatorClassRefs.classSimulatorException, "<init>",
                "(ILjava/lang/String;)V");
    if (!gSimulatorClassRefs.classSimulatorExceptionCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classInvalidArgsExceptionCtor = env->GetMethodID(
                gSimulatorClassRefs.classInvalidArgsException, "<init>",
                "(ILjava/lang/String;)V");
    if (!gSimulatorClassRefs.classInvalidArgsExceptionCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classNoSupportExceptionCtor = env->GetMethodID(
                gSimulatorClassRefs.classNoSupportException, "<init>",
                "(ILjava/lang/String;)V");
    if (!gSimulatorClassRefs.classNoSupportExceptionCtor)
        return JNI_ERR;

    gSimulatorClassRefs.classOperationInProgressExceptionCtor = env->GetMethodID(
                gSimulatorClassRefs.classOperationInProgressException, "<init>",
                "(ILjava/lang/String;)V");
    if (!gSimulatorClassRefs.classOperationInProgressExceptionCtor)
        return JNI_ERR;

    gvm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
}

#ifdef __cplusplus
}
#endif
