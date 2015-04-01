/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_m4.cpp: VDP Mode 4 rendering code. (Part of the Vdp class.)     *
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
#include "VdpTypes.hpp"

// Vdp private class.
#include "Vdp_p.hpp"

namespace LibGens {

/**
 * Update the Sprite Line Cache for the next line.
 * TODO: This function needs testing.
 * @param line Current line number.
 * @return VdpStatus::VDP_STATUS_SOVR if sprite limit is exceeded; otherwise, 0.
 */
FORCE_INLINE unsigned int VdpPrivate::Update_Sprite_Line_Cache_m4(int line)
{
	unsigned int ret = 0;

	// Determine the maximum number of sprites.
	uint8_t max_spr_line;
	if (q->options.spriteLimits) {
		// Sprite limits are enabled:
		// - Max sprites per line:  8
		// - Max sprites per frame: 64
		max_spr_line = 8;
	} else {
		// Sprite limits are disabled.
		max_spr_line = 64;
	}

	// We're updating the cache for the *next* line.
	// However, the sprite table's Y values indicate *this* line.
	int cacheId = (line & 1);
	SprLineCache_t *cache = &sprLineCache[cacheId][0];
	uint8_t count = 0;

	const int screen_h = 192;	// TODO: 224, 240?
	// Sprite height. (8x8 or 8x16, depending on reg1)
	int sprite_h = 8 + ((VDP_Reg.m4.Set2 & 0x02) << 2);
	// Sprite zoom flag. (TODO: Ignore on MD.)
	uint8_t sprite_zoom = (VDP_Reg.m4.Set2 & 0x01);

	const uint8_t *spr_VRam = &VRam.u8[Spr_Tbl_Addr];
	int i = 0;
	do {
		/**
		 * Sprite entries aren't contiguous in Mode 4.
		 * Format:
		 * 00: yyyyyyyyyyyyyyyy
		 * 10: yyyyyyyyyyyyyyyy
		 * 20: yyyyyyyyyyyyyyyy
		 * 30: yyyyyyyyyyyyyyyy
		 * 40: ????????????????
		 * 50: ????????????????
		 * 60: ????????????????
		 * 70: ????????????????
		 * 80: xnxnxnxnxnxnxnxn
		 * 90: xnxnxnxnxnxnxnxn
		 * A0: xnxnxnxnxnxnxnxn
		 * B0: xnxnxnxnxnxnxnxn
		 * C0: xnxnxnxnxnxnxnxn
		 * D0: xnxnxnxnxnxnxnxn
		 * E0: xnxnxnxnxnxnxnxn
		 * F0: xnxnxnxnxnxnxnxn
		 */
		int y = spr_VRam[i ^ U16DATA_U8_INVERT];
		if (y == screen_h+16) {
			// End of sprite list.
			// Y = $D0 (192-line)
			// Y = $F0 (224-line)
			break;
		}

		if (y >= (256-16)) {
			// Wrap around to the top of the screen.
			y -= 256;
		}

		// Determine y_max.
		const int y_max = y + (sprite_h << sprite_zoom) - 1;
		if (line >= y && line <= y_max) {
			// Sprite is in range.
			if (count == max_spr_line) {
				// Sprite overflow!
				// NOTE: Flag is only set in the active area.
				if (line >= 0 && line < screen_h) {
					ret = VdpStatus::VDP_STATUS_SOVR;
				}
				break;
			}

			// Save the sprite information in the line cache.
			// TODO: SMS1 sprite table mask.
			// NOTE: Size_X, Size_Y are not used in Mode 4,
			// since all sprites are the same size.
			const uint8_t sms1_spr_tbl_mask = 0xFF;
			cache->Pos_Y     = y;
			cache->Pos_Y_Max = y_max;
			cache->Pos_X     = spr_VRam[((0x80 + (i << 1)) & sms1_spr_tbl_mask) ^ U16DATA_U8_INVERT];
			cache->Num_Tile  = spr_VRam[((0x81 + (i << 1)) & sms1_spr_tbl_mask) ^ U16DATA_U8_INVERT];

			// Added a sprite.
			count++;
			cache++;
			if (count == 4) {
				// TODO: On SMS2 and GG, allow all sprites to be zoomed.
				sprite_zoom = 0;
			}
		}
	} while (++i < 64);

	// Save the sprite count for the next line.
	sprCountCache[cacheId] = count;

	// Return the SOVR flag.
	return ret;
}

}
