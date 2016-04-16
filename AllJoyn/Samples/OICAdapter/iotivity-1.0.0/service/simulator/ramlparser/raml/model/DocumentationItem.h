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

/**
 * @file   DocumentationItem.h
 *
 * @brief   This file provides data Model for RAML DocumentationItem.
 */

#ifndef DOCUMENTATION_ITEM_H
#define DOCUMENTATION_ITEM_H

#include <string>


namespace RAML
{
    /**
     * @class   DocumentationItem
     * @brief   This class provides data Model for RAML DocumentationItem.
     */
    class DocumentationItem
    {

        public:
            /**
                 * This method is for getting Title from DocumentationItem.
                 *
                 * @return title as string.
                 */
            virtual std::string getTitle() const;

            /**
                 * This method is for setting Title to DocumentationItem.
                 *
                 * @param title - title as string
                 */
            virtual void setTitle(const std::string &title);

            /**
                 * This method is for getting Content from DocumentationItem.
                 *
                 * @return content as string
                 */
            virtual std::string getContent() const;

            /**
                 * This method is for setting Content to DocumentationItem.
                 *
                 * @param content - content as string
                 */
            virtual void setContent(const std::string &content);

            /**
                  * Constructor of DocumentationItem.
                  */
            DocumentationItem();

            /**
                   * Constructor of Action.
                   *
                   * @param title - title of Documentation Item
                   * @param content - content of Documentation Item
                   *
                   */
            DocumentationItem(const std::string &title , const std::string &content)
                : m_title(title), m_content(content) {}
        private:
            std::string m_title;
            std::string m_content;

    };

}
#endif
