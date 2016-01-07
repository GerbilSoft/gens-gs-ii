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

#ifndef __LIBCOMPAT_W32U_W32U_LIBC_H__
#define __LIBCOMPAT_W32U_W32U_LIBC_H__

#ifndef _WIN32
#error W32U_libc.h should only be included on Win32!
#endif

#ifndef __IN_W32U__
#error Do not include W32U_libc.h directly, include W32U_mini.h!
#endif

// libcompat configuration.
#include <libcompat/config.libcompat.h>

// C includes.
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

// Win32 includes.
#include <windows.h>
#include <io.h>

#ifdef __cplusplus
extern "C" {
#endif

/** fopen() **/

// Redefine fopen() as W32U_fopen().
#define fopen(filename, mode) W32U_fopen(filename, mode)

/**
 * Open a file.
 * @param filename Filename.
 * @param mode File mode.
 * @return File pointer, or NULL on error.
 */
FILE *W32U_fopen(const char *filename, const char *mode);

/** fseeko(), ftello() **/
// On Linux, Large File Support redefines fseeko() and ftello()
// to fseeko64() and ftello64(). fseek() and ftell() are left as-is.
// MSVCRT doesn't have fseeko() or ftello(), so we'll define them
// as _fseeki64() and _ftelli64().
// (NOTE: MinGW-w64 does have fseeko(), ftello(), fseeko64() and
//  ftello64(), and it uses the FILE_OFFSET_BITS macro. LFS appears
//  to be required on both 32-bit and 64-bit Windows, unlike on Linux
//  where it's only required on 32-bit.)
// TODO: Use _fseeki64() and _ftelli64() on MinGW-w64 to avoid
// use of wrapper functions?
#ifdef _MSC_VER
#ifndef HAVE__STATI64
#pragma message("MSVC is missing _stati64(); files larger than 2 GB may not be handled correctly.")
#define _stati64 stat
#endif
#ifdef HAVE__FSEEKI64
#define fseeko(stream, offset, origin) _fseeki64(stream, offset, origin)
#else /* !HAVE_FSEEKI64 */
#pragma message("MSVC is missing _fseeki64(); files larger than 2 GB may not be handled correctly.")
#define fseeko(stream, offset, origin) fseek(stream, (int)offset, origin)
#endif /* HAVE_FSEEKI64 */
#ifdef HAVE__FTELLI64
#define ftello(stream) _ftelli64(stream)
#else /* !HAVE_FTELLI64 */
#pragma message("MSVC is missing _ftelli64(); files larger than 2 GB may not be handled correctly.")
#define ftello(stream) (__int64)ftell(stream)
#endif /* HAVE_FTELLI64 */

#if _MSC_VER >= 1400 && _USE_32BIT_TIME_T
/* MSVC 2005 defaults to 64-bit time_t, but this can be overridden. */
/* We don't want it to be overridden. */
#error 32-bit time_t is enabled, please undefine _USE_32BIT_TIME_T.
#endif
#endif /* _MSC_VER */

/** access() **/

// Redefine access() as W32U_access().
#define access(path, mode) W32U_access(path, mode)

// Modes.
#ifndef F_OK
#define F_OK 0
#endif
#ifndef X_OK
#define X_OK 1
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef R_OK
#define R_OK 4
#endif

/**
 * Check if a path can be accessed.
 * @param path Pathname.
 * @param mode Mode.
 * @return 0 if the file has the given mode; -1 if not or if the file does not exist.
 */
int W32U_access(const char *path, int mode);

/** mkdir() **/

// Redefine mkdir() as W32U_mkdir().
#define mkdir(path, mode) W32U_mkdir(path)

/**
 * Create a directory.
 * @param path Pathname.
 * @return 0 on success; -1 on error.
 */
int W32U_mkdir(const char *path);

/** stat() **/

// Not only is 'stat' both a struct and a function on Unix,
// there's also four variants with underscores on Windows
// for various 32-bit vs. 64-bit fields. There's also
// non-underscore versions for compatibility.

// FIXME: How do we handle struct stat?
// For now, the caller will have to ensure it uses struct _stati64.

// Redefine stat() as W32U_stat64().
#ifdef stat
#undef stat
#endif
#define stat(pathname, buf) W32U_stati64(pathname, buf)

/**
 * Get file status.
 * @param pathname Pathname.
 * @param buf Stat buffer.
 * @return 0 on success; -1 on error.
 */
int W32U_stati64(const char *pathname, struct _stati64 *buf);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCOMPAT_W32U_W32U_LIBC_H__ */
