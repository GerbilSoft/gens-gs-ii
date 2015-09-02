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

#include <stdint.h>
#include "../macros/common.h"

namespace LibGens {

class MdFb;
class CrazyEffect
{
	public:
		CrazyEffect();

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
		inline ColorMask colorMask(void) const;
		inline void setColorMask(ColorMask newColorMask);

		/**
		 * Run the "Crazy Effect" on the MD screen.
		 * @param fb MdFb to apply the effect to. 
		 */
		void run(MdFb *fb);
		void run(MdFb *fb, ColorMask newColorMask);

	private:
		/**
		 * Do the "Crazy" effect.
		 * @param pixel     [in]  Type of pixel.
		 * @param RBits     [in]  Number of bits for Red.
		 * @param GBits     [in]  Number of bits for Green.
		 * @param BBits     [in]  Number of bits for Blue.
		 * @param outScreen [out] Destination screen.
		 */
		template<typename pixel, uint8_t RBits, uint8_t GBits, uint8_t BBits>
		inline void T_doCrazyEffect(pixel *outScreen);

		// Color mask.
		ColorMask m_colorMask;
};

inline CrazyEffect::ColorMask CrazyEffect::colorMask(void) const
	{ return m_colorMask; }

inline void CrazyEffect::setColorMask(CrazyEffect::ColorMask newColorMask)
{
	if (newColorMask < CM_BLACK || newColorMask > CM_WHITE)
		return;
	
	m_colorMask = newColorMask;
}


inline void CrazyEffect::run(MdFb *fb, ColorMask newColorMask)
{
	setColorMask(newColorMask);
	run(fb);
}

}

#endif /* __LIBGENS_EFFECTS_CRAZYEFFECT_HPP__ */
