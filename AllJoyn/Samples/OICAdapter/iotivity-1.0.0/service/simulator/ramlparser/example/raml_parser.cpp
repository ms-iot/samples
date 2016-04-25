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

#include "RamlParser.h"
#include "Helpers.h"
#include <iostream>
#include <string>
#include <memory>

//#define PRINT_PARAMS
//#define PRINT_PROTOCOLS
//#define PRINT_BASEURI
//#define PRINT_DOCUMENTATION
//#define PRINT_TYPES
//#define PRINT_TRAITS
//#define PRINT_RESOURCE_URI_BASEURI
//#define PRINT_ACTION_QUERY_PARAM
//#define PRINT_RESPONSE_HEADER
//#define PRINT_REQUEST_RESPONSE_BODY_PARAMS
//#define PRINT_ACTION_HEADERS
//#define PRINT_SCHEMAS
#define PRINT_RAML
#define PRINT_JSON
//#define PRINT_JSON_PROPERTIES

using namespace RAML;

void printParameters(AbstractParam abstractParam)
{

#ifdef PRINT_PARAMS
    std::cout << "Description : "  << abstractParam.getDescription()  << std::endl;
    std::cout << "DefaultValue : "  << abstractParam.getDefaultValue()  << std::endl;
    std::cout << "Example : " << abstractParam.getExample()  << std::endl;
    std::cout << "displayName : "  << abstractParam.getDisplayName()  << std::endl;
    std::cout << "Maxlength : "  << abstractParam.getMaxLength()  << std::endl;
    std::cout << "Max : "  << abstractParam.getMaximum()  << std::endl;
    std::cout << "Minlength : "  << abstractParam.getMinLength()  << std::endl;
    std::cout << "Min : "  << abstractParam.getMinimum()  << std::endl;
    std::cout << "Pattern : "  << abstractParam.getPattern()  << std::endl;
    std::cout << "Type : "  << abstractParam.getType()  << std::endl;
    std::cout << "Repeat : "  << abstractParam.isRepeat()  << std::endl;
    std::cout << "Required : "  << abstractParam.isRequired()  << std::endl;
    std::cout << "Enum : "   ;
    for (auto elem : abstractParam.getEnumeration())
        std::cout << elem   << " 	";
    std::cout << std::endl;
#endif
}

void printRequestResponseBody(const RequestResponseBodyPtr &body)
{
    std::cout << "Body : Type : " << body->getType() << std::endl;
    if ( body->getSchema() == NULL ) return;
    std::cout << "Body : Schema : " << body->getSchema()->getSchema() << std::endl;
    std::cout << "Body : Schema : PROPERTIES :" << std::endl;

    for ( auto pro : body->getSchema()->getProperties()->getProperties() )
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << "Name : " << pro.second->getName() << std::endl;
        std::cout << "-----------------------------" << std::endl;
        try
        {
            switch (pro.second->getVariantType())
            {
                case VariantType::INT : // Integer
                    std::cout << "Defaut: " << pro.second->getValueInt() << std::endl;
                    for (auto tt : pro.second->getAllowedValuesInt())
                    {
                        std::cout << "enum value : " << tt << std::endl;
                    }
                    {
                        int min = 0, max = 0, mul = 0;
                        pro.second->getRange(min, max, mul);
                        std::cout << "Minimum: " << min << std::endl;
                        std::cout << "Maximum: " << max << std::endl;
                    }
                    break;

                case VariantType::DOUBLE : // Double
                    std::cout << "Defaut: " << pro.second->getValueDouble() << std::endl;
                    for (auto tt : pro.second->getAllowedValuesDouble())
                    {
                        std::cout << "enum value : " << tt << std::endl;
                    }
                    {
                        double min = 0, max = 0;
                        int mul = 0;
                        pro.second->getRangeDouble(min, max, mul);
                        std::cout << "MinimumDouble: " << min << std::endl;
                        std::cout << "MaximumDouble: " << max << std::endl;
                    }
                    break;

                case VariantType::BOOL : // Boolean
                    std::cout << "Defaut: " << std::boolalpha << pro.second->getValueBool() << std::noboolalpha <<
                              std::endl;
                    for (auto tt : pro.second->getAllowedValuesBool())
                    {
                        std::cout << "enum value : " << tt << std::endl;
                    }
                    break;

                case VariantType::STRING : // String
                    std::cout << "Defaut: " << pro.second->getValueString() << std::endl;
                    for (auto tt : pro.second->getAllowedValuesString())
                    {
                        std::cout << "enum value : " << tt << std::endl;
                    }
                    {
                        int min = 0, max = 0, mul = 0;
                        pro.second->getRange(min, max, mul);
                        std::cout << "MinimumLength: " << min << std::endl;
                        std::cout << "MaximumLength: " << max << std::endl;
                    }
                    break;
                default:
                    break;

            }
        }
        catch (const boost::bad_lexical_cast &e)
        {
            std::cout << e.what() << std::endl;
        }
        catch ( ... )
        {
            std::cout << "Unknown exception caught!" << std::endl;
        }

    }
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Body : example : " << body->getExample() << std::endl;

