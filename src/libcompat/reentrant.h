/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * reentrant.h: Reentrant functions.                                       *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __LIBCOMPAT_REENTRANT_H__
#define __LIBCOMPAT_REENTRANT_H__

#include <libcompat/config.libcompat.h>

// POSIX macros are required on some platforms,
// including MinGW-w64.
// TODO: Properly detect localtime_r() on MinGW-w64.
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 1

// C includes.
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_LOCALTIME_R
#ifdef HAVE_LOCALTIME_S
/**
 * localtime_r() wrapper using MSVCRT's localtime_s().
 * Based on MinGW-w64's localtime_r() wrapper.
 */
static __forceinline struct tm *__cdecl localtime_r(const time_t *_Time, struct tm *_Tm)
{
	return localtime_s(_Tm, _Time) ? NULL : _Tm;
}
#else /* !_WIN32 */
/**
 * Generic localtime_r() wrapper.
 * Uses localtime(), then immediately memcpy()'s the buffer.
 */
static __forceinline struct tm *__cdecl localtime_r(const time_t *_Time, struct tm *_Tm)
{
	struct tm *ret = localtime(_Time);
	if (ret && _Tm) {
		*_Tm = *ret;
		return _Tm;
	}
	return NULL;
}
#endif /* _WIN32 */
#endif /* !HAVE_LOCALTIME_R */

#if !defined(_WIN32) && \
    (defined(__unix__) || defined(__unix) || defined(unix) || \
     defined(__linux__) || defined(__linux) || defined(linux) || \
     (defined(__APPLE__) && defined(__MACH__)) )
/**
 * System is Unix, Linux, or Mac OS X.
 * getpwuid_r() must be available.
 */
#if !defined(HAVE_GETPWUID_R)
#error Platform is missing getpwuid_r(), please implement it.
#if 0
#include <pwd.h>
int getpwuid_r(uid_t uid, struct passwd *pwd,
	       char *buf, size_t buflen, struct passwd **result);
#endif
#endif /* !defined(HAVE_GETPWUID_R) */

#endif /* not Unix / Linux / MacOS X */

#ifdef __cplusplus
}
#endif

#endif /* __LIBCOMPAT_REENTRANT_H__ */
