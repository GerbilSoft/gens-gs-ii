/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SRam.cpp: Save RAM handler.                                             *
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

#include "SRam.hpp"
#include "macros/common.h"
#include "libgenstext/StringManip.hpp"

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
#include "libcompat/W32U/W32U_mini.h"
#endif

// ZOMG
#include "libzomg/Zomg.hpp"

// C includes. (C++ namespace)
#include <climits>
#include <cstdio>
#include <cstring>

// C++ includes.
#include <string>
using std::string;

namespace LibGens {

/** SRamPrivate **/

// TODO: Optimize the private class more.
// (Move more stuff from SRam into SRamPrivate.)
class SRamPrivate
{
	public:
		SRamPrivate(SRam *q);

	protected:
		friend class SRam;
		SRam *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		SRamPrivate(const SRamPrivate &);
		SRamPrivate &operator=(const SRamPrivate &);

	public:
		uint8_t sram[64*1024];

		// Filename.
		static const char fileExt[];
		std::string filename;		// SRam base filename.
		std::string pathname;		// SRam pathname.
		std::string fullPathname;	// Full pathname. (m_pathname + m_filename)

		/**
		 * Default autosave threshold, in milliseconds.
		 */
		static const int AUTOSAVE_THRESHOLD_DEFAULT = 1000;
		
