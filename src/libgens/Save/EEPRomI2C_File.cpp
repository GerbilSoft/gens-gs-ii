/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRomI2C_File.cpp: I2C Serial EEPROM handler. (File handling functions)*
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "EEPRomI2C.hpp"
#include "EEPRomI2C_p.hpp"

// ARRAY_SIZE(x)
#include "macros/common.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// C++ includes.
#include <string>
using std::string;

namespace LibGens {

// Static variable initialization.
const char EEPRomI2CPrivate::fileExt[] = "epr";	// TODO: .epr, .sep, or just .srm?

/**
 * Set the EEPRom filename based on a ROM filename.
 * The file extension is changed to ".srm".
 * @param filename ROM filename.
 */
void EEPRomI2C::setFilename(const string &filename)
{
	if (filename.empty()) {
		// Empty filename.
		d->filename.clear();
		d->fullPathname = d->pathname;
		return;
	}

	// Remove any subdirectories from the ROM filename.
	d->filename = filename;
	size_t path_pos = d->filename.rfind(LG_PATH_SEP_CHR);
	if (path_pos != string::npos) {
		// Found a subdirectory.
		if (path_pos == d->filename.size()) {
			d->filename.clear();
		} else {
			d->filename = d->filename.substr(path_pos+1);
		}
	}

	// Replace the file extension.
	size_t dot_pos = d->filename.rfind('.');

	if (dot_pos == string::npos) {
		// File extension not found. Add one.
		d->filename += '.';
	} else {
		// File extension found. Change it.
		d->filename.resize(dot_pos + 1);
	}
	d->filename += string(d->fileExt);

	// Set the full pathname.
	d->fullPathname = d->pathname + d->filename;
}

/**
 * Set the EEPRom pathname.
 * @param pathname EEPRom pathname.
 */
void EEPRomI2C::setPathname(const string& pathname)
{
	d->pathname = pathname;
	
	// Rebuild the full pathname.
	if (d->pathname.empty()) {
		d->fullPathname = d->filename;
	} else {
		// Make sure the pathname ends with a separator.
		if (d->pathname.at(d->pathname.size()-1) != LG_PATH_SEP_CHR)
			d->pathname += LG_PATH_SEP_CHR;

		// Create the full pathname.
		d->fullPathname = d->pathname + d->filename;
	}
}

/**
 * Load the EEPRom file.
 * @return Positive value indicating EEPRom size on success; negative on error.
 */
int EEPRomI2C::load(void)
{
	if (!isEEPRomTypeSet())
		return -2;

	// Attempt to open the EEPRom file.
	FILE *f = fopen(d->fullPathname.c_str(), "rb");
	if (!f) {
		// Unable to open the EEPRom file.
		// TODO: Error code constants.
		return -1;
	}

	// Erase EEPRom before loading the file.
	// NOTE: Don't use reset() - reset() also resets the control logic.
	memset(d->eeprom, 0xFF, sizeof(d->eeprom));

	// Load the EEPRom data.
	int ret = fread(d->eeprom, 1, sizeof(d->eeprom), f);
	fclose(f);

	// Return the number of bytes read.
	d->clearDirty();
	return ret;
}

/**
 * Determine how many bytes are used in the EEPRom chip.
 * TODO: Use the EEPRom type to determine the size?
 * @return Number of bytes used, rounded to the highest power of two.
 */
int EEPRomI2C::getUsedSize(void)
{
	int i = (sizeof(d->eeprom) - 1);
	while (i >= 0 && d->eeprom[i] == 0xFF)
		i--;

	// Return the next-highest power of two.
	return d->next_pow2u(i);
}

/**
 * Save the EEPRom file.
 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
 */
int EEPRomI2C::save(void)
{
	if (!isEEPRomTypeSet())
		return -2;
	if (!d->isDirty())
		return 0;

	int size = getUsedSize();
	if (size <= 0) {
		// EEPRom is empty.
		return 0;
	}

	FILE *f = fopen(d->fullPathname.c_str(), "wb");
	if (!f) {
		// Unable to open EEPRom file.
		// TODO: Error code constants.
		return -1;
	}

	// Save EEPRom to the file.
	int ret = fwrite(d->eeprom, 1, size, f);
	fclose(f);

	// Return the number of bytes saved.
	d->clearDirty();
	return ret;
}

/**
 * Autosave the EEPRom file.
 * This saves the EEPRom file if its last modification time is past a certain threshold.
 * @param framesElapsed Number of frames elapsed, or -1 for paused.
 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
 */
int EEPRomI2C::autoSave(int framesElapsed)
{
	if (!isEEPRomTypeSet())
		return -2;
	if (!d->isDirty())
		return 0;

	// TODO: Customizable autosave threshold.
	// TODO: PAL/NTSC detection.
	if (d->framesElapsed >= 0) {
		// Check if we've passed the autosave threshold.
		bool isPal = false;
		d->framesElapsed += framesElapsed;
		if (d->framesElapsed < (d->AUTOSAVE_THRESHOLD_DEFAULT / (isPal ? 20 : 16)))
			return 0;
	}

	// Autosave threshold has passed.
	return save();
}

}
