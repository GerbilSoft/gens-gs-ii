/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * FastBlur.hpp: Fast Blur effect.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_EFFECTS_FASTBLUR_HPP__
#define __LIBGENS_EFFECTS_FASTBLUR_HPP__

// TODO: Move this somewhere else!
#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))
#define HAVE_MMX
#endif

#include <stdint.h>

namespace LibGens
{

class MdFb;

class FastBlur
{
	public:
		static void DoFastBlur(MdFb *outScreen, bool fromMdScreen = true);
	
	protected:
		template<typename pixel, pixel mask>
		static inline void T_DoFastBlur(pixel *mdScreen);
	
#ifdef HAVE_MMX
		static const uint32_t MASK_DIV2_15_MMX[2];
		static const uint32_t MASK_DIV2_16_MMX[2];
		static const uint32_t MASK_DIV2_32_MMX[2];
		
		static void DoFastBlur_16_MMX(uint16_t *mdScreen, const uint32_t *mask);
		static void DoFastBlur_32_MMX(uint32_t *mdScreen);
#endif /* HAVE_MMX */
	
	private:
		FastBlur() { }
		~FastBlur() { }
};

}

#endif /* __LIBGENS_EFFECTS_CRAZYEFFECT_HPP__ */
