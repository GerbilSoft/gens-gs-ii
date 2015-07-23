/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Screenshot.hpp: Screenshot helper.                                      *
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

#ifndef __LIBGENS_UTIL_SCREENSHOT_HPP__
#define __LIBGENS_UTIL_SCREENSHOT_HPP__

#include "../macros/common.h"

namespace LibZomg {
	class ZomgBase;
}

namespace LibGens {

class MdFb;
class Rom;

class Screenshot
{
	private:
		// Static class.
		Screenshot() { }
		~Screenshot() { }

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Screenshot(const Screenshot &);
		Screenshot &operator=(const Screenshot &);

	public:
		/**
		 * Save a screenshot to a file.
		 * File will be in PNG format.
		 * TODO: Metadata flags parameter.
		 * @param fb		[in] MD framebuffer.
		 * @param rom		[in] ROM object. (Needed for some metadata.)
		 * @param filename	[in] Filename for the screenshot.
		 * @return 0 on success; negative errno on error.
		 */
		static int toFile(const MdFb *fb, const Rom *rom, const utf8_str *filename);

		/**
		 * Save a screenshot to a ZOMG savestate.
		 * TODO: Metadata flags parameter.
		 * @param zomg	[in,out] ZOMG savestate.
		 * @param fb	[in] MD framebuffer.
		 * @param rom	[in] ROM object. (Needed for some metadata.)
		 * @return 0 on success; negative errno on error.
		 */
		static int toZomg(LibZomg::ZomgBase *zomg, const MdFb *fb, const Rom *rom);
};

}

#endif /* __LIBGENS_UTIL_SCREENSHOT_HPP__ */
