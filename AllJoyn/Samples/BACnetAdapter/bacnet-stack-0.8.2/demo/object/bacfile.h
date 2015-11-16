/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#ifndef BACFILE_H
#define BACFILE_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacenum.h"
#include "apdu.h"
#include "arf.h"
#include "awf.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void BACfile_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);
    bool bacfile_object_name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool bacfile_valid_instance(
        uint32_t object_instance);
    uint32_t bacfile_count(
        void);
    uint32_t bacfile_index_to_instance(
        unsigned find_index);
    unsigned bacfile_instance_to_index(
        uint32_t instance);
    uint32_t bacfile_instance(
        char *filename);
    /* this is one way to match up the invoke ID with */
    /* the file ID from the AtomicReadFile request. */
    /* Another way would be to store the */
    /* invokeID and file instance in a list or table */
    /* when the request was sent */
    uint32_t bacfile_instance_from_tsm(
        uint8_t invokeID);

    /* handler ACK helper */
    bool bacfile_read_stream_data(
        BACNET_ATOMIC_READ_FILE_DATA * data);
    bool bacfile_read_ack_stream_data(
        uint32_t instance,
        BACNET_ATOMIC_READ_FILE_DATA * data);
    bool bacfile_write_stream_data(
        BACNET_ATOMIC_WRITE_FILE_DATA * data);
    bool bacfile_read_record_data(
        BACNET_ATOMIC_READ_FILE_DATA * data);
    bool bacfile_read_ack_record_data(
        uint32_t instance,
        BACNET_ATOMIC_READ_FILE_DATA * data);
    bool bacfile_write_record_data(
        BACNET_ATOMIC_WRITE_FILE_DATA * data);

    void bacfile_init(
        void);
    uint32_t bacfile_file_size(
        uint32_t instance);

    /* handling for read property service */
    int bacfile_read_property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    /* handling for write property service */
    bool bacfile_write_property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
