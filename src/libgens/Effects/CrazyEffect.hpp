/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * CrazyEffect.hpp: "Crazy" effect.                                        *
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

#ifndef __LIBGENS_EFFECTS_CRAZYEFFECT_HPP__
#define __LIBGENS_EFFECTS_CRAZYEFFECT_HPP__

#include "../macros/common.h"

// C includes.
#include <stdint.h>
#include <stdlib.h>

namespace LibGens {

class MdFb;

class CrazyEffectPrivate;
class CrazyEffect
{
	public:
		CrazyEffect();
		~CrazyEffect();

	private:
		friend class CrazyEffectPrivate;
		CrazyEffectPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		CrazyEffect(const CrazyEffect &);
		CrazyEffect &operator=(const CrazyEffect &);

	public:
		enum ColorMask {
			CM_BLACK	= 0,
			CM_BLUE		= 1,
			CM_GREEN	= 2,
			CM_CYAN		= 3,
			CM_RED		= 4,
			CM_MAGENTA	= 5,
			CM_YELLOW	= 6,
			CM_WHITE	= 7,
		};

		// Color mask property.
		ColorMask colorMask(void) const;
		void setColorMask(ColorMask colorMask);

		/**
		 * Run the "Crazy Effect" on the MD screen.
		 * @param fb MdFb to apply the effect to. 
		 */
		void run(MdFb *fb);
		inline void run(MdFb *fb, ColorMask colorMask);
};

inline void CrazyEffect::run(MdFb *fb, ColorMask colorMask)
{
	setColorMask(colorMask);
	run(fb);
}

}

#endif /* __LIBGENS_EFFECTS_CRAZYEFFECT_HPP__ */
