/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * RomCartridgeMD.cpp: MD ROM cartridge handler.                           *
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

#include <libgens/config.libgens.h>

#include "RomCartridgeMD.hpp"

#include "EmuContext/EmuContext.hpp"
#include "EmuContext/EmuContextFactory.hpp"

#include "libcompat/byteswap.h"
#include "macros/common.h"
#include "Rom.hpp"
#include "cpu/M68K.hpp"
#include "lg_osd.h"

// ZOMG
#include "libzomg/Zomg.hpp"
#include "libzomg/zomg_md_time_reg.h"

// aligned_malloc()
#include "libcompat/aligned_malloc.h"

// C includes. (C++ namespace)
#include <cstdlib>

// C++ includes.
#include <string>
using std::string;

/**
 * References:
 * - ssf2.txt, Second Edition (2000/07/26) by Bart Trzynadlowski
 * - http://segaretro.org/Mega_Drive_Unlicensed_Game_Emulation_Notes
 */

/**
 * BYTE_ADDR_INVERT: Inversion flag for byte-addressing.
 * ROM and RAM is stored in host 16-bit endian.
 * Hence, bytewide access needs to have the LSB inverted
 * on little-endian machines.
 */
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define BYTE_ADDR_INVERT 1
#else /* GENS_BYTEORDER = GENS_BIG_ENDIAN */
#define BYTE_ADDR_INVERT 0
#endif

namespace LibGens
{

class RomCartridgeMDPrivate
{
	public:
		RomCartridgeMDPrivate(RomCartridgeMD *q, Rom *rom);
		~RomCartridgeMDPrivate();

	private:
		RomCartridgeMD *const q;

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		RomCartridgeMDPrivate(const RomCartridgeMDPrivate &);
		RomCartridgeMDPrivate &operator=(const RomCartridgeMDPrivate &);

	public:
		// ROM class this RomCartridgeMD is assigned to.
		Rom *rom;

		/**
		 * ROM fixups table entry.
		 */
		struct MD_RomFixup_t {
			// ROM identification.
			// If any value is 0 or nullptr, that field is ignored.
			struct {
				const char *serial;	// ROM serial number.
				uint16_t checksum;	// MD checksum.
				uint32_t crc32;		// CRC32.
			} id;

			// SRAM fixups.
			// If any value is 0, that field is ignored.
			struct {
				uint32_t start_addr;	// SRAM start address.
				uint32_t end_addr;	// SRAM end address.
				bool force_off;		// Force SRAM off. (Puggsy)
			} sram;

			// Checksum type.
			RomCartridgeMD::ChecksumType_t checksumType;

			// ROM mapper.
			// TODO: Add const register values.
			RomCartridgeMD::MD_MapperType_t mapperType;

			// Mapper-specific initialization data.

			// MAPPER_MD_REGISTERS_RO
			// Read-only registers.
			struct {
				uint32_t addr_mask[4];	// Address mask.
				uint32_t addr[4];	// Register addresses.
				uint16_t reg[4];	// Register values.
			} registers_ro;
		};

		static const MD_RomFixup_t MD_RomFixups[];

		/**
		 * Check for ROM fixups.
		 * @param serialNumber ROM serial number.
		 * @param checksum ROM checksum.
		 * @param crc32 ROM CRC32.
		 * @return Index in MD_RomFixups[], or -1 if no fixup is required.
		 */
		static int CheckRomFixups(const string serialNumber, uint16_t checksum, uint32_t crc32);

		// ROM fixup ID.
		// If less than 0, no fixup should be applied.
		int romFixup;

		// EEPROM type.
		// If less than 0, no EEPROM is in use.
		int eprType;
};

/**
 * ROM fixup table. (Mega Drive)
 * TODO: Allow loading this table from a data file.
 */
const RomCartridgeMDPrivate::MD_RomFixup_t RomCartridgeMDPrivate::MD_RomFixups[] =
{
	// Puggsy: Shows an anti-piracy message after the third level if SRAM is detected.
	{{"GM T-113016", 0, 0}, {0, 0, true},
		RomCartridgeMD::CHKSUM_SEGA,
		RomCartridgeMD::MAPPER_MD_FLAT, {{0}, {0}, {0}}},
	// Puggsy (Beta)
	{{"GM T-550055", 0, 0}, {0, 0, true},
		RomCartridgeMD::CHKSUM_SEGA,
		RomCartridgeMD::MAPPER_MD_FLAT, {{0}, {0}, {0}}},

	// Psy-O-Blade: Incorrect SRAM header.
	{{"GM T-26013 ", 0, 0}, {0x200000, 0x203FFF, false},
		RomCartridgeMD::CHKSUM_SEGA,
		RomCartridgeMD::MAPPER_MD_FLAT, {{0}, {0}, {0}}},

	// Super Street Fighter II: Use SSF2 mapper.
	{{"GM T-12056 ", 0, 0}, {0, 0, true},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_SSF2, {{0}, {0}, {0}}},	// US
	{{"GM MK-12056", 0, 0}, {0, 0, true},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_SSF2, {{0}, {0}, {0}}},	// EU
	{{"GM T-12043 ", 0, 0}, {0, 0, true},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_SSF2, {{0}, {0}, {0}}},	// JP

	/**
	 * Xin Qi Gai Wang Zi (original version of Beggar Prince):
	 * SRAM is located at 0x400000-0x40FFFF; ROM header is invalid.
	 * 
	 * CRC32s:
	 * - Xin Qi Gai Wang Zi (Ch).gen:	DD2F38B5
	 * - Xin Qi Gai Wang Zi (Ch) [a1].gen:	DA5A4BFE
	 */
	{{nullptr, 0, 0xDD2F38B5}, {0x400000, 0x40FFFF, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_FLAT, {{0}, {0}, {0}}},
	{{nullptr, 0, 0xDA5A4BFE}, {0x400000, 0x40FFFF, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_FLAT, {{0}, {0}, {0}}},

	/** ROMs that use MAPPER_MD_REGISTERS_RO. **/

	// Huan Le Tao Qi Shu: Smart Mouse
	{{nullptr, 0, 0xDECDF740}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0x400004, 0x400006},
		 {0x55FF, 0x0FFF, 0xAAFF, 0xF0FF}}},
	// Huan Le Tao Qi Shu: Smart Mouse [h1C]
	{{nullptr, 0, 0xDA5A4587}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0x400004, 0x400006},
		 {0x55FF, 0x0FFF, 0xAAFF, 0xF0FF}}},

	// 777 Casino
	// NOTE: Only the first register is used.
	// The other values are similar to values from
	// other games that use the same hardware.
	{{nullptr, 0, 0x42DC03E4}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0x400004, 0x400006},
		 {0x63FF, 0x98FF, 0xC9FF, 0x18FF}}},
	// 777 Casino [h1C]
	{{nullptr, 0, 0xF14D3F2E}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0x400004, 0x400006},
		 {0x63FF, 0x98FF, 0xC9FF, 0x18FF}}},
	// 777 Casino [h2C]
	{{nullptr, 0, 0x74B17EAF}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0x400004, 0x400006},
		 {0x63FF, 0x98FF, 0xC9FF, 0x18FF}}},

	// Super Bubble Bobble MD
	{{nullptr, 0, 0x4820A161}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0, 0},
		 {0x55FF, 0x0FFF, 0, 0}}},

	// Ya Se Chuan Shuo: "The Legend of Arthur" edition
	{{nullptr, 0, 0x095B9A15}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0, 0},
		 {0x63FF, 0x98FF, 0xC9FF, 0x18FF}}},
	// Ya Se Chuan Shuo: "The Legend of Arthur" edition [f1]
	{{nullptr, 0, 0xFBA90DC4}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0, 0},
		 {0x63FF, 0x98FF, 0xC9FF, 0x18FF}}},
	// Ya Se Chuan Shuo: "The Legend of Arthur" edition [f2]
	{{nullptr, 0, 0x359CB75A}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_REGISTERS_RO,
		{{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		 {0x400000, 0x400002, 0, 0},
		 {0x63FF, 0x98FF, 0xC9FF, 0x18FF}}},

	// End of list.
	{{nullptr, 0, 0}, {0, 0, false},
		RomCartridgeMD::CHKSUM_DISABLED,
		RomCartridgeMD::MAPPER_MD_FLAT, {{0}, {0}, {0}}}
};

