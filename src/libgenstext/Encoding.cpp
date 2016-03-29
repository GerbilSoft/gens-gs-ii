/***************************************************************************
 * libgenstext: Gens/GS II Text Manipulation Library.                      *
 * Encoding.cpp: Character encoding functions.                             *
 *                                                                         *
 * Copyright (c) 2009-2015 by David Korth.                                 *
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

#include "Encoding.hpp"

#include "config.libgenstext.h"
#include "byteorder.h"

// Determine which character set decoder to use.
#if defined(_WIN32)
# include <windows.h>
#elif defined(HAVE_ICONV)
#  include <iconv.h>
#  if SYS_BYTEORDER == SYS_BIG_ENDIAN
#    define UTF16_ENCODING "UTF-16BE"
#    define WCHAR_ENCODING "UTF-32BE"
#  else /* SYS_BYTEORDER == SYS_LIL_ENDIAN */
#    define UTF16_ENCODING "UTF-16LE"
#    define WCHAR_ENCODING "UTF-32LE"
#  endif
#endif

// C++ includes.
#include <string>
using std::string;
using std::u16string;
using std::wstring;

// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>

// TODO: Use std::auto_ptr<>?

namespace LibGensText {

#if defined(_WIN32)
/**
 * Convert a null-terminated multibyte string to UTF-16.
 * @param mbs		[in] Multibyte string. (null-terminated)
 * @param codepage	[in] mbs codepage.
 * @return Allocated UTF-16 string, or NULL on error. (Must be free()'d after use!)
 */
static wchar_t *W32U_mbs_to_UTF16(const char *mbs, unsigned int codepage)
{
	int cchWcs = MultiByteToWideChar(codepage, 0, mbs, -1, nullptr, 0);
	if (cchWcs <= 0)
		return nullptr;

	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(codepage, 0, mbs, -1, wcs, cchWcs);
	return wcs;
}

/**
 * Convert a multibyte string to UTF-16.
 * @param mbs		[in] Multibyte string.
 * @param cbMbs		[in] Length of mbs, in bytes.
 * @param codepage	[in] mbs codepage.
 * @param cchWcs_ret	[out, opt] Number of characters in the returned string.
 * @return Allocated UTF-16 string, or NULL on error. (Must be free()'d after use!)
 * NOTE: Returned string might NOT be NULL-terminated!
 */
static wchar_t *W32U_mbs_to_UTF16(const char *mbs, int cbMbs,
		unsigned int codepage, int *cchWcs_ret)
{
	int cchWcs = MultiByteToWideChar(codepage, 0, mbs, cbMbs, nullptr, 0);
	if (cchWcs <= 0)
		return nullptr;

	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(codepage, 0, mbs, cbMbs, wcs, cchWcs);

	if (cchWcs_ret)
		*cchWcs_ret = cchWcs;
	return wcs;
}

/**
 * Convert a null-terminated UTF-16 string to multibyte.
 * @param wcs		[in] UTF-16 string. (null-terminated)
 * @param codepage	[in] mbs codepage.
 * @return Allocated multibyte string, or NULL on error. (Must be free()'d after use!)
 */
static char *W32U_UTF16_to_mbs(const wchar_t *wcs, unsigned int codepage)
{
	int cbMbs = WideCharToMultiByte(codepage, 0, wcs, -1, nullptr, 0, nullptr, nullptr);
	if (cbMbs <= 0)
		return nullptr;
 
	char *mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(codepage, 0, wcs, -1, mbs, cbMbs, nullptr, nullptr);
	return mbs;
}

/**
 * Convert a UTF-16 string to multibyte.
 * @param wcs		[in] UTF-16 string.
 * @param cchWcs	[in] Length of wcs, in characters.
 * @param codepage	[in] mbs codepage.
 * @param cbMbs_ret	[out, opt] Number of bytes in the returned string.
 * @return Allocated multibyte string, or NULL on error. (Must be free()'d after use!)
 * NOTE: Returned string might NOT be NULL-terminated!
 */
static char *W32U_UTF16_to_mbs(const wchar_t *wcs, int cchWcs,
		unsigned int codepage, int *cbMbs_ret)
{
	int cbMbs = WideCharToMultiByte(codepage, 0, wcs, cchWcs, nullptr, 0, nullptr, nullptr);
	if (cbMbs <= 0)
		return nullptr;

	char *mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(codepage, 0, wcs, cchWcs, mbs, cbMbs, nullptr, nullptr);

	if (cbMbs_ret)
		*cbMbs_ret = cbMbs;
	return mbs;
}

#elif defined(HAVE_ICONV)

/**
 * Convert a string from one character set to another.
 * @param src 		[in] Source string.
 * @param src_bytes_len [in] Source length, in bytes.
 * @param src_charset	[in] Source character set.
 * @param dest_charset	[in] Destination character set.
 * @return malloc()'d UTF-8 string, or nullptr on error.
 */
static char *gens_iconv(const char *src, size_t src_bytes_len,
			const char *src_charset, const char *dest_charset)
{
	if (!src || src_bytes_len == 0)
		return nullptr;

	if (!src_charset)
		src_charset = "";
	if (!dest_charset)
		dest_charset = "";

	// Based on examples from:
	// * http://www.delorie.com/gnu/docs/glibc/libc_101.html
	// * http://www.codase.com/search/call?name=iconv

	// Open an iconv descriptor.
	iconv_t cd;
	cd = iconv_open(dest_charset, src_charset);
	if (cd == (iconv_t)(-1)) {
		// Error opening iconv.
		return nullptr;
	}

	// Allocate the output buffer.
	// UTF-8 is variable length, and the largest UTF-8 character is 4 bytes long.
	const size_t out_bytes_len = (src_bytes_len * 4) + 4;
	size_t out_bytes_remaining = out_bytes_len;
	char *outbuf = (char*)malloc(out_bytes_len);

	// Input and output pointers.
	char *inptr = (char*)(src);	// Input pointer.
	char *outptr = &outbuf[0];	// Output pointer.

	int success = 1;

	while (src_bytes_len > 0) {
		if (iconv(cd, &inptr, &src_bytes_len, &outptr, &out_bytes_remaining) == (size_t)(-1)) {
			// An error occurred while converting the string.
			if (outptr == &outbuf[0]) {
				// No bytes were converted.
				success = 0;
			} else {
				// Some bytes were converted.
				// Accept the string up to this point.
				// Madou Monogatari I has a broken Shift-JIS sequence
				// at position 9, which resulted in no conversion.
				// (Reported by andlabs.)
				success = 1;
			}
			break;
		}
	}

	// Close the iconv descriptor.
	iconv_close(cd);

	if (success) {
		// The string was converted successfully.

		// Make sure the string is null-terminated.
		size_t null_bytes = (out_bytes_remaining > 4 ? 4 : out_bytes_remaining);
		for (size_t i = null_bytes; i > 0; i--) {
			*outptr++ = 0x00;
		}

		// Return the output buffer.
		return outbuf;
	}

	// The string was not converted successfully.
	free(outbuf);
	return nullptr;
}
#endif /* HAVE_ICONV */

/** UTF-16 conversion. **/

/**
 * Convert UTF-16 (host-endian) to UTF-8.
 * @param src UTF-16 string. (host-endian)
 * @param len Length of UTF-16 string, in characters.
 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
 */
string Utf16_to_Utf8(const char16_t *src, size_t len)
{
#if defined(_WIN32)
	// Win32 version.
	int cbMbs;
	char *mbs = W32U_UTF16_to_mbs((wchar_t*)src, (int)len, CP_UTF8, &cbMbs);
	if (!mbs) {
		return string();
	}
	string ret(mbs, cbMbs);
	free(mbs);
	return ret;
#elif defined(HAVE_ICONV)
	// iconv version.
	char *mbs = gens_iconv((char*)src, len * 2, UTF16_ENCODING, "UTF-8");
	if (!mbs) {
		return string();
	}
	string ret(mbs);
	free(mbs);
	return ret;
#else
	// No translation supported.
	// TODO: #error?
	return string();
#endif
}

/**
 * Convert UTF-8 to UTF-16 (host-endian).
 * @param src UTF-8 string.
 * @param len Length of UTF-8 string, in bytes.
 * @return UTF-16 string, or empty string on error.
 */
u16string Utf8_to_Utf16(const char *src, size_t len)
{
#if defined(_WIN32)
	// Win32 version. Use W32U_mini.
	int cchWcs;
	char16_t *wcs = (char16_t*)W32U_mbs_to_UTF16(src, len, CP_UTF8, &cchWcs);
	if (!wcs) {
		return u16string();
	}
	u16string ret(wcs, cchWcs);
	free(wcs);
	return ret;
#elif defined(HAVE_ICONV)
	// iconv version.
	char16_t *wcs = (char16_t*)gens_iconv(src, len, "UTF-8", UTF16_ENCODING);
	if (!wcs) {
		return u16string();
	}
	u16string ret(wcs);
	free(wcs);
	return ret;
#else
	// No translation supported.
	// TODO: #error?
	return u16string();
#endif
}

/** wchar_t conversion. **/

/**
 * Convert wchar_t to UTF-8.
 * @param src wchar_t string.
 * @param len Length of wchar_t string, in characters.
 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
 */
string Wchar_to_Utf8(const wchar_t *src, size_t len)
{
#if defined(_WIN32)
	// Win32 version.
	static_assert(sizeof(wchar_t) == 2, "wchar_t is the wrong size. (Should be 2 on Win32.)");
	int cbMbs;
	char *mbs = W32U_UTF16_to_mbs((wchar_t*)src, (int)len, CP_UTF8, &cbMbs);
	if (!mbs) {
		return string();
	}
	string ret(mbs, cbMbs);
	free(mbs);
	return ret;
#elif defined(HAVE_ICONV)
	// iconv version.
	static_assert(sizeof(wchar_t) == 4, "wchar_t is the wrong size. (Should be 4 on Unix/Linux/Mac.)");
	char *mbs = gens_iconv((char*)src, len * 2, WCHAR_ENCODING, "UTF-8");
	if (!mbs) {
		return string();
	}
	string ret(mbs);
	free(mbs);
	return ret;
#else
	// No translation supported.
	// TODO: #error?
	return string();
#endif
}

/**
 * Convert UTF-8 to wchar_t.
 * @param src UTF-8 string.
 * @param len Length of UTF-8 string, in bytes.
 * @return wchar_t string, or empty string on error.
 */
wstring Utf8_to_Wchar(const char *src, size_t len)
{
#if defined(_WIN32)
	// Win32 version. Use W32U_mini.
	static_assert(sizeof(wchar_t) == 2, "wchar_t is the wrong size. (Should be 2 on Win32.)");
	int cchWcs;
	wchar_t *wcs = W32U_mbs_to_UTF16(src, len, CP_UTF8, &cchWcs);
	if (!wcs) {
		return wstring();
	}
	wstring ret(wcs, cchWcs);
	free(wcs);
	return ret;
#elif defined(HAVE_ICONV)
	// iconv version.
	static_assert(sizeof(wchar_t) == 4, "wchar_t is the wrong size. (Should be 4 on Unix/Linux/Mac.)");
	wchar_t *wcs = (wchar_t*)gens_iconv(src, len, "UTF-8", WCHAR_ENCODING);
	if (!wcs) {
		return wstring();
	}
	wstring ret(wcs);
	free(wcs);
	return ret;
#else
	// No translation supported.
	// TODO: #error?
	return wstring();
#endif
}

/** Other conversion. **/

/**
 * Convert Shift-JIS to UTF-8.
 * @param src Shift-JIS string.
 * @param len Length of Shift-JIS string, in bytes.
 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
 */
string SJIS_to_Utf8(const char *src, size_t len)
{
	char *mbs = nullptr;
	int cbMbs;
#if defined(_WIN32)
	// Win32 version. Use W32U_mini.
	int cchWcs;
	wchar_t *wcs = W32U_mbs_to_UTF16(src, len, 932, &cchWcs);
	if (wcs) {
		mbs = W32U_UTF16_to_mbs(wcs, cchWcs, CP_UTF8, &cbMbs);
		free(wcs);
	}
#elif defined(HAVE_ICONV)
	mbs = gens_iconv(src, len, "SHIFT-JIS", "UTF-8");
	if (mbs) {
		cbMbs = strlen(mbs);
	}
#else
	// No translation supported.
	// TODO: #error?
#endif

	if (!mbs)
		return string();
	string ret(mbs, cbMbs);
	free(mbs);
	return ret;
}

/** Miscellaneous. **/

#ifdef _WIN32
/**
 * Compare two UTF-16 strings.
 * NOTE: This function expects host-endian UTF-16.
 * @param s1 String 1.
 * @param s2 String 2.
 * @param n Maximum number of characters to check.
 * @return Negative value if s1 < s2; 0 if s1 == s2; positive value if s1 > s2.
 */
int Utf16_ncmp(const char16_t *s1, const char16_t *s2, size_t n)
{
	return _wcsnicmp((const wchar_t*)s1, (const wchar_t*)s2, n);
}
#else
/**
 * Compare two UTF-16 strings.
 * NOTE: This function expects host-endian UTF-16.
 * @param s1 String 1.
 * @param s2 String 2.
 * @param n Maximum number of characters to check.
 * @return Negative value if s1 < s2; 0 if s1 == s2; positive value if s1 > s2.
 */
int Utf16_ncmp(const char16_t *s1, const char16_t *s2, size_t n)
{
	// TODO: This expects platform-endian strings.
	// Add a parameter for LE vs. BE?

	// TODO: Surrogate support. (Maybe.)
	// Then again, this function's only really used to check
	// if two strings match, and isn't used for sorting.

	for (; n > 0; n--) {
		if (*s1 != *s2)
			return ((int)((*s1) - (*s2)));

		s1++; s2++;
		if (*s1 == 0 || *s2 == 0)
			break;
	}

	// Verify the last character.
	if (*s1 == 0x00 && *s2 != 0x00)
		return 1;
	else if (*s1 != 0x00 && *s2 == 0x00)
		return -1;

	// Strings match.
	return 0;
}
#endif

}
