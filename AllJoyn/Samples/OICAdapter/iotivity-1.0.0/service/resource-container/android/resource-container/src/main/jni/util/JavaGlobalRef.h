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

#ifndef JAVA_GLOBAL_REF_H_
#define JAVA_GLOBAL_REF_H_

#include <memory>
#include <cassert>

#include "ScopedEnv.h"

class JavaGlobalRef
{
    public:
    JavaGlobalRef(JNIEnv *env, jobject obj) noexcept :
        m_obj { }
        {
            assert(env  &&"JNIEnv is nullptr");

            static auto deleter = [](jobject * obj)
            {
                if (obj && *obj)
                {
                    ScopedEnv env;
                    env->DeleteGlobalRef(*obj);
                }
                delete obj;
            };

            m_obj.reset(new jobject{ env->NewGlobalRef(obj) }, deleter);
        }

        operator jobject() const noexcept
        {
            return *m_obj;
        }

    private:
        std::shared_ptr< jobject > m_obj;
};



#endif // JAVA_GLOBAL_REF_H_
