/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * RomCartridgeMD.hpp: MD ROM cartridge handler.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBGENS_CARTRIDGE_ROMCARTRIDGEMD_HPP__
#define __LIBGENS_CARTRIDGE_ROMCARTRIDGEMD_HPP__

// C includes.
#include <stdint.h>

// Starscream.
#include "libgens/cpu/star_68k.h"

// SRam and EEPRom.
#include "Save/SRam.hpp"
#include "Save/EEPRomI2C.hpp"

namespace LibZomg {
	class Zomg;
}

namespace LibGens {

class Rom;

class RomCartridgeMDPrivate;

class RomCartridgeMD
{
	public:
		RomCartridgeMD(Rom *rom);
		~RomCartridgeMD();

	private:
		friend class RomCartridgeMDPrivate;
		RomCartridgeMDPrivate *const d;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		RomCartridgeMD(const RomCartridgeMD &);
		RomCartridgeMD &operator=(const RomCartridgeMD &);

	public:
		/**
		 * Get the maximum supported ROM size.
		 * @return Maximum supported ROM size.
		 */
		static int MaxRomSize(void);

		/**
		 * Load the ROM image.
		 * @return 0 on success; non-zero on error.
		 * TODO: Use error code constants?
		 */
		int loadRom(void);

		/**
		 * Is the ROM loaded?
		 * @return True if loaded; false if not.
		 */
		bool isRomLoaded(void) const;

		/**
		 * Update M68K CPU program access structs for bankswitching purposes.
		 * @param M68K_Fetch Pointer to first STARSCREAM_PROGRAMREGION to update.
		 * @param banks Maximum number of banks to update.
		 * @return Number of banks updated.
		 */
		int updateSysBanking(STARSCREAM_PROGRAMREGION *M68K_Fetch, int banks);

		/**
		 * Fix the ROM checksum.
		 * This function uses the standard Sega checksum formula.
		 * TODO: Add certain non-standard checksums, and fixups
		 * for games that store code in the header area.
		 * @return 0 on success; non-zero on error.
		 */
		int fixChecksum(void);

		/**
		 * Restore the ROM checksum.
		 * This restores the ROM checksum in m_romData
		 * from the previously-loaded header information.
		 * @return 0 on success; non-zero on error.
		 */
		int restoreChecksum(void);

		/** Save data functions. **/

		/**
		 * Save SRam/EEPRom.
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		int saveData(void);

		/**
		 * AutoSave SRam/EEPRom.
		 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
		 */
		int autoSaveData(int framesElapsed);

		/** ZOMG savestate functions. **/
		void zomgSave(LibZomg::Zomg *zomg) const;
		void zomgRestore(LibZomg::Zomg *zomg, bool loadSaveData);

	protected:
		/**
		 * Initialize SRAM.
		 * If the loaded ROM has an SRAM fixup, use the fixup.
		 * Otherwise, use the ROM header values and/or defaults.
		 * @return Loaded SRAM size on success; negative on error.
		 */
		int initSRam(void);

		/**
		 * Initialize EEPROM.
		 * If the loaded ROM has an entry in the EEPROM database,
		 * that EEPROM setup will be used. Otherwise, EEPROM will
		 * not be enabled.
		 * @return Loaded EEPROM size on success; -1 if no EEPROM; other negative on error.
		 */
		int initEEPRom(void);

	private:
		// ROM access.
		template<uint8_t bank>
		inline uint8_t T_readByte_Rom(uint32_t address);
		template<uint8_t bank>
		inline uint16_t T_readWord_Rom(uint32_t address);

		// MAPPER_MD_REGISTERS_RO
		inline uint8_t readByte_REGISTERS_RO(uint32_t address);
		inline uint16_t readWord_REGISTERS_RO(uint32_t address);

	public:
		// Cartridge access functions. ($000000-$9FFFFF)
		uint8_t readByte(uint32_t address);
		uint16_t readWord(uint32_t address);
		void writeByte(uint32_t address, uint8_t data);
		void writeWord(uint32_t address, uint16_t data);

		// /TIME register access functions. ($A130xx)
		// Only the low byte of the address is needed here.
		uint8_t readByte_TIME(uint8_t address);
		uint16_t readWord_TIME(uint8_t address);
		void writeByte_TIME(uint8_t address, uint8_t data);
		void writeWord_TIME(uint8_t address, uint16_t data);

	private:
		/**
		 * NOTE: These variables are not in RomCartridgeMDPrivate
		 * for performance reasons.
		 */

		// ROM data. (Should be allocated in 512 KB blocks.)
		// (Use malloc() and free() for this pointer.)
		void *m_romData;
		uint32_t m_romData_size;

		// SRam and EEPRom.
		SRam m_SRam;
		EEPRomI2C m_EEPRom;

