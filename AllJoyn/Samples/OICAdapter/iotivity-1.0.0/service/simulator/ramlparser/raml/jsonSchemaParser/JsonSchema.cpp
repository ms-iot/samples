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

#include "JsonSchema.h"
using namespace std;

namespace RAML
{

    void JsonSchema::readJson()
    {
        if (! m_cjson)
            return;

        cJSON *jsonId = cJSON_GetObjectItem(m_cjson, "id");
        if (jsonId)
        {
            m_id = jsonId->valuestring;
        }
        cJSON *jsonSchema = cJSON_GetObjectItem(m_cjson, "$schema");
        if (jsonSchema)
        {
            m_schema = jsonSchema->valuestring;
        }
        cJSON *jsonTitle = cJSON_GetObjectItem(m_cjson, "title");
        if (jsonTitle)
        {
            m_title = jsonTitle->valuestring;
        }
        cJSON *jsonType = cJSON_GetObjectItem(m_cjson, "type");
        if (jsonType)
        {
            m_type = jsonType->valuestring;
        }
        cJSON *jsonDescription = cJSON_GetObjectItem(m_cjson, "description");
        if (jsonDescription)
        {
            m_description = jsonDescription->valuestring;
        }
        cJSON *jsonDefinitions = cJSON_GetObjectItem(m_cjson, "definitions");
        if (jsonDefinitions)
        {
            cJSON *childDefinitions = jsonDefinitions->child;
            while (childDefinitions)
            {
                std::string defName = childDefinitions->string;
                addDefinition(defName, readDef(childDefinitions, defName));
                childDefinitions = childDefinitions->next;
            }
        }
        cJSON *jsonProperties = cJSON_GetObjectItem(m_cjson, "properties");
        if (jsonProperties)
        {
            cJSON *childProperties = jsonProperties->child;
            while (childProperties)
            {
                std::string attName = childProperties->string;
                addProperty(attName, readProp(childProperties, attName));
                childProperties = childProperties->next;
            }
        }
        if (m_type == "array")
        {
            cJSON *jsonItems = cJSON_GetObjectItem(m_cjson, "items");
            if (jsonItems)
            {
                if (jsonItems->type == 5)
                {
                    int item_size = cJSON_GetArraySize(jsonItems);
                    int item_index = 0;
                    do
                    {
                        cJSON *item = cJSON_GetArrayItem(jsonItems, item_index);
                        setItem(readItems(item));
                    }
                    while ( ++item_index < item_size);
                }
                else
                {
                    setItem(readItems(jsonItems));
                }
            }
        }
        cJSON *jsonAdditionalProperties = cJSON_GetObjectItem(m_cjson, "additionalProperties");
        if (jsonAdditionalProperties)
            m_additionalProperties = jsonAdditionalProperties->type;
        else
            m_additionalProperties = cJSON_True;

        cJSON *jsonReference = cJSON_GetObjectItem(m_cjson, "$ref");
        if (jsonReference)
        {
            readJsonRef(jsonReference);
        }
        cJSON *jsonAllOf = cJSON_GetObjectItem(m_cjson, "allOf");
        if (jsonAllOf)
        {
            readAllOf(jsonAllOf);
        }
        cJSON *jsonRequiredValues = cJSON_GetObjectItem(m_cjson, "required");
        if (jsonRequiredValues)
        {
            int size = cJSON_GetArraySize(jsonRequiredValues);
            int index = 0;
            do
            {
                setRequiredValue(cJSON_GetArrayItem(jsonRequiredValues, index)->valuestring);
            }
            while ( ++index < size);
        }
    }

    DefinitionsPtr JsonSchema::readDef(cJSON *childDefinitions, const std::string &defName)
    {
        DefinitionsPtr definition = std::make_shared<Definitions>(defName);

        cJSON *defType = cJSON_GetObjectItem(childDefinitions, "type");
        if (defType)
        {
            std::string type = defType->valuestring;
            definition->setType(type);
        }
        cJSON *defProperties = cJSON_GetObjectItem(childDefinitions, "properties");
        if (defProperties)
        {
            cJSON *childProperties = defProperties->child;
            while (childProperties)
            {
                std::string attName = childProperties->string;
                definition->addProperty(attName, readProp(childProperties, attName));
                childProperties = childProperties->next;
            }
        }
        cJSON *defRequiredValues = cJSON_GetObjectItem(childDefinitions, "required");
        if (defRequiredValues)
        {
            int size = cJSON_GetArraySize(defRequiredValues);
            int index = 0;
            do
            {
                definition->setRequiredValue(cJSON_GetArrayItem(defRequiredValues, index)->valuestring);
            }
            while ( ++index < size);
        }
        cJSON *defReference = cJSON_GetObjectItem(childDefinitions, "$ref");
        if (defReference)
        {
            readDefRef(defReference, definition);
        }
        cJSON *defAllOf = cJSON_GetObjectItem(childDefinitions, "allOf");
        if (defAllOf)
        {
            readDefAllOf(defAllOf, definition);
        }
        return definition;
    }

