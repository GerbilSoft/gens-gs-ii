/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpCache.cpp: VDP pattern cache.                                        *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "VdpCache.hpp"
#include "libcompat/byteorder.h"

#include "VdpTypes.hpp"
using namespace LibGens::VdpTypes;

// C includes. (C++ namespace)
#include <cstring>

namespace LibGens {

VdpCache::VdpCache()
	: dirty_idx(0)
{
	// Clear the pattern cache and dirty flags.
	clear();

	// Initialize the Mode 4 lookup table.
	init_m4_lut();
}

/**
 * Initialize the Mode 4 lookup table.
 */
void VdpCache::init_m4_lut(void)
{
	/**
	 * Sega Master System Mode 4 uses a planar color encoding.
	 * For a given line of pattern data (four bytes), Bit 7 in each
	 * byte represents pixel 0, Bit 6 represents pixel 1, etc.
	 *
	 * Example:
	 * - M4 pattern: 12 34 56 78
	 * - In binary: 00010010 00110100 01010110 01111000
	 * - Bit 7 of each byte: 0000
	 * - Bit 6 of each byte: 0011
	 * - Bit 5 of each byte: 0101
	 * - Bit 4 of each byte: 1111
	 * - Bit 3 of each byte: 0000
	 * - Bit 2 of each byte: 0110
	 * - Bit 1 of each byte: 1010
	 * - Bit 0 of each byte: 0000
	 *
	 * Pack back into bytes: 00000011 01011111 00000110 10100000
	 * M5 pattern: 03 5F 06 A0
	 *
	 * In order to reduce memory usage, a 64K lookup table is used.
	 * Two bytes from the pattern line are checked at a time, and then
	 * they're shifted into place.
	 */
	for (unsigned int lut_idx = 0; lut_idx < 0x10000; lut_idx++) {
		// Convert the planar data to packed.
		// index: abcd efgh abcd efgh
		// value: aa00 bb00 cc00 dd00 ee00 ff00 gg00 hh00
		uint32_t lut_entry = 0;
		for (int x = 7; x >= 0; x--) {
			lut_entry <<= 4;
			if (lut_idx & (1 << (x+8))) {
				// Bitplane 0.
				lut_entry |= 0x0008;
			}
			if (lut_idx & (1 << x)) {
				// Bitplane 1.
				lut_entry |= 0x0004;
			}
		}
		m4_lut[lut_idx] = lut_entry;
	}
}

/**
 * Clear the entire pattern cache.
 * This is used when resetting the VDP.
 */
void VdpCache::clear(void)
{
	// Reset the dirty flags.
	dirty_idx = 0;
	memset(dirty_flags, 0, sizeof(dirty_flags));

	// Clear the cache.
	memset(&cache, 0, sizeof(cache));
}

/**
 * Invalidate the entire pattern cache.
 * This needs to be done when switching modes
 * and/or loading a savestate.
 */
void VdpCache::invalidate(void)
{
	// TODO: Only update patterns 0-511 in TMS/SMS modes.
	dirty_idx = 2048;
	for (int i = dirty_idx-1; i >= 0; i--) {
		dirty_flags[i] = 0xFF;
		dirty_list[i] = i;
	}
}

/**
 * Convert a Mode 4 pattern to Mode 5.
 * @param w1 Word 1.
 * @param w2 Word 2.
 * @return Mode 5 pattern.
 */
inline uint32_t VdpCache::m4_lookup(uint16_t w1, uint16_t w2) const
{
	// TODO: Make sure this function actually does get inlined.
	return (m4_lut[w1] >> 2) | m4_lut[w2];
}

/**
 * H-flip a pattern.
 * @param src Source pattern.
 * @return H-flipped pattern.
 */
inline uint32_t VdpCache::H_flip(uint32_t src)
{
	// References:
	// - http://stackoverflow.com/questions/8484188/how-to-swap-nybbles-in-c
	// - http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel

	// First, swap the nybbles.
	src = ((src >> 4) & 0x0F0F0F0F) | ((src & 0x0F0F0F0F) << 4);
	// Swap the bytes.
	src = ((src >> 8) & 0x00FF00FF) | ((src & 0x00FF00FF) << 8);
	// Rotate the words into place.
	src = (src << 16) | (src >> 16);

	// ...and we're done.
	return src;
}

/**
 * Update the pattern cache. (Mode 4)
 * @param vram VRAM source data.
 */
void VdpCache::update_m4(const VRam_t *vram)
{
	for (int i = dirty_idx-1; i >= 0; i--) {
		// Get the tile VRAM address.
		uint16_t tile = dirty_list[i];
		uint8_t flag = dirty_flags[tile];
		const uint16_t *vram_src = &vram->u16[tile * 16];
		for (int y = 7; y >= 0; y--) {
			// Check if this line is dirty.
			if (flag & (1 << y)) {
				// Line is dirty.
				// TODO: Combine with update_m5, since this function is
				// nearly identical except for the pattern retrieval code?
				uint32_t src = m4_lookup(vram_src[y*2], vram_src[y*2+1]);

				// Update the normal cache.
				cache.x8[0][tile][y] = src;

				// Update the H-flip cache.
				src = H_flip(src);
				cache.x8[1][tile][y] = src;
			}
		}

		// Clear the dirty flag.
		dirty_flags[tile] = 0;
	}

	// Finished updating tiles.
	dirty_idx = 0;
}

/**
 * Update the pattern cache. (Mode 5)
 * @param vram VRAM source data.
 */
void VdpCache::update_m5(const VRam_t *vram)
{
	for (int i = dirty_idx-1; i >= 0; i--) {
		// Get the tile VRAM address.
		// Note that we're assuming 8x8 tiles.
		// Interlaced mode is handled on cache lookup.
		uint16_t tile = dirty_list[i];
		uint8_t flag = dirty_flags[tile];
		const uint32_t *vram_src = &vram->u32[tile * 8];
		for (int y = 7; y >= 0; y--) {
			// Check if this line is dirty.
			if (flag & (1 << y)) {
				// Line is dirty.
				// TODO: Combine with update_m4, since this function is
				// nearly identical except for the pattern retrieval code?
				uint32_t src = vram_src[y];
#if SYS_BYTEORDER == SYS_LIL_ENDIAN
				// Rotate the source data into the correct order.
				src = (src << 16) | (src >> 16);
#endif

				// Update the normal cache.
				cache.x8[0][tile][y] = src;

				// Update the H-flip cache.
				src = H_flip(src);
				cache.x8[1][tile][y] = src;
			}
		}

		// Clear the dirty flag.
		dirty_flags[tile] = 0;
	}

	// Finished updating tiles.
	dirty_idx = 0;
}

}
