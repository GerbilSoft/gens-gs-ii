/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * Zomg.cpp: Savestate handler.                                            *
 *                                                                         *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#include <config.h>

#include "Zomg.hpp"
#include "zomg_byteswap.h"

#ifdef _WIN32
// MiniZip Win32 I/O handler.
#include "../../extlib/minizip/iowin32u.h"
#endif

// C includes.
#include <stdint.h>
#include <string.h>
#include <time.h>

// C++ includes.
#include <string>
using std::string;

namespace LibZomg
{

/**
 * Zomg(): Open a ZOMG savestate file.
 * @param filename ZOMG filename.
 */
Zomg::Zomg(const utf8_str *filename, ZomgFileMode mode)
{
	m_mode = ZOMG_CLOSED;
	m_unz = NULL;
	m_zip = NULL;
	
	// Assume we're not loading a preview image by default.
	m_preview_size = 0;
	
	if (!filename)
		return;
	
	// Open the ZOMG file.
	// TODO: Open for reading to load existing FORMAT.ini even if
	// the current mode is ZOMG_SAVE.
	
	// TODO: Split this up into multiple functions?
	switch (mode)
	{
		case ZOMG_LOAD:
			if (initZomgLoad(filename) != 0)
				return;
			break;
		
		case ZOMG_SAVE:
			if (initZomgSave(filename) != 0)
				return;
			break;
		
		default:
			return;
	}
	
	// ZOMG file is open.
	m_mode = mode;
	
	// Save the filename.
	m_filename = string(filename);
}


/**
 * ~Zomg(): Close the ZOMG savestate file.
 */
Zomg::~Zomg()
{
	// Make sure the ZOMG savestate file is closed.
	close();
}


/**
 * close(): Close the ZOMG savestate file.
 */
void Zomg::close(void)
{
	if (m_unz)
	{
		unzClose(m_unz);
		m_unz = NULL;
	}
	
	if (m_zip)
	{
		zipClose(m_zip, NULL);
		m_zip = NULL;
	}
	
	m_mode = ZOMG_CLOSED;
}


/**
 * initZomgLoad(): Initialize the Zomg class for loading a Zomg.
 * @param filename Zomg file to load.
 * @return 0 on success; non-zero on error.
 */
int Zomg::initZomgLoad(const utf8_str *filename)
{
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	m_unz = unzOpen2_64(filename, &ffunc);
#else
	m_unz = unzOpen(filename);
#endif
	if (!m_unz)
		return -1;
	
	// Check for a PNG preview image.
	// TODO: Add a mode flag to indicate if we want to load the preview image automatically?
	int ret = unzLocateFile(m_unz, "preview.png", 2);
	if (ret == UNZ_OK)
	{
		// Preview image found.
		// Get the filesize.
		unz_file_info unzfi;
		ret = unzGetCurrentFileInfo(m_unz, &unzfi, NULL, 0, NULL, 0, NULL, 0);
		if (ret == UNZ_OK)
			m_preview_size = unzfi.compressed_size;
	}
	
	return 0;
}


/**
 * initZomgSave(): Initialize the Zomg class for saving a Zomg.
 * @param filename Zomg file to save.
 * @return 0 on success; non-zero on error.
 */
int Zomg::initZomgSave(const utf8_str *filename)
{
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	m_zip = zipOpen2_64(filename, APPEND_STATUS_CREATE, NULL, &ffunc);
#else
	m_zip = zipOpen(filename, APPEND_STATUS_CREATE);
#endif
	if (!m_zip)
		return -1;
	
	// Clear the default Zip timestamp first.
	memset(&m_zipfi, 0x00, sizeof(m_zipfi));
	
	// Get the current time for the Zip archive.
	time_t cur_time = time(NULL);
	struct tm *tm_local;
#ifdef HAVE_LOCALTIME_R
	struct tm tm_local_r;
	tm_local = localtime_r(&cur_time, &tm_local_r);
#else
	tm_local = localtime(&cur_time);
#endif /* HAVE_LOCALTIME_R */
	if (tm_local)
	{
		// Local time received.
		// Convert to Zip time.
		m_zipfi.tmz_date.tm_sec  = tm_local->tm_sec;
		m_zipfi.tmz_date.tm_min  = tm_local->tm_min;
		m_zipfi.tmz_date.tm_hour = tm_local->tm_hour;
		m_zipfi.tmz_date.tm_mday = tm_local->tm_mday;
		m_zipfi.tmz_date.tm_mon  = tm_local->tm_mon;
		m_zipfi.tmz_date.tm_year = tm_local->tm_year;
	}
	
	return 0;
}

}
