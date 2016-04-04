/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
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
#include "JniOcSecurity.h"
#include "JniOcStack.h"

/*
 * TODO: Persistant Storage Handling should be done by App.
 * For 0.9.2 , Handling is done at JNI. As of now Plaform Config only
 * SVR Database fileName(fullpath) is passed.
 */
using namespace std;
namespace PH = std::placeholders;
namespace OC {

    string& JniOcSecurity::store_path()
    {
        static string s_dbPath;
        return s_dbPath;
    }

    void JniOcSecurity::StoreDbPath(const string &path)
    {
        store_path() = path;
    }

    OCPersistentStorage* JniOcSecurity::getOCPersistentStorage()
    {
        if (store_path().empty())
        {
            return nullptr;
        }
        static OCPersistentStorage s_ps { &JniOcSecurity::client_open, fread,
            fwrite, fclose, unlink };
        return &s_ps;
    }

    FILE* JniOcSecurity::client_open(const char *path, const char *mode)
    {
        LOGI("Opening SVR Database file '%s' with mode '%s'\n", store_path().c_str(), mode);
        return fopen(store_path().c_str(), mode);
    }
}
