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

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace std;

namespace OIC
{
    namespace Service
    {
        typedef vector< map< string, string > > configInfo;

        struct resourceInfo
        {
            string name;
            string uri;
            string resourceType;
            string address;
            map< string, vector< map< string, string > > > resourceProperty;
        };

        class Configuration
        {
            public:
                Configuration();
                Configuration(string configFile);
                ~Configuration();

                bool isLoaded() const;
                bool isHasInput(std::string & bundleId) const;
                void getConfiguredBundles(configInfo *configOutput);
                void getBundleConfiguration(string bundleId, configInfo *configOutput);
                void getResourceConfiguration(string bundleId, vector< resourceInfo > *configOutput);

            private:
                void getConfigDocument(string pathConfigFile);

                bool m_loaded;
                string m_pathConfigFile;
                string m_strConfigData;
                rapidxml::xml_document< char > m_xmlDoc;
                std::map<std::string, bool> m_mapisHasInput; // bundleId, isHasInput
        };
    }
}

#endif /* CONFIGURATION_H_ */
