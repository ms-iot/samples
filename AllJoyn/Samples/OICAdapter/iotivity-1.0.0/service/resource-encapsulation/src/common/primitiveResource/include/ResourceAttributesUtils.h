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

#ifndef COMMON_RESOURCEATTRIBUTESUTILS_H
#define COMMON_RESOURCEATTRIBUTESUTILS_H

#include <vector>
#include <string>

#include <RCSResourceAttributes.h>

namespace OIC
{
    namespace Service
    {
        typedef std::pair< std::string, RCSResourceAttributes::Value > AttrKeyValuePair;
        typedef std::vector< AttrKeyValuePair > AttrKeyValuePairs;

        bool acceptableAttributes(const RCSResourceAttributes& dest, const RCSResourceAttributes& attr);

        bool acceptableAttributeValue(const RCSResourceAttributes::Value& dest,
                const RCSResourceAttributes::Value& value);

        AttrKeyValuePairs replaceAttributes(RCSResourceAttributes& dest,
                const RCSResourceAttributes& attrs);
    }
}

#endif // COMMON_RESOURCEATTRIBUTESUTILS_H
