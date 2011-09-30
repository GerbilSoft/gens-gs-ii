/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Encoding.cpp: Character encoding helper class.                          *
 *                                                                         *
 * Copyright (c) 2009-2011 by David Korth.                                 *
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

#include <config.h>

#include "Encoding.hpp"

#include "Util/byteswap.h"

// Determine which character set decoder to use.
#if defined(_WIN32)
#  include "Win32/W32U_mini.h"
#elif defined(HAVE_ICONV)
#  include <iconv.h>
#  if GENS_BYTEORDER == GENS_BIG_ENDIAN
#    define UTF16_ENCODING "UTF-16BE"
#  else /* GENS_BYTEORDER == GENS_LIL_ENDIAN */
#    define UTF16_ENCODING "UTF-16LE"
#  endif
#endif

// C++ includes.
#include <string>
using std::string;

// C includes. (C++ namespace)
#include <cstdlib>


#ifdef HAVE_ICONV
/**
 * gens_iconv(): Convert a string from one character set to another.
 * @param src 		[in] Source string.
 * @param src_bytes_len [in] Source length, in bytes.
 * @param src_charset	[in] Source character set.
 * @param dest_charset	[in] Destination character set.
 * @return malloc()'d UTF-8 string, or NULL on error.
 */
static char *gens_iconv(const char *src, size_t src_bytes_len,
			const char *src_charset, const char *dest_charset)
{
	if (!src || src_bytes_len == 0)
		return NULL;
	
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
	if (cd == (iconv_t)(-1))
	{
		// Error opening iconv.
		return NULL;
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
	
	while (src_bytes_len > 0)
	{
		if (iconv(cd, &inptr, &src_bytes_len, &outptr, &out_bytes_remaining) == (size_t)(-1))
		{
			// An error occurred while converting the string.
			if (outptr == &outbuf[0])
			{
				// No bytes were converted.
				success = 0;
			}
			else
			{
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
	
	if (success)
	{
		// The string was converted successfully.
		
		// Make sure the string is null-terminated.
		size_t null_bytes = (out_bytes_remaining > 4 ? 4 : out_bytes_remaining);
		for (size_t i = null_bytes; i > 0; i--)
		{
			*outptr++ = 0x00;
		}
		
		// Return the output buffer.
		return outbuf;
	}
	
	// The string was not converted successfully.
	free(outbuf);
	return NULL;
}
#endif /* HAVE_ICONV */


namespace LibGens
{

/**
 * Convert UTF-16 (host-endian) to UTF-8.
 * @param src UTF-16 string. (host-endian)
 * @param len Length of UTF-16 string, in characters.
 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
 */
string Encoding::Utf16_to_Utf8(const char16_t *src, size_t len)
{
	char *mbs;
#if defined(_WIN32)
	// Win32 version. Use W32U_mini.
	// TODO: Use the "len" parameter.
	mbs = W32U_UTF16_to_mbs((wchar_t*)src, CP_UTF8);
#elif defined(HAVE_ICONV)
	// iconv version.
	mbs = gens_iconv((char*)src, len * 2, UTF16_ENCODING, "UTF-8");
#else
	// No translation supported.
	// TODO: #error?
	return string();
#endif
	
	if (!mbs)
		return string();
	
	// Convert the allocated data to an std::string.
	string ret(mbs);
	free(mbs);
	return ret;
}


/**
 * Convert UTF-8 to UTF-16 (host-endian).
 * @param src UTF-8 string. (null-terminated)
 * @return Allocated null-terminated UTF-16 string, or NULL on error.
 */
char16_t *Encoding::Utf8_to_Utf16(const string& src)
{
#if defined(_WIN32)
	// Win32 version. Use W32U_mini.
	// TODO: Use the source string's length.
	return (char16_t*)W32U_mbs_to_UTF16(src.c_str(), CP_UTF8);
#elif defined(HAVE_ICONV)
	// iconv version.
	return (char16_t*)gens_iconv(src.data(), src.size(), "UTF-8", UTF16_ENCODING);
#else
	// No translation supported.
	// TODO: #error?
	return NULL;
#endif
}


/**
 * Convert Shift-JIS to UTF-8.
 * @param src Shift-JIS string.
 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
 */
std::string Encoding::SJIS_to_Utf8(const std::string& src)
{
	utf8_str *mbs = NULL;
#if defined(_WIN32)
	wchar_t *wcs = W32U_mbs_to_UTF16(src.c_str(), 932); // cp932 == Shift-JIS
	if (wcs)
	{
		mbs = W32U_UTF16_to_mbs(wcs, CP_UTF8);
		free(wcs);
	}
#elif defined(HAVE_ICONV)
	mbs = gens_iconv(src.data(), src.length(), "SHIFT-JIS", "UTF-8");
#else
	// No translation supported.
	// TODO: #error?
#endif
	
	if (!mbs)
		return string();
	string ret(mbs);
	free(mbs);
	return ret;
}


#ifndef _WIN32
/**
 * Compare two UTF-16 strings.
 * NOTE: This function expects host-endian UTF-16.
 * @param s1 String 1.
 * @param s2 String 2.
 * @param n Maximum number of characters to check.
 * @return Negative value if s1 < s2; 0 if s1 == s2; positive value if s1 > s2.
 */
int Encoding::Utf16_ncmp(const char16_t *s1, const char16_t *s2, size_t n)
{
	// TODO: This expects platform-endian strings.
	// Add a parameter for LE vs. BE?
	
	// TODO: Surrogate support. (Maybe.)
	// Then again, this function's only really used to check
	// if two strings match, and isn't used for sorting.
	
	for (; n > 0; n--)
	{
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
