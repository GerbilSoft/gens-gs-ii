/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * GensZomg.hpp: LibGens ZOMG wrapper.                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
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

/**
 * WARNING: This version of ZOMG is not the final version,
 * and is subject to change.
 */

#ifndef __LIBGENS_SAVE_GENSZOMG_HPP__
#define __LIBGENS_SAVE_GENSZOMG_HPP__

// utf8_str
#include "../macros/common.h"

// C includes.
#include <string.h>

// Emulation context.
#include "../EmuContext.hpp"

namespace LibGens
{

/**
 * ZomgLoad(): Load the current state from a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @param context	[out] Emulation context.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int ZomgLoad(const utf8_str *filename, EmuContext *context);

/**
 * ZomgSave(): Save the current state to a ZOMG file.
 * @param filename	[in] ZOMG file.
 * @param context	[in] Emulation context.
 * @param img_buf	[in, opt] Buffer containing PNG image for the ZOMG preview image.
 * @param img_siz	[in, opt] Size of img_buf.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int ZomgSave(const utf8_str *filename, const EmuContext *context,
	     const void *img_buf, size_t img_siz);

}

#endif /* __LIBGENS_SAVE_GENSZOMG_HPP__ */
