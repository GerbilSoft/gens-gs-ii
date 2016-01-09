/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * UnRAR_dll.cpp: UnRAR.dll Management Class.                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2016 by David Korth.                                 *
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

// C includes. (C++ namespace)
#include <cstring>
#include <cstdlib>
#include <cerrno>

#ifdef _WIN32
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
// Also required for large file support.
#include "libcompat/W32U/W32U_mini.h"

// dlsym() compatibility macro.
// TODO: Needs dlopen(), but charset conversion is required...
// TODO: Add LoadLibrary() to W32U.
#define dlsym(handle, name) GetProcAddress((handle), (name))
#define dlclose(handle) FreeLibrary(handle)
#else /* !_WIN32 */
// Use dlopen().
// TODO: Test for dlfcn.h, dlopen() and libdl, etc.
#include <dlfcn.h>
#endif

// dlsym() helper macro.
#define InitFuncPtr_unrar(hDll, fn) p##fn = (UNRAR_TYPEOF(p##fn))dlsym((hDll), #fn)

namespace LibGensFile {

UnRAR_dll::UnRAR_dll(void)
	: m_dll_abi(0)
	, m_hUnrarDll(0)
{ }

UnRAR_dll::~UnRAR_dll()
{
	if (m_hUnrarDll)
		unload();
}

/**
 * Load UnRAR.dll (or libunrar.so).
 * @param filename Filename of UnRAR.dll.
 * @return 0 on success; POSIX error code on error.
 * TODO: More comprehensive error handling for unsupported DLLs?
 */
int UnRAR_dll::load(const char *filename)
{
	// TODO: Close and reload the DLL?
	if (m_hUnrarDll)
		return 0;
	else if (!filename || *filename == 0)
		return -EINVAL;

	// Clear the ABI version variable.
	m_dll_abi = 0;

#ifdef _WIN32
	// TODO: LoadLibraryU() in W32U.

	// Convert the filename from UTF-8 to UTF-16.
	wchar_t *filenameW = W32U_mbs_to_UTF16(filename, CP_UTF8);
	if (!filenameW)
		return -EINVAL;

	if (W32U_IsUnicode()) {
		// Use the Unicode filename.
		hUnrarDll = LoadLibraryW(filenameW);
	} else {
		// System doesn't support Unicode.
		// Convert the filename from UTF-16 to ANSI.
		char *filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		if (!filenameA) {
			free(filenameW);
			return -EINVAL;
		}

		// Use the ANSI filename.
		hUnrarDll = LoadLibraryA(filenameA);
		free(filenameA);
	}
	free(filenameW);
#else /* !_WIN32 */
	m_hUnrarDll = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
	if (!m_hUnrarDll) {
		// TODO: Get errno.
		return -ENOENT;
	}
#endif /* _WIN32 */

	// Check the UnRAR.dll version first.
	InitFuncPtr_unrar(m_hUnrarDll, RARGetDllVersion);
	if (!pRARGetDllVersion) {
		// Either this UnRAR.dll is really old,
		// or it isn't UnRAR.dll. Either way,
		// we're not supporting it.
		dlclose(m_hUnrarDll);
		m_hUnrarDll = nullptr;
		// TODO: Custom errors.
		return -EIO;
	}
	m_dll_abi = pRARGetDllVersion();
	if (m_dll_abi < 2) {
		// UnRAR.dll older than v2 is not supported.
		m_dll_abi = 0;
		dlclose(m_hUnrarDll);
		m_hUnrarDll = nullptr;
		// TODO: Custom errors.
		return -EIO;
	}

	// Disallow m_dll_abi < 5 on 64-bit systems.
	// UnRAR.dll v5 added #pragma pack(1), which didn't have
	// any effect on the 32-bit version, but it shifted
	// some fields around on 64-bit.
	// FIXME: Use a compile-time check instead of sizeof().
	if (sizeof(void*) == 8 && m_dll_abi < 5) {
		// UnRAR.dll older than v5 is not supported on 64-bit.
		m_dll_abi = 0;
		dlclose(m_hUnrarDll);
		m_hUnrarDll = nullptr;
		// TODO: Custom errors.
		return -EIO;
	}

	// Load the function pointers.
	InitFuncPtr_unrar(m_hUnrarDll, RAROpenArchive);
	InitFuncPtr_unrar(m_hUnrarDll, RARCloseArchive);
	InitFuncPtr_unrar(m_hUnrarDll, RARReadHeader);
	InitFuncPtr_unrar(m_hUnrarDll, RARReadHeaderEx);
	InitFuncPtr_unrar(m_hUnrarDll, RARProcessFile);
	InitFuncPtr_unrar(m_hUnrarDll, RARSetCallback);
	// UnRAR.dll v3
	InitFuncPtr_unrar(m_hUnrarDll, RAROpenArchiveEx);
	// Deprecated as of 2001/11/27. (UnRAR.dll v1?)
	// Use RARSetCallback() instead.
	//InitFuncPtr_unrar(m_hUnrarDll, RARSetChangeVolProc);
	//InitFuncPtr_unrar(m_hUnrarDll, RARSetProcessDataProc);
	// Not present in NoCrypt versions.
	InitFuncPtr_unrar(m_hUnrarDll, RARSetPassword);
	// Added in a later release of UnRAR.dll v3.
	InitFuncPtr_unrar(m_hUnrarDll, RARProcessFileW);

	// NOTE: NoCrypt versions were added in a later
	// release of v4, and weren't around for v3 and
	// earlier. Nonetheless, someone could have
	// custom-compiled their own NoCrypt version
	// for legal reasons, so we aren't going to
	// require RARSetPassword() to be present.

	// Check if any of the function pointers are nullptr.
	if (!pRAROpenArchive ||
	    !pRAROpenArchiveEx ||
	    !pRARCloseArchive ||
	    !pRARReadHeader ||
	    !pRARReadHeaderEx ||
	    !pRARProcessFile ||
	    !pRARSetCallback ||
	    // NOTE: RAROpenArciveEx isn't present in UnRAR.dll v2.
	    (m_dll_abi > 2 && !pRAROpenArchiveEx) ||
	    // NOTE: RARProcessFileW isn't present on early
	    // versions on UnRAR.dll v3.
	    (m_dll_abi > 3 && !pRARProcessFileW))
	{
		// nullptr found. That's bad.
		unload();
		// TODO: Custom errors?
		return -EIO;
	}

	// UnRAR.dll loaded successfully.
	return 0;
}

void UnRAR_dll::unload(void)
{
	if (!m_hUnrarDll)
		return;

	// Clear the DLL ABI version.
	m_dll_abi = 0;

	// Clear the function pointers.
	pRAROpenArchive		= nullptr;
	pRAROpenArchiveEx	= nullptr;
	pRARCloseArchive	= nullptr;
	pRARReadHeader		= nullptr;
	pRARReadHeaderEx	= nullptr;
	pRARProcessFile		= nullptr;
	pRARSetCallback		= nullptr;
	pRARGetDllVersion	= nullptr;
	// Deprecated as of 2001/11/27. (UnRAR.dll v2?)
	// Use RARSetCallback() instead.
	//pRARSetChangeVolProc	= nullptr;
	//pRARSetProcessDataProc	= nullptr;
	pRARSetPassword		= nullptr;
	pRARProcessFileW	= nullptr;

	// Unload the DLL.
	dlclose(m_hUnrarDll);
	m_hUnrarDll = nullptr;
}

}
