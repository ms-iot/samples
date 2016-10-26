/**
* @file
* @author Steve Karg <skarg@users.sourceforge.net>
* @date 2013
* @brief Store and retrieve non-volatile data
*
*/
#include <stdint.h>
#include <stdbool.h>
#include "nvmdata.h"
#include "dlmstp.h"
#include "device.h"

/**
* Initializes the non-volatile memory module
*/
void nvm_data_init(void)
{
    uint32_t device_id = 127;
    uint8_t max_master = 127;
    uint8_t mac_address = 127;

    nvm_read(NVM_MAC_ADDRESS, &mac_address, 1);
    if (mac_address == 255) {
        /* uninitialized */
        mac_address = 123;
    }
    dlmstp_set_mac_address(mac_address);
    nvm_read(NVM_MAX_MASTER, &max_master, 1);
    if (max_master > 127) {
        max_master = 127;
    }
    dlmstp_set_max_master(max_master);
    /* Get the device ID from the EEPROM */
    nvm_read(NVM_DEVICE_0, (uint8_t *) & device_id, sizeof(device_id));
    if (device_id < BACNET_MAX_INSTANCE) {
        Device_Set_Object_Instance_Number(device_id);
    } else {
        Device_Set_Object_Instance_Number(mac_address);
    }
}
