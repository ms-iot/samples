/*
* //******************************************************************
* //
* // Copyright 2015 Intel Corporation.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* //
* // Licensed under the Apache License, Version 2.0 (the "License");
* // you may not use this file except in compliance with the License.
* // You may obtain a copy of the License at
* //
* //      http://www.apache.org/licenses/LICENSE-2.0
* //
* // Unless required by applicable law or agreed to in writing, software
* // distributed under the License is distributed on an "AS IS" BASIS,
* // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* // See the License for the specific language governing permissions and
* // limitations under the License.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

#include "JniOcStack.h"
#include <mutex>

#ifndef _JniListenerManager
#define _JniListenerManager

class JniOcResource;

template <class T>
class JniListenerManager
{
public:
    T* addListener(JNIEnv* env, jobject jListener, JniOcResource* owner)
    {
        T *onEventListener = nullptr;

        m_mapMutex.lock();

        for (auto it = m_listenerMap.begin(); it != m_listenerMap.end(); ++it)
        {
            if (env->IsSameObject(jListener, it->first))
            {
                auto refPair = it->second;
                onEventListener = refPair.first;
                refPair.second++;
                it->second = refPair;
                m_listenerMap.insert(*it);
                LOGD("OnEventListener: ref. count is incremented");
                break;
            }
        }
        if (!onEventListener)
        {
            onEventListener = new T(env, jListener, owner);
            jobject jgListener = env->NewGlobalRef(jListener);

            if (jgListener)
            {
                m_listenerMap.insert(
                        std::pair<jobject,
                        std::pair<T*, int>>(jgListener, std::pair<T*, int>(onEventListener, 1)));
            }
            else
            {
                LOGD("OnEventListener: Failed to create global listener ref.");
                delete onEventListener;
            }
            LOGD("OnEventListener: new listener");
        }
        m_mapMutex.unlock();
        return onEventListener;
    }

    void removeListener(JNIEnv* env, jobject jListener)
    {
        m_mapMutex.lock();
        for (auto it = m_listenerMap.begin(); it != m_listenerMap.end(); ++it)
        {
            if (env->IsSameObject(jListener, it->first))
            {
                auto refPair = it->second;
                if (refPair.second > 1)
                {
                    refPair.second--;
                    it->second = refPair;
                    m_listenerMap.insert(*it);
                    LOGI("OnEventListener: ref. count is decremented");
                }
                else
                {
                    env->DeleteGlobalRef(it->first);
                    T* listener = refPair.first;
                    delete listener;
                    m_listenerMap.erase(it);

                    LOGI("OnEventListener is removed");
                }
                break;
            }
        }
        m_mapMutex.unlock();
    }

    void removeAllListeners(JNIEnv* env)
    {
        m_mapMutex.lock();

        for (auto& pair : m_listenerMap)
        {
            env->DeleteGlobalRef(pair.first);
            auto refPair = pair.second;
            delete refPair.first;
        }
        m_listenerMap.clear();

        m_mapMutex.unlock();
    }

private:
    std::map<jobject, std::pair<T*, int>> m_listenerMap;
    std::mutex m_mapMutex;
};

#endif
