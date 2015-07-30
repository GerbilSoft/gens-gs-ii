/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
 * W32U_argv.c: UTF-8 conversion for argv[].                               *
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

#include "W32U_argv.h"

// C includes.
#include <assert.h>
#include <wchar.h>
#include <stdlib.h>

// Windows includes.
#include <windows.h>

// __wgetmainargs()
typedef struct {
	int newmode;
} _startupinfo;
_CRTIMP int __cdecl __wgetmainargs(int * _Argc, wchar_t ***_Argv, wchar_t ***_Env, int _DoWildCard, _startupinfo *_StartInfo);

// Converted parameters are stored here.
static int argcU = 0;
static char **argvU = NULL;
static char **envpU = NULL;

/**
 * Convert the Windows command line to UTF-8.
 * @param p_argc	[out] Pointer to new argc.
 * @param p_argv	[out] Pointer to new argv.
 * @param p_envp	[out, opt] Pointer to new envp.
 * @return 0 on success; non-zero on error.
 */
int W32U_GetArgvU(int *p_argc, char **p_argv[], char **p_envp[])
{
	_startupinfo StartInfo;
	// __wgetmainargs() returns.
	int argcW;
	wchar_t **argvW;
	wchar_t **envpW;
	// Temporary variables.
	int ret, i;
	// Block sizes.
	int argv_ptr_sz, argv_str_sz;
	int argv_sz_remain;
	// UTF-8 block pointer.
	char *str;

	// Check if argv has already been converted.
	if (argcU > 0 || argvU || envpU) {
		// argv has already been converted.
		*p_argc = argcU;
		*p_argv = argvU;
		if (p_envp) {
			*p_envp = envpU;
		}
		return 0;
	}

	// TODO: Get existing newmode from MSVCRT.
	StartInfo.newmode = 0;

	// TODO: Use __getmainargs() if the system is ANSI.

	// NOTE: __wgetmainargs() is in MSVC 2010+.
	// MinGW-w64 should support it as well.
	// (It's present in 2.0.8 and possibly earlier.)
	ret = __wgetmainargs(&argcW, &argvW, &envpW, 0, &StartInfo);
	if (ret != 0) {
		// ERROR!
		// TODO: What values?
		return ret;
	}

	// NOTE: Empty strings should still take up 1 character,
	// since they're NULL-terminated.

	// Determine the total length of the argv block.
	argv_ptr_sz = (int)((argcW + 1) * sizeof(char*));
	argv_str_sz = 0;
	for (i = 0; i < argcW; i++) {
		ret = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed!
			// Stop processing.
			return GetLastError();
		}
		argv_str_sz += ret;
	}

	// Allocate the argv block.
	// The first portion of the block is argv[];
	// the second portion contains the actual string data.
	argvU = (char**)malloc(argv_ptr_sz + argv_str_sz);
	str = (char*)argvU + argv_ptr_sz;
	argv_sz_remain = argv_str_sz;

	// Convert the arguments from UTF-16 to UTF-8.
	for (i = 0; i < argcW; i++) {
		assert(argv_sz_remain > 0);
		if (argv_sz_remain <= 0) {
			// Out of space in argvU...
			free(argvU);
			argvU = NULL;
			return ERROR_INSUFFICIENT_BUFFER;
		}

		ret = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, str, argv_sz_remain, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed.
			// Stop processing.
			free(argvU);
			argvU = NULL;
			return GetLastError();
		}

		argvU[i] = str;
		str += ret;
		argv_sz_remain -= ret;
	}

	// Set the last entry in argvU to NULL.
	argcU = argcW;
	argvU[argcU] = NULL;

	// TODO: Process envp.
	*p_argc = argcU;
	*p_argv = argvU;
	return 0;
}
