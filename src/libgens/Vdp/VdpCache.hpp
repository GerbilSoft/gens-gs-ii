/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpCache.hpp: VDP pattern cache.                                        *
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

// For performance reasons, VRAM patterns are cached for Modes 4 and 5
// with the various 'flip' options, e.g. Hflip, Vflip, and Hflip+Vflip.
// In addition, a lookup table is used to convert Mode 4 planar patterns
// to Mode 5 packed patterns.

#ifndef __LIBGENS_MD_VDPCACHE_HPP__
#define __LIBGENS_MD_VDPCACHE_HPP__

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cassert>

namespace LibGens {

namespace VdpTypes {
	union VRam_t;
}

class VdpCache {
	// NOTE: This class isn't currently in use.
	private:
		VdpCache();
		~VdpCache();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		VdpCache(const VdpCache &);
		VdpCache &operator=(const VdpCache &);

	public:
		/**
		 * Invalidate the pattern cache.
		 * This needs to be done when switching modes
		 * and/or loading a savestate.
		 */
		void invalidate(void);

		/**
		 * Update the pattern cache. (Mode 4)
		 * @param vram VRAM source data.
		 */
		void update_m4(const VdpTypes::VRam_t *vram);

		/**
		 * Update the pattern cache. (Mode 5)
		 * @param vram VRAM source data.
		 */
		void update_m5(const VdpTypes::VRam_t *vram);

		/**
		 * Get a pattern line. (Mode 4, nametable, 8x8 cell)
		 * @param attr Nametable attribute word.
		 * @param y Y offset.
		 */
		inline uint32_t pattern_line_m4_nt_8x8(uint16_t attr, int y);

		/**
		 * Get a pattern line. (Mode 4, sprite, 8x8 cell)
		 * @param tile Sprite tile number.
		 * @param y Y offset.
		 */
		inline uint32_t pattern_line_m4_spr_8x8(uint8_t tile, int y);

		/**
		 * Get a pattern line. (Mode 5, nametable, 8x8 cell)
		 * @param attr Nametable attribute word.
		 * @param y Y offset.
		 */
		inline uint32_t pattern_line_m5_nt_8x8(uint16_t attr, int y);

		/**
		 * Get a pattern line. (Mode 5, sprite, 8x8 cell)
		 * @param attr Sprite table attribute word.
		 * @param y Y offset.
		 */
		inline uint32_t pattern_line_m5_spr_8x8(uint16_t attr, int y);

		/**
		 * Get a pattern line. (Mode 5, nametable, 8x16 cell)
		 * @param attr Nametable attribute word.
		 * @param y Y offset.
		 */
		inline uint32_t pattern_line_m5_nt_8x16(uint16_t attr, int y);

		/**
		 * Get a pattern line. (Mode 5, sprite, 8x16 cell)
		 * @param attr Sprite table attribute word.
		 * @param y Y offset.
		 */
		inline uint32_t pattern_line_m5_spr_8x16(uint16_t attr, int y);

	protected:
		/**
		 * Mode 4 lookup table.
		 *
		 * Each pattern line consists of four bytes.
		 * Each byte represents one bit in the bitplane.
		 *
		 * By looking up two bytes at a time, we can convert
		 * two bitplanes into a partial packed pixel.
		 */
		uint32_t m4_lut[65536];

		/**
		 * Initialize the Mode 4 lookup table.
		 */
		void init_m4_lut(void);

		/**
		 * Convert a Mode 4 pattern to Mode 5.
		 * @param w1 Word 1.
		 * @param w2 Word 2.
		 * @return Mode 5 pattern.
		 */
		inline uint32_t m4_lookup(uint16_t w1, uint16_t w2) const;

		/**
		 * H-flip a pattern.
		 * @param src Source pattern.
		 * @return H-flipped pattern.
		 */
		static inline uint32_t H_flip(uint32_t src);

		/**
		 * Pattern cache for Mode 4 and Mode 5.
		 * Internal data is packed Mode 5 format.
		 *
		 * TODO: Store the data "reversed"?
		 * i.e. left pixel is low nybble for no-flip
		 */
		union {
			/**
			 * Line access. (8x8 cell)
			 * Indexes:
			 * - idx0: H flip. (0=normal, 1=H)
			 * - idx1: Tile number.
			 * - idx2: Line number.
			 */
			uint32_t x8[2][2048][8];

			/**
			 * DWORD access.
			 * Indexes:
			 * - idx0: H flip. (0=normal, 1=H)
			 * - idx1: VRAM address, divided by 4.
			 */
			uint32_t d[2][16384];
		} cache;

