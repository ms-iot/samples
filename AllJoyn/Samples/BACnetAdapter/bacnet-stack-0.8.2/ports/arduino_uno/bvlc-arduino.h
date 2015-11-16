/**
 * @file
 * @author Miguel Fernandes <miguelandre.fernandes@gmail.com>
 * @date 6 de Jun de 2013
 * @brief BACnet Virtual Link Control for Wiznet on Arduino-Uno
 */
#ifndef BVLCARDUINO_H_
#define BVLCARDUINO_H_

#include <stdint.h>
#include "bacenum.h"
#include "bacdef.h"
#include "npdu.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint16_t bvlc_for_non_bbmd(uint8_t * addr,
    uint16_t * port,
    uint8_t * npdu,
    uint16_t received_bytes);

BACNET_BVLC_FUNCTION bvlc_get_function_code(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* BVLCARDUINO_H_ */
