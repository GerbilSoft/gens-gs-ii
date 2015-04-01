/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_tms.cpp: VDP Mode 0-3 rendering code. (Part of the Vdp class.)  *
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

#include "Vdp.hpp"
#include "VdpStructs.hpp"
using LibGens::VdpStructs::SprEntry_tms;

// Vdp private class.
#include "Vdp_p.hpp"

namespace LibGens {

/**
 * Update the Sprite Line Cache for the next line.
 * TODO: This function needs testing.
 * @param line Current line number.
 * @return VdpStatus::VDP_STATUS_SOVR and 5th Sprite number if sprite limit is exceeded; otherwise, 0.
 */
FORCE_INLINE unsigned int VdpPrivate::Update_Sprite_Line_Cache_tms(int line)
{
	unsigned int ret = 0;

	// Determine the maximum number of sprites.
	uint8_t max_spr_line;
	if (q->options.spriteLimits) {
		// Sprite limits are enabled:
		// - Max sprites per line:  4
		// - Max sprites per frame: 32
		max_spr_line = 4;
	} else {
		// Sprite limits are disabled.
		max_spr_line = 32;
	}

	// We're updating the cache for the *next* line.
	// However, the sprite table's Y values indicate *this* line.
	int cacheId = (line & 1);
	SprLineCache_t *cache = &sprLineCache[cacheId][0];
	uint8_t count = 0;

	// Sprite height. (8x8 or 16x16, depending on reg1)
	int sprite_h = 8 + ((VDP_Reg.m4.Set2 & 0x02) << 2);
	// Sprite zoom flag. (If set, sprites become 16x16 or 32x32.)
	sprite_h <<= (VDP_Reg.m4.Set2 & 0x01);

	const SprEntry_tms *spr_VRam = (const SprEntry_tms*)&VRam.u8[Spr_Tbl_Addr];
	int i = 0;	// Needed for Fifth Sprite field.
	do {
		int y = spr_VRam->y;
		if (y == 0xD0) {
			// End of sprite list.
			// Y = $D0
			break;
		}
		if (y >= (256-32)) {
			// Wrap around to the top of the screen.
			y -= 256;
		}

		// Determine y_max.
		const int y_max = y + sprite_h - 1;
		if (line >= y && line <= y_max) {
			// Sprite is in range.
			if (count == max_spr_line) {
				// Sprite overflow!
				// NOTE: Flag is only set in the active area.
				if (line >= 0 && line < 192) {
					ret = VdpStatus::VDP_STATUS_SOVR;
				}
				break;
			}

			// Save the sprite information in the line cache.
			// NOTE: Size_X, Size_Y are not used in TMS modes,
			// since all sprites are the same size.
			cache->Pos_Y     = y;
			cache->Pos_Y_Max = y_max;
			cache->Pos_X     = spr_VRam->x - ((spr_VRam->color_ec & 0x80) >> 2);
			cache->sprite    = spr_VRam->sprite;
			cache->color     = spr_VRam->color_ec & 0x0F;

			// Added a sprite.
			count++;
			cache++;
		}

		// Next sprite.
		spr_VRam++;
	} while (++i < 32);

	// Save the sprite count for the next line.
	sprCountCache[cacheId] = count;

	// Add the last sprite number processed to the flags.
	// NOTE: If all 32 sprites were processed, i == 32;
	// it should be 31.
	ret |= ((i < 32) ? (i & 0x1F) : 0x1F);

	// Return the SOVR flag.
	return ret;
}

}