/**
 * Check for ROM fixups.
 * @param serialNumber ROM serial number.
 * @param checksum ROM checksum.
 * @param crc32 ROM CRC32.
 * @return Index in MD_RomFixups[], or -1 if no fixup is required.
 */
int RomCartridgeMDPrivate::CheckRomFixups(const string serialNumber, uint16_t checksum, uint32_t crc32)
{
	const char *serialNumber_c_str = serialNumber.c_str();

	for (int i = 0; i < ARRAY_SIZE(MD_RomFixups); i++) {
		const MD_RomFixup_t *fixup = &MD_RomFixups[i];
		bool match = false;

		if (fixup->id.serial != nullptr) {
			// Compare the ROM serial number. (Max of 11 characters)
			if (strncmp(serialNumber_c_str, fixup->id.serial, 11) != 0)
				continue;
			match = true;
		}

		if (fixup->id.crc32 != 0 && crc32 != 0) {
			// Compare the ROM CRC32.
			if (crc32 != fixup->id.crc32)
				continue;
			match = true;
		}

		if (fixup->id.checksum != 0) {
			// Compare the ROM checksum.
			if (checksum != fixup->id.checksum)
				continue;
			match = true;
		}

		// Found a fixup for this ROM.
		if (match)
			return i;
	}

	// No fixup found for this ROM.
	return -1;
}

RomCartridgeMDPrivate::RomCartridgeMDPrivate(RomCartridgeMD *q, Rom *rom)
	: q(q)
	, rom(rom)
	, romFixup(-1)
	, eprType(-1)
{ }

RomCartridgeMDPrivate::~RomCartridgeMDPrivate()
{ }

/*****************************
 * RomCartridgeMD functions. *
 *****************************/

RomCartridgeMD::RomCartridgeMD(Rom *rom)
	: d(new RomCartridgeMDPrivate(this, rom))
	, m_romData(nullptr)
	, m_romData_size(0)
	, m_mars(false)
	, m_mars_bank_reg(0)
{
	// Set the SRam and EEPRom pathnames.
	// TODO: Update them if the pathname is changed.
	m_SRam.setPathname(EmuContext::PathSRam());
	m_EEPRom.setPathname(EmuContext::PathSRam());
}

RomCartridgeMD::~RomCartridgeMD()
{
	delete d;
	aligned_free(m_romData);
}

/**
 * Get the maximum supported ROM size.
 * @return Maximum supported ROM size.
 */
int RomCartridgeMD::MaxRomSize(void)
{
	// Maximum ROM sizes:
	// - SSF2 bankswitching: 32 MB
	// - Flat addressing: 10 MB
	return (32 * 1024 * 1024);
}

/**
 * Load the ROM image.
 * @return 0 on success; non-zero on error.
 * TODO: Use error code constants?
 */
int RomCartridgeMD::loadRom(void)
{
	if (!d->rom || !d->rom->isOpen())
		return -1;

	// Verify that this format is supported.
	// TODO: Move isRomFormatSupported() to RomCartridgeMD?
	if (!EmuContextFactory::isRomFormatSupported(d->rom)) {
		// ROM format is not supported.
		return -2;
	}

	// Check if the ROM image is too big.
	if (d->rom->romSize() > MaxRomSize()) {
		// ROM image is too big.
		return -3;
	}

	// Check if this is a 32X ROM.
	switch (d->rom->sysId()) {
		case Rom::MDP_SYSTEM_32X:
		case Rom::MDP_SYSTEM_MCD32X:	// TODO
			// 32X ROM.
			m_mars = true;
			break;

		default:
			// Not a 32X ROM.
			m_mars = false;
			break;
	}

	// Allocate memory for the ROM image.
	// NOTE: malloc() is rounded up to the nearest 512 KB.
	// TODO: Store the rounded-up size.
	m_romData_size = d->rom->romSize();
	uint32_t rnd_512k = ((m_romData_size + 0x7FFFF) & ~0x7FFFF);
	switch (d->rom->romFormat()) {
		case Rom::RFMT_SMD:
		case Rom::RFMT_SMD_SPLIT:
			// ROM buffer needs an extra 512 bytes for the header.
			// TODO: Add a generic function to return this.
			// TODO: Eliminate this by loading the ROM directly
			// after the header.
			rnd_512k += 512;
			break;

		default:
			break;
	}
	// Align to 16 bytes for potential SSE2 optimizations.
	m_romData = aligned_malloc(16, rnd_512k);

	// Load the ROM image.
	// NOTE: Passing the size of the entire ROM buffer,
	// not the expected size of the ROM.
	int ret = d->rom->loadRom(m_romData, rnd_512k);
	if (ret != (int)m_romData_size) {
		// Error loading the ROM.
		// TODO: Set an error number somewhere.
		aligned_free(m_romData);
		m_romData = nullptr;
		m_romData_size = 0;
		return -4;
	}

	// Byteswap the ROM image.
	be16_to_cpu_array((uint16_t*)m_romData, m_romData_size);

	// Initialize the ROM mapper.
	// NOTE: This must be done after loading the ROM;
	// otherwise, d->rom->rom_crc32() will return 0.
	initMemoryMap();

	if (d->rom->sysId() != Rom::MDP_SYSTEM_PICO) {
		// Initialize EEPRom.
		// EEPRom is only used if the ROM is in the EEPRom class's database.
		// Otherwise, SRam is used.
		int cartSaveSize = initEEPRom();
		if (cartSaveSize >= 0) {
			// EEPRom was initialized.
			if (cartSaveSize > 0)
				lg_osd(OSD_EEPROM_LOAD, cartSaveSize);
		} else {
			// EEPRom was not initialized.
			// Initialize SRam.
			cartSaveSize = initSRam();
			if (cartSaveSize > 0)
				lg_osd(OSD_SRAM_LOAD, cartSaveSize);
		}
	} else {
		// Pico cartridges don't have SRAM/EEPROM.
		m_SRam.reset();
		m_SRam.setOn(false);
		m_SRam.setWrite(false);
		m_SRam.setStart(1);
		m_SRam.setEnd(0);

		d->eprType = -1;
		m_EEPRom.setEEPRomType(-1);
	}

	// ...and we're done here.
	return 0;
}

