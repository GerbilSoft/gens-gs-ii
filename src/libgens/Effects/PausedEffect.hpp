/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * PausedEffect.hpp: "Paused" effect.                                      *
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

#ifndef __LIBGENS_EFFECTS_PAUSEDEFFECT_HPP__
#define __LIBGENS_EFFECTS_PAUSEDEFFECT_HPP__

// RESTRICT keyword.
// TODO: Move this to another file.
#ifndef RESTRICT
#if defined(__cplusplus) && defined(__GNUC__)
#define RESTRICT __restrict__
#elif defined(_MSC_VER)
#define RESTRICT __restrict
#elif __STDC_VERSION__ >= 199901L
#define RESTRICT restrict
#else
#define RESTRICT
#endif
#endif /* RESTRICT */

namespace LibGens {

class MdFb;

class PausedEffect
{
	public:
		static void DoPausedEffect(MdFb* RESTRICT outScreen);
		static void DoPausedEffect(MdFb* RESTRICT outScreen, const MdFb* RESTRICT mdScreen);

	private:
		PausedEffect() { }
		~PausedEffect() { }
};

}

#endif /* __LIBGENS_EFFECTS_PAUSEDEFFECT_HPP__ */
