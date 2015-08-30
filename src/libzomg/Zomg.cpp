/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg.cpp: Savestate handler.                                            *
 *                                                                         *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#include <libzomg/config.libzomg.h>

// Reentrant functions.
// MUST be included before everything else due to
// _POSIX_SOURCE and _POSIX_C_SOURCE definitions.
#include "libcompat/reentrant.h"

#include "Zomg.hpp"

#ifdef _WIN32
// MiniZip Win32 I/O handler.
#include "../extlib/minizip/iowin32u.h"
// Win32 Unicode Translation Layer.
#include "libcompat/W32U/W32U_mini.h"
#endif

// C includes.
#include <sys/types.h>
#include <sys/stat.h>

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cerrno>

// C++ includes.
#include <string>
using std::string;

#include "Zomg_p.hpp"
namespace LibZomg {

/** ZomgPrivate **/

ZomgPrivate::ZomgPrivate(Zomg *q)
	: q(q)
	, unz(nullptr)	// TODO: Combine with zip into a union?
	, zip(nullptr)	// Need to double-check all users.
{ }

ZomgPrivate::~ZomgPrivate()
{
	// FIXME: Move ZomgBase stuff here,
	// and close unz and zip here.
}

/**
 * Initialize the Zomg class for loading a Zomg.
 * @param filename Zomg file to load.
 * @return 0 on success; non-zero on error.
 */
int ZomgPrivate::initZomgLoad(const char *filename)
{
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	this->unz = unzOpen2_64(filename, &ffunc);
#else
	this->unz = unzOpen(filename);
#endif

	if (!this->unz) {
		// TODO: Figure out why open failed.
		// On Windows, GetLastError() may work.
		// On Linux, errno may work.
		// Alternatively, try opening the file ourselves.
		return -EIO;
	}

	// Check the file's mtime.
	// TODO: Check for "CreationTime" in ZOMG.ini, and use it
	// if it's available.
#ifdef _WIN32
	struct _stat64 buf;
#else
	struct stat buf;
#endif
	int ret = stat(filename, &buf);
	if (ret == 0) {
		// stat() succeeded.
		q->m_mtime = buf.st_mtime;
	}

	return 0;
}

/**
 * Initialize the Zomg class for saving a Zomg.
 * @param filename Zomg file to save.
 * @return 0 on success; non-zero on error.
 */
int ZomgPrivate::initZomgSave(const char *filename)
{
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	this->zip = zipOpen2_64(filename, APPEND_STATUS_CREATE, nullptr, &ffunc);
#else
	this->zip = zipOpen(filename, APPEND_STATUS_CREATE);
#endif

	if (!this->zip) {
		// TODO: Figure out why open failed.
		// On Windows, GetLastError() may work.
		// On Linux, errno may work.
		// Alternatively, try opening the file ourselves.
		return -EIO;
	}

	// Clear the default Zip timestamp first.
	memset(&this->zipfi, 0, sizeof(this->zipfi));

	// Convert m_mtime to localtime.
	struct tm tm_local;
	if (localtime_r(&q->m_mtime, &tm_local)) {
		// Local time received.
		// Convert to Zip time.
		this->zipfi.tmz_date.tm_sec  = tm_local.tm_sec;
		this->zipfi.tmz_date.tm_min  = tm_local.tm_min;
		this->zipfi.tmz_date.tm_hour = tm_local.tm_hour;
		this->zipfi.tmz_date.tm_mday = tm_local.tm_mday;
		this->zipfi.tmz_date.tm_mon  = tm_local.tm_mon;
		this->zipfi.tmz_date.tm_year = tm_local.tm_year;
	}

	return 0;
}

/** Zomg **/

/**
 * Open a ZOMG savestate file.
 * @param filename ZOMG filename.
 * @param mode File mode.
 */
Zomg::Zomg(const char *filename, ZomgFileMode mode)
	: ZomgBase(filename, mode)
	, d(new ZomgPrivate(this))
{
	if (!filename || !filename[0]) {
		// No filename specified.
		m_lastError = -EINVAL;
		return;
	}

	// Open the ZOMG file.
	// TODO: Open for reading to load existing FORMAT.ini even if
	// the current mode is ZOMG_SAVE.

	// TODO: Split this up into multiple functions?
	int ret;
	switch (mode) {
		case ZOMG_LOAD:
			ret = d->initZomgLoad(filename);
			break;
		case ZOMG_SAVE:
			ret = d->initZomgSave(filename);
			break;
		default:
			ret = -EINVAL;
			break;
	}
	if (ret != 0) {
		m_lastError = ret;
		return;
	}

	// ZOMG file is open.
	m_mode = mode;

	// Save the filename.
	m_filename = string(filename);
}

/**
 * Close the ZOMG savestate file.
 */
Zomg::~Zomg()
{
	close();
	delete d;
}

/**
 * Close the ZOMG savestate file.
 */
void Zomg::close(void)
{
	if (d->unz) {
		unzClose(d->unz);
		d->unz = nullptr;
	}

	if (d->zip) {
		zipClose(d->zip, nullptr);
		d->zip = nullptr;
	}

	m_mode = ZOMG_CLOSED;
	m_lastError = 0;
}


/**
 * Detect if a savestate is supported by this class.
 * @param filename Savestate filename.
 * @return True if the savestate is supported; false if not.
 */
bool Zomg::DetectFormat(const char *filename)
{
	// TODO: This only checks if the file is a ZIP file.
	// Check FORMAT.INI once it's implemented.
	static const uint8_t zip_magic[] = {'P', 'K', 0x03, 0x04};

	// TODO: Win32 Unicode translation.
	FILE *f = fopen(filename, "rb");
	if (!f)
		return false;

	// Read the "magic number".
	uint8_t header[sizeof(zip_magic)];
	size_t ret = fread(&header, 1, sizeof(header), f);
	fclose(f);

	if (ret < sizeof(header)) {
		// Error reading the "magic number".
		return false;
	}

	// Check the "magic number" and return true if it matches.
	return (!memcmp(header, zip_magic, sizeof(header)));
}

}