    PropertiesPtr JsonSchema::readProp(cJSON *childProperties, const std::string &attName )
    {
        PropertiesPtr property = std::make_shared<Properties>(attName);

        cJSON *propertyDescription = cJSON_GetObjectItem(childProperties, "description");
        if (propertyDescription)
        {
            property->setDescription(propertyDescription->valuestring);
        }
        cJSON *propertyType = cJSON_GetObjectItem(childProperties, "type");
        if (propertyType)
        {
            std::string attType;
            if (propertyType->type == 4)
            {
                attType = propertyType->valuestring;
                property->setType(attType);
            }
            else if (propertyType->type == 5)
            {
                attType = cJSON_GetArrayItem(propertyType, 0)->valuestring;
                property->setType(attType);
            }
            readValues(childProperties, property, attType);
        }
        cJSON *defaultValue = cJSON_GetObjectItem(childProperties, "default");
        if (defaultValue)
        {
            if (defaultValue->type == 4)
            {
                property->setValue((std::string)defaultValue->valuestring);
            }
            else if (defaultValue->type == 3)
            {
                if (property->getType() == "number")
                    property->setValue((double)defaultValue->valuedouble);
                else
                    property->setValue((int)defaultValue->valueint );
            }
            else if (defaultValue->type == 1)
            {
                property->setValue((bool)true);
            }
            else if (defaultValue->type == 0)
            {
                property->setValue((bool)false);
            }

        }
        cJSON *allowedvalues = cJSON_GetObjectItem(childProperties, "enum");
        if (allowedvalues)
        {
            if ((cJSON_GetArrayItem(allowedvalues, 0)->type) == 4)
            {
                int size = cJSON_GetArraySize(allowedvalues);
                int idx = 0;
                std::vector<std::string> allwdValues;
                do
                {
                    allwdValues.push_back(cJSON_GetArrayItem(allowedvalues, idx)->valuestring);
                }
                while ( ++idx < size);
                property->setAllowedValues(allwdValues);
            }
            else if ((cJSON_GetArrayItem(allowedvalues, 0)->type) == 3)
            {
                int size = cJSON_GetArraySize(allowedvalues);
                int idx = 0;
                if (property->getType() == "number")
                {
                    std::vector<double> allwdValues;
                    do
                    {
                        allwdValues.push_back(cJSON_GetArrayItem(allowedvalues, idx)->valuedouble);
                    }
                    while ( ++idx < size);
                    property->setAllowedValues(allwdValues);
                }
                else
                {
                    std::vector<int> allwdValues;
                    do
                    {
                        allwdValues.push_back(cJSON_GetArrayItem(allowedvalues, idx)->valueint);
                    }
                    while ( ++idx < size);
                    property->setAllowedValues(allwdValues);
                }
            }
            else if (((cJSON_GetArrayItem(allowedvalues, 0)->type) == 1)
                     || ((cJSON_GetArrayItem(allowedvalues, 0)->type) == 0))
            {
                int size = cJSON_GetArraySize(allowedvalues);
                int idx = 0;
                std::vector<bool> allwdValues;
                do
                {
                    if (cJSON_GetArrayItem(allowedvalues, idx)->type)
                        allwdValues.push_back(true);
                    else
                        allwdValues.push_back(false);
                }
                while ( ++idx < size);
                property->setAllowedValues(allwdValues);
            }
        }
        return property;
    }

    void JsonSchema::readValues(cJSON *childProperties,  PropertiesPtr property ,
                                const std::string &attType)
    {
        if (attType == "string")
        {
            readString(childProperties, property);
        }
        else if (attType == "integer")
        {
            readInteger(childProperties, property);
        }
        else if (attType == "array")
        {
            readArray(childProperties, property);
        }
        else if (attType == "number")
        {
            readDouble(childProperties, property);
        }
    }

