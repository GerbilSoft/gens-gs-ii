/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * UnRAR_dll.cpp: UnRAR.dll Management Class.                              *
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

#include "UnRAR_dll.hpp"

// C includes.
#include <string.h>
#include <stdlib.h>

// Win32 Unicode Translation Layer.
#include "../Win32/W32U_mini.h"

#define InitFuncPtr_unrar(hDll, fn) p##fn = (typeof(p##fn))GetProcAddress((hDll), #fn)

UnRAR_dll::UnRAR_dll(void)
{
	m_loaded = false;
}


/**
 * Load UnRAR.dll.
 * @param filename Filename of UnRAR.dll.
 * @return true on success; false on failure.
 */
bool UnRAR_dll::load(const utf8_str *filename)
{
	if (m_loaded)
		return true;
	
	if (!filename)
		return false;

	// Convert the filename from UTF-8 to UTF-16.
	wchar_t *filenameW = W32U_mbs_to_UTF16(filename, CP_UTF8);
	if (!filename)
		return false;

	if (W32U_IsUnicode) {
		// Use the Unicode filename.
		hUnrarDll = LoadLibraryW(filenameW);
	} else {
		// System doesn't support Unicode.
		// Convert the filename from UTF-16 to ANSI.
		char *filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		if (!filenameA) {
			free(filenameW);
			return false;
		}

		// Use the ANSI filename.
		hUnrarDll = LoadLibraryA(filename);
		free(filenameA);
	}
	free(filenameW);

	if (!hUnrarDll)
		return false;

	// Load the function pointers.
	InitFuncPtr_unrar(hUnrarDll, RAROpenArchiveEx);
	InitFuncPtr_unrar(hUnrarDll, RARCloseArchive);
	InitFuncPtr_unrar(hUnrarDll, RARReadHeaderEx);
	InitFuncPtr_unrar(hUnrarDll, RARProcessFile);
	InitFuncPtr_unrar(hUnrarDll, RARSetCallback);
	InitFuncPtr_unrar(hUnrarDll, RARGetDllVersion);

	// Check if any of the function pointers are nullptr.
	if (!pRAROpenArchiveEx || !pRARCloseArchive ||
	    !pRARReadHeaderEx  || !pRARProcessFile ||
	    !pRARSetCallback   || !pRARGetDllVersion)
	{
		// nullptr found. That's bad.
		m_loaded = true;	// Needed for unload() to work.
		unload();
		return false;
	}

	// UnRAR.dll loaded successfully.
	m_loaded = true;
	return true;
}


UnRAR_dll::~UnRAR_dll()
{
	if (!m_loaded)
		return;

	// Unload the DLL.
	unload();
}


void UnRAR_dll::unload(void)
{
	if (!m_loaded)
		return;

	// Mark the DLL as unloaded.
	m_loaded = false;

	// Clear the function pointers.
	pRAROpenArchiveEx	= nullptr;
	pRARCloseArchive	= nullptr;
	pRARReadHeaderEx	= nullptr;
	pRARProcessFile		= nullptr;
	pRARSetCallback		= nullptr;
	pRARGetDllVersion	= nullptr;

	// Unload the DLL.
	FreeLibrary(hUnrarDll);
	hUnrarDll = nullptr;
}
