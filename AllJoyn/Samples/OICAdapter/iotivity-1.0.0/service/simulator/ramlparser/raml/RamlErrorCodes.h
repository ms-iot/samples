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
 * @file   RamlErrorCodes.h
 *
 * @brief   This file provides list of error codes while parsing Raml file.
 */

#ifndef RAML_ERROR_CODES_H_
#define RAML_ERROR_CODES_H_

/** RamlParserResult - This enum provides list of error codes from RamlParser*/
typedef enum
{
    RAML_PARSER_OK = 0,
    RAML_FILE_PATH_REQUIRED,
    RAML_PARSER_ERROR = 255
} RamlParserResult;
#endif
