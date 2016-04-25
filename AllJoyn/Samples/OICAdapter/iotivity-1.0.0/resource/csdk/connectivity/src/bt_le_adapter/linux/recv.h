/******************************************************************
 *
 * Copyright 2015 Intel Corporation All Rights Reserved.
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

#ifndef CA_BLE_LINUX_RECV_H
#define CA_BLE_LINUX_RECV_H

#include "context.h"

#include "caleinterface.h"

#include <stdint.h>
#include <stdlib.h>


/**
 * Information used to keep track of received data fragments.
 */
typedef struct _CAGattRecvInfo
{

    /**
     * @name Peer Connection-specific Fields
     *
     * These fields are valid only as long as the peer is connected.
     */
    //@{
    /// Peer address.
    char * peer;

    /// Callback invoked upon receiving all data from GATT peer.
    CABLEDataReceivedCallback on_packet_received;

    /**
     * Context object containing lock used for synchronized access to
     * the @c on_packet_received callback since that callback is actually
     * owned by it.
     */
    CALEContext * context;
    //@}

} CAGattRecvInfo;

/**
 * Initialize a @c CAGattRecvInfo object.
 *
 * @param[in] info Pointer to @c CAGattRecvInfo object being
 *                 initialized.  No memory is allocated by this
 *                 function.  The caller is responsible for
 *                 instantiating the object prior to calling this
 *                 function.
 */
void CAGattRecvInfoInitialize(CAGattRecvInfo * info);

/**
 * Destroy a @c CAGattRecvInfo object.
 *
 * Destruction of @a info involves deallocating and clearing out all
 * fields, as necessary.
 *
 * @param[in] info Pointer to @c CAGattRecvInfo object being
 *                 destroyed.  Only the @a info fields are finalized.
 *                 Memory of @a info itself is retained by the
 *                 caller.
 */
void CAGattRecvInfoDestroy(CAGattRecvInfo * info);

/**
 * Handle data received from GATT a peer.
 *
 * @param[in] info        Information required to complete the receive
 *                        operation.
 * @param[in] data        Octet array containing received data
 *                        fragment.
 * @param[in] length      Length of the @a data array.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CAGattRecv(CAGattRecvInfo * info,
                uint8_t const * data,
                uint32_t length);


#endif  /* CA_BLE_LINUX_RECV_H */
