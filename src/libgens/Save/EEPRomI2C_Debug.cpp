/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRomI2C_Debug.cpp: I2C Serial EEPROM handler. (Debugging functions)   *
 * For use by MDP plugins and test suites.                                 *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "EEPRomI2C.hpp"
#include "EEPRomI2C_p.hpp"

// C includes. (C++ namespace)
#include <cstring>

namespace LibGens {

// TODO: Use MDP error codes.

/**
 * Get the EEPROM chip type.
 * @param eprType Buffer for EEPROM chip type.
 * @return MDP error code.
 */
int EEPRomI2C::dbg_getEEPRomType(EEPRomType_t *eprType) const
{
	*eprType = (EEPRomType_t)d->eprMapper.epr_type;
	return 0;
}

/**
 * Set the EEPROM chip type.
 * This function will reset the EEPROM status and clear the data.
 * @param eprType EEPROM chip type.
 * @return MDP error code.
 */
int EEPRomI2C::dbg_setEEPRomType(EEPRomType_t eprType)
{
	if (eprType < EPR_NONE || eprType >= EPR_MAX)
		return -1;

	// Set the EEPROM type.
	memset(&d->eprMapper, 0, sizeof(d->eprMapper));
	d->eprMapper.epr_type = eprType;

	// Set the EEPROM chip specification.
	memcpy(&d->eprSpec, &d->eeprom_spec[eprType], sizeof(d->eprSpec));
	return 0;
}

/**
 * Get the EEPROM size.
 * This is the total size of the EEPROM based on the chip specification.
 * @param sz Buffer for the EEPROM size.
 * @return MDP error code.
 */
int EEPRomI2C::dbg_getEEPRomSize(unsigned int *sz) const
{
	if (d->eprSpec.sz_mask == 0) {
		*sz = 0;
	} else {
		*sz = d->eprSpec.sz_mask + 1;
	}
	return 0;
}

/**
 * Get the page size.
 * This is used for sequential write, and varies based on EEPROM.
 * @param sz Buffer for the page size.
 * @return MDP error code.
 */

int EEPRomI2C::dbg_getPageSize(unsigned int *pgSize) const
{
	if (d->eprSpec.sz_mask == 0) {
		*pgSize = 0;
	} else {
		*pgSize = d->eprSpec.pg_mask + 1;
	}
	return 0;
}

/**
 * Read data from the EEPROM.
 * NOTE: Wraparound is not supported.
 * @param address Start address.
 * @param data Buffer to store the data.
 * @param length Length of data to read.
 * @return MDP error code.
 */
int EEPRomI2C::dbg_readEEPRom(uint32_t address, uint8_t *data, int length) const
{
	unsigned int eprSize;
	dbg_getEEPRomSize(&eprSize);
	if (eprSize <= 0 || address >= eprSize || address + length > eprSize) {
		return -1;
	}

	// Read the data.
	memcpy(data, &d->eeprom[address], length);
	return 0;
}

/**
 * Write data to the EEPROM.
 * NOTE: Wraparound is not supported.
 * @param address Start address.
 * @param data Buffer to store the data.
 * @param length Length of data to read.
 * @return MDP error code.
 */
int EEPRomI2C::dbg_writeEEPRom(uint32_t address, const uint8_t *data, int length)
{
	unsigned int eprSize;
	dbg_getEEPRomSize(&eprSize);
	if (eprSize <= 0 || address >= eprSize || address + length > eprSize) {
		return -1;
	}

	// Write the data.
	memcpy(&d->eeprom[address], data, length);
	d->setDirty();
	return 0;
}

/**
 * Get the state of the /SCL line.
 * @param scl Buffer to store the /SCL line state. (0 or 1)
 * @return MDP error code.
 */
int EEPRomI2C::dbg_getSCL(uint8_t *scl) const
{
	*scl = d->scl;
	return 0;
}

/**
 * Set the state of the /SCL line.
 * @param scl New /SCL line state. (0 or 1)
 * @return MDP error code.
 */
int EEPRomI2C::dbg_setSCL(uint8_t scl)
{
	d->scl = scl & 1;
	// TODO: Only if /SCL has actually changed?
	d->processI2Cbit();
	return 0;
}

/**
 * Get the state of the /SDL line.
 * @param scl Buffer to store the /SDL line state. (0 or 1)
 * @return MDP error code.
 */
int EEPRomI2C::dbg_getSDA(uint8_t *sda) const
{
	*sda = d->getSDA();
	return 0;
}

/**
 * Set the state of the /SCL line.
 * @param scl New /SCL line state. (0 or 1)
 * @return MDP error code.
 */
int EEPRomI2C::dbg_setSDA(uint8_t sda)
{
	d->sda_in = sda & 1;
	// TODO: Only if /SDA has actually changed?
	d->processI2Cbit();
	return 0;
}

}
