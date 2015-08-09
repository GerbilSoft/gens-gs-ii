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

// W32U_IsUnicode()
#define ISUNICODE_MAIN_INSTANCE
#include "is_unicode.h"

/**
 * Check if the system supports UTF-8.
 * If it doesn't, the program will show an
 * error message and then exit.
 */
void W32U_CheckUTF8(void)
{
	// U+2602: UMBRELLA
	// NOTE: Buffers are padded with 0x5A to match the
	// expected values for memcmp().
	// TODO: Use "random" data?
	static const int cbUtf8 = 4;
	static const char utf8_src[] =
		{0xE2, 0x98, 0x82, 0x00, 0x5A, 0x5A, 0x5A, 0x5A,
		 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A};
	static const int cchUtf16 = 2;
	static const wchar_t utf16_src[] =
		{0x2602, 0x0000, 0x5A5A, 0x5A5A,
		 0x5A5A, 0x5A5A, 0x5A5A, 0x5A5A};

	// Temporary buffers.
	int ret;
	union {
		char utf8[16];
		wchar_t utf16[8];
	} buf;

	// Attempt to convert U+2602 from UTF-8 to UTF-16.
	memset(buf.utf8, 0x5A, sizeof(buf.utf8));
	ret = MultiByteToWideChar(CP_UTF8, 0, utf8_src, -1,
		buf.utf16, sizeof(buf.utf16)/sizeof(buf.utf16[0]));
	if (ret != cchUtf16) {
		// Wrong number of characters converted.
		goto fail;
	}
	if (memcmp(buf.utf16, utf16_src, sizeof(utf16_src)) != 0) {
		// Conversion is wrong.
		goto fail;
	}

	// Attempt to convert U+2602 from UTF-16 to UTF-8.
	memset(buf.utf8, 0x5A, sizeof(buf.utf8));
	ret = WideCharToMultiByte(CP_UTF8, 0, utf16_src, -1,
		buf.utf8, sizeof(buf.utf8), NULL, NULL);
	if (ret != cbUtf8) {
		// Wrong number of characters converted.
		goto fail;
	}
	if (memcmp(buf.utf8, utf8_src, sizeof(utf8_src)) != 0) {
		// Conversion is wrong.
		goto fail;
	}

	// UTF-8 conversions succeeded.
	return;

fail:
	// UTF-8 conversions failed.
	MessageBoxA(NULL,
		"This system does not support the UTF-8 encoding for Unicode text.\n\n"
		"Because Gens/GS II uses UTF-8 internally for all text,\n"
		"it requires a system that supports UTF-8.\n\n"
		"Please upgrade to an OS released in the 21st century.",
		"UTF-8 Error", MB_ICONSTOP);
	exit(EXIT_FAILURE);
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
