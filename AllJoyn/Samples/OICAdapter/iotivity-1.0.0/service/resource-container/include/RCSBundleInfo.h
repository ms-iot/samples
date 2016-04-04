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

/**
 * @file
 *
 * This file contains RCSBundleInfo class, which provides APIs related to Bundle information.
 */

#ifndef RCSBUNDLEINFO_H_
#define RCSBUNDLEINFO_H_

#include <string>

namespace OIC
{
    namespace Service
    {

        /**
        * @class  RCSBundleInfo
        * @brief   This class provides APIs for creating, getting and setting the Bundle Information
        *
        */
        class RCSBundleInfo
        {
            public:
                /**
                * API for getting the Id of the bundle
                *
                * @return Id of the bundle
                *
                */
                virtual const std::string &getID() = 0;

                /**
                * API for getting the path of the bundle
                *
                * @return path of the bundle
                *
                */
                virtual const std::string &getPath() = 0;

                /**
                * API for setting the Activator name for the bundle
                *
                * @return Name of the activator
                *
                */
                virtual const std::string &getActivatorName() = 0;

                /**
                * API for getting the library path for the bundle
                *
                * @return Library path  in string form
                *
                */
                virtual const std::string &getLibraryPath() = 0;

                /**
                * API for getting the version of the bundle
                *
                * @return version of the bundle
                *
                */
                virtual const std::string &getVersion() = 0;

                RCSBundleInfo();
                virtual ~RCSBundleInfo();
            protected:
                std::string m_ID, m_path, m_version;


        };
    }
}

#endif /* RCSBUNDLEINFO_H_ */
