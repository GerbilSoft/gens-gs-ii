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

#include "Zomg.hpp"
#include "zomg_byteswap.h"

#ifdef _WIN32
// MiniZip Win32 I/O handler.
#include "../../extlib/minizip/iowin32u.h"
#endif

// C includes.
#include <stdint.h>
#include <string.h>

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
	
	if (!filename)
		return;
	
	// Open the ZOMG file.
	// TODO: Open for reading to load existing FORMAT.ini even if
	// the current mode is ZOMG_SAVE.
	
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
#endif
	
	switch (mode)
	{
		case ZOMG_LOAD:
#ifdef _WIN32
			m_unz = unzOpen2_64(filename, &ffunc);
#else
			m_unz = unzOpen(filename);
#endif
			if (!m_unz)
				return;
			break;
		
		case ZOMG_SAVE:
#ifdef _WIN32
			m_zip = zipOpen2_64(filename, APPEND_STATUS_CREATE, NULL, &ffunc);
#else
			m_zip = zipOpen(filename, APPEND_STATUS_CREATE);
#endif
			if (!m_zip)
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

}
