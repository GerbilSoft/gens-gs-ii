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

#include <cassert>
#include <cstring>
#include <stdint.h>

#include <vector>

namespace LibGens
{

class MdFb
{
	public:
		MdFb();

		MdFb *ref(void) const;
		void unref(void) const;

	protected:
		~MdFb();
		mutable int m_refcnt;	// Allow ref()/unref() even for const MdFb.

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		MdFb(const MdFb &);
		MdFb &operator=(const MdFb &);

	public:
		// Clear the screen.
		void clear(void);

		// Line access.
		uint16_t *lineBuf16(int line);
		const uint16_t *lineBuf16(int line) const;

		uint32_t *lineBuf32(int line);
		const uint32_t *lineBuf32(int line) const;

		template<typename pixel> pixel *lineBuf(int line);
		template<typename pixel> const pixel *lineBuf(int line) const;

		/** Framebuffer access. **/

		/**
		 * Get the framebuffer. (16-bit color)
		 * @return First pixel of the framebuffer.
		 */
		uint16_t *fb16(void);

		/**
		 * Get the framebuffer. (16-bit color) [const pointer]
		 * @return First pixel of the framebuffer.
		 */
		const uint16_t *fb16(void) const;

		/**
		 * Get the framebuffer. (32-bit color)
		 * @return First pixel of the framebuffer.
		 */
		uint32_t *fb32(void);

		/**
		 * Get the framebuffer. (32-bit color) [const pointer]
		 * @return First pixel of the framebuffer.
		 */
		const uint32_t *fb32(void) const;

		/**
		 * Get the number of visible pixels per line.
		 * @return Number of visible pixels per line.
		 */
		int pxPerLine(void) const;

		/**
		 * Get the total number of pixels per line, including offscreen area.
		 * @return Total number of pixels per line.
		 */
		int pxPitch(void) const;

		/**
		 * Get the starting pixel offset.
		 * @return Starting pixel offset.
		 */
		int pxStart(void) const;

		/**
		 * Get the number of lines.
		 * @return Number of lines.
		 */
		int numLines(void) const;
	
	private:
		/**
		 * Number of visible pixels per line.
		 * Default is 320.
		 */
		int m_pxPerLine;

		/**
		 * Total number of pixels per line.
		 * Default is 336.
		 */
		int m_pxPitch;

		/**
		 * Starting pixel offset.
		 * Default is 8.
		 */
		int m_pxStart;

		/**
		 * Number of lines.
		 * Default is 240.
		 */
		int m_numLines;	

		/**
		 * Framebuffer.
		 */
		void *m_fb;

		/**
		 * Size of framebuffer, in bytes.
		 */
		size_t m_fb_sz;

		/**
		 * Line number lookup table.
		 */
		std::vector<uint32_t> m_lineNumTable;

		/**
		 * Reinitialize the framebuffer.
		 */
		void reinitFb(void);
};


/** Reference counter. **/

inline MdFb* MdFb::ref(void) const
{
	// TODO: Use atomic access?
	m_refcnt++;
	return (MdFb*)this;
}

inline void MdFb::unref(void) const
{
	// TODO: Use atomic access?
	assert(m_refcnt > 0);
	m_refcnt--;
	if (m_refcnt <= 0)
		delete this;
}

/**
 * Clear the screen.
 */
inline void MdFb::clear(void)
{
	memset(m_fb, 0x00, m_fb_sz);
}

/** Line access. **/

/**
 * Get a pointer to the specified line buffer. (16-bit color)
 * @param line Line number.
 * @return Pointer to the specified line buffer. (16-bit color)
 */
inline uint16_t *MdFb::lineBuf16(int line)
{
	assert(line >= 0 && line < m_numLines);
	uint16_t *fb = static_cast<uint16_t*>(m_fb);
	return &fb[m_lineNumTable[line] + m_pxStart];
}

/**
 * Get a pointer to the specified line buffer. (16-bit color) [const pointer]
 * @param line Line number.
 * @return Pointer to the specified line buffer. (16-bit color)
 */
inline const uint16_t *MdFb::lineBuf16(int line) const
{
	assert(line >= 0 && line < m_numLines);
	const uint16_t *fb = static_cast<const uint16_t*>(m_fb);
	return &fb[m_lineNumTable[line] + m_pxStart];
}

/**
 * Get a pointer to the specified line buffer. (32-bit color)
 * @param line Line number.
 * @return Pointer to the specified line buffer. (32-bit color)
 */
inline uint32_t *MdFb::lineBuf32(int line)
{
	assert(line >= 0 && line < m_numLines);
	uint32_t *fb = static_cast<uint32_t*>(m_fb);
	return &fb[m_lineNumTable[line] + m_pxStart];
}

/**
 * Get a pointer to the specified line buffer. (32-bit color) [const pointer]
 * @param line Line number.
 * @return Pointer to the specified line buffer. (32-bit color)
 */
inline const uint32_t *MdFb::lineBuf32(int line) const
{
	assert(line >= 0 && line < m_numLines);
	const uint32_t *fb = static_cast<const uint32_t*>(m_fb);
	return &fb[m_lineNumTable[line] + m_pxStart];
}

/**
 * Get a pointer to the specified line buffer. (templated version)
 * @param pixel Pixel type. (uint16_t or uint32_t)
 * @param line Line number.
 * @return Pointer to the specified line buffer.
 */
template<typename pixel>
inline pixel *MdFb::lineBuf(int line)
{
	assert(sizeof(pixel) == 2 || sizeof(pixel) == 4);
	if (sizeof(pixel) == 4)
		return (pixel*)lineBuf32(line);
	else
		return (pixel*)lineBuf16(line);
}

/**
 * Get a pointer to the specified line buffer. (templated version) [const pointer]
 * @param pixel Pixel type. (uint16_t or uint32_t)
 * @param line Line number.
 * @return Pointer to the specified line buffer.
 */
template<typename pixel>
inline const pixel *MdFb::lineBuf(int line) const
{
	assert(sizeof(pixel) == 2 || sizeof(pixel) == 4);
	if (sizeof(pixel) == 4)
		return (pixel*)lineBuf32(line);
	else
		return (pixel*)lineBuf16(line);
}

/** Framebuffer access. **/

inline uint16_t *MdFb::fb16(void)
	{ return lineBuf16(0); }

inline const uint16_t *MdFb::fb16(void) const
	{ return lineBuf16(0); }

inline uint32_t *MdFb::fb32(void)
	{ return lineBuf32(0); }

inline const uint32_t *MdFb::fb32(void) const
	{ return lineBuf32(0); }

inline int MdFb::pxPerLine(void) const
	{ return m_pxPerLine; }

inline int MdFb::pxPitch(void) const
	{ return m_pxPitch; }

inline int MdFb::pxStart(void) const
	{ return m_pxStart; }

inline int MdFb::numLines(void) const
	{ return m_numLines; }

}

#endif /* __LIBGENS_UTIL_TIMING_HPP__ */