		/**
		 * Cache dirty flags.
		 * Index: 8x8 tile number.
		 * Value: Bitfield indicating which lines are dirty.
		 */
		uint8_t dirty_flags[2048];

		/**
		 * Cache dirty list.
		 * Value indicates a dirty tile number.
		 */
		uint16_t dirty_list[2048];
		unsigned int dirty_idx;
};

/**
 * Get a pattern line. (Mode 4, nametable, 8x8 cell)
 * @param attr Nametable attribute word.
 * @param y Y offset.
 */
inline uint32_t VdpCache::pattern_line_m4_nt_8x8(uint16_t attr, int y)
{
	assert(y >= 0 && y <= 7);
	if (attr & 0x400) {
		// V-flip.
		y ^= 7;
	}
	// FIXME: Verify the assembly output. If it isn't that good,
	// use a struct without a separate HV dimension.
	return cache.x8[(attr >> 9) & 1][attr & 0x1FF][y];
}

/**
 * Get a pattern line. (Mode 4, sprite, 8x8 cell)
 * @param tile Sprite tile number.
 * @param y Y offset.
 */
inline uint32_t VdpCache::pattern_line_m4_spr_8x8(uint8_t tile, int y)
{
	assert(y >= 0 && y <= 7);
	// TODO: Sprite pattern generator base address.
	// FIXME: Verify the assembly output. If it isn't that good,
	// use a struct without a separate HV dimension.
	return cache.x8[0][tile][y];
}

/**
 * Get a pattern line. (Mode 5, nametable, 8x8 cell)
 * @param attr Nametable attribute word.
 * @param y Y offset.
 */
inline uint32_t VdpCache::pattern_line_m5_nt_8x8(uint16_t attr, int y)
{
	assert(y >= 0 && y <= 7);
	if (attr & 0x1000) {
		// V-flip.
		y ^= 7;
	}
	// TODO: 128 KB support.
	// FIXME: Verify the assembly output. If it isn't that good,
	// use a struct without a separate HV dimension.
	return cache.x8[(attr >> 11) & 1][attr & 0x7FF][y];
}

/**
 * Get a pattern line. (Mode 5, sprite, 8x8 cell)
 * @param attr Sprite table attribute word.
 * @param y Y offset.
 */
inline uint32_t VdpCache::pattern_line_m5_spr_8x8(uint16_t attr, int y)
{
	assert(y >= 0 && y <= 7);
	if (attr & 0x1000) {
		// V-flip.
		y ^= 7;
	}
	// TODO: 128 KB support.
	// FIXME: Verify the assembly output. If it isn't that good,
	// use a struct without a separate HV dimension.
	return cache.x8[(attr >> 11) & 1][attr & 0x7FF][y];
}

/**
 * Get a pattern line. (Mode 5, nametable, 8x16 cell)
 * @param attr Nametable attribute word.
 * @param y Y offset.
 */
inline uint32_t VdpCache::pattern_line_m5_nt_8x16(uint16_t attr, int y)
{
	assert(y >= 0 && y <= 15);
	// TODO: 128 KB support.

	// Interlaced Mode: Convert to non-interlaced tile number.
	// FIXME: Verify vflip behavior.
	uint16_t tile = (attr & 0x3FF) << 1 | (y >> 3);
	if (attr & 0x1000) {
		// V-flip.
		y ^= 15;
		tile ^= 1;
	}

	// FIXME: Verify the assembly output. If it isn't that good,
	// use a struct without a separate HV dimension.
	return cache.x8[(attr >> 11) & 1][tile][y & 7];
}

/**
 * Get a pattern line. (Mode 5, sprite, 8x16 cell)
 * @param attr Sprite table attribute word.
 * @param y Y offset.
 */
inline uint32_t VdpCache::pattern_line_m5_spr_8x16(uint16_t attr, int y)
{
	assert(y >= 0 && y <= 15);
	// TODO: 128 KB support.

	// Interlaced Mode: Convert to non-interlaced tile number.
	// FIXME: Verify vflip behavior.
	uint16_t tile = (attr & 0x3FF) << 1 | (y >> 3);
	if (attr & 0x1000) {
		// V-flip.
		y ^= 15;
		tile ^= 1;
	}

	// FIXME: Verify the assembly output. If it isn't that good,
	// use a struct without a separate HV dimension.
	return cache.x8[(attr >> 11) & 1][tile][y & 7];
}

}

#endif /* __LIBGENS_MD_VDPCACHE_HPP__ */
