/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SRam.cpp: Save RAM handler.                                             *
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

#include "SRam.hpp"
#include "macros/common.h"

// C includes.
#include <stdio.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

namespace LibGens
{

// Static variable initialization.
const char *SRam::ms_FileExt = "srm";

// TODO: SRam directory path.
// For now, just save in the ROM directory.

/**
 * reset(): Clear SRam and initialize settings.
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


/**
 * setFilename(): Set the SRam filename based on a ROM filename.
 * The file extension is changed to ".srm".
 * @param filename ROM filename.
 */
void SRam::setFilename(const string &filename)
{
	if (filename.empty())
	{
		// Empty filename.
		m_filename.clear();
		return;
	}
	
	// Set the filename to be the same as the ROM filename,
	// but with a different extension.
	m_filename = filename;
	size_t dot_pos = m_filename.rfind('.');
	size_t path_pos = m_filename.rfind(LG_PATH_SEP_CHR);
	
	if ((dot_pos == string::npos) ||
	    (path_pos != string::npos && dot_pos < path_pos))
	{
		// File extension not found. Add one.
		m_filename += '.';
		m_filename += ms_FileExt;
	}
	else
	{
		// File extension found. Change it.
		m_filename.resize(dot_pos + 1);
		m_filename += ms_FileExt;
	}
}


/**
 * load(): Load the SRam file.
 * @return Positive value indicating SRam size on success; negative on error.
 */
int SRam::load(void)
{
	// Attempt to open the SRam file.
	FILE *f = fopen(m_filename.c_str(), "rb");
	if (!f)
	{
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
 * getUsedSize(): Determine how many bytes are used in the SRam chip.
 * @return Number of bytes used, rounded to the highest power of two.
 */
int SRam::getUsedSize(void)
{
	int i = (sizeof(m_sram) - 1);
	while (i > 0 && m_sram[i] == 0xFF)
		i--;
	
	// Make sure we return 0 if SRam is empty.
	if (i <= 0)
		return 0;
	
	// Return the next-highest power of two.
	return next_pow2u(i);
}


/**
 * save(): Save the SRam file.
 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
 */
int SRam::save(void)
{
	if (!m_dirty)
		return 0;
	
	int size = getUsedSize();
	if (size <= 0)
	{
		// SRam is empty.
		return 0;
	}
	
	FILE *f = fopen(m_filename.c_str(), "wb");
	if (!f)
	{
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
 * autoSave(): Autosave the SRam file.
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
	if (framesElapsed >= 0)
	{
		// Check if we've passed the autosave threshold.
		bool isPal = false;
		m_framesElapsed += framesElapsed;
		if (m_framesElapsed < (AUTOSAVE_THRESHOLD_DEFAULT / (isPal ? 20 : 16)))
			return 0;
	}
	
	// Autosave threshold has passed.
	return save();
}

}
