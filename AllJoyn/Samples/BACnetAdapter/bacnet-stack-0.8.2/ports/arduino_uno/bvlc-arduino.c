/**
 * @file
 * @author Miguel Fernandes <miguelandre.fernandes@gmail.com>
 * @date 6 de Jun de 2013
 * @brief BACnet Virtual Link Control for Wiznet on Arduino-Uno
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "bvlc-arduino.h"
#include "bip.h"
#include "bacint.h"
#include "socketWrapper.h"
#include "w5100Wrapper.h"

/** result from a client request */
BACNET_BVLC_RESULT BVLC_Result_Code = BVLC_RESULT_SUCCESSFUL_COMPLETION;
/** The current BVLC Function Code being handled. */
BACNET_BVLC_FUNCTION BVLC_Function_Code = BVLC_RESULT;  /* A safe default */

static int bvlc_encode_bvlc_result(uint8_t * pdu,
    BACNET_BVLC_RESULT result_code)
{
    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_RESULT;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 6);
        encode_unsigned16(&pdu[4], (uint16_t) result_code);
    }

    return 6;
}

static int bvlc_send_mpdu(uint8_t * dest_addr,  /* the destination address */

    uint16_t * dest_port,       /* the destination port */

    uint8_t * mtu,      /* the data */

    uint16_t mtu_len)
{       /* amount of data to send  */
    /* assumes that the driver has already been initialized */
    if (bip_valid()) {
        return 0;
    }

    return sendto_func(bip_socket(), mtu, mtu_len, dest_addr, *dest_port);
}

static void bvlc_send_result(uint8_t * dest_addr,
    uint16_t * dest_port,       /* the destination address */

    BACNET_BVLC_RESULT result_code)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t) bvlc_encode_bvlc_result(&mtu[0], result_code);
    bvlc_send_mpdu(dest_addr, dest_port, mtu, mtu_len);

    return;
}

uint16_t bvlc_for_non_bbmd(uint8_t * addr,
    uint16_t * port,
    uint8_t * npdu,
    uint16_t received_bytes)
{

    uint16_t result_code = 0;   /* aka, BVLC_RESULT_SUCCESSFUL_COMPLETION */
    BVLC_Function_Code = npdu[1];       /* The BVLC function */
    switch (BVLC_Function_Code) {
        case BVLC_RESULT:
            if (received_bytes >= 6) {
                /* This is the result of our foreign device registration */
                (void) decode_unsigned16(&npdu[4], &result_code);
                BVLC_Result_Code = (BACNET_BVLC_RESULT) result_code;
                fprintf(stderr, "BVLC: Result Code=%d\n", BVLC_Result_Code);
                /* But don't send any response */
                result_code = 0;
            }
            break;
        case BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE:
            result_code = BVLC_RESULT_WRITE_BROADCAST_DISTRIBUTION_TABLE_NAK;
            break;
        case BVLC_READ_BROADCAST_DIST_TABLE:
            result_code = BVLC_RESULT_READ_BROADCAST_DISTRIBUTION_TABLE_NAK;
            break;
            /* case BVLC_READ_BROADCAST_DIST_TABLE_ACK: */
        case BVLC_REGISTER_FOREIGN_DEVICE:
            result_code = BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK;
            break;
        case BVLC_READ_FOREIGN_DEVICE_TABLE:
            result_code = BVLC_RESULT_READ_FOREIGN_DEVICE_TABLE_NAK;
            break;
            /* case BVLC_READ_FOREIGN_DEVICE_TABLE_ACK: */
        case BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY:
            result_code = BVLC_RESULT_DELETE_FOREIGN_DEVICE_TABLE_ENTRY_NAK;
            break;
        case BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK:
            result_code = BVLC_RESULT_DISTRIBUTE_BROADCAST_TO_NETWORK_NAK;
            break;
            /* case BVLC_FORWARDED_NPDU: */
            /* case BVLC_ORIGINAL_UNICAST_NPDU: */
            /* case BVLC_ORIGINAL_BROADCAST_NPDU: */
        default:
            break;
    }

    if (result_code > 0) {
        bvlc_send_result(addr, port, result_code);
        fprintf(stderr, "BVLC: NAK code=%d\n", result_code);
    }
    return result_code;
}

BACNET_BVLC_FUNCTION bvlc_get_function_code(void)
{
    return BVLC_Function_Code;
}
