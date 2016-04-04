/**
 * \file
 *
 * \brief Non volatile memories management for XMEGA devices
 *
 * Copyright (c) 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include "common_nvm.h"
#include "conf_nvm.h"
#include "nvm.h"

status_code_t nvm_init(mem_type_t mem)
{
	switch (mem) {
	case INT_FLASH:
	case INT_USERPAGE:
	case INT_EEPROM:
		/* No initialization required for internal memory */
		break;

#if defined(USE_EXTMEM) && defined(CONF_BOARD_AT45DBX)
	case AT45DBX:
		/* Initialize dataflash */
		at45dbx_init();
		/* Perform memory check */
		if (!at45dbx_mem_check()) {
			return ERR_NO_MEMORY;
		}
		break;
#endif

	default:
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_read_char(mem_type_t mem, uint32_t address, uint8_t *data)
{
	switch (mem) {
	case INT_FLASH:
		*data = nvm_flash_read_byte((flash_addr_t)address);
		break;

	case INT_USERPAGE:
		nvm_user_sig_read_buffer((flash_addr_t)address, (void *)data,
				1);
		break;

	case INT_EEPROM:
		*data = nvm_eeprom_read_byte((eeprom_addr_t)address);
		break;

#if defined(USE_EXTMEM) && defined(CONF_BOARD_AT45DBX)
	case AT45DBX:
		if (!at45dbx_read_byte_open(address)) {
			return ERR_BAD_ADDRESS;
		}

		*data = at45dbx_read_byte();
		at45dbx_read_close();
		break;
#endif

	default:
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_write_char(mem_type_t mem, uint32_t address, uint8_t data)
{
	switch (mem) {
	case INT_FLASH:
		nvm_flash_erase_and_write_buffer((flash_addr_t)address,
				(const void *)&data, 1, true);
		break;

	case INT_USERPAGE:
		nvm_user_sig_write_buffer((flash_addr_t)address,
				(const void *)&data, 1, true);
		break;

	case INT_EEPROM:
		nvm_eeprom_write_byte((eeprom_addr_t)address, data);
		break;

#if defined(USE_EXTMEM) && defined(CONF_BOARD_AT45DBX)
	case AT45DBX:
		if (!at45dbx_write_byte_open(address)) {
			return ERR_BAD_ADDRESS;
		}

		at45dbx_write_byte(data);
		at45dbx_write_close();
#endif

	default:
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_read(mem_type_t mem, uint32_t address, void *buffer,
		uint32_t len)
{
	switch (mem) {
	case INT_FLASH:
		nvm_flash_read_buffer((flash_addr_t)address, buffer,
				(uint16_t)len);
		break;

	case INT_USERPAGE:
		nvm_user_sig_read_buffer((flash_addr_t)address, buffer,
				(uint16_t)len);
		break;

	case INT_EEPROM:
		nvm_eeprom_read_buffer((eeprom_addr_t)address, buffer,
				(uint16_t)len);
		break;

#if defined(USE_EXTMEM) && defined(CONF_BOARD_AT45DBX)
	case AT45DBX:
	{
		uint32_t sector = address / AT45DBX_SECTOR_SIZE;
		if (!at45dbx_read_sector_open(sector)) {
			return ERR_BAD_ADDRESS;
		}

		at45dbx_read_sector_to_ram(buffer);
		at45dbx_read_close();
	}
	break;
#endif

	default:
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_write(mem_type_t mem, uint32_t address, void *buffer,
		uint32_t len)
{
	switch (mem) {
	case INT_FLASH:
		nvm_flash_erase_and_write_buffer((flash_addr_t)address,
				(const void *)buffer, len, true);
		break;

	case INT_USERPAGE:
		nvm_user_sig_write_buffer((flash_addr_t)address,
				(const void *)buffer, len, true);
		break;

	case INT_EEPROM:
		nvm_eeprom_erase_and_write_buffer((eeprom_addr_t)address,
				(const void *)buffer, len);
		break;

#if defined(USE_EXTMEM) && defined(CONF_BOARD_AT45DBX)
	case AT45DBX:
	{
		uint32_t sector = address / AT45DBX_SECTOR_SIZE;
		if (!at45dbx_write_sector_open(sector)) {
			return ERR_BAD_ADDRESS;
		}

		at45dbx_write_sector_from_ram((const void *)buffer);
		at45dbx_write_close();
	}
	break;
#endif

	default:
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_page_erase(mem_type_t mem, uint32_t page_number)
{
	switch (mem) {
	case INT_FLASH:
		if ((page_number >= 0) &&
				(page_number <
				(BOOT_SECTION_START / FLASH_PAGE_SIZE))) {
			nvm_flash_erase_app_page((flash_addr_t)(page_number *
					FLASH_PAGE_SIZE));
		} else if ((page_number >= 0) &&
				(page_number <
				(BOOT_SECTION_END / FLASH_PAGE_SIZE))) {
			nvm_flash_erase_boot_page((flash_addr_t)(page_number *
					FLASH_PAGE_SIZE));
		} else {
			return ERR_INVALID_ARG;
		}

		break;

	case INT_USERPAGE:
		nvm_flash_erase_user_section();
		break;

	case INT_EEPROM:
		nvm_eeprom_erase_page((uint8_t)page_number);
		break;

	default:
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_get_size(mem_type_t mem, uint32_t *size)
{
	switch (mem) {
	case INT_FLASH:
		*size = (uint32_t)FLASH_SIZE;
		break;

	case INT_USERPAGE:
		*size = (uint32_t)FLASH_PAGE_SIZE;
		break;

	case INT_EEPROM:
		*size = (uint32_t)EEPROM_SIZE;
		break;

#if defined(USE_EXTMEM) && defined(CONF_BOARD_AT45DBX)
	case AT45DBX:
		*size = (uint32_t)AT45DBX_MEM_SIZE;
		break;
#endif

	default:
		/* Other memories not supported */
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_get_page_size(mem_type_t mem, uint32_t *size)
{
	switch (mem) {
	case INT_FLASH:
	case INT_USERPAGE:
		*size = (uint32_t)FLASH_PAGE_SIZE;
		break;

	case INT_EEPROM:
		*size = (uint32_t)EEPROM_PAGE_SIZE;
		break;

	default:
		/* Other memories not supported */
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_get_pagenumber(mem_type_t mem, uint32_t address,
		uint32_t *num)
{
	switch (mem) {
	case INT_FLASH:
		*num = (uint32_t)(address / FLASH_PAGE_SIZE);
		break;

	case INT_EEPROM:
		*num = (uint32_t)(address / EEPROM_PAGE_SIZE);
		break;

	default:
		/* Other memories not supported */
		return ERR_INVALID_ARG;
	}

	return STATUS_OK;
}

status_code_t nvm_set_security_bit(void)
{
	/* Block external programming access to the device */
	nvm_lb_lock_bits_write(NVM_LB_RWLOCK_gc);
	return STATUS_OK;
}
