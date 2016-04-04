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

#ifndef REQUEST_LIST_H_
#define REQUEST_LIST_H_

#include <map>
#include <mutex>

template <typename T>
class RequestList
{
    public:
        RequestList() : m_id(0) {}

        int add(T request)
        {
            if (!request)
                return -1;

            std::lock_guard<std::recursive_mutex> lock(m_listMutex);
            m_requestList[m_id++] = request;
            return m_id - 1;
        }

        T get(int id)
        {
            std::lock_guard<std::recursive_mutex> lock(m_listMutex);
            if (m_requestList.end() != m_requestList.find(id))
                return m_requestList[id];

            return nullptr;
        }

        T remove(int id)
        {
            std::lock_guard<std::recursive_mutex> lock(m_listMutex);
            if (m_requestList.end() != m_requestList.find(id))
            {
                T request = m_requestList[id];
                m_requestList.erase(m_requestList.find(id));
                return request;
            }

            return nullptr;
        }

        int size()
        {
            std::lock_guard<std::recursive_mutex> lock(m_listMutex);
            return m_requestList.size();
        }

        void clear()
        {
            std::lock_guard<std::recursive_mutex> lock(m_listMutex);
            m_requestList.clear();
        }

    private:
        int m_id;
        std::recursive_mutex m_listMutex;
        std::map<int, T> m_requestList;
};

#endif
