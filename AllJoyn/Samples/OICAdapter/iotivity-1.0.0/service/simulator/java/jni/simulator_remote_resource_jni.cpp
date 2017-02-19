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

#include "simulator_remote_resource_jni.h"
#include "simulator_common_jni.h"
#include "simulator_error_codes.h"
#include "simulator_resource_jni_util.h"
#include "simulator_resource_model_jni.h"
#include "simulator_client_types.h"
#include "simulator_exceptions.h"
#include "simulator_jni_utils.h"

extern SimulatorClassRefs gSimulatorClassRefs;

SimulatorRemoteResourceSP JniSimulatorRemoteResource::getResourceHandle
(JNIEnv *env, jobject object)
{
    JniSimulatorRemoteResource *jniResource = GetHandle<JniSimulatorRemoteResource>(env, object);
    if (env->ExceptionCheck() || !jniResource)
    {
        return nullptr;
    }

    return jniResource->m_resource;
}

class JNIOnObserveListener
{
    public:
        void setJavaOnObserveListener(JNIEnv *env, jobject listener)
        {
            m_listener = env->NewWeakGlobalRef(listener);
        }

        void onObserveCallback(const std::string &uId, const int errorCode,
                               SimulatorResourceModelSP representation,
                               const int seqNumber)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject onObserveListener = env->NewLocalRef(m_listener);
            if (!onObserveListener)
            {
                releaseEnv();
                return;
            }

            jclass onObserveCls = env->GetObjectClass(onObserveListener);
            if (!onObserveCls)
            {
                releaseEnv();
                return;
            }

            if (OC_STACK_OK != errorCode && OC_STACK_RESOURCE_CREATED != errorCode
                && OC_STACK_RESOURCE_DELETED != errorCode)
            {
                jmethodID midL = env->GetMethodID(onObserveCls, "onObserveFailed", "(Ljava/lang/Throwable;)V");
                if (!midL)
                {
                    releaseEnv();
                    return;
                }
                env->CallVoidMethod(onObserveListener, midL);
            }
            else
            {
                JSimulatorResourceModel *jniModel = new JSimulatorResourceModel(representation);
                if (!jniModel)
                {
                    releaseEnv();
                    return;
                }

                jobject jRepresentation = JSimulatorResourceModel::toJava(env,
                        reinterpret_cast<jlong>(jniModel));
                if (!jRepresentation)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jmethodID midL = env->GetMethodID(onObserveCls, "onObserveCompleted",
                        "(Ljava/lang/String;Lorg/oic/simulator/SimulatorResourceModel;I)V");
                if (!midL)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jstring jUid = env->NewStringUTF(uId.c_str());

                env->CallVoidMethod(onObserveListener, midL, jUid, jRepresentation,
                                    static_cast<jint>(seqNumber));
                if (env->ExceptionCheck())
                {
                    delete jniModel;
                    releaseEnv();
                }
            }
        }

    private:
        jweak m_listener;
};

class JNIOnGetListener
{
    public:
        void setJavaOnGetListener(JNIEnv *env, jobject listener)
        {
            m_listener = env->NewWeakGlobalRef(listener);
        }

        void onGetCallback(const std::string &uId, int errorCode,
                           SimulatorResourceModelSP representation)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject onGetListener = env->NewLocalRef(m_listener);
            if (!onGetListener)
            {
                releaseEnv();
                return;
            }

            jclass onGetCls = env->GetObjectClass(onGetListener);
            if (!onGetCls)
            {
                releaseEnv();
                return;
            }

            // TODO: Revisit is it required?
            if (OC_STACK_OK != errorCode && OC_STACK_RESOURCE_CREATED != errorCode
                && OC_STACK_RESOURCE_DELETED != errorCode)
            {
                jmethodID midL = env->GetMethodID(onGetCls, "onGetFailed", "(Ljava/lang/Throwable;)V");
                if (!midL)
                {
                    releaseEnv();
                    return;
                }
                env->CallVoidMethod(onGetListener, midL);
            }
            else
            {
                JSimulatorResourceModel *jniModel = new JSimulatorResourceModel(representation);
                if (!jniModel)
                {
                    releaseEnv();
                    return;
                }

                jobject jRepresentation = JSimulatorResourceModel::toJava(env, reinterpret_cast<jlong>(jniModel));
                if (!jRepresentation)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jmethodID midL = env->GetMethodID(onGetCls, "onGetCompleted",
                                                  "(Ljava/lang/String;Lorg/oic/simulator/SimulatorResourceModel;)V");
                if (!midL)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jstring jUid = env->NewStringUTF(uId.c_str());
                env->CallVoidMethod(onGetListener, midL, jUid, jRepresentation);
                if (env->ExceptionCheck())
                {
                    delete jniModel;
                    releaseEnv();
                }
            }
        }

    private:
        jweak m_listener;
};