    void JsonSchema::readString(cJSON *childProperties, PropertiesPtr property)
    {
        cJSON *stringMax = cJSON_GetObjectItem(childProperties, "maxLength");
        if (stringMax)
        {
            cJSON *exclusiveMax = cJSON_GetObjectItem(childProperties, "exclusiveMaximum");
            if (exclusiveMax)
            {
                if (exclusiveMax->type == cJSON_True)
                    property->setMax (--(stringMax->valueint));
                else
                    property->setMax(stringMax->valueint);
            }
            else
                property->setMax(stringMax->valueint);
        }
        cJSON *stringMin = cJSON_GetObjectItem(childProperties, "minLength");
        if (stringMin)
        {
            cJSON *exclusiveMin = cJSON_GetObjectItem(childProperties, "exclusiveMinimum");
            if (exclusiveMin)
            {
                if (exclusiveMin->type == cJSON_True)
                    property->setMin( ++(stringMin->valueint));
                else
                    property->setMin(stringMin->valueint);
            }
            else
                property->setMin(stringMin->valueint);
        }
        cJSON *stringFormat = cJSON_GetObjectItem(childProperties, "format");
        if (stringFormat)
        {
            property->setFormat(stringFormat->valuestring);
        }
        cJSON *stringPattern = cJSON_GetObjectItem(childProperties, "pattern");
        if (stringPattern)
        {
            property->setPattern(stringPattern->valuestring);
        }
    }

    void JsonSchema::readArray(cJSON *childProperties,  PropertiesPtr property)
    {
        cJSON *itemValues = cJSON_GetObjectItem(childProperties, "items");
        if (itemValues)
        {
            if (itemValues->type == 5)
            {
                int item_size = cJSON_GetArraySize(itemValues);
                int item_index = 0;
                do
                {
                    cJSON *item = cJSON_GetArrayItem(itemValues, item_index);
                    property->setItem(readItems(item));
                }
                while ( ++item_index < item_size);
            }
            else
            {
                property->setItem(readItems(itemValues));
            }
        }
        cJSON *itemsMax = cJSON_GetObjectItem(childProperties, "maxItems");
        if (itemsMax)
        {
            cJSON *exclusiveMax = cJSON_GetObjectItem(childProperties, "exclusiveMaximum");
            if (exclusiveMax)
            {
                if (exclusiveMax->type == cJSON_True)
                    property->setMax( --(itemsMax->valueint));
                else
                    property->setMax(itemsMax->valueint);
            }
            else
                property->setMax(itemsMax->valueint);
        }
        cJSON *itemsMin = cJSON_GetObjectItem(childProperties, "minLength");
        if (itemsMin)
        {
            cJSON *exclusiveMin = cJSON_GetObjectItem(childProperties, "exclusiveMinimum");
            if (exclusiveMin)
            {
                if (exclusiveMin->type == cJSON_True)
                    property->setMin( ++(itemsMin->valueint));
                else
                    property->setMin(itemsMin->valueint);
            }
            else
                property->setMin(itemsMin->valueint);
        }
        cJSON *uniqueItems = cJSON_GetObjectItem(childProperties, "uniqueItems");
        if (uniqueItems)
        {
            property->setUnique(uniqueItems->type);
        }
        else
        {
            property->setUnique(cJSON_True);
        }
        cJSON *additionalItems = cJSON_GetObjectItem(childProperties, "additionalItems");
        if (additionalItems)
        {
            property->setAdditionalItems(additionalItems->type);
        }
        else
        {
            property->setAdditionalItems(cJSON_True);
        }
    }

    void JsonSchema::readInteger(cJSON *childProperties,  PropertiesPtr property)
    {
        cJSON *Max = cJSON_GetObjectItem(childProperties, "maximum");
        if (Max)
        {
            cJSON *exclusiveMax = cJSON_GetObjectItem(childProperties, "exclusiveMaximum");
            if (exclusiveMax)
            {
                if (exclusiveMax->type == cJSON_True)
                    property->setMax( --(Max->valueint));
                else
                    property->setMax(Max->valueint);
            }
            else
                property->setMax(Max->valueint);
        }
        cJSON *Min = cJSON_GetObjectItem(childProperties, "minimum");
        if (Min)
        {
            cJSON *exclusiveMin = cJSON_GetObjectItem(childProperties, "exclusiveMinimum");
            if (exclusiveMin)
            {
                if (exclusiveMin->type == cJSON_True)
                    property->setMin( ++(Min->valueint));
                else
                    property->setMin(Min->valueint);
            }
            else
                property->setMin(Min->valueint);
        }
        cJSON *multipleOf = cJSON_GetObjectItem(childProperties, "multipleOf");
        if (multipleOf)
        {
            property->setMultipleOf(multipleOf->valueint);
        }

    }

