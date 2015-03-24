/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SRam.hpp: Save RAM handler.                                             *
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

#ifndef __LIBGENS_SAVE_SRAM_HPP__
#define __LIBGENS_SAVE_SRAM_HPP__

// C includes.
#include <stdint.h>
// C++ includes.
#include <string>

// ZOMG
#include "libzomg/Zomg.hpp"

namespace LibGens {

class SRamPrivate;
class SRam
{
	public:
		SRam();
		~SRam();

	protected:
		friend class SRamPrivate;
		SRamPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		SRam(const SRam &);
		SRam &operator=(const SRam &);

	public:
		/**
		 * Clear SRam and initialize settings.
		 */
		void reset(void);
		
		/** Settings. **/
		inline bool isOn(void) const;
		inline void setOn(bool on);
		inline bool isWrite(void) const;
		inline void setWrite(bool write);
		inline uint32_t start(void) const;
		inline void setStart(uint32_t start);
		inline uint32_t end(void) const;
		inline void setEnd(uint32_t end);

		/** Convenience functions. **/

		inline bool canRead(void) const;
		inline bool canWrite(void) const;

		inline void writeCtrl(uint8_t data);
		inline uint8_t zomgReadCtrl(void) const;

		inline bool isAddressInRange(uint32_t address) const;

		/** Memory read/write. **/
		/** NOTE: These functions use absolute addresses! **/
		/** e.g. 0x200000 - 0x20FFFF **/
		uint8_t readByte(uint32_t address) const;
		uint16_t readWord(uint32_t address) const;
		void writeByte(uint32_t address, uint8_t data);
		void writeWord(uint32_t address, uint16_t data);

		/**
		 * Check if the SRam is dirty.
		 * @return True if SRam has been modified since the last save; false otherwise.
		 */
		inline bool isDirty(void) const;

		// SRam filename and pathname.
		void setFilename(const std::string &filename);
		void setPathname(const std::string &pathname);

		/**
		 * Load the SRam file.
		 * @return Positive value indicating SRam size on success; negative on error.
		 */
		int load(void);

		/**
		 * Save the SRam file.
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int save(void);
		
		/**
		 * Autosave the SRam file.
		 * This saves the SRam file if its last modification time is past a certain threshold.
		 * @param framesElapsed Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int autoSave(int framesElapsed);
		
		/** ZOMG functions. **/
		// TODO: Do we really want to access the ZOMG file from SRam.cpp directly?
		// The other method requires accessing m_sram from GensZomg.cpp.
		int loadFromZomg(LibZomg::Zomg *zomg);
		int saveToZomg(LibZomg::Zomg *zomg) const;

	protected:
		// Dirty flag.
		void setDirty(void);
		void clearDirty(void);

	private:
		// SRam data.
		uint8_t m_sram[64*1024];
		bool m_on;	// Is SRam enabled?
		bool m_write;	// Is SRam writable?
		
		uint32_t m_start;	// SRam starting address.
		uint32_t m_end;		// SRam ending address.
		
		// Dirty flag.
		bool m_dirty;
		int m_framesElapsed;
};

/** Settings. **/

inline bool SRam::isOn(void) const
	{ return m_on; }
inline void SRam::setOn(bool on)
	{ m_on = on; }

inline bool SRam::isWrite(void) const
	{ return m_write; }
inline void SRam::setWrite(bool write)
	{ m_write = write; }

// TODO: Mask off the high byte in addresses?
inline uint32_t SRam::start(void) const
	{ return m_start; }
inline void SRam::setStart(uint32_t start)
	{ m_start = start; }

inline uint32_t SRam::end(void) const
	{ return m_end; }
inline void SRam::setEnd(uint32_t end)
	{ m_end = end; }

/** Convenience functions. **/

/**
 * Check if the M68K can read from SRam.
 * @return True if SRam is readable by the M68K; false if it can't.
 */
inline bool SRam::canRead(void) const
	{ return (m_on); }

/**
 * Check if the M68K can write to SRam.
 * @return True if SRam is writable by the M68K; false if it can't.
 */
inline bool SRam::canWrite(void) const
	{ return (m_on && m_write); }

/**
 * Write the SRam control register. (0xA130F1)
 * @param ctrl Control register data.
 */
inline void SRam::writeCtrl(uint8_t data)
{
	m_on = (data & 1);
	m_write = !(data & 2);
}

/**
 * Read the SRam control register. (0xA130F1)
 * NOTE: This is NOT to be used by emulation code!
 * It is ONLY to be used by the ZOMG savestate function!
 * (Reading 0xA130F1 doesn't work on hardware.)
 * TODO: Rename to dbg_readCtrl()?
 */
inline uint8_t SRam::zomgReadCtrl(void) const
	{ return (!!m_on | (!m_write << 2)); }

/**
 * Check if a given address is in the SRam's range.
 * return True if the address is in range; false if it isn't.
 */
inline bool SRam::isAddressInRange(uint32_t address) const
	{ return (address >= m_start && address <= m_end); }

/**
 * Check if the SRam is dirty.
 * @return True if SRam has been modified since the last save; false otherwise.
 */
inline bool SRam::isDirty(void) const
	{ return m_dirty; }

/** Inline protected functions. **/

inline void SRam::setDirty(void)
	{ m_dirty = true; m_framesElapsed = 0; }

inline void SRam::clearDirty(void)
	{ m_dirty = false; m_framesElapsed = 0; }

}

#endif /* __LIBGENS_SAVE_SRAM_HPP__ */