/**
 * Is the ROM loaded?
 * @return True if loaded; false if not.
 */
bool RomCartridgeMD::isRomLoaded(void) const
{
	return (m_romData != nullptr);
}

/**
 * Update M68K CPU program access structs for bankswitching purposes.
 * @param M68K_Fetch Pointer to first STARSCREAM_PROGRAMREGION to update.
 * @param banks Maximum number of banks to update.
 * @return Number of banks updated.
 */
int RomCartridgeMD::updateSysBanking(STARSCREAM_PROGRAMREGION *M68K_Fetch, int banks)
{
#ifdef GENS_ENABLE_EMULATION
	int banksUpdated = 0;
	if (banks > ARRAY_SIZE(m_cartBanks))
		banks = ARRAY_SIZE(m_cartBanks);

	// TODO: This is not 64-bit clean!
	for (int i = 0; i < banks; i++) {
		if (/*m_cartBanks[i] >= BANK_ROM_00 &&*/
		    m_cartBanks[i] <= BANK_ROM_3F) {
			// ROM bank.
			// NOTE: m_romData is always a multiple of 512 KB.
			const uint32_t romAddrStart = (0x80000 * (m_cartBanks[i] - BANK_ROM_00));
			if (romAddrStart < m_romData_size) {
				// Valid bank. Map it.
				M68K_Fetch->lowaddr = romAddrStart;
				M68K_Fetch->highaddr = (romAddrStart + 0x7FFFF);
				M68K_Fetch->offset = ((uint32_t)m_romData);
				M68K_Fetch++;
				banksUpdated++;
			}
		}
	}

	// Updated.
	return banksUpdated;
#else /* !GENS_ENABLE_EMULATION */
	// Emulation is disabled.
	return 0;
#endif /* GENS_ENABLE_EMULATION */
}

/**
 * Fix the ROM checksum.
 * @return 0 on success; non-zero on error.
 */
int RomCartridgeMD::fixChecksum(void)
{
	if (!m_romData || m_romData_size <= 0x200)
		return -1;

	// Check if this ROM uses a non-standard checksum.
	ChecksumType_t checksumType = CHKSUM_SEGA;
	if (d->romFixup >= 0) {
		// Apply a ROM fixup.
		const RomCartridgeMDPrivate::MD_RomFixup_t *fixup =
			&RomCartridgeMDPrivate::MD_RomFixups[d->romFixup];
		checksumType = fixup->checksumType;
	}

	switch (checksumType) {
		case CHKSUM_DISABLED:
			// Game does not use a checksum.
			break;

		case CHKSUM_SEGA: {
			// Standard Sega checksum.

			// Calculate the ROM checksum.
			// NOTE: ROM is byteswapped. (Header data is read before byteswapping.)
			// NOTE: If ROM is an odd number of bytes, it'll be padded by 1 byte.
			uint16_t checksum = 0;
			const uint16_t *rom_ptr = &(reinterpret_cast<uint16_t*>(m_romData))[0x200>>1];
			const uint16_t *end_ptr = rom_ptr + ((m_romData_size - 0x200) >> 1);
			if (m_romData_size & 1)
				end_ptr++;

			for (; rom_ptr != end_ptr; rom_ptr++) {
				checksum += *rom_ptr;
			}

			// Set the new checksum.
			uint16_t *chk_ptr = &(reinterpret_cast<uint16_t*>(m_romData))[0x18E>>1];
			*chk_ptr = checksum;
		}
	}

	return 0;
}

/**
 * Restore the ROM checksum.
 * This restores the ROM checksum in m_romData
 * from the previously-loaded header information.
 * @return 0 on success; non-zero on error.
 */
int RomCartridgeMD::restoreChecksum(void)
{
	if (!m_romData || m_romData_size <= 0x200)
		return -1;

	// Restore the ROM checksum.
	// NOTE: ROM is byteswapped. (Header data is read before byteswapping.)
	uint16_t *chk_ptr = &(reinterpret_cast<uint16_t*>(m_romData))[0x18E>>1];
	*chk_ptr = d->rom->checksum();
	return 0;
}


/** Save data functions. **/


/**
 * Save SRam/EEPRom.
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int RomCartridgeMD::saveData(void)
{
	// TODO: Return a value indicating what was saved and the size.
	// (that is, move lg_osd out of this function.)
	if (m_EEPRom.isEEPRomTypeSet()) {
		// Save EEPRom.
		int eepromSize = m_EEPRom.save();
		if (eepromSize > 0) {
			lg_osd(OSD_EEPROM_SAVE, eepromSize);
			return 2;
		}
	} else {
		// Save SRam.
		int sramSize = m_SRam.save();
		if (sramSize > 0) {
			lg_osd(OSD_SRAM_SAVE, sramSize);
			return 1;
		}
	}

	// Nothing was saved.
	return 0;
}

/**
 * AutoSave SRam/EEPRom.
 * @param frames Number of frames elapsed, or -1 for paused. (force autosave)
 * @return 1 if SRam was saved; 2 if EEPRom was saved; 0 if nothing was saved. (TODO: Enum?)
 */
int RomCartridgeMD::autoSaveData(int framesElapsed)
{
	// TODO: Return a value indicating what was saved and the size.
	// (that is, move lg_osd out of this function.)
	if (m_EEPRom.isEEPRomTypeSet()) {
		// Save EEPRom.
		int eepromSize = m_EEPRom.autoSave(framesElapsed);
		if (eepromSize > 0) {
			lg_osd(OSD_EEPROM_AUTOSAVE, eepromSize);
			return 2;
		}
	} else {
		// Save SRam.
		int sramSize = m_SRam.autoSave(framesElapsed);
		if (sramSize > 0)
		{
			lg_osd(OSD_SRAM_AUTOSAVE, sramSize);
			return 1;
		}
	}

	// Nothing was saved.
	return 0;
}

/**
 * Initialize SRAM.
 * If the loaded ROM has an SRAM fixup, use the fixup.
 * Otherwise, use the ROM header values and/or defaults.
 * @return Loaded SRAM size on success; negative on error.
 */
