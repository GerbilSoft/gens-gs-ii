/***************************************************************************
 * libcompat/W32U: Win32 Unicode Translation Layer. (Mini Version)         *
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

#include <config.libcompat.h>

#define __IN_W32U__
#include "W32U_mini.h"
#include "W32U_libc.h"

// C includes.
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>

// Win32 includes.
#include <io.h>
#include <fcntl.h>
#include <share.h>

// W32U alloca() helper.
#include "W32U_alloca.h"

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
	int errno_ret = 0;

	// Convert the arguments from UTF-8 to UTF-16.
	UtoW_filename(filename);
	UtoW(mode);
	if (!filenameW || !modeW) {
		errno = EINVAL;
		return nullptr;
	}

	fRet = nullptr;
	if (W32U_IsUnicode()) {
		// Unicode version.
		fRet = _wfopen(filenameW, modeW);
		errno_ret = errno;
	} else {
#ifdef ENABLE_ANSI_WINDOWS
		// ANSI version.
		// Convert the arguments from UTF-16 to ANSI.
		char *filenameA, *modeA;
		WtoA_filename(filename);
		WtoA(mode);
		if (!filenameA || !modeA) {
			errno_ret = EINVAL;
			goto fail;
		}

		// Open the file.
		fRet = fopen(filenameA, modeA);
#else /* !ENABLE_ANSI_WINDOWS */
		// ANSI is not supported in this build.
		// TODO: Fail earlier to avoid an alloc()?
		fRet = NULL;
		errno_ret = ENOSYS;
		goto fail; /* MSVC complains if a label is unreferenced. (C4102) */
#endif /* ENABLE_ANSI_WINDOWS */
	}

fail:
	errno = errno_ret;
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
	int errno_ret = 0;

	// NOTE: MSVCRT in Windows Vista and later will fail
	// if mode contains X_OK.
	mode &= ~X_OK;

	// Convert the arguments from UTF-8 to UTF-16.
	UtoW_filename(path);
	if (!pathW) {
		errno = EINVAL;
		return -1;
	}

	if (W32U_IsUnicode()) {
		// Unicode version.
		ret = _waccess(pathW, mode);
		errno_ret = errno;
	} else {
#ifdef ENABLE_ANSI_WINDOWS
		// ANSI version.
		// Convert the arguments from UTF-16 to ANSI.
		char *pathA;
		WtoA_filename(path);
		if (!pathA) {
			errno_ret = EINVAL;
			goto fail;
		}

		// Check the access.
		ret = _access(pathA, mode);
#else /* !ENABLE_ANSI_WINDOWS */
		// ANSI is not supported in this build.
		// TODO: Fail earlier to avoid an alloc()?
		ret = -1;
		errno_ret = ENOSYS;
		goto fail; /* MSVC complains if a label is unreferenced. (C4102) */
#endif /* ENABLE_ANSI_WINDOWS */
	}

fail:
	errno = errno_ret;
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
	int errno_ret = 0;

	// Convert the arguments from UTF-8 to UTF-16.
	UtoW_filename(path);
	if (!pathW) {
		errno = EINVAL;
		return -1;
	}

	if (W32U_IsUnicode()) {
		// Unicode version.
		ret = _wmkdir(pathW);
		errno_ret = errno;
	} else {
#ifdef ENABLE_ANSI_WINDOWS
		// ANSI version.
		// Convert the arguments from UTF-16 to ANSI.
		char *pathA;
		WtoA_filename(path);
		if (!pathA) {
			errno_ret = EINVAL;
			goto fail;
		}

		// Create the directory.
		ret = _mkdir(pathA);
#else /* !ENABLE_ANSI_WINDOWS */
		// ANSI is not supported in this build.
		// TODO: Fail earlier to avoid an alloc()?
		ret = -1;
		errno_ret = ENOSYS;
		goto fail; /* MSVC complains if a label is unreferenced. (C4102) */
#endif /* ENABLE_ANSI_WINDOWS */
	}

fail:
	errno = errno_ret;
	return ret;
}

/**
 * Internal implementation of _wstat64().
 * MSVCRT's _wstat64() fails if the filename contains '?' or '*',
 * which breaks long filename support, e.g. \\?\.
 * @param pathname Pathname.
 * @param buf Stat buffer.
 * @return 0 on success; -1 on error.
 */
static int W32U_wstat64(const wchar_t *pathname, struct _stat64 *buffer)
{
	int fd, ret;

	if (!pathname || !buffer) {
		errno = EINVAL;
		return -1;
	}

	// _fstati64() can obtain all the information.
	// We just need to open the file first.
	// FIXME: Use FindFirstFileW() instead to avoid having to open the file?
	fd = _wopen(pathname, _O_RDONLY, 0);
	if (fd < 0) {
		// Error opening the file.
		return fd;
	}

	ret = _fstati64(fd, buffer);
	_close(fd);
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
	int errno_ret = 0;

	/**
	 * All versions of MSVCRT prior to MSVC 2015's UCRT
	 * have a bug where it will blindly access pathname[1]
	 * without checking if pathname[0] is not NULL.
	 * This can cause an out-of-bounds memory access if
	 * pathname[0] is the last byte of a page, and the
	 * next page isn't allocated.
	 *
	 * References:
	 * - http://blogs.msdn.com/b/vcblog/archive/2014/06/18/crt-features-fixes-and-breaking-changes-in-visual-studio-14-ctp1.aspx
	 * - http://connect.microsoft.com/VisualStudio/feedback/details/796796/msvcrt-stat-actually-stat64i32-blindly-refers-to-2nd-char-of-string-parameter
	 * - https://github.com/dynamorio/drmemory/issues/1298#c1
	 */
	if (!pathname || !buf) {
		// NOTE: MSVCRT would normally crash here.
		// We're going to set errno = EFAULT and return,
		// which is what glibc does.
		errno = EFAULT;
		return -1;
	} else if (pathname && !pathname[0]) {
		// pathname is a valid string, but it's empty.
		errno = ENOENT;
		return -1;
	}

	// Convert the arguments from UTF-8 to UTF-16.
	UtoW_filename(pathname);
	if (!pathnameW) {
		errno = EINVAL;
		return -1;
	}

	if (W32U_IsUnicode()) {
		// Unicode version.
		// NOTE: MSVCRT's _wstat64() fails if the filename
		// contains '?' or '*', which breaks long filename
		// support, e.g. \\?\.
		ret = W32U_wstat64(pathnameW, buf);
		errno_ret = errno;
	} else {
#ifdef ENABLE_ANSI_WINDOWS
		// ANSI version.
		// Convert the arguments from UTF-16 to ANSI.
		char *pathnameA;
		WtoA_filename(pathname);
		if (!pathnameA) {
			errno_ret = EINVAL;
			goto fail;
		}

		// Get the file status.
		ret = _stat64(pathnameA, buf);
#else /* !ENABLE_ANSI_WINDOWS */
		// ANSI is not supported in this build.
		// TODO: Fail earlier to avoid an alloc()?
		ret = -1;
		errno_ret = ENOSYS;
		goto fail; /* MSVC complains if a label is unreferenced. (C4102) */
#endif /* ENABLE_ANSI_WINDOWS */
	}

fail:
	errno = errno_ret;
	return ret;
}
