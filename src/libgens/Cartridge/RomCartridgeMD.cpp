/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * RomCartridgeMD.cpp: MD ROM cartridge handler.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2013 by David Korth.                                 *
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

#include "RomCartridgeMD.hpp"

#include "../EmuContext.hpp"
#include "../Util/byteswap.h"
#include "../macros/common.h"

/**
 * References:
 * - ssf2.txt, Second Edition (2000/07/26) by Bart Trzynadlowski
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
	private:
		RomCartridgeMDPrivate() { }
		~RomCartridgeMDPrivate();

		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		RomCartridgeMDPrivate(const RomCartridgeMDPrivate &);
		RomCartridgeMDPrivate &operator=(const RomCartridgeMDPrivate &);

	public:
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

			// ROM mapper.
			// TODO: Add const register values.
			RomCartridgeMD::MD_MapperType_t mapperType;
		};

		static const MD_RomFixup_t MD_RomFixups[];
};

/**
 * ROM fixup table. (Mega Drive)
 * TODO: Allow loading this table from a data file.
 */
const RomCartridgeMDPrivate::MD_RomFixup_t RomCartridgeMDPrivate::MD_RomFixups[] =
{
	// Puggsy: Shows an anti-piracy message after the third level if SRAM is detected.
	{{"GM T-113016", 0, 0}, {0, 0, true}, RomCartridgeMD::MAPPER_MD_FLAT},
	{{"GM T-550055", 0, 0}, {0, 0, true}, RomCartridgeMD::MAPPER_MD_FLAT},	// Puggsy (Beta)

	// Psy-O-Blade: Incorrect SRAM header.
	{{"GM T-26013 ", 0, 0}, {0x200000, 0x203FFF, false}, RomCartridgeMD::MAPPER_MD_FLAT},

	// Super Street Fighter II: Use SSF2 mapper.
	{{"GM T-12056 ", 0, 0}, {0, 0, true}, RomCartridgeMD::MAPPER_MD_SSF2},	// US
	{{"GM MK-12056", 0, 0}, {0, 0, true}, RomCartridgeMD::MAPPER_MD_SSF2},	// EU
	{{"GM T-12043 ", 0, 0}, {0, 0, true}, RomCartridgeMD::MAPPER_MD_SSF2},	// JP

	/**
	 * Xin Qi Gai Wang Zi (original version of Beggar Prince):
	 * SRAM is located at 0x400000-0x40FFFF; ROM header is invalid.
	 * 
	 * CRC32s:
	 * - Xin Qi Gai Wang Zi (Ch).gen:	DD2F38B5
	 * - Xin Qi Gai Wang Zi (Ch) [a1].gen:	DA5A4BFE
	 */
	{{nullptr, 0, 0xDD2F38B5}, {0x400000, 0x40FFFF, false}, RomCartridgeMD::MAPPER_MD_FLAT},
	{{nullptr, 0, 0xDA5A4BFE}, {0x400000, 0x40FFFF, false}, RomCartridgeMD::MAPPER_MD_FLAT},

	// End of list.
	{{nullptr, 0, 0}, {0, 0, false}, RomCartridgeMD::MAPPER_MD_FLAT}
};


/*****************************
 * RomCartridgeMD functions. *
 *****************************/

RomCartridgeMD::RomCartridgeMD()
	: m_romData(nullptr)
	, m_romData_size(0)
{ }

RomCartridgeMD::~RomCartridgeMD()
{
	free(m_romData);
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
 * Read a byte from the constant register area.
 * MAPPER_MD_CONST_400000
 * @param address Address.
 * @return Byte from constant register area.
 */
inline uint8_t RomCartridgeMD::readByte_CONST_400000(uint32_t address)
{
	address &= 0x7FFFF;
	if (address > sizeof(m_mapper.const_400000.reg))
		return 0xFF;

	// Check if this register is defined.
	if (m_mapper.const_400000.byte_read_mask & (1 << address)) {
		// Register is defined.
		return m_mapper.const_400000.reg[address];
	}

	// Register is not defined.
	return 0xFF;
}

/**
 * Read a Word from the constant register area.
 * MAPPER_MD_CONST_400000
 * @param address Address.
 * @return Word from constant register area.
 */
inline uint16_t RomCartridgeMD::readWord_CONST_400000(uint32_t address)
{
	address &= 0x7FFFE;
	if (address > sizeof(m_mapper.const_400000.reg))
		return 0xFF;

	uint16_t ret = 0;

	// Check if the MSB register is defined.
	if (m_mapper.const_400000.byte_read_mask & (1 << address)) {
		// Register is defined.
		ret |= (m_mapper.const_400000.reg[address] << 8);
	} else {
		// Register is not defined.
		ret |= 0xFF00;
	}

	// Check if the LSB register is defined.
	address++;
	if (m_mapper.const_400000.byte_read_mask & (1 << address)) {
		// Register is defined.
		ret |= m_mapper.const_400000.reg[address];
	} else {
		// Register is not defined.
		ret |= 0xFF;
	}

	return ret;;
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
		case BANK_MD_CONST_400000:	return readByte_CONST_400000(address);
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
			if (m_EEPRom.isReadBytePort(address)) {
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
		case BANK_MD_CONST_400000:	return readWord_CONST_400000(address);
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
	// No registers are readable from this area...
	((void)address);
	return 0xFF;
}

/**
 * Read a word from the /TIME registers. ($A130xx)
 * @param address Register address.
 * @return Word from /TIME.
 */
uint16_t RomCartridgeMD::readWord_TIME(uint8_t address)
{
	// No registers are readable from this area...
	((void)address);
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
		m_SRam.writeCtrl(address);
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
			// TODO: Update Starscream instruction fetch.
			m_cartBanks[phys_bank] = (BANK_ROM_00 + virt_bank);
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
		m_SRam.writeCtrl(address);
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
			// TODO: Update Starscream instruction fetch.
			m_cartBanks[phys_bank] = (BANK_ROM_00 + virt_bank);
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
	// TODO: Identify the ROM.
	// For now, assume MAPPER_MD_FLAT.
	m_mapper.type = MAPPER_MD_FLAT;

	switch (m_mapper.type) {
		default:
		case MAPPER_MD_FLAT:
			// Assign all banks normally.
			for (int i = 0; i < ARRAY_SIZE(m_cartBanks); i++)
				m_cartBanks[i] = BANK_ROM_00 + i;
			break;

		case MAPPER_MD_SSF2:
			// Assign banks $000000-$3FFFFF only.
			for (int i = 0; i < 7; i++)
				m_cartBanks[i] = BANK_ROM_00 + i;
			for (int i = 8; i < ARRAY_SIZE(m_cartBanks); i++)
				m_cartBanks[i] = BANK_UNUSED;

			// Initialize SSF2 banks to 0xFF (default).
			memset(m_mapper.ssf2.banks, 0xFF, sizeof(m_mapper.ssf2.banks));
			break;

		case MAPPER_MD_CONST_400000:
		case MAPPER_MD_REALTEC:
			// TODO: Implement these mappers.
			break;
	}
}

}
