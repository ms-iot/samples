/**************************************************************************
*
* Copyright (C) 2005 John Goulah
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/
#ifndef BACPROP_H
#define BACPROP_H

#include <stdbool.h>
#include <stdint.h>
#include "bacenum.h"

typedef struct {
    signed prop_id;     /* index number that matches the text */
    signed tag_id;      /* text pair - use NULL to end the list */
} PROP_TAG_DATA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    signed bacprop_tag_by_index_default(
        PROP_TAG_DATA * data_list,
        signed index,
        signed default_ret);

    signed bacprop_property_tag(
        BACNET_OBJECT_TYPE type,
        signed prop);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
