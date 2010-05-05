/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_m5.hpp: VDP rendering class. (Mode 5)                           *
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

#ifndef __LIBGENS_MD_VDPREND_M5_HPP__
#define __LIBGENS_MD_VDPREND_M5_HPP__

#include <stdint.h>

namespace LibGens
{

class VdpRend_m5
{
	public:
		// Interlaced rendering mode.
		enum IntRend_Mode_t
		{
			INTREND_EVEN	= 0,	// Even lines only. (Old Gens)
			INTREND_ODD	= 1,	// Odd lines only.
			INTREND_FLICKER	= 2,	// Alternating fields. (Flickering Interlaced)
			INTREND_2X	= 3,	// 2x Resolution. (TODO)
		};
		static IntRend_Mode_t IntRend_Mode;
		
		/** Line rendering functions. **/
		static void Render_Line(void);
	
	protected:
		// Temporary VDP data.
		static unsigned int Y_FineOffset;
		static unsigned int TotalSprites;
		
		// TODO: Port FORCE_INLINE from Gens/GS.
		#define FORCE_INLINE inline
		template<bool interlaced>
		static FORCE_INLINE int T_GetLineNumber(void);
		
		template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		static FORCE_INLINE void T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		static FORCE_INLINE void T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool priority, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		static FORCE_INLINE uint8_t T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool plane, bool h_s, bool flip>
		static FORCE_INLINE void T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool plane, bool h_s, bool flip>
		static FORCE_INLINE void T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool priority, bool h_s, bool flip>
		static FORCE_INLINE void T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool plane>
		static FORCE_INLINE uint16_t T_Get_X_Offset(void);
		
		template<bool plane, bool interlaced>
		static FORCE_INLINE unsigned int T_Update_Y_Offset(int cell_cur);
		
		template<bool plane>
		static FORCE_INLINE uint16_t T_Get_Pattern_Info(unsigned int x, unsigned int y);
		
		template<bool interlaced>
		static FORCE_INLINE unsigned int T_Get_Pattern_Data(uint16_t pattern);
		
		template<bool plane, bool interlaced, bool vscroll, bool h_s>
		static FORCE_INLINE void T_Render_Line_Scroll(int cell_start, int cell_length);
		
		template<bool interlaced, bool vscroll, bool h_s>
		static FORCE_INLINE void T_Render_Line_ScrollA(void);
		
		template<bool interlaced, bool partial>
		static FORCE_INLINE void T_Make_Sprite_Struct(void);
		
		template<bool sprite_limit, bool interlaced>
		static FORCE_INLINE unsigned int T_Update_Mask_Sprite(void);
		
		template<bool interlaced, bool h_s>
		static FORCE_INLINE void T_Render_Line_Sprite(void);
		
		template<bool interlaced, bool h_s>
		static FORCE_INLINE void T_Render_Line_m5(void);
		
		template<typename pixel>
		static FORCE_INLINE void T_Render_LineBuf(pixel *dest, pixel *md_palette);
	
	private:
		VdpRend_m5() { }
		~VdpRend_m5() { }
};

}

#endif /* __LIBGENS_MD_VDPREND_M5_HPP__ */
