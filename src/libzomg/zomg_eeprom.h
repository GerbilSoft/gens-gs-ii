/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * zomg_eeprom.h: ZOMG save definitions for EEPROM.                        *
 *                                                                         *
 * Copyright (c) 2015 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef __LIBZOMG_ZOMG_EEPROM_H__
#define __LIBZOMG_ZOMG_EEPROM_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// EEPROM type.
#define ZOMG_EPR_TYPE_I2C	0x49324320	/* 'I2C ' */
#define ZOMG_EPR_TYPE_SPI	0x53504920	/* 'SPI ' */
#define ZOMG_EPR_TYPE_93C	0x39334320	/* '93C ' */

// I2C EEPROM internal state.
enum Zomg_EEPRom_I2C_state_t {
	ZOMG_EPR_I2C_STANDBY			= 0,
	ZOMG_EPR_I2C_MODE1_WORD_ADDRESS		= 1,
	ZOMG_EPR_I2C_READ_DATA			= 2,
	ZOMG_EPR_I2C_WRITE_DATA			= 3,
	ZOMG_EPR_I2C_MODE2_DEVICE_ADDRESS	= 4,
	ZOMG_EPR_I2C_MODE2_WORD_ADDRESS_LOW	= 5,
	ZOMG_EPR_I2C_MODE3_WORD_ADDRESS_HIGH	= 6,
};

/**
 * EEPROM control struct.
 * ZOMG file: common/EPR_ctrl.bin
 */
#define ZOMG_EPR_CTRL_HEADER	0x45505220
#pragma pack(1)
typedef struct _Zomg_EPR_ctrl_t {
	uint32_t header;	// 32BE: Should be 'EPR '. (0x45505220)
	uint32_t epr_type;	// 32BE: EEPROM type. (See above.)
	uint8_t count;		// 8-bit: Number of EEPROM chips.
				// NOTE: Only 1 is officially supported right now.
	uint8_t reserved[3];	// Reserved.

	union {
		struct {
			// I2C EEPROM configuration.
			uint8_t mode;		// 8-bit: Mode.
						// 1 = X24C01
						// 2 = 24C01-24C16
						// 3 = 24C32-24C512
			uint8_t dev_addr;	// 8-bit: Device address. (3 bits)
						// Used on some Mode 2 and
						// all Mode 3 devices.
			uint8_t dev_mask;	// 8-bit: Device address mask. (3 bits)
						// Used for Modes 2 and 3.
						// (Set to 0xFF for Mode 1.)
						// Masks bits of the device
						// address. Needed for
						// certain chips.
			uint8_t reserved1;	// Reserved.

			uint32_t size;		// 32BE: EEPROM size, in bytes.
						// Should be a power of two.
			uint16_t page_size;	// 16BE: Page size.
						// Usually at least 4 bytes.
						// Should be a power of two.
			uint8_t reserved2[2];	// Reserved.

			// I2C EEPROM state.
			uint8_t state;		// Current EEPROM state.
			uint8_t i2c_lines;	// Current I2C line state.
						// Bit 0 == /SCL
						// Bit 1 == /SDA input from host
						// Bit 2 == /SDA output from EEPROM
			uint8_t i2c_prev;	// Previous I2C line state.
						// Bit 0 == /SCL
						// Bit 1 == /SDA input from host
						// Bit 2 == /SDA output from EEPROM
			uint8_t counter;	// Cycle counter.

			uint32_t address;	// Word Address register.
			uint8_t data_buf;	// Internal data buffer.
			uint8_t rw;		// R/W mode. (1 == read; 0 == write)
		} i2c;
	};
} Zomg_EPR_ctrl_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBZOMG_ZOMG_EEPROM_H__ */
