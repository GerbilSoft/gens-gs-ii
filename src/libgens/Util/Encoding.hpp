/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Encoding.hpp: Character encoding helper class.                          *
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

#ifndef __LIBGENS_UTIL_ENCODING_HPP__
#define __LIBGENS_UTIL_ENCODING_HPP__

// TODO: Figure out a better way to #include this.
#include "../c++11-compat.h"

// C++ includes.
#include <string>

#ifdef _WIN32
// Required for Win32.
#include <cstring>
#endif

// C includes.
#include <stdint.h>

// LibGens includes.
#include "macros/common.h"

namespace LibGens
{

class Encoding
{
	public:
		/**
		 * Convert UTF-16 (host-endian) to UTF-8.
		 * @param src UTF-16 string. (host-endian)
		 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
		 */
		static std::string Utf16_to_Utf8(const std::u16string& src);
		
		/**
		 * Convert UTF-16 (host-endian) to UTF-8.
		 * @param src UTF-16 string. (host-endian)
		 * @param len Length of UTF-16 string, in characters.
		 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
		 */
		static std::string Utf16_to_Utf8(const char16_t *src, size_t len);
		
		/**
		 * Convert UTF-8 to UTF-16 (host-endian).
		 * @param src UTF-8 string.
		 * @return UTF-16 string, or empty string on error.
		 */
		static std::u16string Utf8_to_Utf16(const std::string& src);
		
		/**
		 * Convert Shift-JIS to UTF-8.
		 * @param src Shift-JIS string.
		 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
		 */
		static std::string SJIS_to_Utf8(const std::string& src);
		
		/**
		 * Compare two UTF-16 strings.
		 * NOTE: This function expects host-endian UTF-16.
		 * @param s1 String 1.
		 * @param s2 String 2.
		 * @param n Maximum number of characters to check.
		 * @return Negative value if s1 < s2; 0 if s1 == s2; positive value if s1 > s2.
		 */
		static int Utf16_ncmp(const char16_t *s1, const char16_t *s2, size_t n);
	
	private:
		Encoding() { }
		~Encoding() { }
};

/**
 * Convert UTF-16 (host-endian) to UTF-8.
 * @param src UTF-16 string. (host-endian)
 * @return UTF-8 string, or empty string on error. (TODO: Better error handling?)
 */
inline std::string Encoding::Utf16_to_Utf8(const std::u16string& src)
{
	// The raw char16_t* version is used in Dc7z.cpp,
	// so we'll make the std::u16string version a wrapper
	// instead of converting the char16_t* to an std::u16string.
	return Utf16_to_Utf8(src.data(), src.size());
}

#ifdef _WIN32
// Win32: wchar_t is 16-bit, so use _wcsicmp() directly.
inline int Encoding::Utf16_ncmp(const char16_t *s1, const char16_t *s2, size_t n)
	{ return _wcsnicmp((const wchar_t*)s1, (const wchar_t*)s2, n); }
#endif

}

#endif /* __LIBGENS_UTIL_ENCODING_HPP__ */