		// Internal cartridge banking IDs.
		enum CartBank_t {
			// ROM banks. (512 KB each)
			BANK_ROM_00 = 0, BANK_ROM_01, BANK_ROM_02, BANK_ROM_03,
			BANK_ROM_04, BANK_ROM_05, BANK_ROM_06, BANK_ROM_07,
			BANK_ROM_08, BANK_ROM_09, BANK_ROM_0A, BANK_ROM_0B,
			BANK_ROM_0C, BANK_ROM_0D, BANK_ROM_0E, BANK_ROM_0F,
			BANK_ROM_10, BANK_ROM_11, BANK_ROM_12, BANK_ROM_13,
			BANK_ROM_14, BANK_ROM_15, BANK_ROM_16, BANK_ROM_17,
			BANK_ROM_18, BANK_ROM_19, BANK_ROM_1A, BANK_ROM_1B,
			BANK_ROM_1C, BANK_ROM_1D, BANK_ROM_1E, BANK_ROM_1F,
			BANK_ROM_20, BANK_ROM_21, BANK_ROM_22, BANK_ROM_23,
			BANK_ROM_24, BANK_ROM_25, BANK_ROM_26, BANK_ROM_27,
			BANK_ROM_28, BANK_ROM_29, BANK_ROM_2A, BANK_ROM_2B,
			BANK_ROM_2C, BANK_ROM_2D, BANK_ROM_2E, BANK_ROM_2F,
			BANK_ROM_30, BANK_ROM_31, BANK_ROM_32, BANK_ROM_33,
			BANK_ROM_34, BANK_ROM_35, BANK_ROM_36, BANK_ROM_37,
			BANK_ROM_38, BANK_ROM_39, BANK_ROM_3A, BANK_ROM_3B,
			BANK_ROM_3C, BANK_ROM_3D, BANK_ROM_3E, BANK_ROM_3F,

			// MAPPER_MD_REGISTERS_RO: Read-only registers.
			BANK_MD_REGISTERS_RO,

			// Unused bank. (Return 0xFF)
			// Also used for SRAM-only banks.
			BANK_UNUSED = 0xFF,
		};

		// Physical memory map: 20 banks of 512 KB each.
		uint8_t m_cartBanks[20];

		// Checksum types.
		enum ChecksumType_t {
			CHKSUM_DISABLED = 0,	// No checksum.
			CHKSUM_SEGA = 1,	// Sega standard checksum.
		};

		// 32X mode.
		bool m_mars;

		/**
		 * Initialize the memory map for the loaded ROM.
		 * This identifies the mapper type to use.
		 */
		void initMemoryMap(void);

		// Mapper types.
		enum MD_MapperType_t {
			/**
			 * Flat addressing.
			 * Phys: $000000-$9FFFFF
			 *
			 * Technically, the area above $3FFFFF is "reserved",
			 * but it is usable in MD-only mode.
			 *
			 * SRAM may be enabled if the game attempts to use it.
			 * EEPROM is enabled if the ROM is in the EEPROM database.
			 */
			MAPPER_MD_FLAT,

			/**
			 * Super Street Fighter II.
			 * Phys: $000000-$3FFFFF
			 * Supports up to 32 banks of 512 KB. (total is 16 MB)
			 * Physical bank 0 ($000000-$07FFFF) cannot be reampped.
			 */
			MAPPER_MD_SSF2,

			/**
			 * Realtec mapper.
			 */
			MAPPER_MD_REALTEC,

			/**
			 * Read-only registers.
			 *
			 * This is used by some unlicensed games,
			 * including Ya Se Chuan Shuo and
			 * Super Bubble Bobble MD.
			 *
			 * The specific constants are set in the
			 * MD ROM fixups function.
			 */
			MAPPER_MD_REGISTERS_RO,
		};

		// Mapper information.
		struct {
			MD_MapperType_t type;

			union {
				// Super Street Fighter II mapper.
				struct {
					/**
					 * Map physical banks to virtual bank.
					 * Array index: Physical bank.
					 * Value: Virtual bank.
					 * If the virtual bank is >0x1F, use the default value.
					 */
					uint8_t banks[8];
				} ssf2;

				// Read-only registers.
				struct {
					uint32_t addr_mask[4];	// Address mask.
					uint32_t addr[4];	// Register addresses.
					uint16_t reg[4];	// Register values.
				} registers_ro;
			};
		} m_mapper;

		// Mars banking register.
		uint8_t m_mars_bank_reg;

		/**
		 * Update Mars banking at $900000-$9FFFFF.
		 */
		void updateMarsBanking(void);
};

}

#endif /* __LIBGENS_CARTRIDGE_ROMCARTRIDGEMD_HPP__ */
