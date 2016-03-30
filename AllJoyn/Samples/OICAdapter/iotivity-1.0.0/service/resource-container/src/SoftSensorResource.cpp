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

#include "SoftSensorResource.h"
#include <algorithm>

using namespace OIC::Service;

namespace
{
    const std::string SS_RESOURCE_OUTPUT = std::string("output");
    const std::string SS_RESOURCE_OUTPUTNAME = std::string("name");
}

namespace OIC
{
    namespace Service
    {
        SoftSensorResource::SoftSensorResource()
        {

        }

        SoftSensorResource::~SoftSensorResource()
        {

        }

        void SoftSensorResource::initAttributes()
        {
            std::vector< std::map< std::string, std::string > >::iterator itor;

            // initialize output attributes
            for (itor = m_mapResourceProperty[SS_RESOURCE_OUTPUT].begin();
                 itor != m_mapResourceProperty[SS_RESOURCE_OUTPUT].end(); itor++)
                BundleResource::setAttribute((*itor)[SS_RESOURCE_OUTPUTNAME], nullptr);
        }
    }
}

