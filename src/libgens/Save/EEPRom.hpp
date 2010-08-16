/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRom.hpp: Serial EEPROM handler.                                      *
 *                                                                         *
 * Copyright (C) 2007, 2008, 2009  Eke-Eke (Genesis Plus GCN/Wii port)     *
 * Copyright (c) 2010 by David Korth.                                      *
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

/**
 * Serial EEPROM information from gen_eeprom.pdf
 * http://genplus-gx.googlecode.com/files/gen_eeprom.pdf
 */

#ifndef __LIBGENS_SAVE_EEPROM_HPP__
#define __LIBGENS_SAVE_EEPROM_HPP__

// C includes.
#include <stdint.h>
#include <string.h>
#include <limits.h>

// C++ includes.
#include <string>

namespace LibGens
{

class EEPRom
{
	public:
		EEPRom();
		
		void reset(void);
		
		/**
		 * DetectEEPRomType(): Detect the EEPRom type used by the specified ROM.
		 * @param serial Serial number. (NOTE: This does NOT include the "GM " prefix!)
		 * @param serial_len Length of the serial number string.
		 * @param checksum Checksum.
		 * @return EEPRom type, or -1 if this ROM isn't known.
		 */
		static int DetectEEPRomType(const char *serial, size_t serial_len, uint16_t checksum);
		
		/**
		 * setEEPRomType(): Set the EEPRom type.
		 * @param type EEPRom type. (Specify a negative number to clear)
		 * @return 0 on success; non-zero on error.
		 */
		int setEEPRomType(int type);
		
		/**
		 * isEEPRomTypeSet(): Determine if the EEPRom type is set.
		 * @return True if the EEPRom type is set; false if not.
		 */
		inline bool isEEPRomTypeSet(void)
		{
			return (!(m_eprType.type.scl_adr == 0));
		}
		
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
		
		inline bool isReadBytePort(uint32_t address)
		{
			return (address == m_eprType.type.sda_out_adr);
		}
		
		inline bool isReadWordPort(uint32_t address)
		{
			return ((address | 1) == (m_eprType.type.sda_out_adr | 1));
		}
		
		inline bool isWriteBytePort(uint32_t address)
		{
			return (address == m_eprType.type.scl_adr ||
				address == m_eprType.type.sda_in_adr);
		}
		
		inline bool isWriteWordPort(uint32_t address)
		{
			address |= 1;
			return ((address == (m_eprType.type.scl_adr | 1)) ||
				(address == (m_eprType.type.sda_in_adr | 1)));
		}
		
		uint8_t readByte(uint32_t address);
		uint16_t readWord(uint32_t address);
		
		void writeByte(uint32_t address, uint8_t data);
		void writeWord(uint32_t address, uint16_t data);
		
		/**
		 * isDirty(): Determine if the SRam is dirty.
		 * @return True if SRam has been modified since the last save; false otherwise.
		 */
		inline bool isDirty(void) const { return m_dirty; }
		
		// TODO: EEPRom directory path.
		// For now, just save in the ROM directory.
		void setFilename(const std::string &filename);
		
		/**
		 * load(): Load the EEPRom file.
		 * @return Positive value indicating EEPRom size on success; negative on error.
		 */
		int load(void);
		
		/**
		 * save(): Save the EEPRom file.
		 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
		 */
		int save(void);
		
		/**
		 * autoSave(): Autosave the EEPRom file.
		 * This saves the EEPRom file if its last modification time is past a certain threshold.
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int autoSave(void);
	
	protected:
		void processWriteCmd(void);
		
		// EEPRom. (8 KB)
		static const uint16_t EEPROM_ADDRESS_MASK = 0x1FFF;
		uint8_t m_eeprom[0x2000];
		
		// EEPRom state.
		bool m_scl;		// /SCL: Clock.
		bool m_sda;		// /SDA: Data.
		
		bool m_old_scl;		// /SCL: Clock. (Previous state)
		bool m_old_sda;		// /SDA: Data. (Previous state)
		
		int m_counter;		// Cycle counter.
		bool m_rw;		// Read/Write mode. (1 == read; 0 == write)
		
		uint16_t m_slave_mask;		// Device address. (shifted by the memory address width)
		uint16_t m_word_address;	// Memory address.
		
		enum EEPRomState
		{
			EEP_STANDBY,
			EEP_WAIT_STOP,
			EEP_GET_WORD_ADR_7BITS,
			EEP_GET_SLAVE_ADR,
			EEP_GET_WORD_ADR_HIGH,
			EEP_GET_WORD_ADR_LOW,
			EEP_READ_DATA,
			EEP_WRITE_DATA,
		};
		EEPRomState m_state;
		
		void checkStart(void);
		void checkStop(void);
		
		// EEPROM types.
		// Ported from Genesis Plus.
		struct EEPRomType
		{
			uint8_t address_bits;		// Number of bits needed to address memory: 7, 8, or 16.
			uint16_t size_mask;		// Depends on the maximum size of the memory. (in bytes)
			uint16_t pagewrite_mask;	// Depends on the maximum number of bytes that can be written in a single write cycle.
			
			uint32_t sda_in_adr;		// 68000 memory address mapped to SDA_IN.
			uint32_t sda_out_adr;		// 68000 memory address mapped to SDA_OUT.
			uint32_t scl_adr;		// 68000 memory address mapped to SCL.
			uint8_t sda_in_bit;		// Bit offset for SDA_IN.
			uint8_t sda_out_bit;		// Bit offset for SDA_OUT.
			uint8_t scl_bit;		// Bit offset for SCL.
		};
		
		// EEPROM definitions.
		// Ported from Genesis Plus.
		struct GameEEPRomInfo
		{
			char game_id[16];
			uint16_t checksum;
			EEPRomType type;
		};
		static const GameEEPRomInfo ms_Database[];
		
		// Current EEPRom type.
		GameEEPRomInfo m_eprType;
		
		// Dirty flag.
		bool m_dirty;
		
		/** EEPRom file handling functions. **/
		
		// Filename.
		static const char *ms_FileExt;
		std::string m_filename;
		
		// Find the next highest power of two. (unsigned integers)
		// http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
		template <class T>
		static inline T next_pow2u(T k)
		{
			if (k == 0)
				return 1;
			k--;
			for (unsigned int i = 1; i < sizeof(T)*CHAR_BIT; i <<= 1)
				k = k | k >> i;
			return k + 1;
		}
		
		int getUsedSize(void);
};

}

#endif /* __LIBGENS_SAVE_EEPROM_HPP__ */
