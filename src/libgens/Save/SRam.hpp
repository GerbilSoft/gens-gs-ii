/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SRam.hpp: Save RAM handler.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_SAVE_SRAM_HPP__
#define __LIBGENS_SAVE_SRAM_HPP__

// C includes.
#include <stdint.h>
#include <limits.h>

// C++ includes.
#include <string>

// ZOMG
#include "libzomg/Zomg.hpp"

namespace LibGens
{

class SRam
{
	public:
		SRam()
		{
			// Reset SRam on startup.
			reset();
		}
		
		/**
		 * reset(): Clear SRam and initialize settings.
		 */
		void reset(void);
		
		/** Settings. **/
		
		bool isOn(void) const { return m_on; }
		void setOn(bool newOn) { m_on = newOn; }
		
		bool isWrite(void) const { return m_write; }
		void setWrite(bool newWrite) { m_write = newWrite; }
		
		// TODO: Mask off the high byte in addresses?
		uint32_t start(void) const { return m_start; }
		void setStart(uint32_t newStart) { m_start = newStart; }
		
		uint32_t end(void) const { return m_end; }
		void setEnd(uint32_t newEnd) { m_end = newEnd; }
		
		/** Convenience functions. **/
		
		/**
		 * canRead(): Determines if the M68K can read from SRam.
		 * @return True if SRam is readable by the M68K; false if it can't.
		 */
		bool canRead(void) const
		{
			return (m_on);
		}
		
		/**
		 * canWrite(): Determines if the M68K can write to SRam.
		 * @return True if SRam is writable by the M68K; false if it can't.
		 */
		bool canWrite(void) const
		{
			return (m_on && m_write);
		}
		
		/**
		 * writeCtrl(): Write the SRam control register. (0xA130F1)
		 * @param ctrl Control register data.
		 */
		void writeCtrl(uint8_t data)
		{
			m_on = (data & 1);
			m_write = !(data & 2);
		}
		
		/**
		 * zomgReadCtrl(): Read the SRam control register. (0xA130F1)
		 * NOTE: This is NOT to be used by emulation code!
		 * It is ONLY to be used by the ZOMG savestate function!
		 * (Reading 0xA130F1 doesn't work on hardware.)
		 */
		uint8_t zomgReadCtrl(void) const
		{
			return (m_on | (!m_write << 2));
		}
		
		/**
		 * isAddressInRange(): Determines if a given address is in the SRam's range.
		 * return True if the address is in range; false if it isn't.
		 */
		bool isAddressInRange(uint32_t address) const
		{
			return (address >= m_start && address <= m_end);
		}
		
		/** Memory read/write. **/
		/** NOTE: These functions use absolute addresses! **/
		/** e.g. 0x200000 - 0x20FFFF **/
		
		uint8_t readByte(uint32_t address) const
		{
			// Note: SRam is NOT byteswapped.
			// TODO: Bounds checking.
			// TODO: SRAM enable check.
			return m_sram[address - m_start];
		}
		
		uint16_t readWord(uint32_t address) const
		{
			// Note: SRam is NOT byteswapped.
			// TODO: Bounds checking.
			// TODO: Proper byteswapping.
			// TODO: SRAM enable check.
			address -= m_start;
			return ((m_sram[address] << 8) | m_sram[address + 1]);
		}
		
		void writeByte(uint32_t address, uint8_t data)
		{
			// Note: SRam is NOT byteswapped.
			// TODO: Bounds checking.
			// TODO: Write protection, SRAM enable check.
			m_sram[address - m_start] = data;
			
			// Set the dirty flag.
			// TODO: Only if the word was actually modified?
			setDirty();
		}
		
		void writeWord(uint32_t address, uint16_t data)
		{
			// Note: SRam is NOT byteswapped.
			// TODO: Bounds checking.
			// TODO: Proper byteswapping.
			// TODO: Write protection, SRAM enable check.
			address -= m_start;
			m_sram[address] = ((data >> 8) & 0xFF);
			m_sram[address] = (data & 0xFF);
			
			// Set the dirty flag.
			// TODO: Only if the word was actually modified?
			setDirty();
		}
		
		/**
		 * isDirty(): Determine if the SRam is dirty.
		 * @return True if SRam has been modified since the last save; false otherwise.
		 */
		inline bool isDirty(void) const { return m_dirty; }
		
		// TODO: SRam directory path.
		// For now, just save in the ROM directory.
		void setFilename(const std::string &filename);
		
		/**
		 * load(): Load the SRam file.
		 * @return Positive value indicating SRam size on success; negative on error.
		 */
		int load(void);
		
		/**
		 * save(): Save the SRam file.
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int save(void);
		
		/**
		 * autoSave(): Autosave the SRam file.
		 * This saves the SRam file if its last modification time is past a certain threshold.
		 * @param framesElapsed Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int autoSave(int framesElapsed);
		
		/**
		 * AUTOSAVE_THRESHOLD_DEFAULT: Default autosave threshold, in milliseconds.
		 */
		static const int AUTOSAVE_THRESHOLD_DEFAULT = 1000;
		
		/**
		 * getUsedSize(): Determine how many bytes are used in the SRam chip.
		 * @return Number of bytes used, rounded to the highest power of two.
		 */
		int getUsedSize(void) const;
		
		/** ZOMG functions. **/
		// TODO: Do we really want to access the ZOMG file from SRam.cpp directly?
		// The other method requires accessing m_sram from GensZomg.cpp.
		int loadFromZomg(LibZomg::Zomg &zomg);
		int saveToZomg(LibZomg::Zomg &zomg) const;
	
	protected:
		uint8_t m_sram[64*1024];
		bool m_on;	// Is SRam enabled?
		bool m_write;	// Is SRam writable?
		
		uint32_t m_start;	// SRam starting address.
		uint32_t m_end;		// SRam ending address.
		
		// Dirty flag.
		bool m_dirty;
		int m_framesElapsed;
		
		inline void setDirty(void) { m_dirty = true; m_framesElapsed = 0; }
		inline void clearDirty(void) { m_dirty = false; m_framesElapsed = 0; }
		
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
};

}

#endif /* __LIBGENS_SAVE_SRAM_HPP__ */
