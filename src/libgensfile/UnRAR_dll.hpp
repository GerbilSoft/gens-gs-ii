/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * UnRAR_dll.hpp: UnRAR.dll Management Class.                              *
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

#ifndef __LIBGENSFILE_UNRAR_DLL_HPP__
#define __LIBGENSFILE_UNRAR_DLL_HPP__

// unrar.h requires windows.h on Windows.
#ifdef _WIN32
#include <windows.h>
#endif

// unrar.h (dll.hpp) should be the latest version.
// As of 2016/01/08, the latest version is v7. (2015/07/19, UnRAR 5.3.1)
#include "unrar/dll.hpp"
#if RAR_DLL_VERSION != 7
#define xstr(s) str(s)
#define str(s) #s
#pragma message("unrar.h version is " str(RAR_DLL_VERSION) ", but UnRAR_dll.hpp expects 7.")
#error Wrong unrar.h version - please update UnRAR_dll.hpp.
#endif

// typeof() or equivalent.
#if defined(__GNUC__)
#define UNRAR_TYPEOF(f) typeof(f)
#elif defined(_MSC_VER)
// MSVC 2010+ (TODO: What about earlier versions?)
#define UNRAR_TYPEOF(f) decltype(f)
#else
#error typeof() or equivalent not available for this compiler.
#endif

#define MAKE_CLASSFUNCPTR(f) UNRAR_TYPEOF(f) * p##f

namespace LibGensFile {

class UnRAR_dll
{
	public:
		UnRAR_dll(void);
		~UnRAR_dll();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		UnRAR_dll(const UnRAR_dll &);
		UnRAR_dll &operator=(const UnRAR_dll &);

	public:
		/**
		 * Load UnRAR.dll (or libunrar.so).
		 * @param filename Filename of UnRAR.dll.
		 * @return 0 on success; POSIX error code on error.
		 * TODO: More comprehensive error handling for unsupported DLLs?
		 */
		int load(const char *filename);

		/**
		 * Unload UnRAR.dll.
		 */
		void unload(void);

		/**
		 * Is UnRAR.dll loaded?
		 * @return True if loaded; false if not loaded.
		 */
		inline bool isLoaded(void) const;

		/**
		 * Get the UnRAR.dll ABI version.
		 * @return UnRAR.dll ABI version, or 0 if not loaded.
		 */
		inline int abiVersion(void) const;

		// UnRAR.dll v2
		MAKE_CLASSFUNCPTR(RAROpenArchive);
		MAKE_CLASSFUNCPTR(RARCloseArchive);
		MAKE_CLASSFUNCPTR(RARReadHeader);
		MAKE_CLASSFUNCPTR(RARReadHeaderEx);
		MAKE_CLASSFUNCPTR(RARProcessFile);
		MAKE_CLASSFUNCPTR(RARSetCallback);
		MAKE_CLASSFUNCPTR(RARGetDllVersion);

		// UnRAR.dll v3
		MAKE_CLASSFUNCPTR(RAROpenArchiveEx);

		// Deprecated as of 2001/11/27. (UnRAR.dll v1?)
		// Use RARSetCallback() instead.
		//MAKE_CLASSFUNCPTR(RARSetChangeVolProc);
		//MAKE_CLASSFUNCPTR(RARSetProcessDataProc);

		// Not present in NoCrypt versions.
		MAKE_CLASSFUNCPTR(RARSetPassword);

		// This function was added in a later
		// release of UnRAR.dll v3.
		// It's always present in v4 and later.
		MAKE_CLASSFUNCPTR(RARProcessFileW);

	private:
		/**
		 * UnRAR.dll ABI version.
		 * If 0, the DLL isn't loaded.
		 */
		int m_dll_abi;

		/**
		 * UnRAR.dll handle.
		 */
		void *m_hUnrarDll;
};

/**
 * Is UnRAR.dll loaded?
 * @return True if loaded; false if not loaded.
 */
inline bool UnRAR_dll::isLoaded(void) const
{
	return (m_dll_abi > 0);
}

/**
 * Get the UnRAR.dll ABI version.
 * @return UnRAR.dll ABI version, or 0 if not loaded.
 */
inline int UnRAR_dll::abiVersion(void) const
{
	return m_dll_abi;
}

};

#endif /* __LIBGENSFILE_UNRAR_DLL_HPP__ */