    void JsonSchema::readDouble(cJSON *childProperties,  PropertiesPtr property)
    {
        cJSON *Max = cJSON_GetObjectItem(childProperties, "maximum");
        if (Max)
        {
            cJSON *exclusiveMax = cJSON_GetObjectItem(childProperties, "exclusiveMaximum");
            if (exclusiveMax)
            {
                if (exclusiveMax->type == cJSON_True)
                    property->setMaxDouble( --(Max->valuedouble));
                else
                    property->setMaxDouble(Max->valuedouble);
            }
            else
                property->setMaxDouble(Max->valuedouble);
        }
        cJSON *Min = cJSON_GetObjectItem(childProperties, "minimum");
        if (Min)
        {
            cJSON *exclusiveMin = cJSON_GetObjectItem(childProperties, "exclusiveMinimum");
            if (exclusiveMin)
            {
                if (exclusiveMin->type == cJSON_True)
                    property->setMinDouble( ++(Min->valuedouble));
                else
                    property->setMinDouble(Min->valuedouble);
            }
            else
                property->setMinDouble(Min->valuedouble);
        }
        cJSON *multipleOf = cJSON_GetObjectItem(childProperties, "multipleOf");
        if (multipleOf)
        {
            property->setMultipleOf(multipleOf->valueint);
        }

    }

