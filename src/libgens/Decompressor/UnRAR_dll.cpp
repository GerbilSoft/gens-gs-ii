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

#define InitFuncPtr_unrar(hDll, fn) p##fn = (typeof(p##fn))GetProcAddress((hDll), #fn)

UnRAR_dll::UnRAR_dll(void)
{
	m_loaded = false;
}


/**
 * UnRAR_dll::load(): Load UnRAR.dll.
 * @param filename Filename of UnRAR.dll.
 * @return true on success; false on failure.
 */
bool UnRAR_dll::load(const utf8_str *filename)
{
	if (m_loaded)
		return true;
	
	if (!filename)
		return false;
	
	// Check if we're using Unicode or ANSI.
	// TODO: Move to a "Mini w32u" subsystem?
	m_unicode = (GetModuleHandleW(NULL) != NULL);
	
	// TODO: Convert filename to ANSI or Unicode.
	hUnrarDll = LoadLibraryA(filename);
	if (!hUnrarDll)
		return false;
	
	// Load the function pointers.
	InitFuncPtr_unrar(hUnrarDll, RAROpenArchiveEx);
	InitFuncPtr_unrar(hUnrarDll, RARCloseArchive);
	InitFuncPtr_unrar(hUnrarDll, RARReadHeaderEx);
	InitFuncPtr_unrar(hUnrarDll, RARProcessFile);
	InitFuncPtr_unrar(hUnrarDll, RARSetCallback);
	InitFuncPtr_unrar(hUnrarDll, RARGetDllVersion);
	
	// Check if any of the function pointers are NULL.
	if (!pRAROpenArchiveEx || !pRARCloseArchive ||
	    !pRARReadHeaderEx  || !pRARProcessFile ||
	    !pRARSetCallback   || !pRARGetDllVersion)
	{
		// NULL pointers found. That's bad.
		unload();
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
	pRAROpenArchiveEx	= NULL;
	pRARCloseArchive	= NULL;
	pRARReadHeaderEx	= NULL;
	pRARProcessFile		= NULL;
	pRARSetCallback		= NULL;
	pRARGetDllVersion	= NULL;
	
	// Unload the DLL.
	FreeLibrary(hUnrarDll);
	hUnrarDll = NULL;
}