int RomCartridgeMD::initSRam(void)
{
	// Reset SRam before applying any settings.
	m_SRam.reset();

	// TODO; Move some of this to the SRam class?

	// SRAM information.
	uint32_t sramInfo, sramStartAddr, sramEndAddr;
	int ret = d->rom->romSramInfo(&sramInfo, &sramStartAddr, &sramEndAddr);
	if (ret != 0) {
		// Error reading SRAM information.
		// TODO: Error constant?
		return -1;
	}

	// Check if the ROM header has SRam information.
	// Mask the SRam info value with 0xFFFF4000 and check
	// if it matches the Magic Number.
	// Magic Number: 0x52414000 ('R', 'A', 0x40, 0x00)
	if ((sramInfo & 0xFFFF4000) == 0x52414000) {
		// ROM header has SRam information. Use these addresses..
		// SRam starting position must be a multiple of 0xF80000.
		// TODO: Is that really necessary?
		sramStartAddr &= 0xF80000;
		sramEndAddr &= 0xFFFFFF;
	} else {
		// ROM header does not have SRam information.
		// Use default settings.
		sramStartAddr = 0x200000;
		sramEndAddr = 0x20FFFF;	// 64 KB
	}

	// TODO: If S&K, check the lock-on ROM for SRAM headers.

	// Check for invalid SRam addresses.
	if ((sramStartAddr > sramEndAddr) ||
	    ((sramEndAddr - sramStartAddr) > 0xFFFF))
	{
		// Invalid ending address.
		// Set the end address to the start + 0xFFFF.
		sramEndAddr = sramStartAddr + 0xFFFF;
	}

	// Make sure SRam starts on an even byte and ends on an odd byte.
	sramStartAddr &= ~1;
	sramEndAddr |= 1;

	/**
	 * If the ROM is smaller than the SRAM starting address, always enable SRAM.
	 * Notes:
	 * - HardBall '95: SRAM is at $300000; ROM is 3 MB; cartridge does NOT have $A130F1 register.
	 *                 Need to enable SRAM initially; otherwise, an error appears on startup.
	 */
	const bool enableSRam = (m_romData_size <= sramStartAddr);
	m_SRam.setOn(enableSRam);
	m_SRam.setWrite(enableSRam);

	// Check if a ROM fixup needs to be applied.
	if (d->romFixup >= 0) {
		// Apply a ROM fixup.
		const RomCartridgeMDPrivate::MD_RomFixup_t *fixup =
			&RomCartridgeMDPrivate::MD_RomFixups[d->romFixup];

		if (fixup->sram.force_off) {
			// Force SRAM off.
			m_SRam.setOn(false);
			m_SRam.setWrite(false);
			m_SRam.setStart(1);
			m_SRam.setEnd(0);
			return 0;
		}

		// Fix SRAM start/end addresses.
		if (fixup->sram.start_addr != 0)
			sramStartAddr = fixup->sram.start_addr;
		if (fixup->sram.end_addr != 0)
			sramEndAddr = fixup->sram.end_addr;
	}

	// Set the addresses.
	m_SRam.setStart(sramStartAddr);
	m_SRam.setEnd(sramEndAddr);

	// Load the SRam file.
	// NOTE: SRam::setFilename() uses LibGensText::FilenameNoExt().
	string rom_filename;
	if (d->rom->isMultiFile()) {
		rom_filename = d->rom->z_filename();
	}
	if (rom_filename.empty()) {
		rom_filename = d->rom->filename();
	}
	m_SRam.setFilename(rom_filename);
	return m_SRam.load();
}

/**
 * Initialize EEPROM.
 * If the loaded ROM has an entry in the EEPROM database,
 * that EEPROM setup will be used. Otherwise, EEPROM will
 * not be enabled.
 * @return Loaded EEPROM size on success; -1 if no EEPROM; other negative on error.
 */
int RomCartridgeMD::initEEPRom(void)
{
	// TODO: Should that be implemented here or in SRam.cpp?

	// Reset the EEPRom and set the type.
	m_EEPRom.reset();
	m_EEPRom.setEEPRomType(d->eprType);

	// Don't do anything if the ROM isn't in the EEPRom database.
	if (d->eprType < 0)
		return -1;

	// Load the EEPRom file.
	// NOTE: EEPRomI2C::setFilename() uses LibGensText::FilenameNoExt().
	string rom_filename;
	if (d->rom->isMultiFile()) {
		rom_filename = d->rom->z_filename();
	}
	if (rom_filename.empty()) {
		rom_filename = d->rom->filename();
	}
	m_EEPRom.setFilename(rom_filename);
	return m_EEPRom.load();
}


/** ROM access functions. **/

/**
 * Read a byte from ROM.
 * @param bank Physical ROM bank.
 * @param address Address.
 * @return Byte from ROM.
 */
template<uint8_t bank>
inline uint8_t RomCartridgeMD::T_readByte_Rom(uint32_t address)
{
	// TODO: Mirroring; CPU prefetch.
	address &= 0x7FFFF;
	address ^= ((bank << 19) | BYTE_ADDR_INVERT);
	if (address >= m_romData_size)
		return 0xFF;
	return (reinterpret_cast<uint8_t*>(m_romData))[address];
}

/**
 * Read a word from ROM.
 * @param bank Physical ROM bank.
 * @param address Address.
 * @return Word from ROM.
 */
template<uint8_t bank>
inline uint16_t RomCartridgeMD::T_readWord_Rom(uint32_t address)
{
	// TODO: Mirroring; CPU prefetch.
	address &= 0x7FFFF;
	address |= (bank << 19);
	if (address >= m_romData_size)
		return 0xFFFF;
	return (reinterpret_cast<uint16_t*>(m_romData))[address >> 1];
}

/** Mapper functions. **/

/**
 * Read a byte from the mapper register area.
 * MAPPER_MD_REGISTERS_RO
 * @param address Address.
 * @return Byte from constant register area.
 */
inline uint8_t RomCartridgeMD::readByte_REGISTERS_RO(uint32_t address)
{
	for (int i = 0; i < ARRAY_SIZE(m_mapper.registers_ro.reg); i++) {
		if ((address & m_mapper.registers_ro.addr_mask[i]) ==
			m_mapper.registers_ro.addr[i])
		{
			// Address match.
			if (!(address & 1)) {
				// Even byte.
				return ((m_mapper.registers_ro.reg[i] >> 8) & 0xFF);
			} else {
				// Odd byte.
				return (m_mapper.registers_ro.reg[i] & 0xFF);
			}
		}
	}

	// Register is not defined.
	return 0xFF;
}

/**
 * Read a Word from the constant register area.
 * MAPPER_MD_REGISTERS_RO
 * @param address Address.
 * @return Word from constant register area.
 */
inline uint16_t RomCartridgeMD::readWord_REGISTERS_RO(uint32_t address)
{
	for (int i = 0; i < ARRAY_SIZE(m_mapper.registers_ro.reg); i++) {
		if ((address & m_mapper.registers_ro.addr_mask[i]) ==
			m_mapper.registers_ro.addr[i])
		{
			// Address match.
			return m_mapper.registers_ro.reg[i];
		}
	}

	// Register is not defined.
	return 0xFFFF;
}


