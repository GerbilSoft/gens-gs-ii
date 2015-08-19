/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * MdFb.hpp: MD framebuffer class.                                         *
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

#include "MdFb.hpp"

// C includes.
#include <stdlib.h>

// C++ includes.
#include <vector>
using std::vector;

// aligned_malloc()
#include "libcompat/aligned_malloc.h"

namespace LibGens {

MdFb::MdFb()
	: m_refcnt(1)
	// Framebuffer parameters.
	, m_pxPerLine(320)
	, m_pxPitch(336)
	, m_pxStart(8)
	, m_numLines(240)
	// Color depth.
	, m_bpp(BPP_32)
	, m_fb(nullptr)
	// Image parameters.
	, m_imgWidth(m_pxPerLine)
	, m_imgHeight(m_numLines)
	, m_imgXStart(0)
	, m_imgYStart(0)
{
	reinitFb();
}

MdFb::~MdFb() {
	aligned_free(m_fb);
}

/**
 * Reinitialize the framebuffer.
 */
void MdFb::reinitFb(void) {
	// Free and reallocate the framebuffer.
	// An extra 16 pixels are allocated to prevent overrunning
	// the framebuffer if pxPitch is used at the first pixel.
	// TODO: Maybe it should only be 8?
	aligned_free(m_fb);
	m_fb_sz = (m_pxPitch * m_numLines + 16) * sizeof(uint32_t);
	m_fb = aligned_malloc(16, m_fb_sz);
	if (!m_fb) {
		// Error allocating the framebuffer.
		// TODO: What do we do here?
		m_fb_sz = 0;
		return;
	}

	// Zero the framebuffer.
	memset(m_fb, 0, m_fb_sz);

	// Initialize the line number lookup table.
	m_lineNumTable.resize(m_numLines);
	for (int y = 0, px = 0; y < m_numLines; y++, px += m_pxPitch) {
		m_lineNumTable[y] = px;
	}
}

}
