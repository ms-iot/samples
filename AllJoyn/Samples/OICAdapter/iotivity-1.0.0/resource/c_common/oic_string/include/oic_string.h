/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
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
#ifndef OIC_STRING_H_
#define OIC_STRING_H_

#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * Duplicates the source string and returns it.
 *
 * @note Caller needs to release this memory by calling OICFree().
 *
 * @param str Original valid string which needs to be duplicated.
 *
 * @return a pointer to the duplicated string
 */
char *OICStrdup(const char *str);

/**
 * Copies a C string into destination buffer.  Ensures that the destination
 * is null terminated.
 *
 * @param dest Destination C buffer.
 * @param destSize The allocated size of the destination parameter.
 * @param source Source C string.
 *
 * @return
 *      returns a pointer to the passed in 'dest' parameter
 */
char* OICStrcpy(char* dest, size_t destSize, const char* source);

/**
 * Appends a C string into a previously allocated and initialized C string in a buffer. dest
 * parameter is guaranteed to be null-terminated.
 *
 * @param dest Destination C buffer containing initial string.
 * @param destSize The allocated size of the destination parameter.
 * @param source Source C string.
 *
 * @return
 *      returns a pointer to the passed in 'dest' parameter
 */
char* OICStrcat(char* dest, size_t destSize, const char* source);

/**
 * Copies a partial C string into destination buffer.
 * Ensures that the destination is null terminated.
 *
 * @param dest Destination C buffer.
 * @param destSize The allocated size of the destination parameter.
 * @param source Source C string.
 * @param sourceLen maximum number of characters to copy.
 *
 * @return
 *      returns a pointer to the passed in 'dest' parameter
 */
char* OICStrcpyPartial(char* dest, size_t destSize, const char* source, size_t sourceLen);

/**
 * Appends a C string into a previously allocated and initialized C string in a buffer. dest
 * parameter is guaranteed to be null-terminated.
 *
 * @param dest Destination C buffer containing initial string.
 * @param destSize The allocated size of the destination parameter.
 * @param source Source C string.
 * @param sourceLen maximum number of characters to append, not including null termination
 *
 * @return
 *      returns a pointer to the passed in 'dest' parameter
 */
char* OICStrcatPartial(char* dest, size_t destSize, const char* source, size_t sourceLen);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // OIC_STRING_H_