/** Cartridge access functions. ($000000-$9FFFFF) **/

/**
 * Read a byte from the standard cartridge area. ($000000-$9FFFFF)
 * @param address Cartridge address.
 * @return Byte from cartridge.
 */
uint8_t RomCartridgeMD::readByte(uint32_t address)
{
	address &= 0xFFFFFF;

	// Check for save data access.
	// TODO: Determine the physical banks for SRAM/EEPROM and
	// check that in order to optimize this.
	if (EmuContext::GetSaveDataEnable()) {
		if (m_EEPRom.isEEPRomTypeSet()) {
			// EEPRom is enabled.
			if (m_EEPRom.isReadBytePort(address)) {
				// EEPRom read port.
				return m_EEPRom.readByte(address);
			}
		} else if (m_SRam.canRead() && m_SRam.isAddressInRange(address)) {
			// SRam data request.
			// Return the byte from SRam.
			// NOTE: SRam is NOT byteswapped.
			// TODO: Check boundaries.
			// TODO: Should start/end addressing be handled here or in SRam?
			return m_SRam.readByte(address);
		}
	}

	const uint8_t phys_bank = ((address >> 19) & 0x1F);

	switch (m_cartBanks[phys_bank]) {
		default:
		case BANK_UNUSED:	return 0xFF;

		// Physical ROM bank.
		case BANK_ROM_00:	return T_readByte_Rom<0x00>(address);
		case BANK_ROM_01:	return T_readByte_Rom<0x01>(address);
		case BANK_ROM_02:	return T_readByte_Rom<0x02>(address);
		case BANK_ROM_03:	return T_readByte_Rom<0x03>(address);
		case BANK_ROM_04:	return T_readByte_Rom<0x04>(address);
		case BANK_ROM_05:	return T_readByte_Rom<0x05>(address);
		case BANK_ROM_06:	return T_readByte_Rom<0x06>(address);
		case BANK_ROM_07:	return T_readByte_Rom<0x07>(address);
		case BANK_ROM_08:	return T_readByte_Rom<0x08>(address);
		case BANK_ROM_09:	return T_readByte_Rom<0x09>(address);
		case BANK_ROM_0A:	return T_readByte_Rom<0x0A>(address);
		case BANK_ROM_0B:	return T_readByte_Rom<0x0B>(address);
		case BANK_ROM_0C:	return T_readByte_Rom<0x0C>(address);
		case BANK_ROM_0D:	return T_readByte_Rom<0x0D>(address);
		case BANK_ROM_0E:	return T_readByte_Rom<0x0E>(address);
		case BANK_ROM_0F:	return T_readByte_Rom<0x0F>(address);
		case BANK_ROM_10:	return T_readByte_Rom<0x10>(address);
		case BANK_ROM_11:	return T_readByte_Rom<0x11>(address);
		case BANK_ROM_12:	return T_readByte_Rom<0x12>(address);
		case BANK_ROM_13:	return T_readByte_Rom<0x13>(address);
		case BANK_ROM_14:	return T_readByte_Rom<0x14>(address);
		case BANK_ROM_15:	return T_readByte_Rom<0x15>(address);
		case BANK_ROM_16:	return T_readByte_Rom<0x16>(address);
		case BANK_ROM_17:	return T_readByte_Rom<0x17>(address);
		case BANK_ROM_18:	return T_readByte_Rom<0x18>(address);
		case BANK_ROM_19:	return T_readByte_Rom<0x19>(address);
		case BANK_ROM_1A:	return T_readByte_Rom<0x1A>(address);
		case BANK_ROM_1B:	return T_readByte_Rom<0x1B>(address);
		case BANK_ROM_1C:	return T_readByte_Rom<0x1C>(address);
		case BANK_ROM_1D:	return T_readByte_Rom<0x1D>(address);
		case BANK_ROM_1E:	return T_readByte_Rom<0x1E>(address);
		case BANK_ROM_1F:	return T_readByte_Rom<0x1F>(address);
		case BANK_ROM_20:	return T_readByte_Rom<0x20>(address);
		case BANK_ROM_21:	return T_readByte_Rom<0x21>(address);
		case BANK_ROM_22:	return T_readByte_Rom<0x22>(address);
		case BANK_ROM_23:	return T_readByte_Rom<0x23>(address);
		case BANK_ROM_24:	return T_readByte_Rom<0x24>(address);
		case BANK_ROM_25:	return T_readByte_Rom<0x25>(address);
		case BANK_ROM_26:	return T_readByte_Rom<0x26>(address);
		case BANK_ROM_27:	return T_readByte_Rom<0x27>(address);
		case BANK_ROM_28:	return T_readByte_Rom<0x28>(address);
		case BANK_ROM_29:	return T_readByte_Rom<0x29>(address);
		case BANK_ROM_2A:	return T_readByte_Rom<0x2A>(address);
		case BANK_ROM_2B:	return T_readByte_Rom<0x2B>(address);
		case BANK_ROM_2C:	return T_readByte_Rom<0x2C>(address);
		case BANK_ROM_2D:	return T_readByte_Rom<0x2D>(address);
		case BANK_ROM_2E:	return T_readByte_Rom<0x2E>(address);
		case BANK_ROM_2F:	return T_readByte_Rom<0x2F>(address);
		case BANK_ROM_30:	return T_readByte_Rom<0x30>(address);
		case BANK_ROM_31:	return T_readByte_Rom<0x31>(address);
		case BANK_ROM_32:	return T_readByte_Rom<0x32>(address);
		case BANK_ROM_33:	return T_readByte_Rom<0x33>(address);
		case BANK_ROM_34:	return T_readByte_Rom<0x34>(address);
		case BANK_ROM_35:	return T_readByte_Rom<0x35>(address);
		case BANK_ROM_36:	return T_readByte_Rom<0x36>(address);
		case BANK_ROM_37:	return T_readByte_Rom<0x37>(address);
		case BANK_ROM_38:	return T_readByte_Rom<0x38>(address);
		case BANK_ROM_39:	return T_readByte_Rom<0x39>(address);
		case BANK_ROM_3A:	return T_readByte_Rom<0x3A>(address);
		case BANK_ROM_3B:	return T_readByte_Rom<0x3B>(address);
		case BANK_ROM_3C:	return T_readByte_Rom<0x3C>(address);
		case BANK_ROM_3D:	return T_readByte_Rom<0x3D>(address);
		case BANK_ROM_3E:	return T_readByte_Rom<0x3E>(address);
		case BANK_ROM_3F:	return T_readByte_Rom<0x3F>(address);

		// Mappers.
		case BANK_MD_REGISTERS_RO:	return readByte_REGISTERS_RO(address);
	}
}

/**
 * Read a word from the standard cartridge area. ($000000-$9FFFFF)
 * @param address Cartridge address.
 * @return Word from cartridge.
 */
