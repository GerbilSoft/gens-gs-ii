/***************************************************************************
 * msvc-c99-compat.h: MSVC C99 compatibility header.                       *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

#ifndef __MSVC_C99_COMPAT_H__
#define __MSVC_C99_COMPAT_H__

/**
 * C library functions that have different names in MSVCRT
 * compared to POSIX and C99.
 *
 * Note that later versions of MSVC (esp. 2013 and 2015)
 * have added more C99 functionality, since C99 is included
 */

#ifndef _MSC_VER
#error msvc-c99-compat.h should only be included in MSVC builds.
#endif

/** snprintf(), vsnprintf() **/

/**
 * MSVC 2015 (14.0) supports snprintf() and vsnprintf().
 * Older versions have them prefixed with underscores.
 */
#if _MSC_VER < 1900
#define snprintf(str, size, format, ...) _snprintf(str, size, format, __VA_ARGS__)
#define vsnprintf(str, size, format, ap) _vsnprintf(str, size, format, ap)
#endif /* _MSC_VER < 1900 */

/** strcasecmp(), strncasecmp() **/

/**
 * MSVC does not, and probably never will, define these functions.
 * It has equivalent functions with different names, though.
 */
#define strcasecmp(s1, s2)     _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)

/** strtoll(), strtoull() **/

/**
 * MSVC 2013 added proper support for strtoll() and strtoull().
 * Older verisons don't have these functions, but they do have
 * the equivalent functions _strtoi64() and _strtoui64().
 */
#if _MSC_VER < 1800
#define strtoll(nptr, endptr, base)  _strtoi64(nptr, endptr, base)
#define strtoull(nptr, endptr, base) _strtoui64(nptr, endptr, base)
#endif /* _MSC_VER < 1800 */

#endif /* __MSVC_C99_COMPAT_H__ */
