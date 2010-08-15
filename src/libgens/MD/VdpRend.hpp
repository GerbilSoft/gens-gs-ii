/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend.hpp: VDP rendering class.                                       *
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

#ifndef __LIBGENS_MD_VDPREND_HPP__
#define __LIBGENS_MD_VDPREND_HPP__

// C includes.
#include <stdint.h>

// Byteswapping macros.
// TODO: LibGens doesn't -I the src/ directory;
// however, gens-qt4 doesn't -I the libgens/ directory.
// So, we're using relative paths for now.
#include "../Util/byteswap.h"

// VDP palette manager.
#include "VdpPalette.hpp"

namespace LibGens
{

class VdpRend
{
	public:
		/** NOTE: Init(), End(), and Reset() should ONLY be called from VdpIo()! **/
		static void Init(void);
		static void End(void);
		static void Reset(void);
		
		// Palette manager.
		static VdpPalette m_palette;
		
		// Screen buffer.
		union Screen_t
		{
			uint16_t u16[336 * 240];
			uint32_t u32[336 * 240];
		};
		static Screen_t MD_Screen;

		// Sprite structs.
		struct Sprite_Struct_t
		{
			int Pos_X;
			int Pos_Y;
			unsigned int Size_X;
			unsigned int Size_Y;
			int Pos_X_Max;
			int Pos_Y_Max;
			unsigned int Num_Tile;	// Includes palette, priority, and flip bits.
			int Pos_X_Max_Vis;	// Number of visible horizontal pixels. (Used for Sprite Limit.)
		};
		static Sprite_Struct_t Sprite_Struct[128];
		static unsigned int Sprite_Visible[128];
		
		// If set, enforces sprite limits.
		static int Sprite_Limits;
	
		// VDP layer flags.
		enum VDP_Layer_Flags
		{
			VDP_LAYER_SCROLLA_LOW		= (1 << 0),
			VDP_LAYER_SCROLLA_HIGH		= (1 << 1),
			VDP_LAYER_SCROLLA_SWAP		= (1 << 2),
			VDP_LAYER_SCROLLB_LOW		= (1 << 3),
			VDP_LAYER_SCROLLB_HIGH		= (1 << 4),
			VDP_LAYER_SCROLLB_SWAP		= (1 << 5),
			VDP_LAYER_SPRITE_LOW		= (1 << 6),
			VDP_LAYER_SPRITE_HIGH		= (1 << 7),
			VDP_LAYER_SPRITE_SWAP		= (1 << 8),
			VDP_LAYER_SPRITE_ALWAYSONTOP	= (1 << 9),
			VDP_LAYER_PALETTE_LOCK		= (1 << 10),
		};
		
		static const unsigned int VDP_LAYERS_DEFAULT =
			(VDP_LAYER_SCROLLA_LOW	|
			 VDP_LAYER_SCROLLA_HIGH	|
			 VDP_LAYER_SCROLLB_LOW	|
			 VDP_LAYER_SCROLLB_HIGH	|
			 VDP_LAYER_SPRITE_LOW	|
			 VDP_LAYER_SPRITE_HIGH);
		
		// VDP layer control.
		static unsigned int VDP_Layers;
		
		/** Line rendering functions. **/
		static void Render_Line(void);
		
		// Line buffer for current line.
		// TODO: Endianness conversions.
		union LineBuf_t
		{
			struct LineBuf_px_t
			{
				// TODO: Is byteswapping here really the best option?
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
				uint8_t pixel;
				uint8_t layer;
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
				uint8_t layer;
				uint8_t pixel;
#endif
			};
			LineBuf_px_t px[336];
			uint8_t  u8[336<<1];
			uint16_t u16[336];
			uint32_t u32[336>>1];
		};
		static LineBuf_t LineBuf;
	
	protected:
		template<bool hs, typename pixel>
		static inline void T_Update_Palette(pixel *MD_palette, const pixel *palette);
	
	private:
		VdpRend() { }
		~VdpRend() { }
};

}

#endif /* __LIBGENS_MD_VDPREND_HPP__ */
