/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRomI2C.hpp: I2C Serial EEPROM handler.                               *
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

#ifndef __LIBGENS_SAVE_EEPROMI2C_HPP__
#define __LIBGENS_SAVE_EEPROMI2C_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <string>

// ZOMG
namespace LibZomg {
	class Zomg;
}

namespace LibGens {

class EEPRomI2CPrivate;
class EEPRomI2C
{
	public:
		EEPRomI2C();
		~EEPRomI2C();

	protected:
		friend class EEPRomI2CPrivate;
		EEPRomI2CPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		EEPRomI2C(const EEPRomI2C &);
		EEPRomI2C &operator=(const EEPRomI2C &);

	public:
		/**
		 * Clear the EEPRom and initialize settings.
		 * This function does NOT reset the EEPRom type!
		 */
		void reset(void);

		/**
		 * Detect the EEPRom type used by the specified ROM.
		 * @param serial Serial number. (NOTE: This does NOT include the "GM " prefix!)
		 * @param serial_len Length of the serial number string.
		 * @param checksum Checksum.
		 * @return EEPRom type, or -1 if this ROM isn't known.
		 */
		static int DetectEEPRomType(const char *serial, size_t serial_len, uint16_t checksum);

		/**
		 * Set the EEPRom type.
		 * @param type EEPRom type. (Specify a negative number to clear)
		 * @return 0 on success; non-zero on error.
		 */
		int setEEPRomType(int type);

		/**
		 * Determine if the EEPRom type is set.
		 * @return True if the EEPRom type is set; false if not.
		 */
		bool isEEPRomTypeSet(void) const;

		/**
		 * Address verification functions.
		 *
		 * Notes:
		 *
		 * - Address 0 doesn't need to be checked, since the M68K memory handler
		 *   never checks EEPROM in the first bank (0x000000 - 0x07FFFF).
		 *
		 * - Word-wide addresses are checked by OR'ing both the specified address
		 *   and the preset address with 1.
		 *
		 * @param address Address.
		 * @return True if the address is usable for the specified purpose.
		 */
		inline bool isReadBytePort(uint32_t address) const;
		inline bool isReadWordPort(uint32_t address) const;
		inline bool isWriteBytePort(uint32_t address) const;
		inline bool isWriteWordPort(uint32_t address) const;

		/**
		 * Check if the EEPRom is dirty.
		 * @return True if EEPRom has been modified since the last save; false otherwise.
		 */
		bool isDirty(void) const;

		/** EEPRom access functions. **/

		uint8_t readByte(uint32_t address);
		uint16_t readWord(uint32_t address);

		void writeByte(uint32_t address, uint8_t data);
		void writeWord(uint32_t address, uint16_t data);

		// EEPRom filename and pathname.
		void setFilename(const std::string& filename);
		void setPathname(const std::string& pathname);

		/**
		 * Load the EEPRom file.
		 * @return Positive value indicating EEPRom size on success; negative on error.
		 */
		int load(void);

		/**
		 * Save the EEPRom file.
		 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
		 */
		int save(void);

		/**
		 * Autosave the EEPRom file.
		 * This saves the EEPRom file if its last modification time is past a certain threshold.
		 * @param framesElapsed Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int autoSave(int framesElapsed);

		/** ZOMG functions. **/
		int zomgRestore(LibZomg::Zomg *zomg, bool loadSaveData);
		int zomgSave(LibZomg::Zomg *zomg) const;

	public:
		// Super secret debug stuff!
		// For use by MDP plugins and test suites.
		// Return value is an MDP error code.

		/** EEPRom modes. **/
		enum EEPRomMode_t {
			EPR_NONE = 0,
			EPR_MODE1,	// 7-bit addressing. (X24C01)
			EPR_MODE2,	// 8-11 bit addressing. (24C02-24C16)
			EPR_MODE3,	// 12-bit+ addressing. (24C32+)
			EPR_MAX
		};

		int dbg_getEEPRomMode(EEPRomMode_t *eprMode) const;
		int dbg_setEEPRomMode(EEPRomMode_t eprMode);
		int dbg_getEEPRomSize(unsigned int *sz) const;
		int dbg_setEEPRomSize(unsigned int sz);
		int dbg_getPageSize(unsigned int *pgSize) const;
		int dbg_setPageSize(unsigned int pgSize);
		int dbg_getDevAddr(uint8_t *devAddr) const;
		int dbg_setDevAddr(uint8_t devAddr);

		int dbg_readEEPRom(uint32_t address, uint8_t *data, int length) const;
		int dbg_writeEEPRom(uint32_t address, const uint8_t *data, int length);

		int dbg_getSCL(uint8_t *scl) const;
		int dbg_setSCL(uint8_t scl);
		int dbg_getSDA(uint8_t *sda) const;
		int dbg_setSDA(uint8_t sda);

	protected:
		// For performance reasons, these structs and functions
		// need to be moved here.

		/** EEPROM map. **/
		struct EEPRomMap_t {
			uint32_t sda_in_adr;		// 68000 memory address mapped to SDA_IN.
			uint32_t sda_out_adr;		// 68000 memory address mapped to SDA_OUT.
			uint32_t scl_adr;		// 68000 memory address mapped to SCL.
			uint8_t sda_in_bit;		// Bit offset for SDA_IN.
			uint8_t sda_out_bit;		// Bit offset for SDA_OUT.
			uint8_t scl_bit;		// Bit offset for SCL.
		};

		// Current EEPROM mapper.
		EEPRomMap_t eprMapper;
};

// For performance reasons, these structs and functions
// need to be moved here.

/**
 * Address verification functions.
 *
 * Notes:
 *
 * - Address 0 doesn't need to be checked, since the M68K memory handler
 *   never checks EEPROM in the first bank (0x000000 - 0x07FFFF).
 *
 * - Word-wide addresses are checked by OR'ing both the specified address
 *   and the preset address with 1.
 *
 * @param address Address.
 * @return True if the address is usable for the specified purpose.
 */

bool EEPRomI2C::isReadBytePort(uint32_t address) const
{
	return (address == eprMapper.sda_out_adr);
}

bool EEPRomI2C::isReadWordPort(uint32_t address) const
{
	return ((address | 1) == (eprMapper.sda_out_adr | 1));
}

bool EEPRomI2C::isWriteBytePort(uint32_t address) const
{
	return (address == eprMapper.scl_adr ||
		address == eprMapper.sda_in_adr);
}

bool EEPRomI2C::isWriteWordPort(uint32_t address) const
{
	address |= 1;
	return ((address == (eprMapper.scl_adr | 1)) ||
		(address == (eprMapper.sda_in_adr | 1)));
}

}

#endif /* __LIBGENS_SAVE_EEPROMI2C_HPP__ */
