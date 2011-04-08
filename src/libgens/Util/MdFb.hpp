/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * MdFb.hpp: MD framebuffer class.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __LIBGENS_UTIL_MDFB_HPP__
#define __LIBGENS_UTIL_MDFB_HPP__

#include <stdint.h>
#include <assert.h>

namespace LibGens
{

class MdFb
{
	public:
		// Line access.
		uint16_t *lineBuf16(int line);
		uint32_t *lineBuf32(int line);
		
		// Framebuffer access.
		uint16_t *fb16(void);
		uint32_t *fb32(void);
		int pxPerLine(void) const;
		int numLines(void) const;
	
	private:
		// NOTE: These are subject to change in the future.
		// Specifically, they may become dynamic.
		// TODO: Eliminate the 8px left/right borders.
		static const int ms_PxPerLine = 336;
		static const int ms_NumLines = 240;
		
		// 336px and 320px tables.
		static const unsigned int TAB336[240];
		static const unsigned int TAB320[240];
		
		// Framebuffer.
		union FB_t
		{
			uint16_t u16[ms_PxPerLine * ms_NumLines + 8];
			uint32_t u32[ms_PxPerLine * ms_NumLines + 8];
		};
		FB_t m_fb;
};


/** Line access. **/

/**
 * lineBuf16(): Get a pointer to the specified line buffer. (16-bit color)
 * @param line Line number.
 * @return Pointer to the specified line buffer. (16-bit color)
 */
inline uint16_t *MdFb::lineBuf16(int line)
{
	assert(line >= 0 && line < ms_NumLines);
	return &m_fb.u16[TAB336[line] + 8];
}

/**
 * lineBuf32(): Get a pointer to the specified line buffer. (32-bit color)
 * @param line Line number.
 * @return Pointer to the specified line buffer. (32-bit color)
 */
inline uint32_t *MdFb::lineBuf32(int line)
{
	assert(line >= 0 && line < ms_NumLines);
	return &m_fb.u32[TAB336[line] + 8];
}


/** Framebuffer access. **/

inline uint16_t *MdFb::fb16(void)
	{ return &m_fb.u16[8]; }

inline uint32_t *MdFb::fb32(void)
	{ return &m_fb.u32[8]; }

inline int MdFb::pxPerLine(void) const
	{ return ms_PxPerLine; }

inline int MdFb::numLines(void) const
	{ return ms_NumLines; }

}

#endif /* __LIBGENS_UTIL_TIMING_HPP__ */
