/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * W32U_mini.h: Win32 Unicode Translation Layer. (Mini Version)            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#ifndef __W32U_MINI_H__
#define __W32U_MINI_H__

#ifndef _WIN32
#error W32U_mini.h should only be included on Win32!
#endif

// utf8_str
#include "../macros/common.h"

// C includes.
#include <wchar.h>
#include <stdio.h>
#include <io.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Win32 Unicode Translation Layer.
 * @return 0 on success; non-zero on error.
 */
int W32U_Init(void);

/**
 * Shut down the Win32 Unicode Translation Layer.
 * @return 0 on success; non-zero on error.
 */
int W32U_End(void);

/**
 * Indicates if the system is Unicode.
 * NOTE: Do NOT edit this variable outside of W32U!
 */
extern int W32U_IsUnicode;

/**
 * Convert a null-terminated multibyte string to UTF-16.
 * @param mbs Multibyte string. (null-terminated)
 * @param codepage mbs codepage.
 * @return UTF-16 string, or nullptr on error.
 */
wchar_t *W32U_mbs_to_UTF16(const utf8_str *mbs, unsigned int codepage);

/**
 * Convert a null-terminated UTF-16 string to multibyte.
 * @param wcs UTF-16 string. (null-terminated)
 * @param codepage mbs codepage.
 * @return Multibyte string, or nullptr on error.
 */
char *W32U_UTF16_to_mbs(const wchar_t *wcs, unsigned int codepage);

// Redefine fopen() as W32U_fopen().
#define fopen(filename, mode) W32U_fopen(filename, mode)

/**
 * Open a file.
 * @param filename Filename.
 * @param mode File mode.
 * @return File pointer, or nullptr on error.
 */
FILE *W32U_fopen(const utf8_str *filename, const utf8_str *mode);

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
int W32U_access(const utf8_str *path, int mode);

#ifdef __cplusplus
}
#endif

#endif /* __W32U_MINI_H__ */
