/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
 * W32U_libc.h: MSVCRT functions.                                          *
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

#define __IN_W32U__
#include "W32U_mini.h"
#include "W32U_libc.h"

// C includes.
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>

// Win32 includes.
#include <io.h>

// direct.h calls some "deprecated" functions directly.
#ifdef mkdir
#undef mkdir
#endif
#include <direct.h>

// W32U_IsUnicode()
#include "is_unicode.h"

/** fopen() **/

// Make sure fopen() isn't redefined.
#ifdef fopen
#undef fopen
#endif

/**
 * Open a file.
 * @param filename Filename.
 * @param mode File mode.
 * @return File pointer, or NULL on error.
 */
FILE *W32U_fopen(const char *filename, const char *mode)
{
	wchar_t *filenameW, *modeW;
	FILE *fRet;

	// Convert the filename from UTF-8 to UTF-16.
	filenameW = W32U_mbs_to_UTF16(filename, CP_UTF8);
	if (!filenameW)
		return nullptr;

	// Convert the mode from UTF-8 to UTF-16.
	modeW = W32U_mbs_to_UTF16(mode, CP_UTF8);
	if (!modeW) {
		free(filenameW);
		return nullptr;
	}

	fRet = nullptr;
	if (W32U_IsUnicode()) {
		// Unicode version.
		fRet = _wfopen(filenameW, modeW);
	} else {
		// ANSI version.
		char *filenameA;
		char *modeA;

		// Convert the filename from UTF-16 to ANSI.
		filenameA = W32U_UTF16_to_mbs(filenameW, CP_ACP);
		if (!filenameA)
			goto fail;

		// Convert the mode from UTF-16 to ANSI.
		modeA = W32U_UTF16_to_mbs(modeW, CP_ACP);
		if (!modeA) {
			free(filenameA);
			goto fail;
		}

		// Open the file.
		fRet = fopen(filenameA, modeA);
		free(filenameA);
		free(modeA);
	}

fail:
	free(filenameW);
	free(modeW);
	return fRet;
}

/** access() **/

// Make sure access() isn't redefined.
#ifdef access
#undef access
#endif

/**
 * Check if a path can be accessed.
 * @param path Pathname.
 * @param mode Mode.
 * @return 0 if the file has the given mode; -1 if not or if the file does not exist.
 */
int W32U_access(const char *path, int mode)
{
	wchar_t *pathW;
	int ret = -1;

	// NOTE: MSVCRT in Windows Vista and later will fail
	// if mode contains X_OK.
	mode &= ~X_OK;

	// Convert the path from UTF-8 to UTF-16.
	pathW = W32U_mbs_to_UTF16(path, CP_UTF8);
	if (!pathW) {
		errno = EINVAL;
		return -1;
	}

	if (W32U_IsUnicode()) {
		// Unicode version.
		ret = _waccess(pathW, mode);
	} else {
		// ANSI version.
		char *pathA;

		// Convert the filename from UTF-16 to ANSI.
		pathA = W32U_UTF16_to_mbs(pathW, CP_ACP);
		if (!pathA) {
			errno = EINVAL;
			goto fail;
		}

		// Check the access.
		ret = _access(pathA, mode);
		free(pathA);
	}

fail:
	free(pathW);
	return ret;
}

/** mkdir() **/

// Make sure mkdir() isn't redefined.
#ifdef mkdir
#undef mkdir
#endif

/**
 * Create a directory.
 * @param path Pathname.
 * @return 0 on success; -1 on error.
 */
int W32U_mkdir(const char *path)
{
	wchar_t *pathW;
	int ret = -1;

	pathW = W32U_mbs_to_UTF16(path, CP_UTF8);
	if (!pathW) {
		errno = EINVAL;
		return -1;
	}

	if (W32U_IsUnicode()) {
		// Unicode version.
		ret = _wmkdir(pathW);
	} else {
		// ANSI version.
		char *pathA;

		// Convert the filename from UTF-16 to ANSI.
		pathA = W32U_UTF16_to_mbs(pathW, CP_ACP);
		if (!pathA) {
			errno = EINVAL;
			goto fail;
		}

		// Create the directory.
		ret = _mkdir(pathA);
		free(pathA);
	}

fail:
	free(pathW);
	return ret;
}

/**
 * Get file status.
 * @param pathname Pathname.
 * @param buf Stat buffer.
 * @return 0 on success; -1 on error.
 */
int W32U_stat64(const char *pathname, struct _stat64 *buf)
{
	wchar_t *pathnameW;
	int ret = -1;

	pathnameW = W32U_mbs_to_UTF16(pathname, CP_UTF8);
	if (!pathnameW) {
		errno = EINVAL;
		return -1;
	}

	if (W32U_IsUnicode()) {
		// Unicode version.
		ret = _wstat64(pathnameW, buf);
	} else {
		// ANSI version.
		char *pathnameA;

		// Convert the filename from UTF-16 to ANSI.
		pathnameA = W32U_UTF16_to_mbs(pathnameW, CP_ACP);
		if (!pathnameA) {
			errno = EINVAL;
			goto fail;
		}

		// Get the file status.
		ret = _stat64(pathnameA, buf);
		free(pathnameA);
	}

fail:
	free(pathnameW);
	return ret;
}