		/**
		 * Determine how many bytes are used in the SRam chip.
		 * @return Number of bytes used, rounded to the highest power of two.
		 */
		int getUsedSize(void) const;

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

const char SRamPrivate::fileExt[] = "srm";

SRamPrivate::SRamPrivate(SRam *q)
	: q(q)
{ }

/**
 * Determine how many bytes are used in the SRam chip.
 * @return Number of bytes used, rounded to the highest power of two.
 */
int SRamPrivate::getUsedSize(void) const
{
	int i = (sizeof(q->m_sram) - 1);
	while (i > 0 && q->m_sram[i] == 0xFF) {
		i--;
	}

	// Make sure we return 0 if SRam is empty.
	if (i <= 0)
		return 0;

	// Return the next-highest power of two.
	return next_pow2u(i);
}

/** SRam **/

SRam::SRam()
	: d(new SRamPrivate(this))
{
	// Reset SRam on startup.
	reset();
}

SRam::~SRam()
{
	delete d;
}

/**
 * Clear SRam and initialize settings.
 */
void SRam::reset(void)
{
	/**
	 * SRam is initialized with 0xFF.
	 * This is similar to how it's initialized on the actual hardware.
	 * It's also required in order to work around bugs in at least
	 * two games that expect factory-formatted SRam:
	 * - Micro Machines 2
	 * - Dino Dini's Soccer
	 * 
	 * NOTE: m_enabled is not set here,
	 * since that's a user setting.
	 */
	
	memset(m_sram, 0xFF, sizeof(m_sram));
	m_on = false;
	m_write = false;
	clearDirty();
}

/** Memory read/write. **/
/** NOTE: These functions use absolute addresses! **/
/** e.g. 0x200000 - 0x20FFFF **/

uint8_t SRam::readByte(uint32_t address) const
{
	if (address < m_start || address > m_end)
		return 0xFF;

	// Note: SRam is NOT byteswapped.
	// TODO: SRAM enable check.
	// NOTE: SRAM enable is currently handled in RomCartridgeMD.
	address -= m_start;
	return m_sram[address];
}

uint16_t SRam::readWord(uint32_t address) const
{
	if (address < m_start || address > m_end)
		return 0xFFFF;

	// Note: SRam is NOT byteswapped.
	// TODO: Proper byteswapping.
	// TODO: SRAM enable check.
	// NOTE: SRAM enable is currently handled in RomCartridgeMD.
	address -= m_start;
	return ((m_sram[address] << 8) | m_sram[address + 1]);
}

void SRam::writeByte(uint32_t address, uint8_t data)
{
	if (address < m_start || address > m_end)
		return;

	// Note: SRam is NOT byteswapped.
	// TODO: Write protection, SRAM enable check.
	// NOTE: Both of these are currently handled in RomCartridgeMD.
	address -= m_start;
	if (m_sram[address] != data) {
		m_sram[address] = data;
		// Set the dirty flag.
		setDirty();
	}
}

void SRam::writeWord(uint32_t address, uint16_t data)
{
	if (address < m_start || address > m_end)
		return;

	// Note: SRam is NOT byteswapped.
	// TODO: Proper byteswapping.
	// TODO: Write protection, SRAM enable check.
	// NOTE: Both of these are currently handled in RomCartridgeMD.
	address -= m_start;
	const uint8_t hi = ((data >> 8) & 0xFF);
	const uint8_t lo = (data & 0xFF);
	if (m_sram[address] != hi || m_sram[address+1] != lo) {
		m_sram[address] = hi;
		m_sram[address+1] = lo;
		// Set the dirty flag.
		setDirty();
	}
}

/**
 * Set the SRam filename based on a ROM filename.
 * The file extension is changed to ".srm".
 * @param filename ROM filename.
 */
void SRam::setFilename(const string& filename)
{
	if (filename.empty()) {
		// Empty filename.
		d->filename.clear();
		d->fullPathname = d->pathname;
		return;
	}

	// Remove any subdirectories and extensions from the ROM filename.
	d->filename = LibGensText::FilenameBaseNoExt(filename);
	// Append our extension.
	d->filename += '.';
	d->filename += d->fileExt;

	// Set the full pathname.
	d->fullPathname = d->pathname + d->filename;
}

/**
 * Set the SRam pathname.
 * @param pathname SRam pathname.
 */
void SRam::setPathname(const string& pathname)
{
	d->pathname = pathname;

	// Rebuild the full pathname.
	if (d->pathname.empty()) {
		d->fullPathname = d->filename;
	} else {
		// Make sure the pathname ends with a separator.
		if (d->pathname.at(d->pathname.size()-1) != LG_PATH_SEP_CHR) {
			d->pathname += LG_PATH_SEP_CHR;
		}

		// Create the full pathname.
		d->fullPathname = d->pathname + d->filename;
	}
}

/**
 * Load the SRam file.
 * @return Positive value indicating SRam size on success; negative on error.
 */
int SRam::load(void)
{
	// Attempt to open the SRam file.
	FILE *f = fopen(d->fullPathname.c_str(), "rb");
	if (!f) {
		// Unable to open the SRam file.
		// TODO: Error code constants.
		return -1;
	}

	// Erase SRam before loading the file.
	// NOTE: Don't use reset() - reset() also resets the control logic.
	memset(m_sram, 0xFF, sizeof(m_sram));

	// Load the SRam data.
	int ret = fread(m_sram, 1, sizeof(m_sram), f);
	fclose(f);

	// Return the number of bytes read.
	clearDirty();
	return ret;
}

/**
 * Save the SRam file.
 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
 */
int SRam::save(void)
{
	if (!m_dirty)
		return 0;

	int size = d->getUsedSize();
	if (size <= 0) {
		// SRam is empty.
		return 0;
	}

	FILE *f = fopen(d->fullPathname.c_str(), "wb");
	if (!f) {
		// Unable to open SRam file.
		// TODO: Error code constants.
		return -1;
	}

	// Save SRam to the file.
	int ret = fwrite(m_sram, 1, size, f);
	fclose(f);

	// Return the number of bytes saved.
	clearDirty();
	return ret;
}

/**
 * Autosave the SRam file.
 * This saves the SRam file if its last modification time is past a certain threshold.
 * @param framesElapsed Number of frames elapsed, or -1 for paused. (force autosave)
 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
 */
int SRam::autoSave(int framesElapsed)
{
	if (!m_dirty)
		return 0;

	// TODO: Customizable autosave threshold.
	// TODO: PAL/NTSC detection.
	if (framesElapsed >= 0) {
		// Check if we've passed the autosave threshold.
		bool isPal = false;
		m_framesElapsed += framesElapsed;
		if (m_framesElapsed < (d->AUTOSAVE_THRESHOLD_DEFAULT / (isPal ? 20 : 16)))
			return 0;
	}

	// Autosave threshold has passed.
	return save();
}

/**
 * Load an SRAM file from a ZOMG savestate.
 * @param zomg ZOMG savestate.
 * @return 0 on success; non-zero on error.
 */
int SRam::zomgRestore(LibZomg::Zomg *zomg)
{
	// Load the SRam.
	int ret = zomg->loadSRam(m_sram, sizeof(m_sram));
	if (ret > 0) {
		// SRam loaded.
		setDirty();
		return 0;
	}

	// SRam not loaded.
	return -1;
}


/**
 * Save an SRAM file to a ZOMG savestate.
 * @param zomg ZOMG savestate.
 * @return 0 on success; non-zero on error.
 */
int SRam::zomgSave(LibZomg::Zomg *zomg) const
{
	// Determine how much of the SRam is currently in use.
	int bytesUsed = d->getUsedSize();
	if (bytesUsed <= 0)
		return 0;

	// Save the SRam.
	return zomg->saveSRam(m_sram, bytesUsed);
}

}
