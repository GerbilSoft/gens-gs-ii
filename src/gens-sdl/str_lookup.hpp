/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * str_lookup.cpp: String lookups for some enumerations.                   *
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

// LibGens includes.
#include "libgens/Rom.hpp"

namespace GensSdl {

/**
 * Convert a ROM format ID to a string.
 * @param romFormat ROM format ID.
 * @return String.
 */
const char *romFormatToString(LibGens::Rom::RomFormat romFormat);

/**
 * Convert a ROM system ID to a string.
 * @param sysId ROM system ID.
 * @return String.
 */
const char *sysIdToString(LibGens::Rom::MDP_SYSTEM_ID sysId);

}