    DefinitionsPtr JsonSchema::readRef(std::string m_ref)
    {
        std::string delimiter1 = "#";
        std::string delimiter2 = "/";
        std::string fileName;
        if (! m_ref.empty())
        {
            std::size_t pos = m_ref.find(delimiter1);
            if ( (pos = m_ref.find(delimiter1)) != std::string::npos)
            {
                fileName = m_ref.substr(0, pos);
                m_ref.erase(0, pos);
            }
            m_ref.erase(0, delimiter1 .length());
            std::string defName;

            if (! m_ref.empty())
            {
                m_ref.erase(0, delimiter2 .length());
                std::string keyName;
                if ( (pos = m_ref.find(delimiter2)) != std::string::npos)
                {
                    keyName = m_ref.substr(0, pos);
                    m_ref.erase(0, pos + delimiter2.length());
                    if (keyName == "definitions")
                    {
                        if ( (pos = m_ref.find(delimiter2)) != std::string::npos)
                        {
                            defName = m_ref.substr(0, pos);
                        }
                        else if (! m_ref.empty())
                        {
                            defName = m_ref;
                        }
                    }
                }
            }
            if (!fileName.empty())
            {
                if (!(defName.empty()))
                {
                    cJSON *m_json = m_includeResolver->readToJson(fileName);
                    JsonSchemaPtr Refparser = std::make_shared<JsonSchema>(m_json, m_includeResolver);
                    DefinitionsPtr definition = Refparser->getDefinition(defName);
                    if (definition == nullptr)
                        throw JsonException("Definition Name Incorrect");
                    return definition;
                }
            }
            else
            {
                if (!(defName.empty()))
                {
                    if (getDefinition(defName) == nullptr)
                        throw JsonException("Definition Name Incorrect");
                    return getDefinition(defName);
                }
            }
        }
        throw JsonException("Definition Name Empty");
        return nullptr;
    }
    void JsonSchema::readAllOf(cJSON *allofValues)
    {
        int size = cJSON_GetArraySize(allofValues);
        int index = 0;
        do
        {
            cJSON *childAllOf = cJSON_GetArrayItem(allofValues, index);
            cJSON *jsonReference = cJSON_GetObjectItem(childAllOf, "$ref");
            if (jsonReference)
            {
                readJsonRef(jsonReference );
            }
            cJSON *jsonRequiredValues = cJSON_GetObjectItem(childAllOf, "required");
            if (jsonRequiredValues)
            {
                int len = cJSON_GetArraySize(jsonRequiredValues);
                int idx = 0;
                do
                {
                    setRequiredValue(cJSON_GetArrayItem(jsonRequiredValues, idx)->valuestring);
                }
                while ( ++idx < len);
            }
        }
        while ( ++index < size);
    }
    void JsonSchema::readJsonRef(cJSON *jsonReference)
    {
        std::string m_ref = jsonReference->valuestring;
        std::map<std::string, PropertiesPtr > properties;
        std::vector<std::string> required;

        std::string web = "http://";
        std::string delimiter = "#";
        std::size_t pos = m_ref.find(web);

        if (pos == std::string::npos)   // If Web Link Is GIVEN TO READ
        {
            pos = m_ref.find(delimiter);
            if ( pos ==  (m_ref.length() - 1) )
            {
                std::string fileName = m_ref.substr(0, pos);
                cJSON *m_json = m_includeResolver->readToJson(fileName);
                JsonSchemaPtr Refparser = std::make_shared<JsonSchema>(m_json, m_includeResolver);

                properties = Refparser->getProperties();
                required = Refparser->getRequiredValues();
            }
            else
            {
                DefinitionsPtr definition = readRef(m_ref);
                properties = definition->getProperties();
                required = definition->getRequiredValues();
            }
            for ( auto it : properties)
            {
                std:: string name = it.first;
                addProperty(name, it.second);
            }
            for (auto it : required )
            {
                setRequiredValue(it);
            }

        }
    }
    void JsonSchema::readDefAllOf(cJSON *allofValues, DefinitionsPtr definition)
    {
        int size = cJSON_GetArraySize(allofValues);
        int index = 0;
        do
        {
            cJSON *childAllOf = cJSON_GetArrayItem(allofValues, index);
            cJSON *defReference = cJSON_GetObjectItem(childAllOf, "$ref");
            if (defReference)
            {
                readDefRef(defReference , definition);
            }
            cJSON *defRequiredValues = cJSON_GetObjectItem(allofValues, "required");
            if (defRequiredValues)
            {
                int len = cJSON_GetArraySize(defRequiredValues);
                int idx = 0;
                do
                {
                    definition->setRequiredValue(cJSON_GetArrayItem(defRequiredValues, idx)->valuestring);
                }
                while ( ++idx < len);
            }
        }
        while ( ++index < size);
    }
    void JsonSchema::readDefRef(cJSON *defReference, DefinitionsPtr definition)
    {
        std::string m_ref = defReference->valuestring;
        std::map<std::string, PropertiesPtr > properties;
        std::vector<std::string> required;
        std::string type;

        std::string web = "http://";
        std::string delimiter = "#";
        std::size_t pos = m_ref.find(web);

        if (pos == std::string::npos)   // If Web Link Is GIVEN TO READ
        {
            pos = m_ref.find(delimiter);
            if ( pos ==  (m_ref.length() - 1) )
            {
                std::string fileName = m_ref.substr(0, pos);
                cJSON *m_json = m_includeResolver->readToJson(fileName);
                JsonSchemaPtr Refparser = std::make_shared<JsonSchema>(m_json, m_includeResolver);

                properties = Refparser->getProperties();
                required = Refparser->getRequiredValues();
                type =    Refparser->getType();
            }
            else
            {
                DefinitionsPtr definitionRef = readRef(m_ref);
                properties = definitionRef->getProperties();
                required = definitionRef->getRequiredValues();
                type =    definitionRef->getType();
            }
            for (auto it : properties)
            {
                definition->addProperty(it.first, it.second);
            }
            for ( auto it : required)
            {
                definition->setRequiredValue(it);
            }
            definition->setType(type);
        }
    }
    ItemsPtr JsonSchema::readItems(cJSON *item)
    {
        ItemsPtr newItem = std::make_shared<Items>();
        cJSON *itemType = cJSON_GetObjectItem(item, "type");
        if (itemType)
        {
            std::string type = itemType->valuestring;
            newItem->setType(type);
        }

        cJSON *itemProperties = cJSON_GetObjectItem(item, "properties");
        if (itemProperties)
        {
            cJSON *childProperties = itemProperties->child;
            while (childProperties)
            {
                std::string attName = childProperties->string;

                newItem->addProperty(attName, readProp(childProperties, attName));
                childProperties = childProperties->next;
            }
        }

        cJSON *allowedvalues = cJSON_GetObjectItem(item, "enum");
        if (allowedvalues)
        {
            if ((cJSON_GetArrayItem(allowedvalues, 0)->type) == 4)
            {
                int size = cJSON_GetArraySize(allowedvalues);
                int idx = 0;
                std::vector<std::string> allwdValues;
                do
                {
                    allwdValues.push_back(cJSON_GetArrayItem(allowedvalues, idx)->valuestring);
                }
                while ( ++idx < size);
                newItem->setAllowedValues(allwdValues);
            }
            else if ((cJSON_GetArrayItem(allowedvalues, 0)->type) == 3)
            {
                int size = cJSON_GetArraySize(allowedvalues);
                int idx = 0;
                if (newItem->getType() == "number")
                {
                    std::vector<double> allwdValues;
                    do
                    {
                        allwdValues.push_back(cJSON_GetArrayItem(allowedvalues, idx)->valuedouble);
                    }
                    while ( ++idx < size);
                    newItem->setAllowedValues(allwdValues);
                }
                else
                {
                    std::vector<int> allwdValues;
                    do
                    {
                        allwdValues.push_back(cJSON_GetArrayItem(allowedvalues, idx)->valueint);
                    }
                    while ( ++idx < size);
                    newItem->setAllowedValues(allwdValues);
                }
            }
            else if (((cJSON_GetArrayItem(allowedvalues, 0)->type) == 1)
                     || ((cJSON_GetArrayItem(allowedvalues, 0)->type) == 0))
            {
                int size = cJSON_GetArraySize(allowedvalues);
                int idx = 0;
                std::vector<bool> allwdValues;
                do
                {
                    if (cJSON_GetArrayItem(allowedvalues, idx)->type)
                        allwdValues.push_back(true);
                    else
                        allwdValues.push_back(false);
                }
                while ( ++idx < size);
                newItem->setAllowedValues(allwdValues);
            }
        }
        cJSON *itemRequiredValues = cJSON_GetObjectItem(item, "required");
        if (itemRequiredValues)
        {
            int size = cJSON_GetArraySize(itemRequiredValues);
            int index = 0;
            do
            {
                newItem->setRequiredValue(cJSON_GetArrayItem(itemRequiredValues, index)->valuestring);
            }
            while ( ++index < size);
        }
        cJSON *itemReference = cJSON_GetObjectItem(item, "$ref");
        if (itemReference)
        {
            readItemRef(itemReference , newItem);
        }
        cJSON *itemAllOf = cJSON_GetObjectItem(item, "allOf");
        if (itemAllOf)
        {
            readItemAllOf(itemAllOf , newItem);
        }
        return newItem;
    }

