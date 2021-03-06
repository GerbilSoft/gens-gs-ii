/***************************************************************************
 * libgenstext: Gens/GS II Text Manipulation Library.                      *
 * StringManip.hpp: String manipulation functions.                         *
 *                                                                         *
 * Copyright (c) 2010-2015 by David Korth.                                 *
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

#include "StringManip.hpp"
#include "Encoding.hpp"

// C++ includes.
#include <string>
using std::string;
using std::u16string;

// C includes. (C++ namespace)
#include <cctype>
#include <cwctype>

namespace LibGensText {

#ifdef _WIN32
#define PATH_SEP_CHR '\\'
#else
#define PATH_SEP_CHR '/'
#endif

/**
 * Determine if a character is a graphics character.
 * @param wchr Character to check.
 * @return True if this is a graphics character; false otherwise.
 */
static inline bool IsGraphChar(char16_t wchr)
{
	// NOTE: On some systems, wctype doesn't work properly.
	// TODO: Optimize this!
	if (!iswspace(0x3000) || iswgraph(0x3000)) {
		// U+3000 is IDEOGRAPHIC SPACE.
		// It's not being detected as such, so fall back to the old algorithm.
		if (wchr < 0x7F) {
			return isgraph(wchr);
		} else if (wchr == 0x3000) {
			// U+3000: IDEOGRAPHIC SPACE
			// Used in "Columns"' ROM headers.
			return false;
		}

		return true;
	}

	// wctype appears to work on this system.
	return iswgraph(wchr);
}

/**
 * Remove excess spaces from a string.
 * This removes spaces at the end of the string,
 * as well as double-spaces within the string.
 * TODO: Remove spaces from the beginning of the string?
 * @param src String. (UTF-8)
 * @return String with excess spaces removed. (UTF-8)
 */
string SpaceElim(const string& src)
{
	// Convert the string to UTF-16 first.
	// TODO: Check for invalid UTF-8 sequences and handle them as cp1252?
	u16string wcs_src = Utf8_to_Utf16(src);
	if (wcs_src.empty()) {
		// Error converting the string. Assume the string is ASCII.
		wcs_src.resize(src.size());
		for (size_t i = 0; i < src.size(); i++) {
			wcs_src[i] = (src[i] & 0x7F);
		}
	}

	// Allocate the destination string. (UTF-16)
	u16string wcs_dest(src.size(), 0);
	int i_dest = 0;

	// Was the last character a graphics character?
	bool lastCharIsGraph = false;

	// Process the string.
	for (size_t i = 0; i < wcs_src.size(); i++) {
		char16_t wchr = wcs_src[i];
		if (!lastCharIsGraph && !IsGraphChar(wchr)) {
			// This is a space character, and the previous
			// character was not a space character.
			continue;
		}

		// This is not a space character,
		// or it is a space character and the previous character wasn't.
		wcs_dest[i_dest++] = wchr;
		lastCharIsGraph = IsGraphChar(wchr);
	}

	if (i_dest == 0) {
		// Empty string.
		return string();
	}

	// Make sure there's no space at the end of the string.
	if (!IsGraphChar(wcs_dest[i_dest - 1]))
		wcs_dest.resize(i_dest - 2 + 1);
	else
		wcs_dest.resize(i_dest - 1 + 1);

	// Convert the string back to UTF-8.
	return Utf16_to_Utf8(wcs_dest);
}

/**
 * Get the filename portion of a path, with its extension.
 * @param filename Original filename.
 * @return Filename without directories.
 */
string FilenameBase(const string &filename)
{
	// Remove the directories from the ROM filename.
	string tmp = filename;

	// Get the filename portion.
	size_t dirSep = tmp.rfind(PATH_SEP_CHR);
	if (dirSep != string::npos)
		tmp.erase(0, dirSep+1);

	return tmp;
}

/**
 * Get the filename portion of a path, without its extension.
 * @param filename Original filename.
 * @return Filename without directories or its extension.
 */
string FilenameBaseNoExt(const string &filename)
{
	// Remove the directories and extension from the ROM filename.
	// TODO: Remove all extensions (e.g. ".gen.gz")?
	string tmp = filename;

	// Get the filename portion.
	size_t dirSep = tmp.rfind(PATH_SEP_CHR);
	if (dirSep != string::npos)
		tmp.erase(0, dirSep+1);

	// Remove the file extension.
	size_t extSep = tmp.rfind('.');
	if (extSep != string::npos)
		tmp.erase(extSep, (tmp.size() - extSep));

	return tmp;
}

}
