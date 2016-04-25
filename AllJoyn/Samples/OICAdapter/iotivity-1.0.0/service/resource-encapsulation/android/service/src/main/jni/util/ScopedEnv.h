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

#ifndef RCS_JIN_SCOPEDENV_H_
#define RCS_JIN_SCOPEDENV_H_

#include <utility>

#include <jni.h>

#include "JNIEnvWrapper.h"
#include "Log.h"

extern JavaVM* g_jvm;

namespace Detail
{
    inline std::pair<JNIEnv*, bool> getEnv()
    {
        JNIEnv* env{ };
        bool needToDetach{ };

        auto ret = g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6);

        switch (ret)
        {
            case JNI_OK:
                break;

            case JNI_EDETACHED:
            {
                auto attachRet = g_jvm->AttachCurrentThread(&env, NULL);

                if (attachRet != JNI_OK)
                {
                    LOGT_E("JNI-ScopedEnv", "Failed to get the environment : %d", attachRet);
                }
                else
                {
                    needToDetach = true;
                }
                break;
            }
            case JNI_EVERSION:
                LOGT_E("JNI-ScopedEnv", "JNI version not supported");
                break;

            default:
                LOGT_E("JNI-ScopedEnv", "Failed to get the environment");
                break;
        }

        return { env, needToDetach };
    }
}

class ScopedEnv
{
public:
    ScopedEnv() noexcept :
        m_env { },
        m_needToDetach{ false }
    {
        auto val = Detail::getEnv();

        m_env = val.first;
        m_needToDetach = val.second;
    }

    ~ScopedEnv()
    {
        if (m_env && m_needToDetach)
        {
            g_jvm->DetachCurrentThread();
        }
    }

    ScopedEnv(const ScopedEnv&) = delete;
    ScopedEnv& operator=(const ScopedEnv&) = delete;

    operator bool() const noexcept
    {
        return m_env;
    }

    JNIEnv* operator->() noexcept
    {
        return m_env;
    }

    JNIEnv* get() noexcept
    {
        return m_env;
    }

private:
    JNIEnv* m_env;
    bool m_needToDetach;
};

class ScopedEnvWrapper
{
public:
    ScopedEnvWrapper() noexcept :
        m_env { },
        m_needToDetach{ false }
    {
        auto val = Detail::getEnv();

        m_env = val.first;
        m_needToDetach = val.second;
    }

    ~ScopedEnvWrapper()
    {
        if (m_env && m_needToDetach)
        {
            g_jvm->DetachCurrentThread();
        }
    }

    ScopedEnvWrapper(const ScopedEnvWrapper&) = delete;
    ScopedEnvWrapper& operator=(const ScopedEnvWrapper&) = delete;

    operator bool() const noexcept
    {
        return m_env;
    }

    JNIEnvWrapper* operator->() noexcept
    {
        return &m_env;
    }

    JNIEnvWrapper* get() noexcept
    {
        return &m_env;
    }

private:
    JNIEnvWrapper m_env;
    bool m_needToDetach;
};

#endif // RCS_JIN_SCOPEDENV_H_