uint16_t RomCartridgeMD::readWord(uint32_t address)
{
	address &= 0xFFFFFF;

	// Check for save data access.
	// TODO: Determine the physical banks for SRAM/EEPROM and
	// check that in order to optimize this.
	if (EmuContext::GetSaveDataEnable()) {
		if (m_EEPRom.isEEPRomTypeSet()) {
			// EEPRom is enabled.
			if (m_EEPRom.isReadWordPort(address)) {
				// EEPRom read port.
				return m_EEPRom.readWord(address);
			}
		} else if (m_SRam.canRead() && m_SRam.isAddressInRange(address)) {
			// SRam data request.
			// Return the byte from SRam.
			// NOTE: SRam is NOT byteswapped.
			// TODO: Check boundaries.
			// TODO: Should start/end addressing be handled here or in SRam?
			return m_SRam.readWord(address);
		}
	}

	const uint8_t phys_bank = ((address >> 19) & 0x1F);

	switch (m_cartBanks[phys_bank]) {
		default:
		case BANK_UNUSED:	return 0xFFFF;

		// Physical ROM bank.
		case BANK_ROM_00:	return T_readWord_Rom<0x00>(address);
		case BANK_ROM_01:	return T_readWord_Rom<0x01>(address);
		case BANK_ROM_02:	return T_readWord_Rom<0x02>(address);
		case BANK_ROM_03:	return T_readWord_Rom<0x03>(address);
		case BANK_ROM_04:	return T_readWord_Rom<0x04>(address);
		case BANK_ROM_05:	return T_readWord_Rom<0x05>(address);
		case BANK_ROM_06:	return T_readWord_Rom<0x06>(address);
		case BANK_ROM_07:	return T_readWord_Rom<0x07>(address);
		case BANK_ROM_08:	return T_readWord_Rom<0x08>(address);
		case BANK_ROM_09:	return T_readWord_Rom<0x09>(address);
		case BANK_ROM_0A:	return T_readWord_Rom<0x0A>(address);
		case BANK_ROM_0B:	return T_readWord_Rom<0x0B>(address);
		case BANK_ROM_0C:	return T_readWord_Rom<0x0C>(address);
		case BANK_ROM_0D:	return T_readWord_Rom<0x0D>(address);
		case BANK_ROM_0E:	return T_readWord_Rom<0x0E>(address);
		case BANK_ROM_0F:	return T_readWord_Rom<0x0F>(address);
		case BANK_ROM_10:	return T_readWord_Rom<0x10>(address);
		case BANK_ROM_11:	return T_readWord_Rom<0x11>(address);
		case BANK_ROM_12:	return T_readWord_Rom<0x12>(address);
		case BANK_ROM_13:	return T_readWord_Rom<0x13>(address);
		case BANK_ROM_14:	return T_readWord_Rom<0x14>(address);
		case BANK_ROM_15:	return T_readWord_Rom<0x15>(address);
		case BANK_ROM_16:	return T_readWord_Rom<0x16>(address);
		case BANK_ROM_17:	return T_readWord_Rom<0x17>(address);
		case BANK_ROM_18:	return T_readWord_Rom<0x18>(address);
		case BANK_ROM_19:	return T_readWord_Rom<0x19>(address);
		case BANK_ROM_1A:	return T_readWord_Rom<0x1A>(address);
		case BANK_ROM_1B:	return T_readWord_Rom<0x1B>(address);
		case BANK_ROM_1C:	return T_readWord_Rom<0x1C>(address);
		case BANK_ROM_1D:	return T_readWord_Rom<0x1D>(address);
		case BANK_ROM_1E:	return T_readWord_Rom<0x1E>(address);
		case BANK_ROM_1F:	return T_readWord_Rom<0x1F>(address);
		case BANK_ROM_20:	return T_readWord_Rom<0x20>(address);
		case BANK_ROM_21:	return T_readWord_Rom<0x21>(address);
		case BANK_ROM_22:	return T_readWord_Rom<0x22>(address);
		case BANK_ROM_23:	return T_readWord_Rom<0x23>(address);
		case BANK_ROM_24:	return T_readWord_Rom<0x24>(address);
		case BANK_ROM_25:	return T_readWord_Rom<0x25>(address);
		case BANK_ROM_26:	return T_readWord_Rom<0x26>(address);
		case BANK_ROM_27:	return T_readWord_Rom<0x27>(address);
		case BANK_ROM_28:	return T_readWord_Rom<0x28>(address);
		case BANK_ROM_29:	return T_readWord_Rom<0x29>(address);
		case BANK_ROM_2A:	return T_readWord_Rom<0x2A>(address);
		case BANK_ROM_2B:	return T_readWord_Rom<0x2B>(address);
		case BANK_ROM_2C:	return T_readWord_Rom<0x2C>(address);
		case BANK_ROM_2D:	return T_readWord_Rom<0x2D>(address);
		case BANK_ROM_2E:	return T_readWord_Rom<0x2E>(address);
		case BANK_ROM_2F:	return T_readWord_Rom<0x2F>(address);
		case BANK_ROM_30:	return T_readWord_Rom<0x30>(address);
		case BANK_ROM_31:	return T_readWord_Rom<0x31>(address);
		case BANK_ROM_32:	return T_readWord_Rom<0x32>(address);
		case BANK_ROM_33:	return T_readWord_Rom<0x33>(address);
		case BANK_ROM_34:	return T_readWord_Rom<0x34>(address);
		case BANK_ROM_35:	return T_readWord_Rom<0x35>(address);
		case BANK_ROM_36:	return T_readWord_Rom<0x36>(address);
		case BANK_ROM_37:	return T_readWord_Rom<0x37>(address);
		case BANK_ROM_38:	return T_readWord_Rom<0x38>(address);
		case BANK_ROM_39:	return T_readWord_Rom<0x39>(address);
		case BANK_ROM_3A:	return T_readWord_Rom<0x3A>(address);
		case BANK_ROM_3B:	return T_readWord_Rom<0x3B>(address);
		case BANK_ROM_3C:	return T_readWord_Rom<0x3C>(address);
		case BANK_ROM_3D:	return T_readWord_Rom<0x3D>(address);
		case BANK_ROM_3E:	return T_readWord_Rom<0x3E>(address);
		case BANK_ROM_3F:	return T_readWord_Rom<0x3F>(address);

		// Mappers.
		case BANK_MD_REGISTERS_RO:	return readWord_REGISTERS_RO(address);
	}
}

/**
 * Write a byte to the standard cartridge area. ($000000-$9FFFFF)
 * @param address Cartridge address.
 * @param data Byte to write.
 * @return Byte from cartridge.
 */
