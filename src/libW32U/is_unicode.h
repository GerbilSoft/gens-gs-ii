/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
 * is_unicode.h: W32U_IsUnicode() function.                                *
 * INTERNAL USE ONLY; do NOT include this file directly!                   *
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

#ifndef __LIBW32U_IS_UNICODE_H__
#define __LIBW32U_IS_UNICODE_H__

#ifndef _WIN32
#error is_unicode.h should only be included on Win32!
#endif

#ifndef __IN_W32U__
#error Do not include is_unicode.h directly, include W32U_mini.h!
#endif

#include <windows.h>

/**
 * Indicates if the system is Unicode:
 * - -1 == uninitialized
 * -  0 == ANSI
 *    1 == Unicode
 * Define ISUNICODE_MAIN_INSTANCE to make W32U_IsUnicode()
 * visible outside of the compilation unit and to declare
 * the internal isUnicode variable. This should only be done
 * once, in W32U_mini.c.
 */
extern int W32U_internal_isUnicode;
#ifdef ISUNICODE_MAIN_INSTANCE
int W32U_internal_isUnicode = -1;
#endif

/**
 * Check if the system is Unicode.
 * @return 1 if the system is Unicode; 0 if the system is ANSI.
 */
#ifdef ISUNICODE_MAIN_INSTANCE
int W32U_IsUnicode(void)
#else
static __inline int W32U_IsUnicode(void)
#endif
{
	// NOTE: MSVC 2010 doesn't support initializing
	// static variables with a function.
	// TODO: Initialize it better on MinGW-w64?
	// TODO: How slow is GetModuleHandleW()?
	if (W32U_internal_isUnicode < 0) {
		W32U_internal_isUnicode = (GetModuleHandleW(nullptr) != nullptr);
	}
	return W32U_internal_isUnicode;
}

#endif /* __LIBW32U_IS_UNICODE_H__ */
