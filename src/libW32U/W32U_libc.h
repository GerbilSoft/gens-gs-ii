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

#ifndef __LIBW32U_W32U_LIBC_H__
#define __LIBW32U_W32U_LIBC_H__

#ifndef _WIN32
#error W32U_libc.h should only be included on Win32!
#endif

#ifndef __IN_W32U__
#error Do not include W32U_libc.h directly, include W32U_mini.h!
#endif

// C includes.
#include <wchar.h>
#include <stdio.h>
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
 * @return File pointer, or nullptr on error.
 */
FILE *W32U_fopen(const char *filename, const char *mode);

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
// For now, the caller will have to ensure it uses struct _stat64.

// Redefine stat() as W32U_stat64().
#define stat(pathname, buf) W32U_stat64(pathname, buf)

/**
 * Get file status.
 * @param pathname Pathname.
 * @param buf Stat buffer.
 * @return 0 on success; -1 on error.
 */
int W32U_stat64(const char *pathname, struct _stat64 *buf);

#ifdef __cplusplus
}
#endif

#endif /* __LIBW32U_W32U_LIBC_H__ */
