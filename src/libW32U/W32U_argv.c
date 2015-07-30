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

// __wgetmainargs(), __getmainargs()
typedef struct {
	int newmode;
} _startupinfo;
_CRTIMP int __cdecl __wgetmainargs(int * _Argc, wchar_t ***_Argv, wchar_t ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
_CRTIMP int __cdecl __getmainargs(int * _Argc, char ***_Argv, char ***_Env, int _DoWildCard, _startupinfo *_StartInfo);

// Converted parameters are stored here.
static int argcU = 0;
static char **argvU = NULL;
static char **envpU = NULL;

/**
 * Convert ANSI argv/envp to UTF-16.
 * Caller must free the allocated argvW and envpW.
 * @param p_argcW	[out] Pointer to new argc.
 * @param p_argvW	[out] Pointer to new argv.
 * @param p_envpW	[out, opt] Pointer to new envp.
 * @return 0 on success; non-zero on error.
 */
static int getArgvAtoW(int *p_argcW, wchar_t **p_argvW[], wchar_t **p_envpW[])
{
	_startupinfo StartInfo;
	// __getmainargs() returns.
	int argcA;
	char **argvA;
	char **envpA;
	// Temporary variables.
	int ret, i;
	// Block sizes.
	int cbArgvWPtr, cchArgvWStr;
	int cchArgvWStrLeft;
	// UTF-16 data.
	int argcW;
	wchar_t **argvW = NULL;
	// UTF-16 block pointer.
	wchar_t *strW;

	// TODO: Get existing newmode from MSVCRT.
	StartInfo.newmode = 0;

	// NOTE: __getmainargs() is in MSVC 2010+.
	// MinGW-w64 should support it as well.
	// (It's present in 2.0.8 and possibly earlier.)
	ret = __getmainargs(&argcA, &argvA, &envpA, 0, &StartInfo);
	if (ret != 0) {
		// ERROR!
		// TODO: What values?
		return ret;
	}

	// NOTE: Empty strings should still take up 1 character,
	// since they're NULL-terminated.

	// Determine the total length of the argv block.
	cbArgvWPtr = (int)((argcA + 1) * sizeof(wchar_t*));
	cchArgvWStr = 0;
	for (i = 0; i < argcA; i++) {
		ret = MultiByteToWideChar(CP_ACP, 0, argvA[i], -1, NULL, 0);
		if (ret <= 0) {
			// MultiByteToWideChar() failed!
			// Stop processing.
			ret = GetLastError();
			goto out;
		}
		cchArgvWStr += ret;
	}

	// Allocate the argv block.
	// The first portion of the block is argv[];
	// the second portion contains the actual string data.
	argvW = (wchar_t**)malloc(cbArgvWPtr + (cchArgvWStr * sizeof(wchar_t)));
	strW = (wchar_t*)((char*)argvW + cbArgvWPtr);
	cchArgvWStrLeft = cchArgvWStr;

	// Convert the arguments from UTF-16 to UTF-8.
	for (i = 0; i < argcA; i++) {
		assert(cchArgvWStrLeft > 0);
		if (cchArgvWStrLeft <= 0) {
			// Out of space in argvW...
			ret = ERROR_INSUFFICIENT_BUFFER;
			goto out;
		}

		ret = MultiByteToWideChar(CP_ACP, 0, argvA[i], -1, strW, cchArgvWStrLeft);
		if (ret <= 0) {
			// MultiByteToWideChar() failed.
			// Stop processing.
			ret = GetLastError();
			goto out;
		}

		argvW[i] = strW;
		strW += ret;
		cchArgvWStrLeft -= ret;
	}

	// Set the last entry in argvW to NULL.
	argcW = argcA;
	argvW[argcA] = NULL;
	ret = 0;

out:
	if (ret != 0) {
		// An error occurred.
		free(argvW);
		return ret;
	}

	// TODO: Process envp.
	*p_argcW = argcW;
	*p_argvW = argvW;
	if (p_envpW) {
		*p_envpW = NULL;
	}
	return 0;
}

/**
 * Convert the Windows command line to UTF-8.
 * @param p_argc	[out] Pointer to new argc.
 * @param p_argv	[out] Pointer to new argv.
 * @param p_envp	[out, opt] Pointer to new envp.
 * @return 0 on success; non-zero on error.
 */
int W32U_GetArgvU(int *p_argc, char **p_argv[], char **p_envp[])
{
	// __wgetmainargs() returns.
	int argcW;
	wchar_t **argvW = NULL;
	wchar_t **envpW = NULL;
	// Temporary variables.
	int ret = -1;
	int i;
	// Block sizes.
	int cbArgvUPtr, cbArgvUStr;
	int cbArgvUStrLeft;
	// UTF-8 block pointer.
	char *strU;
	// Is the system Unicode?
	int isUnicode;

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

	// TODO: W32U_IsUnicode()?
	isUnicode = (GetModuleHandleW(NULL) != NULL);
	if (!isUnicode) {
		// ANSI. Use __getmainargs().
		// TODO: Free these variables later.
		ret = getArgvAtoW(&argcW, &argvW, &envpW);
		if (ret != 0) {
			// ERROR!
			// TODO: What values?
			goto out;
		}
	} else {
		// Unicode. Use __wgetmainargs().
		// TODO: Get existing newmode from MSVCRT.
		_startupinfo StartInfo;
		StartInfo.newmode = 0;

		// NOTE: __wgetmainargs() is in MSVC 2010+.
		// MinGW-w64 should support it as well.
		// (It's present in 2.0.8 and possibly earlier.)
		ret = __wgetmainargs(&argcW, &argvW, &envpW, 0, &StartInfo);
		if (ret != 0) {
			// ERROR!
			// TODO: What values?
			goto out;
		}
	}

	// NOTE: Empty strings should still take up 1 character,
	// since they're NULL-terminated.

	// Determine the total length of the argv block.
	cbArgvUPtr = (int)((argcW + 1) * sizeof(char*));
	cbArgvUStr = 0;
	for (i = 0; i < argcW; i++) {
		ret = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed!
			// Stop processing.
			ret = GetLastError();
			goto out;
		}
		cbArgvUStr += ret;
	}

	// Allocate the argv block.
	// The first portion of the block is argv[];
	// the second portion contains the actual string data.
	argvU = (char**)malloc(cbArgvUPtr + (cbArgvUStr * sizeof(char)));
	strU = (char*)argvU + cbArgvUPtr;
	cbArgvUStrLeft = cbArgvUStr;

	// Convert the arguments from UTF-16 to UTF-8.
	for (i = 0; i < argcW; i++) {
		assert(cbArgvUStrLeft > 0);
		if (cbArgvUStrLeft <= 0) {
			// Out of space in argvU...
			ret = ERROR_INSUFFICIENT_BUFFER;
			goto out;
		}

		ret = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, strU, cbArgvUStrLeft, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed.
			// Stop processing.
			ret = GetLastError();
			goto out;
		}

		argvU[i] = strU;
		      strU += ret;
		      cbArgvUStrLeft -= ret;
	}

	// Set the last entry in argvU to NULL.
	argcU = argcW;
	argvU[argcU] = NULL;
	ret = 0;

out:
	if (!isUnicode) {
		// ANSI system.
		// Free the temporary UTF-16 buffers.
		free(argvW);
		free(envpW);
	}

	if (ret != 0) {
		// An error occurred.
		free(argvU);
		free(envpU);
		argcU = 0;
		argvU = NULL;
		envpU = NULL;
		return ret;
	}

	// TODO: Process envp.
	*p_argc = argcU;
	*p_argv = argvU;
	return 0;
}
