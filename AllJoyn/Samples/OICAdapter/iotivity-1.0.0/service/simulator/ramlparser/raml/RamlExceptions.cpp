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

#include "RamlExceptions.h"
namespace RAML
{
    const char *RamlException::what() const noexcept
    {
        return m_message.c_str();
    }

    RamlException::RamlException(const YAML::Mark &mark, const std::string &message): m_mark(mark)
    {
        std::stringstream output;
        output << "Error at line " << m_mark.line + 1 << ", column "
               << m_mark.column + 1 << ": " << message;
        m_message = output.str();
    }
}
