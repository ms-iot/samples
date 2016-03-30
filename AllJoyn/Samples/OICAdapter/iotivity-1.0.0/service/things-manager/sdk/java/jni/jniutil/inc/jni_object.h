/* *****************************************************************
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

/**
  * @file
  * This file contains the JObject class  declarations and its functions required
  * for getting and setting basic data types in C++ and Java.
  */

#ifndef JNI_OBJECT_H_
#define JNI_OBJECT_H_

#include <jni.h>
#include "JniOcResource.h"

/**
 * This class provides a set of functions for JNI object.
 */
class JObject
{
    public:
        /**
         * constructor
         */
        JObject(JNIEnv *env);

        /**
         * constructor
         */
        JObject(JNIEnv *env, jobject obj);

        /**
         * constructor
         */
        JObject(JNIEnv *env, const char *classPath);

        /**
         * destructor
         */
        virtual ~JObject();

        /**
         * Function to get the jobject.
         *
         * @return jobject, returns a new JNI object or NULL otherwise.
         */
        virtual jobject getObject() const;

        /**
         * Function to detach the jobject.
         */
        void detachObject();

    protected:
        /**
         *  JNI Environment Pointer
         */
        JNIEnv *m_pEnv;
        /**
         *  Java Object
         */
        jobject m_pObject;
        /**
         *  Java Class
         */
        jclass  m_pClazz;
        /**
         *  Boolean variable to check if an object is new.
         */
        bool m_fIsNewObject;
};
#endif //JNI_OBJECT_H_