#ifdef PRINT_REQUEST_RESPONSE_BODY_PARAMS
    std::cout << "Body : FormParameters	"  << std::endl;
    for (auto  tw : body->getFormParameters())
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << tw.first << " : "  << std::endl;
        std::cout << "-----------------------------" << std::endl;
        FormParameter formParameter = *tw.second;
        printParameters((AbstractParam)formParameter);
    }
#endif
}


void printResponse(const ResponsePtr &response)
{
    std::cout << "#############################################" << std::endl;
    std::cout << "Response : Description : " << response->getDescription() << std::endl;

    for (auto  tv :  response->getResponseBody())
        printRequestResponseBody(tv.second);
#ifdef PRINT_RESPONSE_HEADER
    std::cout << "Header" << std::endl;
    for (auto  tw :  response->getHeaders())
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << tw.first << " : "  << std::endl;
        std::cout << "-----------------------------" << std::endl;

        Header header = *tw.second;
        printParameters((AbstractParam)header);
    }
#endif
}

void printAction(const ActionPtr &action)
{
    std::cout << "Description : " << action->getDescription() << std::endl;
    std::cout << "----Action Body--------------" << std::endl;
    for (auto  tv :  action->getRequestBody())
        printRequestResponseBody(tv.second);
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Responses	" << std::endl;
    for (auto  tu :  action->getResponses())
    {
        std::cout << "Response : " << tu.first << std::endl;
        printResponse(tu.second);
    }
#ifdef PRINT_ACTION_QUERY_PARAM
    std::cout << "QueryParameter" << std::endl;
    for (auto  tw :  action->getQueryParameters())
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << tw.first << " : "  << std::endl;
        std::cout << "-----------------------------" << std::endl;
        QueryParameter queryParam = *tw.second;
        printParameters((AbstractParam)queryParam);
    }
#endif
#ifdef PRINT_ACTION_HEADERS
    std::cout << "Headers" << std::endl;
    for (auto  tw :  action->getHeaders())
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << tw.first << " : "  << std::endl;
        std::cout << "-----------------------------" << std::endl;
        Header header = *tw.second;
        printParameters((AbstractParam)header);
    }
#endif

#ifdef PRINT_TRAITS

    std::cout << "Traits  " << std::endl;
    std::cout << "-----------------------------" << std::endl;
    for (auto  tt :  action->getTraits())
    {
        std::cout << tt  << "     ";
    }
    std::cout << std::endl << "-----------------------------" << std::endl;
#endif
}

void printResource(const RamlResourcePtr &resource)
{
    std::cout << "Displayname : " << resource->getDisplayName()   << std::endl;
    std::cout << "Description : " << resource->getDescription()   << std::endl;
#ifdef PRINT_RESOURCE_URI_BASEURI
    std::cout << "#############################################" << std::endl;
    std::cout << "ResourceURI 	"   << resource->getResourceUri() << std::endl;
    std::cout << "UriParameters	"    << std::endl;
    for (auto  tt :  resource->getUriParameters())
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << tt.first << " : "  << std::endl;
        std::cout << "-----------------------------" << std::endl;
        UriParameter uriParameter = *tt.second;
        printParameters((AbstractParam)uriParameter);
    }
    std::cout << "#############################################" << std::endl;
    std::cout << "BaseUriParameters	"    << std::endl;
    for (auto  tt :  resource->getBaseUriParameters())
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << tt.first << " : "  << std::endl;
        std::cout << "-----------------------------" << std::endl;

        UriParameter uriParameter = *tt.second;
        printParameters((AbstractParam)uriParameter);
    }
#endif
    std::cout << "Actions  " << std::endl;
    for (auto  tt :  resource->getActions())
    {
        std::cout << "#############################################" << std::endl;
        std::cout << "ActionsType  " << std::endl;
        printAction(tt.second);
    }
