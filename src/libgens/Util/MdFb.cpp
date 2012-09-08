/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * MdFb.hpp: MD framebuffer class.                                         *
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

#include "MdFb.hpp"

#include <cstdlib>

#include <vector>
using std::vector;

namespace LibGens
{

MdFb::MdFb()
	: m_refcnt(1)
	, m_pxPerLine(320)
	, m_pxPitch(336)
	, m_pxStart(8)
	, m_numLines(240)
	, m_fb(NULL)
{
	reinitFb();
}

MdFb::~MdFb() {
	free(m_fb);
}

/**
 * Reinitialize the framebuffer.
 */
void MdFb::reinitFb(void) {
	// Free and reallocate the framebuffer.
	free(m_fb);
	m_fb_sz = m_pxPitch * m_numLines * sizeof(uint32_t);
	m_fb = malloc(m_fb_sz);

	// Initialize the line number lookup table.
	m_lineNumTable.resize(m_numLines);
	for (int y = 0, px = 0; y < m_numLines; y++, px += m_pxPitch)
		m_lineNumTable[y] = px;
}

}