void RomCartridgeMD::writeByte(uint32_t address, uint8_t data)
{
	if (!EmuContext::GetSaveDataEnable()) {
		// Save data is disabled.
		return;
	}

	address &= 0xFFFFFF;

	// TODO: Determine the physical banks for SRAM/EEPROM and
	// check that in order to optimize this.
	if (m_EEPRom.isEEPRomTypeSet()) {
		// EEPRom is enabled.
		if (m_EEPRom.isWriteBytePort(address)) {
			// EEPRom write port.
			return m_EEPRom.writeByte(address, data);
		}
	} else if (m_SRam.canWrite() && m_SRam.isAddressInRange(address)) {
		// SRam data request.
		// Write the word to SRam.
		// NOTE: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		// TODO: Should start/end addressing be handled here or in SRam?
		m_SRam.writeByte(address, data);
	}
}

/**
 * Write a word to the standard cartridge area. ($000000-$9FFFFF)
 * @param address Cartridge address.
 * @param data Word to write.
 * @return Word from cartridge.
 */
void RomCartridgeMD::writeWord(uint32_t address, uint16_t data)
{
	if (!EmuContext::GetSaveDataEnable()) {
		// Save data is disabled.
		return;
	}

	address &= 0xFFFFFF;

	// TODO: Determine the physical banks for SRAM/EEPROM and
	// check that in order to optimize this.
	if (m_EEPRom.isEEPRomTypeSet()) {
		// EEPRom is enabled.
		if (m_EEPRom.isWriteWordPort(address)) {
			// EEPRom write port.
			return m_EEPRom.writeWord(address, data);
		}
	} else if (m_SRam.canWrite() && m_SRam.isAddressInRange(address)) {
		// SRam data request.
		// Write the word to SRam.
		// NOTE: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		// TODO: Should start/end addressing be handled here or in SRam?
		m_SRam.writeWord(address, data);
	}
}


/** /TIME register access functions. ($A130xx) **/

/**
 * Read a byte from the /TIME registers. ($A130xx)
 * @param address Register address.
 * @return Byte from /TIME.
 */
uint8_t RomCartridgeMD::readByte_TIME(uint8_t address)
{
	// The only readable /TIME registers are MARS registers.
	if (m_mars && (address >= 0xEC && address <= 0xEF)) {
		// MARS identification.
		static const uint8_t mars_id[4] = {'M', 'A', 'R', 'S'};
		return mars_id[address & 0x3];
	}

	// Not a readable register.
	return 0xFF;
}

/**
 * Read a word from the /TIME registers. ($A130xx)
 * @param address Register address.
 * @return Word from /TIME.
 */
uint16_t RomCartridgeMD::readWord_TIME(uint8_t address)
{
	// The only readable /TIME registers are MARS registers.
	if (m_mars && (address >= 0xEC && address <= 0xEF)) {
		// MARS identification.
		static const uint16_t mars_id[2] = {'MA', 'RS'};
		static_assert('MA' == 0x4D41, "MARS identification: 'MA' != 0x4D41");
		static_assert('RS' == 0x5253, "MARS identification: 'RS' != 0x5253");
		return mars_id[(address >> 1) & 0x1];
	}

	// Not a readable register.
	return 0xFFFF;
}

/**
 * Write a byte to the /TIME registers. ($A130xx)
 * @param address Register address.
 * @param data Byte to write.
 */
void RomCartridgeMD::writeByte_TIME(uint8_t address, uint8_t data)
{
	if (address == 0xF1) {
		// $A130F1: SRAM control register.
		m_SRam.writeCtrl(data);
		return;
	}

	// Check for SSF2 bankswitching.
	if (m_mapper.type == MAPPER_MD_SSF2) {
		if (address >= 0xF2 /*&& address <= 0xFF*/) {
			// SSF2 bankswitch register.
			// TODO: Update Starscream instruction fetch.
			const uint8_t phys_bank = (address & 0xF) >> 1;
			uint8_t virt_bank = (data & 0x3F);

			// Check if the virtual bank address is in range.
			const uint8_t max_virt_bank = (m_romData_size / 524288);
			if (virt_bank >= max_virt_bank) {
				// Out of range. Use the default value.
				virt_bank = phys_bank;
				m_mapper.ssf2.banks[phys_bank] = 0xFF;
			} else {
				// Save the bank number.
				m_mapper.ssf2.banks[phys_bank] = virt_bank;
			}

			// Update the memory map.
			m_cartBanks[phys_bank] = (BANK_ROM_00 + virt_bank);
			if (m_mars)
				updateMarsBanking();
			// TODO: Better way to update Starscream?
			M68K::UpdateSysBanking();
			return;
		}
	}
}

/**
 * Write a word to the /TIME registers. ($A130xx)
 * @param address Register address.
 * @param data Word to write.
 */
void RomCartridgeMD::writeWord_TIME(uint8_t address, uint16_t data)
{
	if (address == 0xF0) {
		// $A130F0: SRAM control register.
		m_SRam.writeCtrl(data);
		return;
	}

	// Check for SSF2 bankswitching.
	if (m_mapper.type == MAPPER_MD_SSF2) {
		if (address >= 0xF2 /*&& address <= 0xFF*/) {
			// SSF2 bankswitch register.
			// TODO: Update Starscream instruction fetch.
			const uint8_t phys_bank = (address & 0xF) >> 1;
			uint8_t virt_bank = (data & 0x3F);

			// Check if the virtual bank address is in range.
			const uint8_t max_virt_bank = (m_romData_size / 524288);
			if (virt_bank >= max_virt_bank) {
				// Out of range. Use the default value.
				virt_bank = phys_bank;
				m_mapper.ssf2.banks[phys_bank] = 0xFF;
			} else {
				// Save the bank number.
				m_mapper.ssf2.banks[phys_bank] = virt_bank;
			}

			// Update the memory map.
			m_cartBanks[phys_bank] = (BANK_ROM_00 + virt_bank);
			if (m_mars)
				updateMarsBanking();
			// TODO: Better way to update Starscream?
			M68K::UpdateSysBanking();
			return;
		}
	}
}


/** Initialization functions. **/

/**
 * Initialize the memory map for the loaded ROM.
 * This identifies the mapper type to use.
 */
