/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
 * W32U_mini.c: Main source file.                                          *
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

// C includes.
#include <wchar.h>
#include <stdlib.h>

// Windows includes.
#include <windows.h>

// Indicates if the system is Unicode.
// -1 = uninitialized; 0 == ANSI; 1 == Unicode.
extern int W32U_internal_isUnicode;
int W32U_internal_isUnicode = -1;

/**
 * Check if the system is Unicode.
 * NOTE: This specific instance is visible outside of this file.
 * No functions in this file use it, so it's not marked inline.
 * @return 1 if the system is Unicode; 0 if the system is ANSI.
 */
int W32U_IsUnicode(void)
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

/**
 * Convert a null-terminated multibyte string to UTF-16.
 * @param mbs Multibyte string. (null-terminated)
 * @param codepage mbs codepage.
 * @return UTF-16 string, or NULL on error.
 */
wchar_t *W32U_mbs_to_UTF16(const char *mbs, unsigned int codepage)
{
	int cchWcs;
	wchar_t *wcs;
W32U_IsUnicode();
	cchWcs = MultiByteToWideChar(codepage, 0, mbs, -1, nullptr, 0);
	if (cchWcs <= 0)
		return nullptr;

	wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(codepage, 0, mbs, -1, wcs, cchWcs);
	return wcs;
}

/**
 * Convert a null-terminated UTF-16 string to multibyte.
 * @param wcs UTF-16 string. (null-terminated)
 * @param codepage mbs codepage.
 * @return Multibyte string, or NULL on error.
 */
char *W32U_UTF16_to_mbs(const wchar_t *wcs, unsigned int codepage)
{
	int cbMbs;
	char *mbs;

	cbMbs = WideCharToMultiByte(codepage, 0, wcs, -1, nullptr, 0, nullptr, nullptr);
	if (cbMbs <= 0)
		return nullptr;

	mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(codepage, 0, wcs, -1, mbs, cbMbs, nullptr, nullptr);
	return mbs;
}
