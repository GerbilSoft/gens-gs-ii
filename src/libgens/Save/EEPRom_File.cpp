/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRom_File.cpp: Serial EEPROM handler. (File handling functions)       *
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

#include "EEPRom.hpp"
#include "macros/common.h"

// C includes.
#include <stdio.h>

// C++ includes.
#include <string>
using std::string;

namespace LibGens
{

// Static variable initialization.
const char *EEPRom::ms_FileExt = "epr";	// TODO: .epr, .sep, or just .srm?

// TODO: EEPRom directory path.
// For now, just save in the ROM directory.

/**
 * setFilename(): Set the EEPRom filename based on a ROM filename.
 * The file extension is changed to ".srm".
 * @param filename ROM filename.
 */
void EEPRom::setFilename(const string &filename)
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
 * load(): Load the EEPRom file.
 * @return Positive value indicating EEPRom size on success; negative on error.
 */
int EEPRom::load(void)
{
	if (!isEEPRomTypeSet())
		return -2;
	
	// Attempt to open the EEPRom file.
	FILE *f = fopen(m_filename.c_str(), "rb");
	if (!f)
	{
		// Unable to open the EEPRom file.
		// TODO: Error code constants.
		return -1;
	}
	
	// Erase EEPRom before loading the file.
	// NOTE: Don't use reset() - reset() also resets the control logic.
	memset(m_eeprom, 0xFF, sizeof(m_eeprom));
	
	// Load the EEPRom data.
	int ret = fread(m_eeprom, 1, sizeof(m_eeprom), f);
	fclose(f);
	
	// Return the number of bytes read.
	clearDirty();
	return ret;
}


/**
 * getUsedSize(): Determine how many bytes are used in the EEPRom chip.
 * @return Number of bytes used, rounded to the highest power of two.
 */
int EEPRom::getUsedSize(void)
{
	int i = (sizeof(m_eeprom) - 1);
	while (i >= 0 && m_eeprom[i] == 0xFF)
		i--;
	
	// Return the next-highest power of two.
	return next_pow2u(i);
}


/**
 * save(): Save the EEPRom file.
 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
 */
int EEPRom::save(void)
{
	if (!isEEPRomTypeSet())
		return -2;
	if (!m_dirty)
		return 0;
	
	int size = getUsedSize();
	if (size <= 0)
	{
		// EEPRom is empty.
		return 0;
	}
	
	FILE *f = fopen(m_filename.c_str(), "wb");
	if (!f)
	{
		// Unable to open EEPRom file.
		// TODO: Error code constants.
		return -1;
	}
	
	// Save EEPRom to the file.
	int ret = fwrite(m_eeprom, 1, size, f);
	fclose(f);
	
	// Return the number of bytes saved.
	clearDirty();
	return ret;
}


/**
 * autoSave(): Autosave the EEPRom file.
 * This saves the EEPRom file if its last modification time is past a certain threshold.
 * @param framesElapsed Number of frames elapsed, or 0 for paused (force autosave).
 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
 */
int EEPRom::autoSave(int framesElapsed)
{
	if (!isEEPRomTypeSet())
		return -2;
	if (!m_dirty)
		return 0;
	
	// TODO: Customizable autosave threshold.
	// TODO: PAL/NTSC detection.
	if (framesElapsed > 0)
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
