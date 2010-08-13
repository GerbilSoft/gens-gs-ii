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

#include <stdint.h>

namespace LibGens
{

class EEPRom
{
	public:
		EEPRom();
		
		void reset(void);
		
		uint8_t portReadByte(uint32_t address);
		uint16_t portReadWord(uint32_t address);
		
		void portWriteByte(uint32_t address, uint8_t data);
		void portWriteWord(uint32_t address, uint16_t data);
		
		// TODO: Add load/save functions.
		
	protected:
		void portWriteInt(void);
		
		// EEPRom. (8 KB)
		static const uint16_t EEPROM_ADDRESS_MASK = 0x1FFF;
		uint8_t m_eeprom[0x2000];
		
		// EEPRom type.
		int m_type;
		
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
};

}

#endif /* __LIBGENS_SAVE_EEPROM_HPP__ */
