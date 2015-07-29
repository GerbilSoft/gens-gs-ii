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

#include "macros/common.h"
#include "libgenstext/StringManip.hpp"

#ifdef _WIN32
#include "libW32U/W32U_mini.h"
#endif

// ZOMG
#include "libzomg/Zomg.hpp"
#include "libzomg/zomg_eeprom.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// C++ includes.
#include <string>
using std::string;

namespace LibGens {

/** EEPRomI2CPrivate **/

// Static variable initialization.
const char EEPRomI2CPrivate::fileExt[] = "epr";	// TODO: .epr, .sep, or just .srm?

/**
 * Determine how many bytes are used in the EEPRom chip.
 * TODO: Use the EEPRom type to determine the size?
 * @return Number of bytes used, rounded to the highest power of two.
 */
int EEPRomI2CPrivate::getUsedSize(void) const
{
	int i = (sizeof(eeprom) - 1);
	while (i >= 0 && eeprom[i] == 0xFF) {
		i--;
	}

	// Return the next-highest power of two.
	return next_pow2u(i);
}

/** EEPRomI2C **/

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

	// Remove any subdirectories and extensions from the ROM filename.
	d->filename = LibGensText::FilenameBaseNoExt(filename);
	// Append our extension.
	d->filename += '.';
	d->filename += d->fileExt;

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
 * Save the EEPRom file.
 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
 */
int EEPRomI2C::save(void)
{
	if (!isEEPRomTypeSet())
		return -2;
	if (!d->isDirty())
		return 0;

	int size = d->getUsedSize();
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

/**
 * Load the EEPROM state and data from a ZOMG savestate.
 * @param zomg ZOMG savestate.
 * @param loadData If true, load the save data in addition to the state.
 * @return 0 on success; non-zero on error.
 */
int EEPRomI2C::zomgRestore(LibZomg::Zomg *zomg, bool loadSaveData)
{
	// TODO
	return -1;
}


/**
 * Save the EEPROM state and data to a ZOMG savestate.
 * @param zomg ZOMG savestate.
 * @return 0 on success; non-zero on error.
 */
int EEPRomI2C::zomgSave(LibZomg::Zomg *zomg) const
{
	// Save the EEPROM state.
	Zomg_EPR_ctrl_t ctrl;
	memset(&ctrl, 0, sizeof(ctrl));

	// Main header.
	ctrl.header = ZOMG_EPR_CTRL_HEADER;
	ctrl.epr_type = ZOMG_EPR_TYPE_I2C;
	ctrl.count = 1;

	// I2C configuration.
	ctrl.i2c.mode		= d->eprChip.epr_mode;
	ctrl.i2c.dev_addr	= d->eprChip.dev_addr;
	ctrl.i2c.dev_mask	= 0xFF;	// TODO: Implement for Mode 2 and Mode 3.
	ctrl.i2c.size		= d->eprChip.sz_mask + 1;
	ctrl.i2c.page_size	= d->eprChip.pg_mask + 1;

	// I2C state.
	ctrl.i2c.state		= d->state;
	ctrl.i2c.i2c_lines	= d->scl | (d->sda_in << 1) | (d->sda_out << 2);
	ctrl.i2c.i2c_prev	= d->scl_prev | (d->sda_in_prev << 1) | (d->sda_out_prev << 2);
	ctrl.i2c.counter	= d->counter;
	ctrl.i2c.address	= d->address;
	ctrl.i2c.data_buf	= d->data_buf;
	ctrl.i2c.rw		= d->rw;

	int ret = zomg->saveEEPRomCtrl(&ctrl);
	if (ret != 0)
		return ret;

	// Save the EEPROM page cache.
	if (d->state == d->EEPRomI2CPrivate::EPR_WRITE_DATA) {
		// Page cache is valid.
		ret = zomg->saveEEPRomCache(d->page_cache, d->eprChip.pg_mask+1);
		if (ret != 0)
			return ret;
	}

	// Save the EEPROM data.
	// Determine how much of the EEPROM is currently in use.
	int bytesUsed = d->getUsedSize();
	if (bytesUsed > 0) {
		ret = zomg->saveEEPRom(d->eeprom, bytesUsed);
	}
	return ret;
}

}