class JNIOnPutListener
{
    public:
        void setJavaOnPutListener(JNIEnv *env, jobject listener)
        {
            m_listener = env->NewWeakGlobalRef(listener);
        }

        void onPutCallback(const std::string &uId, int errorCode,
                           SimulatorResourceModelSP representation)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject onPutListener = env->NewLocalRef(m_listener);
            if (!onPutListener)
            {
                releaseEnv();
                return;
            }

            jclass onGetCls = env->GetObjectClass(onPutListener);
            if (!onGetCls)
            {
                releaseEnv();
                return;
            }

            // TODO: Revisit is it required?
            if (OC_STACK_OK != errorCode && OC_STACK_RESOURCE_CREATED != errorCode
                && OC_STACK_RESOURCE_DELETED != errorCode)
            {
                jmethodID midL = env->GetMethodID(onGetCls, "onPutFailed", "(Ljava/lang/Throwable;)V");
                if (!midL)
                {
                    releaseEnv();
                    return;
                }
                env->CallVoidMethod(onPutListener, midL);
            }
            else
            {
                JSimulatorResourceModel *jniModel = new JSimulatorResourceModel(representation);
                if (!jniModel)
                {
                    releaseEnv();
                    return;
                }

                jobject jRepresentation = JSimulatorResourceModel::toJava(env,
                        reinterpret_cast<jlong>(jniModel));
                if (!jRepresentation)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jmethodID midL = env->GetMethodID(onGetCls, "onPutCompleted",
                            "(Ljava/lang/String;Lorg/oic/simulator/SimulatorResourceModel;)V");
                if (!midL)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jstring jUid = env->NewStringUTF(uId.c_str());
                env->CallVoidMethod(onPutListener, midL, jUid, jRepresentation);
                if (env->ExceptionCheck())
                {
                    delete jniModel;
                    releaseEnv();
                }
            }
        }

    private:
        jweak m_listener;
};

class JNIOnPostListener
{
    public:
        void setJavaOnPostListener(JNIEnv *env, jobject listener)
        {
            m_listener = env->NewWeakGlobalRef(listener);
        }

        void onPostCallback(const std::string &uId, int errorCode,
                            SimulatorResourceModelSP representation)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject onPostListener = env->NewLocalRef(m_listener);
            if (!onPostListener)
            {
                releaseEnv();
                return;
            }

            jclass onGetCls = env->GetObjectClass(onPostListener);
            if (!onGetCls)
            {
                releaseEnv();
                return;
            }

            // TODO: Revisit is it required?
            if (OC_STACK_OK != errorCode && OC_STACK_RESOURCE_CREATED != errorCode
                && OC_STACK_RESOURCE_DELETED != errorCode)
            {
                jmethodID midL = env->GetMethodID(onGetCls, "onPostFailed", "(Ljava/lang/Throwable;)V");
                if (!midL)
                {
                    releaseEnv();
                    return;
                }
                env->CallVoidMethod(onPostListener, midL);
            }
            else
            {
                JSimulatorResourceModel *jniModel = new JSimulatorResourceModel(representation);
                if (!jniModel)
                {
                    releaseEnv();
                    return;
                }

                jobject jRepresentation = JSimulatorResourceModel::toJava(env,
                        reinterpret_cast<jlong>(jniModel));
                if (!jRepresentation)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jmethodID midL = env->GetMethodID(onGetCls, "onPostCompleted",
                        "(Ljava/lang/String;Lorg/oic/simulator/SimulatorResourceModel;)V");
                if (!midL)
                {
                    delete jniModel;
                    releaseEnv();
                    return;
                }

                jstring jUid = env->NewStringUTF(uId.c_str());

                env->CallVoidMethod(onPostListener, midL, jUid, jRepresentation);
                if (env->ExceptionCheck())
                {
                    delete jniModel;
                    releaseEnv();
                }
            }
        }

    private:
        jweak m_listener;

};

