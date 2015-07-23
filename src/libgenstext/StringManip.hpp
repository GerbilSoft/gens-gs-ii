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

#ifndef __LIBGENSTEXT_STRINGMANIP_HPP__
#define __LIBGENSTEXT_STRINGMANIP_HPP__

// C++ includes.
#include <string>

namespace LibGensText {

/**
 * Remove excess spaces from a string.
 * This removes spaces at the end of the string,
 * as well as double-spaces within the string.
 * TODO: Remove spaces from the beginning of the string?
 * @param src String. (UTF-8)
 * @return String with excess spaces removed. (UTF-8)
 */
std::string SpaceElim(const std::string &src);

/**
 * Get the filename portion of a path, with its extension.
 * @param filename Original filename.
 * @return Filename without directories.
 */
std::string FilenameBase(const std::string &filename);

/**
 * Get the filename portion of a path, without its extension.
 * @param filename Original filename.
 * @return Filename without directories or its extension.
 */
std::string FilenameBaseNoExt(const std::string &filename);

}

#endif /* __LIBGENSTEXT_STRINGMANIP_HPP__ */