#ifdef PRINT_TRAITS
    std::cout << "Traits  " << std::endl;
    std::cout << "-----------------------------" << std::endl;
    for (auto  tt :  resource->getTraits())
    {
        std::cout << tt  << "     ";
    }
    std::cout << std::endl << "-----------------------------" << std::endl;
#endif
    std::cout << "Number of Child Resource for " << resource->getDisplayName() << " : " <<
              resource->getResources().size() << std::endl;

    for (auto  tt :  resource->getResources())
    {
        std::cout << "Child Resource" << std::endl;
        std::cout << "-----------------------------" << std::endl;
        std::cout << "ResourceName :" << tt.first << std::endl;
        printResource(tt.second);
    }

}
void printProperties(const PropertiesPtr &prop)
{
#ifdef PRINT_JSON_PROPERTIES
    std::cout << "-------------------------------" << std::endl;
#endif
    std::cout << "Name: " << prop->getName() << std::endl;
#ifdef PRINT_JSON_PROPERTIES
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Type: " << prop->getType() << std::endl;
    std::cout << "Description: " << prop->getDescription() << std::endl;
    try
    {
        switch (prop->getVariantType())
        {
            case VariantType::INT : // Integer
                std::cout << "Defaut: " << prop->getValueInt() << std::endl;
                for (auto tt : prop->getAllowedValuesInt())
                {
                    std::cout << "enum value : " << tt << std::endl;
                }
                {
                    int min = 0, max = 0, mul = 0;
                    prop->getRange(min, max, mul);
                    std::cout << "Minimum: " << min << std::endl;
                    std::cout << "Maximum: " << max << std::endl;
                }
                break;

            case VariantType::DOUBLE : // Double
                std::cout << "Defaut: " << prop->getValueDouble() << std::endl;
                for (auto tt : prop->getAllowedValuesDouble())
                {
                    std::cout << "enum value : " << tt << std::endl;
                }
                {
                    double min = 0, max = 0;
                    int mul = 0;
                    prop->getRangeDouble(min, max, mul);
                    std::cout << "MinimumDouble: " << min << std::endl;
                    std::cout << "MaximumDouble: " << max << std::endl;
                }
                break;

            case VariantType::BOOL : // Boolean
                std::cout << "Defaut: " << std::boolalpha << prop->getValueBool() << std::noboolalpha << std::endl;
                for (auto tt : prop->getAllowedValuesBool())
                {
                    std::cout << std::boolalpha << "enum value : " << tt << std::noboolalpha << std::endl;
                }
                break;

            case VariantType::STRING : // String
                std::cout << "Defaut: " << prop->getValueString() << std::endl;
                for (auto tt : prop->getAllowedValuesString())
                {
                    std::cout << "enum value : " << tt << std::endl;
                }
                {
                    int min = 0, max = 0, mul = 0;
                    prop->getRange(min, max, mul);
                    std::cout << "MinimumLength: " << min << std::endl;
                    std::cout << "MaximumLength: " << max << std::endl;
                }
                break;
            default:
                break;

        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        std::cout << e.what() << std::endl;
    }
    catch ( ... )
    {
        std::cout << "Unknown exception caught!" << std::endl;
    }

    if (prop->getType() == "array")
    {
        for (auto it : prop->getItems())
        {
            std::cout << "items Type : " << it->getType() << std::endl;
            if (it->getType() == "string")
                for (auto tt : it->getAllowedValuesString())
                {
                    std::cout << "enum value : " << tt << std::endl;
                }
            for (auto tt : it->getProperties())
            {
                printProperties(tt.second);
            }
            std::cout << "Item Required Values : " << std::endl;
            for (auto tt : it->getRequiredValues())
            {
                std::cout << tt << std::endl;
            }
        }
    }
#endif

}
void printJsonSchema(JsonSchemaPtr js)
{
    std::cout << "##############################" << std::endl;
    std::cout << "------JSON Schema Parser------" << std::endl;
    std::cout << "##############################" << std::endl;

    std::cout << "Id: " << js->getId() << std::endl;
    std::cout << "Schema: " << js->getSchema() << std::endl;
    std::cout << "Title: " << js->getTitle() << std::endl;
    std::cout << "Type: " << js->getType() << std::endl;
    std::cout << "Description: " << js->getDescription() << std::endl;
    std::cout << "AdditionalProperties: " << js->getAdditionalProperties() << std::endl;

    std::cout << "-------------------------------" << std::endl;
    std::cout << "Definitions." << std::endl;
    for (auto  tt : js->getDefinitions())
    {
        std::cout << "-------------------------------" << std::endl;
        std::cout << tt.first << std::endl;
        for (auto  it : tt.second->getProperties())
        {
            printProperties(it.second);
        }
    }

    std::cout << "##############################" << std::endl;
    std::cout << "Properties." << std::endl;
    for (auto  it : js->getProperties())
    {
        printProperties(it.second);
    }
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Required." << std::endl;
    std::cout << "-------------------------------" << std::endl;
    for (auto it : js->getRequiredValues())
    {
        std::cout << it << std::endl;
    }
    std::cout << "-------------------------------" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        return 0;
    }
    char *value = argv[1];
    std::string s(value);

    try
    {
        std::shared_ptr<RamlParser> ramlParser = std::make_shared<RamlParser>(s);
        RamlPtr m_raml = ramlParser->getRamlPtr();
#ifdef PRINT_RAML

        std::cout << "#############################################" << std::endl;
        std::cout << "Test Raml Parser" << std::endl;
        std::cout << "#############################################" << std::endl;

        std::cout << "Title : " << m_raml->getTitle() << std::endl;
        std::cout << "Version : " <<  m_raml->getVersion() << std::endl;
#ifdef PRINT_PROTOCOLS
        std::cout << "Protocols : "   ;
        for (auto  it : m_raml->getProtocols())
        {
            std::cout << it  << "     ";
        }

        std::cout << std::endl;
#endif
#ifdef PRINT_BASEURI
        std::cout << "BaseUri : " <<  m_raml->getBaseUri() << std::endl;

        std::cout << "BaseUriParameters : " << std::endl;
        for (auto  it : m_raml->getBaseUriParameters())
        {
            std::cout << "-----------------------------" << std::endl;
            std::cout << it.first << " : "  << std::endl;
            std::cout << "-----------------------------" << std::endl;
            UriParameter uriParameter = *it.second;
            printParameters((AbstractParam)uriParameter);
        }
#endif
#ifdef PRINT_SCHEMAS
        std::cout << "#############################################" << std::endl;
        std::cout << "Schemas" << std::endl;
        std::cout << "-----------------------------" << std::endl;
        for (auto  it : m_raml->getSchemas())
        {
            std::cout << it.first   << " : " << it.second->getSchema() << std::endl;
        }
#endif
        std::cout << "MediaType : " <<  m_raml->getMediaType() << std::endl;
        std::cout << "#############################################" << std::endl;
#ifdef PRINT_DOCUMENTATION
        std::cout << "#############################################" << std::endl;

        std::cout << "Documentation" << std::endl;
        std::cout << "-----------------------------" << std::endl;
        for (auto  it : m_raml->getDocumentation())
        {
            std::cout << it->getTitle() << " : " << it->getContent() << std::endl;
        }
        std::cout << "#############################################" << std::endl;
#endif

        std::cout << "Resources" << std::endl;
        for (auto  it : m_raml->getResources())
        {
            std::cout << "-----------------------------" << std::endl;
            std::cout << "ResourceName :" << it.first << std::endl;
            printResource(it.second);
        }
#ifdef PRINT_TYPES

        std::cout << "#############################################" << std::endl;

        std::cout << "ResourceTypes " << std::endl;
        for (auto  it : m_raml->getResourceTypes())
        {
            std::cout << "------------" << it.first << "-----------------" << std::endl;
            printResource(it.second);
        }
#endif
#ifdef PRINT_TRAITS

        std::cout << "#############################################" << std::endl;

        std::cout << "Traits " << std::endl;
        for (auto  it : m_raml->getTraits())
        {
            std::cout << "-------------" << it.first << "----------------" << std::endl;
            printAction(it.second);
        }
#endif
#endif
#ifdef PRINT_JSON
        for (auto  it : m_raml->getResources())
        {
            for (auto  tt :  it.second->getActions())
            {
                for (auto  tu :  tt.second->getResponses())
                {
                    for (auto  tv :  tu.second->getResponseBody())
                    {
                        auto pro = tv.second->getSchema()->getProperties();
                        printJsonSchema(pro);
                        break;
                    }
                }
            }
        }
#endif
    }
    catch (RamlException &e)
    {
        std::cout << e.what() << std::endl;
    }

}