class JNIOnVerificationListener
{
    public:
        void setJavaOnVerificationListener(JNIEnv *env, jobject listener)
        {
            m_listener = env->NewWeakGlobalRef(listener);
        }

        void onVerificationCallback(const std::string &uId, int id, OperationState opState)
        {
            JNIEnv *env = getEnv();
            if (nullptr == env)
                return;

            jobject onVerificationListener = env->NewLocalRef(m_listener);
            if (!onVerificationListener)
            {
                releaseEnv();
                return;
            }

            jclass onVerificationCls = env->GetObjectClass(onVerificationListener);
            if (!onVerificationCls)
            {
                releaseEnv();
                return;
            }

            jmethodID midL;

            if (OP_START == opState)
            {
                midL = env->GetMethodID(onVerificationCls, "onVerificationStarted", "(Ljava/lang/String;I)V");
            }
            else if (OP_COMPLETE == opState)
            {
                midL = env->GetMethodID(onVerificationCls, "onVerificationCompleted", "(Ljava/lang/String;I)V");
            }
            else
            {
                midL = env->GetMethodID(onVerificationCls, "onVerificationAborted", "(Ljava/lang/String;I)V");
            }

            if (!midL)
            {
                releaseEnv();
                return;
            }

            jstring jUid = env->NewStringUTF(uId.c_str());

            env->CallVoidMethod(onVerificationListener, midL, jUid, (jint)id);

            if (env->ExceptionCheck())
            {
                releaseEnv();
            }
        }

    private:
        jweak m_listener;

};


JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_startObserve
(JNIEnv *env, jobject thiz, jint observeType, jobject jQueryParamsMap, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::map<std::string, std::string> queryParams;
    if (jQueryParamsMap)
        convertJavaMapToQueryParamsMap(env, jQueryParamsMap, queryParams);

    ObserveType type = ObserveType::OBSERVE;
    if (1 == observeType)
        type = ObserveType::OBSERVE_ALL;

    JNIOnObserveListener *onObserveListener = new JNIOnObserveListener();
    onObserveListener->setJavaOnObserveListener(env, jListener);

    try
    {
        resource->observe(type,
                          std::bind(&JNIOnObserveListener::onObserveCallback,
                                    onObserveListener, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3,
                                    std::placeholders::_4));
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
    }
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_stopObserve
(JNIEnv *env, jobject thiz)
{
    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    try
    {
        resource->cancelObserve();
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_nativeGet
(JNIEnv *env, jobject thiz, jstring jResourceInterface,
 jobject jQueryParamsMap, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    // Interface type
    const char *interfaceCStr = NULL;
    std::string interfaceType;
    if (jResourceInterface)
    {
        interfaceCStr = env->GetStringUTFChars(jResourceInterface, NULL);
        interfaceType = interfaceCStr;
    }

    // Query parameters
    std::map<std::string, std::string> queryParams;
    if (jQueryParamsMap)
        convertJavaMapToQueryParamsMap(env, jQueryParamsMap, queryParams);

    // Create listener
    JNIOnGetListener *onGetListener = new JNIOnGetListener();
    onGetListener->setJavaOnGetListener(env, jListener);

    try
    {
        resource->get(interfaceType,
                      queryParams,
                      std::bind(&JNIOnGetListener::onGetCallback,
                                onGetListener, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3));
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return;
    }
    catch (NoSupportException &e)
    {
        throwNoSupportException(env, e.code(), e.what());
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

    if (interfaceCStr)
        env->ReleaseStringUTFChars(jResourceInterface, interfaceCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_nativePut
(JNIEnv *env, jobject thiz, jstring jResourceInterface,
 jobject jRepresentation, jobject jQueryParamsMap, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    // Interface type
    const char *interfaceCStr = NULL;
    std::string interfaceType;
    if (jResourceInterface)
    {
        interfaceCStr = env->GetStringUTFChars(jResourceInterface, NULL);
        interfaceType = interfaceCStr;
    }

    // Query parameters
    std::map<std::string, std::string> queryParams;
    if (jQueryParamsMap)
        convertJavaMapToQueryParamsMap(env, jQueryParamsMap, queryParams);

    SimulatorResourceModelSP resourceModel =
        JSimulatorResourceModel::getResourceModelPtr(env, jRepresentation);

    // Create listener
    JNIOnPutListener *onPutListener = new JNIOnPutListener();
    onPutListener->setJavaOnPutListener(env, jListener);

    try
    {
        resource->put(interfaceType,
                      queryParams,
                      resourceModel,
                      std::bind(&JNIOnPutListener::onPutCallback,
                                onPutListener, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3));
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return;
    }
    catch (NoSupportException &e)
    {
        throwNoSupportException(env, e.code(), e.what());
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

    if (interfaceCStr)
        env->ReleaseStringUTFChars(jResourceInterface, interfaceCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_nativePost
(JNIEnv *env, jobject thiz, jstring jResourceInterface,
 jobject jRepresentation, jobject jQueryParamsMap, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return;
    }

    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    // Interface type
    const char *interfaceCStr = NULL;
    std::string interfaceType;
    if (jResourceInterface)
    {
        interfaceCStr = env->GetStringUTFChars(jResourceInterface, NULL);
        interfaceType = interfaceCStr;
    }

    // Query parameters
    std::map<std::string, std::string> queryParams;
    if (jQueryParamsMap)
        convertJavaMapToQueryParamsMap(env, jQueryParamsMap, queryParams);

    SimulatorResourceModelSP resourceModel =
        JSimulatorResourceModel::getResourceModelPtr(env, jRepresentation);

    // Create listener
    JNIOnPostListener *onPostListener = new JNIOnPostListener();
    onPostListener->setJavaOnPostListener(env, jListener);

    try
    {
        resource->post(interfaceType,
                       queryParams,
                       resourceModel,
                       std::bind(&JNIOnPostListener::onPostCallback,
                                 onPostListener, std::placeholders::_1,
                                 std::placeholders::_2, std::placeholders::_3));
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return;
    }
    catch (NoSupportException &e)
    {
        throwNoSupportException(env, e.code(), e.what());
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

    if (interfaceCStr)
        env->ReleaseStringUTFChars(jResourceInterface, interfaceCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_setConfigInfo
(JNIEnv *env, jobject thiz, jstring jConfigPath)
{
    if (!jConfigPath)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM,
                                  "Configuration file path is empty!");
        return;
    }

    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    // Interface type
    const char *configCStr = NULL;
    std::string configPath;
    if (jConfigPath)
    {
        configCStr = env->GetStringUTFChars(jConfigPath, NULL);
        configPath = configCStr;
    }

    try
    {
        resource->configure(configPath);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return;
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return;
    }

    if (configCStr)
        env->ReleaseStringUTFChars(jConfigPath, configCStr);
}

JNIEXPORT jint JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_startVerification
(JNIEnv *env, jobject thiz, jint jReqType, jobject jListener)
{
    if (!jListener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
        return SIMULATOR_INVALID_CALLBACK;
    }

    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return SIMULATOR_BAD_OBJECT;
    }

    // Convert RequestType
    RequestType reqType;

    switch (jReqType)
    {
        case 0:
            reqType = RequestType::RQ_TYPE_GET;
            break;

        case 1:
            reqType = RequestType::RQ_TYPE_PUT;
            break;

        case 2:
            reqType = RequestType::RQ_TYPE_POST;
            break;

        case 3:
            reqType = RequestType::RQ_TYPE_DELETE;
            break;

        default:
            return -1;
    }

    // Create listener
    JNIOnVerificationListener *onVerificationListener = new JNIOnVerificationListener();
    onVerificationListener->setJavaOnVerificationListener(env, jListener);

    int automationId = -1;

    try
    {
        automationId = resource->startVerification(reqType,
                       std::bind(&JNIOnVerificationListener::onVerificationCallback,
                                 onVerificationListener, std::placeholders::_1,
                                 std::placeholders::_2, std::placeholders::_3));
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (NoSupportException &e)
    {
        throwNoSupportException(env, e.code(), e.what());
    }
    catch (OperationInProgressException &e)
    {
        throwOperationInProgressException(env, e.code(), e.what());
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }

    return automationId;
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_stopVerification
(JNIEnv *env, jobject thiz, jint jId)
{
    SimulatorRemoteResourceSP resource = JniSimulatorRemoteResource::getResourceHandle(env,
                                         thiz);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    try
    {
        resource->stopVerification((int)jId);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (NoSupportException &e)
    {
        throwNoSupportException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }
}