void RomCartridgeMD::initMemoryMap(void)
{
	// Check for a ROM fixup.
	const string serialNumber = d->rom->rom_serial();
	const uint16_t checksum = d->rom->checksum();
	const uint32_t crc32 = d->rom->rom_crc32();
	d->romFixup = d->CheckRomFixups(serialNumber, checksum, crc32);

	// Check for EEPROM.
	// TODO: Change DetectEEPRomType to use a full serial?
	const char *eep_serial = (serialNumber.c_str() + 3);
	d->eprType = EEPRomI2C::DetectEEPRomType(
			eep_serial,
			(serialNumber.size() - 3),
			checksum);

	// Set the ROM mapper.
	const RomCartridgeMDPrivate::MD_RomFixup_t *fixup = nullptr;
	if (d->romFixup >= 0) {
		fixup = &RomCartridgeMDPrivate::MD_RomFixups[d->romFixup];
		m_mapper.type = fixup->mapperType;
	} else {
		// No fixup for this ROM.
		// Use flat addressing.
		m_mapper.type = MAPPER_MD_FLAT;
	}

	// Set mapper registers based on type.
	switch (m_mapper.type) {
		default:
		case MAPPER_MD_FLAT:
			// Assign all banks normally.
			for (int i = 0; i < ARRAY_SIZE(m_cartBanks); i++)
				m_cartBanks[i] = BANK_ROM_00 + i;
			break;

		case MAPPER_MD_SSF2:
			// Assign banks $000000-$3FFFFF only.
			for (int i = 0; i < 8; i++)
				m_cartBanks[i] = BANK_ROM_00 + i;
			for (int i = 8; i < ARRAY_SIZE(m_cartBanks); i++)
				m_cartBanks[i] = BANK_UNUSED;

			// Initialize SSF2 banks to 0xFF (default).
			memset(m_mapper.ssf2.banks, 0xFF, sizeof(m_mapper.ssf2.banks));
			break;

		case MAPPER_MD_REGISTERS_RO:
			// Read-only registers after the ROM area.
			for (int i = 0; i < 8; i++)
				m_cartBanks[i] = BANK_ROM_00 + i;
			for (int i = 8; i < ARRAY_SIZE(m_cartBanks); i++)
				m_cartBanks[i] = BANK_MD_REGISTERS_RO;

			// Copy the register data from the fixups table.
			memcpy(&m_mapper.registers_ro, &fixup->registers_ro,
				sizeof(m_mapper.registers_ro));
			break;

#if 0
		case MAPPER_MD_REALTEC:
			// TODO: Implement these mappers.
			break;
#endif
	}

	// 32X banking.
	if (m_mars) {
		// TODO: 0x400000-0x7FFFFF?

		// 0x800000-0x87FFFF: Framebuffer.
		m_cartBanks[16] = BANK_UNUSED;

		// 0x880000-0x8FFFFF: Maps to ROM 0x000000-0x7FFFFF.
		m_cartBanks[17] = BANK_ROM_00;

		// 0x900000-0x9FFFFF: Maps to a selectable 1 MB ROM bank.
		// Defaults to 0. (ROM 0x000000 - 0x0FFFFF)
		m_mars_bank_reg = 0;
		updateMarsBanking();
	}
}

/**
 * Update Mars banking at $900000-$9FFFFF.
 */
void RomCartridgeMD::updateMarsBanking(void)
{
	if (!m_mars)
		return;

	// 0x900000-0x9FFFFF: Maps to a selectable 1 MB ROM bank.
	// Defaults to 0. (ROM 0x000000 - 0x0FFFFF)
	const uint8_t bank_start = BANK_ROM_00 + ((m_mars_bank_reg & 3) * 2);
	m_cartBanks[18] = bank_start;
	m_cartBanks[19] = bank_start + 1;
}

/** ZOMG savestate functions. **/

/**
 * Save the cartridge data, including /TIME, SRAM, and/or EEPROM.
 * @param zomg ZOMG savestate to save to.
 */
void RomCartridgeMD::zomgSave(LibZomg::Zomg *zomg) const
{
	// Save the MD /TIME registers.
	Zomg_MD_TimeReg_t md_time_reg_save;
	memset(md_time_reg_save.reg, 0xFF, sizeof(md_time_reg_save.reg));

	// SRAM / EEPROM control registers.
	if (!m_EEPRom.isEEPRomTypeSet()) {
		// EEPRom is disabled. Use SRam.
		// Save SRam control registers to the /TIME register bank.
		md_time_reg_save.SRAM_ctrl = m_SRam.zomgReadCtrl();

		// Save SRAM.
		// TODO: Make this optional.
		m_SRam.zomgSave(zomg);
	} else {
		// Save the EEPROM control registers and data.
		// TODO: Make saving EEPROM data optional?
		m_EEPRom.zomgSave(zomg);
	}

	// Check if we have to save any bankswitching registers.
	switch (m_mapper.type) {
		case MAPPER_MD_SSF2: {
			// TODO: Move SSF2 bankswitching data out of TIME_reg?
			for (int address = 0xF3; address <= 0xFF; address += 2) {
				const uint8_t phys_bank = (address & 0xF) >> 1;
				md_time_reg_save.reg[address] = m_mapper.ssf2.banks[phys_bank];
			}
			break;
		}

		default:
			break;
	}

	// Write MD /TIME registers.
	zomg->saveMD_TimeReg(&md_time_reg_save);
}

/**
 * Restore the cartridge data, including /TIME, SRAM, and/or EEPROM.
 * @param zomg ZOMG savestate to restore from.
 * @param loadSaveData If true, load the save data in addition to the state.
 */
void RomCartridgeMD::zomgRestore(LibZomg::Zomg *zomg, bool loadSaveData)
{
	Zomg_MD_TimeReg_t md_time_reg_save;
	int ret = zomg->loadMD_TimeReg(&md_time_reg_save);

	// SRAM / EEPROM control registers.
	if (!m_EEPRom.isEEPRomTypeSet()) {
		// EEPRom is disabled. Use SRam.
		// Load SRam control registers from the /TIME register bank.
		if (ret <= 0xF1) {
			// SRAM control register wasn't present.
			// If the ROM is less than 2 MB, force SRAM access on, write-enabled.
			// Otherwise, set SRAM off, write-protected.
			// TODO: Save a flag somewhere to indicate that this should be set
			// instead of checking M68K_Mem::Rom_Size.
			if (m_romData_size < 0x200000)
				m_SRam.writeCtrl(1);
			else
				m_SRam.writeCtrl(2);
		} else {
			// SRAM control register was present.
			// Write the value from the savestate.
			m_SRam.writeCtrl(md_time_reg_save.SRAM_ctrl);
		}

		// Load SRAM.
		if (loadSaveData) {
			m_SRam.zomgRestore(zomg);
		}
	} else {
		// Load EEPROM.
		m_EEPRom.zomgRestore(zomg, loadSaveData);
	}

	// Check if we have to restore any bankswitching registers.
	switch (m_mapper.type) {
		case MAPPER_MD_SSF2: {
			// TODO: Move SSF2 bankswitching data out of TIME_reg?
			for (int address = 0xF3; address <= 0xFF; address += 2) {
				const uint8_t phys_bank = (address & 0xF) >> 1;
				uint8_t virt_bank = (md_time_reg_save.reg[address] & 0x3F);

				// Check if the virtual bank address is in range.
				const uint8_t max_virt_bank = (m_romData_size / 524288);
				if (virt_bank >= max_virt_bank) {
					// Out of range. Use the default value.
					virt_bank = phys_bank;
					m_mapper.ssf2.banks[phys_bank] = 0xFF;
				} else {
					// Save the bank number.
					m_mapper.ssf2.banks[phys_bank] = virt_bank;
				}

				// Update the memory map.
				m_cartBanks[phys_bank] = (BANK_ROM_00 + virt_bank);
			}

			if (m_mars)
				updateMarsBanking();
			// TODO: Better way to update Starscream?
			M68K::UpdateSysBanking();
			break;
		}

		default:
			break;
	}
}

}
