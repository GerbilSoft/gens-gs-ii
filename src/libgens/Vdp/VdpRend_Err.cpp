/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_Err.cpp: VDP error message class.                               *
 *                                                                         *
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

#include "Vdp.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// VDP includes.
#include "Vdp.hpp"
#include "VdpPalette.hpp"
#include "VGA_charset.h"

// Private classes.
#include "Vdp_p.hpp"
#include "VdpRend_Err_p.hpp"

namespace LibGens {

/**
 * Initialize the VdpRend_Err_Private object.
 * @param q Vdp object that owns this VdpRend_Err_Private object.
 */
VdpRend_Err_Private::VdpRend_Err_Private(Vdp *q)
	: q(q)
	, curVdpMode(~0)
	, lastVdpMode(~0)
	, lastHPix(~0)
	, lastVPix(~0)
	, lastBpp(MdFb::BPP_32)
	, lastBorderColor(~0)
{ }

/**
 * 15-bit Color Bar colors.
 */
const uint16_t VdpRend_Err_Private::ColorBarsPalette_15[22] = {
	// Primary Color Bars.
	0x6318, 0x6300, 0x0318, 0x0300,	// Gray, Yellow, Cyan, Green
	0x6018, 0x6000, 0x0018,		// Magenta, Red, Blue
	
	// Secondary Color Bars.
	0x0018, 0x0C63, 0x6018, 0x0C63,	// Blue, NTSC Black, Magenta, NTSC Black
	0x0318, 0x0C63, 0x6318,		// Cyan, NTSC Black, Gray
	
	// Final Color Bars.
	0x0089, 0x7FFF, 0x180D, 0x0C63,	// -I, White, +Q, NTSC Black
	0x0421, 0x0C63, 0x1084, 0x0C63,	// -4, NTSC Black, +4, NTSC Black
};

/**
 * 16-bit Color Bar colors.
 */
const uint16_t VdpRend_Err_Private::ColorBarsPalette_16[22] = {
	// Primary Color Bars.
	0xC618, 0xC600, 0x0618, 0x600,	// Gray, Yellow, Cyan, Green
	0xC018, 0xC000, 0x0018,		// Magenta, Red, Blue
	
	// Secondary Color Bars.
	0x0018, 0x18C3, 0xC018, 0x18C3,	// Blue, NTSC Black, Magenta, NTSC Black
	0x0618, 0x18C3, 0xC618,		// Cyan, NTSC Black, Gray
	
	// Final Color Bars.
	0x0109, 0xFFFF, 0x300D, 0x18C3,	// -I, White, +Q, NTSC Black
	0x0841, 0x18C3, 0x2104, 0x18C3,	// -4, NTSC Black, +4, NTSC Black
};

/**
 * 32-bit Color Bar colors.
 */
const uint32_t VdpRend_Err_Private::ColorBarsPalette_32[22] = {
	// Primary Color Bars.
	0xC0C0C0, 0xC0C000, 0x00C0C0, 0x00C000,	// Gray, Yellow, Cyan, Green
	0xC000C0, 0xC00000, 0x0000C0,		// Magenta, Red, Blue
	
	// Secondary Color Bars.
	0x0000C0, 0x131313, 0xC000C0, 0x131313,	// Blue, NTSC Black, Magenta, NTSC Black
	0x00C0C0, 0x131313, 0xC0C0C0,		// Cyan, NTSC Black, Gray
	
	// Final Color Bars.
	0x00214C, 0xFFFFFF, 0x32006A, 0x131313,	// -I, White, +Q, NTSC Black
	0x090909, 0x131313, 0x1D1D1D, 0x131313,	// -4, NTSC Black, +4, NTSC Black
};

/** Templated color bar functions. **/

/**
 * Draw color bars.
 * @param pixel Pixel type.
 * @param fb MdFb pointer.
 * @param palette Color bar palette.
 */
template<typename pixel>
inline void VdpRend_Err_Private::T_DrawColorBars(MdFb *fb, const pixel palette[22])
{
	// Go to the correct position in the screen.
	// TODO: Update to use MdFb.
	pixel *screen = fb->lineBuf<pixel>(q->VDP_Lines.Border.borderSize);
	const int HPix = q->getHPix();			// Get horizontal pixel count.
	const int pitch_diff = (fb->pxPitch() - HPix);	// Calculate pitch difference.

	// X bar positions.
	int barX_1[7];
	for (int i = 1; i <= 7; i++) {
		barX_1[i-1] = ((HPix * i) / 7);
	}
	barX_1[6] = 999;

	int barX_2[8];
	for (int i = 1; i <= 4; i++) {
		barX_2[i-1] = ((HPix * i * 120) / 672);
	}
	for (int i = 1; i <= 3; i++) {
		barX_2[i+3] = barX_2[3] + ((HPix * i * 32) / 672);
	}
	barX_2[7] = 999;

	// Y bar positions.
	const int barY_1 = ((q->VDP_Lines.totalVisibleLines * 2) / 3);
	const int barY_2 = (barY_1 + (q->VDP_Lines.totalVisibleLines / 12));

	// Adjust the screen position for the horizontal resolution.
	screen += ((320 - HPix) / 2);

	// Current color.
	int color;

	for (int y = 0; y < q->VDP_Lines.totalVisibleLines; y++) {
		color = 0;

		if (y < barY_1) {
			// Primary bars.
			for (int x = 0; x < HPix; x++) {
				if (x >= barX_1[color])
					color++;

				// Draw the color.
				*screen++ = palette[color];
			}
		} else if (y < barY_2) {
			// Secondary bars.
			for (int x = 0; x < HPix; x++) {
				if (x >= barX_1[color])
					color++;

				// Draw the color.
				*screen++ = palette[color+7];
			}
		} else {
			// Final bars.
			for (int x = 0; x < HPix; x++) {
				if (x >= barX_2[color])
					color++;

				// Draw the color.
				*screen++ = palette[color+14];
			}
		}

		// Next row.
		screen += pitch_diff;
	}
}

/**
 * Draw the border area for the color bars.
 * @param pixel Pixel type.
 * @param fb MdFb pointer.
 * @param bg_color Background color.
 */
template<typename pixel>
inline void VdpRend_Err_Private::T_DrawColorBars_Border(MdFb *fb, const pixel bg_color)
{
	// Draw the top border.
	pixel *screen = fb->lineBuf<pixel>(0);
	for (unsigned int i = (q->VDP_Lines.Border.borderSize * fb->pxPitch());
	     i != 0; i -= 4, screen += 4)
	{
		*screen = bg_color;
		*(screen + 1) = bg_color;
		*(screen + 2) = bg_color;
		*(screen + 3) = bg_color;
	}

	const int HPix = q->getHPix();
	if (HPix < 320) {
		// Draw the left and right borders.
		const int HPixBegin = q->getHPixBegin();

		for (int y = q->VDP_Lines.totalVisibleLines; y != 0; y--) {
			// Left border.
			for (int x = HPixBegin; x != 0; x -= 4, screen += 4) {
				*screen = bg_color;
				*(screen + 1) = bg_color;
				*(screen + 2) = bg_color;
				*(screen + 3) = bg_color;
			}

			// Skip the visible area.
			screen += HPix;

			// Right border.
			for (int x = HPixBegin; x != 0; x -= 4, screen += 4) {
				*screen = bg_color;
				*(screen + 1) = bg_color;
				*(screen + 2) = bg_color;
				*(screen + 3) = bg_color;
			}

			// Next line.
			screen += 16;
		}

		// Go back to the (invisible) start of the line.
		screen -= 8;
	} else {
		// Go to the bottom border.
		// TODO: Update to use MdFb.
		screen += (q->VDP_Lines.totalVisibleLines * fb->pxPitch());
	}

	// Draw the bottom border.
	// TODO: Update to use MdFb.
	for (unsigned int i = (q->VDP_Lines.Border.borderSize * fb->pxPitch());
	     i != 0; i -= 4, screen += 4)
	{
		*screen = bg_color;
		*(screen + 1) = bg_color;
		*(screen + 2) = bg_color;
		*(screen + 3) = bg_color;
	}
}

/**
 * Draw a character.
 * @param pixel Pixel type.
 * @param text_color Text color.
 * @param screen Screen pointer.
 * @param chr Character.
 */
template<typename pixel, pixel text_color>
inline void VdpRend_Err_Private::T_DrawChr(pixel *screen, int chr)
{
	// TODO: Use MdFb to get pixels per line instead of hard-coding 336.
	if (chr < 0x20)
		return;
	chr = (chr & 0x7F) - 0x20;

	// Draw the shadowed character first.
	pixel *scr_ptr = (screen + 1 + 336);
	for (unsigned int row = 0; row < 16; row++) {
		unsigned int chr_data = VGA_charset_ASCII[chr][row];
		if (chr_data == 0) {
			// Empty line.
			scr_ptr += 336;
			continue;
		}

		for (unsigned int col = 8; col != 0; col--, scr_ptr++)
		{
			if (chr_data & 0x80)
				*scr_ptr = 0;
			chr_data <<= 1;
		}

		// Next line.
		scr_ptr += (336 - 8);
	}

	// Draw the normal character.
	scr_ptr = screen;
	for (unsigned int row = 0; row < 16; row++) {
		unsigned int chr_data = VGA_charset_ASCII[chr][row];
		if (chr_data == 0){
			// Empty line.
			scr_ptr += 336;
			continue;
		}

		for (unsigned int col = 8; col != 0; col--, scr_ptr++) {
			if (chr_data & 0x80)
				*scr_ptr = text_color;
			chr_data <<= 1;
		}

		// Next line.
		scr_ptr += (336 - 8);
	}
}

/**
 * Draw text. (ASCII only!)
 * @param pixel Pixel type.
 * @param text_color Text color.
 * @param fb MdFb pointer.
 * @param x X coordinate in the screen.
 * @param y Y coordinate in the screen.
 * @param str String. (ASCII only!)
 */
template<typename pixel, pixel text_color>
inline void VdpRend_Err_Private::T_DrawText(MdFb *fb, int x, int y, const char *str)
{
	pixel *screen = fb->lineBuf<pixel>(y) + x;

	for (; *str != 0x00; screen += 8, str++) {
		T_DrawChr<pixel, text_color>(screen, *str);
	}
}

/**
 * Draw the VDP error message..
 * @param pixel Pixel type.
 * @param text_color Text color.
 * @param fb MdFb pointer.
 */
template<typename pixel, pixel text_color>
inline void VdpRend_Err_Private::T_DrawVDPErrorMessage(MdFb *fb)
{
	// Determine the starting position.
	const int barY_1 = ((q->VDP_Lines.totalVisibleLines * 2) / 3);
	int y = q->VDP_Lines.Border.borderSize + ((barY_1 - (16*5)) / 2);
	int x = ((q->getHPix() - (29*8)) / 2) + q->getHPixBegin();

	T_DrawText<pixel, text_color>(fb, x, y,    "Gens/GS II does not currently");
	T_DrawText<pixel, text_color>(fb, x+((8*7)/2), y+16, "support this VDP mode.");
	y += 48;

	// Mode bits.
	char buf[32];
	const uint8_t vdpMode = curVdpMode;
	snprintf(buf, sizeof(buf), "Mode Bits: %d%d%d%d%d",
			(vdpMode & VdpTypes::VDP_MODE_M5) >> 4,
			(vdpMode & VdpTypes::VDP_MODE_M4) >> 3,
			(vdpMode & VdpTypes::VDP_MODE_M3) >> 2,
			(vdpMode & VdpTypes::VDP_MODE_M2) >> 1,
			(vdpMode & VdpTypes::VDP_MODE_M1));
	x = ((q->getHPix() - (16*8)) / 2) + q->getHPixBegin();
	T_DrawText<pixel, text_color>(fb, x, y, buf);

	// TMS9918 modes.
	static const char *const tms9918_modes[8] = {
		"0 (Graphic I)",
		"1 (Text)",
		"2 (Graphic II)",
		"1+2",
		"3 (Multicolor)",
		"1+3",
		"2+3",
		"1+2+3"
	};
	const char *cur_mode;

	if (vdpMode & VdpTypes::VDP_MODE_M5) {
		// Mode 5. (SHOULDN'T HAPPEN!)
		cur_mode = "5 (Mega Drive)";
	} else if (vdpMode & VdpTypes::VDP_MODE_M4) {
		// Mode 4.
		cur_mode = "4 (SMS/GG)";
	} else {
		// TMS9918 mode.
		cur_mode = tms9918_modes[vdpMode & 0x07];
	}

	// Determine the horizontal starting position.
	// TODO: Don't use strlen().
	x = ((q->getHPix() - ((10+strlen(cur_mode))*8)) / 2) + q->getHPixBegin();
	T_DrawText<pixel, text_color>(fb, x, y+16, "VDP Mode:");
	T_DrawText<pixel, text_color>(fb, x+(8*10), y+16, cur_mode);
}

/** Vdp class functions. **/

/**
 * Draw a render error message.
 */
void VdpPrivate::renderLine_Err(void)
{
	bool updateBorders = false;

	// Store the current VDP mode.
	d_err->curVdpMode = VDP_Mode;

	if (d_err->lastVdpMode != d_err->curVdpMode ||
	    d_err->lastHPix != q->getHPix() ||
	    d_err->lastVPix != q->getVPix() ||
	    d_err->lastBpp != palette.bpp())
	{
		// VDP mode has changed.
		// Redraw the color bars and reprint the error message.
		switch (palette.bpp()) {
			case MdFb::BPP_15:
				d_err->T_DrawColorBars<uint16_t>(q->MD_Screen, VdpRend_Err_Private::ColorBarsPalette_15);
				d_err->T_DrawVDPErrorMessage<uint16_t, 0x7FFF>(q->MD_Screen);
				break;

			case MdFb::BPP_16:
				d_err->T_DrawColorBars<uint16_t>(q->MD_Screen, VdpRend_Err_Private::ColorBarsPalette_16);
				d_err->T_DrawVDPErrorMessage<uint16_t, 0xFFFF>(q->MD_Screen);
				break;

			case MdFb::BPP_32:
			default:
				d_err->T_DrawColorBars<uint32_t>(q->MD_Screen, VdpRend_Err_Private::ColorBarsPalette_32);
				d_err->T_DrawVDPErrorMessage<uint32_t, 0xFFFFFF>(q->MD_Screen);
				break;
		}

		// Update the borders.
		updateBorders = true;
	}

	// Update the palette.
	palette.update();

	// Get the current border color.
	uint32_t newBorderColor;
	if (palette.bpp() != MdFb::BPP_32)
		newBorderColor = (uint32_t)palette.m_palActive.u16[0];
	else
		newBorderColor = palette.m_palActive.u32[0];

	// Check if we need to update the borders.
	if (updateBorders || newBorderColor != d_err->lastBorderColor) {
		// TODO: Check for horizontal borders too.
		if (q->VDP_Lines.Border.borderSize != 0) {
			// Update the color bar borders.
			if (palette.bpp() != MdFb::BPP_32)
				d_err->T_DrawColorBars_Border<uint16_t>(q->MD_Screen, (uint16_t)newBorderColor);
			else
				d_err->T_DrawColorBars_Border<uint32_t>(q->MD_Screen, newBorderColor);
		}

		// Save the new border color.
		d_err->lastBorderColor = newBorderColor;
	}
}

/**
 * Update the VDP Render Error cache.
 */
void VdpPrivate::updateErr(void)
{
	// Save the VDP mode.
	d_err->lastVdpMode = VDP_Mode;
	d_err->lastHPix = q->getHPix();
	d_err->lastVPix = q->getVPix();
	d_err->lastBpp = palette.bpp();

	// Check border color.
	if (palette.bpp() != MdFb::BPP_32)
		d_err->lastBorderColor = (uint32_t)palette.m_palActive.u16[0];
	else
		d_err->lastBorderColor = palette.m_palActive.u32[0];
}

}