    void JsonSchema::readItemRef(cJSON *itemReference, ItemsPtr item)
    {
        std::string m_ref = itemReference->valuestring;
        std::map<std::string, PropertiesPtr > properties;
        std::vector<std::string> required;
        std::string type;

        std::string web = "http://";
        std::string delimiter = "#";
        std::size_t pos = m_ref.find(web);

        if (pos == std::string::npos)   // If Web Link Is GIVEN TO READ
        {
            pos = m_ref.find(delimiter);
            if ( pos ==  (m_ref.length() - 1 ) )
            {
                std::string fileName = m_ref.substr(0, pos);
                cJSON *m_json = m_includeResolver->readToJson(fileName);
                JsonSchemaPtr Refparser = std::make_shared<JsonSchema>(m_json, m_includeResolver);

                properties = Refparser->getProperties();
                required = Refparser->getRequiredValues();
                type =    Refparser->getType();
            }
            else
            {
                DefinitionsPtr definitionRef = readRef(m_ref);
                properties = definitionRef->getProperties();
                required = definitionRef->getRequiredValues();
                type =    definitionRef->getType();
            }
            for ( auto it : properties)
            {
                std:: string name = it.first;
                item->addProperty(name, it.second);
            }
            for ( auto it : required)
            {
                item->setRequiredValue(it);
            }
            item->setType(type);
        }
    }

    void JsonSchema::readItemAllOf(cJSON *allofValues, ItemsPtr item)
    {
        int size = cJSON_GetArraySize(allofValues);
        int index = 0;
        do
        {
            cJSON *childAllOf = cJSON_GetArrayItem(allofValues, index);
            cJSON *itemReference = cJSON_GetObjectItem(childAllOf, "$ref");
            if (itemReference)
            {
                readItemRef(itemReference, item);
            }
            cJSON *itemRequiredValues = cJSON_GetObjectItem(allofValues, "required");
            if (itemRequiredValues)
            {
                int len = cJSON_GetArraySize(itemRequiredValues);
                int idx = 0;
                do
                {
                    item->setRequiredValue(cJSON_GetArrayItem(itemRequiredValues, idx)->valuestring);
                }
                while ( ++idx < len);
            }
        }
        while ( ++index < size);
    }
}

