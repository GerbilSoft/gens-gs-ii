/***************************************************************************
 * libcompat/W32U: Win32 Unicode Translation Layer. (Mini Version)         *
 * W32U_alloca.h: Character set conversion using alloca().                 *
 * Used internally by W32U to avoid repeated heap allocations.             *
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

#ifndef __LIBCOMPAT_W32U_W32U_ALLOCA_H__
#define __LIBCOMPAT_W32U_W32U_ALLOCA_H__

#ifndef _WIN32
#error W32U_alloca.h should only be included on Win32!
#endif

#ifndef __IN_W32U__
#error Do not include W32U_alloca.h outside of W32U!
#endif

// C includes.
#include <string.h>

#ifdef _MSC_VER
// MSVC: _alloca() is in malloc.h.
#include <malloc.h>
#define alloca(size) _alloca(size)
#else /* !_MSC_VER */
// Other compilers: alloca() is in stdlib.h.
#include <stdlib.h>
#endif /* _MSC_VER */

/**
 * Text conversion macros using alloca().
 * Do NOT return the allocated buffer; this will result
 * in undefined behavior, since alloca() uses the stack!
 *
 * Based on KernelEx's WtoA and AtoW code:
 * http://remood.org:8080/kernelex/artifact/566dbc9c83ba5a0394db0c6789c268482c75ca51
 */

/**
 * Convert a string from UTF-8 to UTF-16.
 * @param str UTF-8 string variable.
 * Variable "wchar_t *str##W" must have been previously declared,
 * and it will contain:
 * - NULL on error
 * - UTF-16 string in an alloca()'d buffer on success.
 */
#define UtoW(str) do { \
	int cchWcs = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0); \
	if (cchWcs > 0) { \
		str##W = alloca(cchWcs * sizeof(*str##W)); \
		MultiByteToWideChar(CP_UTF8, 0, str, -1, str##W, cchWcs); \
	} else { \
		str##W = NULL; \
	} \
} while (0)

/**
 * Convert a string from UTF-16 to ANSI.
 * @param str UTF-16 string variable without the trailing W.
 * Variable "char *str##A" must have been previously declared,
 * and it will contain:
 * - NULL on error
 * - ANSI string in an alloca()'d buffer on success.
 */
#define WtoA(str) do { \
	int cbMbs = WideCharToMultiByte(CP_ACP, 0, str##W, -1, NULL, 0, NULL, NULL); \
	if (cbMbs > 0) { \
		str##A = alloca(cbMbs * sizeof(*str##A)); \
		WideCharToMultiByte(CP_ACP, 0, str##W, -1, str##A, cbMbs, NULL, NULL); \
	} else { \
		str##A = NULL; \
	} \
} while (0)

#endif /* __LIBCOMPAT_W32U_W32U_ALLOCA_H__ */
