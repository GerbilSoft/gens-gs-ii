/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_VdpPalette_generate.h: VdpPalette tests. (Data file generator)     *
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

#include "test_VdpPalette.h"

// C includes.
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

// TODO: Use ColorScaleMethod enum from LibGens.
typedef enum
{
	CSM_RAW = 0,
	CSM_FULL = 1,
	CSM_FULL_SH = 2,
} ColorScaleMethod;

// TODO: Combine these functios together.

static int write_paltype_md(const char *filename, ColorScaleMethod csm)
{
	static const uint8_t PalComponent_MD_Raw[16] =
		{0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		 0xE0, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0};
	static const uint8_t PalComponent_MD_Full[16] =
		{  0,  18,  36,  54,  72,  91, 109, 127,
		 145, 163, 182, 200, 218, 236, 255, 255};
	static const uint8_t PalComponent_MD_Full_SH[16] =
		{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	
	FILE *f = fopen(filename, "w");
	if (!f)
	{
		fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
		return -1;
	}
	
	// Write the file header.
	fprintf(f, "%s:%04X\n", PALTEST_MAGIC, PALTEST_VERSION);
	fprintf(f, "PalMode:%s\n", PALTEST_PALMODE_MD);
	
	// Color Scale Method.
	const uint8_t *palcomponent_md;
	const char *palTest_ColorScale;
	switch (csm)
	{
		case CSM_RAW:
			palcomponent_md = PalComponent_MD_Raw;
			palTest_ColorScale = PALTEST_COLORSCALE_RAW;
			break;
		case CSM_FULL:
		default:
			palcomponent_md = PalComponent_MD_Full;
			palTest_ColorScale = PALTEST_COLORSCALE_FULL;
			break;
		case CSM_FULL_SH:
			palcomponent_md = PalComponent_MD_Full_SH;
			palTest_ColorScale = PALTEST_COLORSCALE_FULL_SH;
			break;
	}
	fprintf(f, "ColorScale:%s\n", palTest_ColorScale);
	
	// Write the normal palette.
	fprintf(f, "\n");
	fprintf(f, "SHMode:%s\n", PALTEST_SHMODE_NORMAL);
	for (unsigned i = 0; i < 512; i++)
	{
		// Get the MD color components.
		const uint8_t r = ((i << 1) & 0x00E);
		const uint8_t g = ((i >> 2) & 0x00E);
		const uint8_t b = ((i >> 5) & 0x00E);
		const uint16_t color_md = ((b << 8) | (g << 4) | r);
		
		// Extract color components for the normal color.
		const uint8_t r_md = palcomponent_md[r];
		const uint8_t g_md = palcomponent_md[g];
		const uint8_t b_md = palcomponent_md[b];
		
		// Calculate the scaled RGB colors.
		const uint16_t rgb555 = ((r_md & 0xF8) << 7) | ((g_md & 0xF8) << 2) | (b_md >> 3);
		const uint16_t rgb565 = ((r_md & 0xF8) << 8) | ((g_md & 0xFC) << 3) | (b_md >> 3);
		const uint32_t rgb888 = (r_md << 16) | (g_md << 8) | b_md;
		
		// Write the palette entry.
		// Format: C:[CRAM]:[RGB555]:[RGB565]:[RGB888]
		// - CRAM: Color RAM value. (16-bit for MD)
		// - RGB555: 15-bit RGB value.
		// - RGB565: 16-bit RGB value.
		// - RGB888: 32-bit RGB value.
		fprintf(f, "C:%04X:%04X:%04X:%06X\n",
			color_md, rgb555, rgb565, rgb888);
	}
	
	// Write the shadow palette.
	fprintf(f, "\n");
	fprintf(f, "SHMode:%s\n", PALTEST_SHMODE_SHADOW);
	for (unsigned i = 0; i < 512; i++)
	{
		// Get the MD color components.
		const uint8_t r = ((i << 1) & 0x00E);
		const uint8_t g = ((i >> 2) & 0x00E);
		const uint8_t b = ((i >> 5) & 0x00E);
		const uint16_t color_md = ((b << 8) | (g << 4) | r);
		
		// Extract color components for the shadow color.
		const uint8_t r_md = palcomponent_md[r >> 1];
		const uint8_t g_md = palcomponent_md[g >> 1];
		const uint8_t b_md = palcomponent_md[b >> 1];
		
		// Calculate the scaled RGB colors.
		const uint16_t rgb555 = ((r_md & 0xF8) << 7) | ((g_md & 0xF8) << 2) | (b_md >> 3);
		const uint16_t rgb565 = ((r_md & 0xF8) << 8) | ((g_md & 0xFC) << 3) | (b_md >> 3);
		const uint32_t rgb888 = (r_md << 16) | (g_md << 8) | b_md;
		
		// Write the palette entry.
		// Format: C:[CRAM]:[RGB555]:[RGB565]:[RGB888]
		// - CRAM: Color RAM value.
		// - RGB555: 15-bit RGB value.
		// - RGB565: 16-bit RGB value.
		// - RGB888: 32-bit RGB value.
		fprintf(f, "C:%04X:%04X:%04X:%06X\n",
			color_md, rgb555, rgb565, rgb888);
	}
	
	// Write the highlight palette.
	fprintf(f, "\n");
	fprintf(f, "SHMode:%s\n", PALTEST_SHMODE_HIGHLIGHT);
	for (unsigned i = 0; i < 512; i++)
	{
		// Get the MD color components.
		const uint8_t r = ((i << 1) & 0x00E);
		const uint8_t g = ((i >> 2) & 0x00E);
		const uint8_t b = ((i >> 5) & 0x00E);
		const uint16_t color_md = ((b << 8) | (g << 4) | r);
		
		// Extract color components for the highlight color.
		const uint8_t r_md = palcomponent_md[(r >> 1) + 8 - 1];
		const uint8_t g_md = palcomponent_md[(g >> 1) + 8 - 1];
		const uint8_t b_md = palcomponent_md[(b >> 1) + 8 - 1];
		
		// Calculate the scaled RGB colors.
		const uint16_t rgb555 = ((r_md & 0xF8) << 7) | ((g_md & 0xF8) << 2) | (b_md >> 3);
		const uint16_t rgb565 = ((r_md & 0xF8) << 8) | ((g_md & 0xFC) << 3) | (b_md >> 3);
		const uint32_t rgb888 = (r_md << 16) | (g_md << 8) | b_md;
		
		// Write the palette entry.
		// Format: C:[CRAM]:[RGB555]:[RGB565]:[RGB888]
		// - CRAM: Color RAM value.
		// - RGB555: 15-bit RGB value.
		// - RGB565: 16-bit RGB value.
		// - RGB888: 32-bit RGB value.
		fprintf(f, "C:%04X:%04X:%04X:%06X\n",
			color_md, rgb555, rgb565, rgb888);
	}
	
	// MD palettes written successfully.
	fclose(f);
	return 0;
}

static int write_paltype_sms(const char *filename)
{
	static const uint8_t PalComponent_SMS[4] = {0x00, 0x55, 0xAA, 0xFF};
	
	FILE *f = fopen(filename, "w");
	if (!f)
	{
		fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
		return -1;
	}
	
	// Write the file header.
	fprintf(f, "%s:%04X\n", PALTEST_MAGIC, PALTEST_VERSION);
	fprintf(f, "PalMode:%s\n", PALTEST_PALMODE_SMS);
	
	// Write the SMS palette.
	fprintf(f, "\n");
	for (unsigned i = 0; i < 64; i++)
	{
		// Get the SMS color components.
		const uint8_t r = PalComponent_SMS[i & 0x03];
		const uint8_t g = PalComponent_SMS[(i >> 2) & 0x03];
		const uint8_t b = PalComponent_SMS[(i >> 4) & 0x03];
		
		// Calculate the scaled RGB colors.
		const uint16_t rgb555 = ((r & 0xF8) << 7) | ((g & 0xF8) << 2) | (b >> 3);
		const uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		const uint32_t rgb888 = (r << 16) | (g << 8) | b;
		
		// Write the palette entry.
		// Format: C:[CRAM]:[RGB555]:[RGB565]:[RGB888]
		// - CRAM: Color RAM value. (8-bit for SMS)
		// - RGB555: 15-bit RGB value.
		// - RGB565: 16-bit RGB value.
		// - RGB888: 32-bit RGB value.
		fprintf(f, "C:%02X:%04X:%04X:%06X\n",
			i, rgb555, rgb565, rgb888);
	}
	
	fclose(f);
	return 0;
}

static int write_paltype_gg(const char *filename)
{
	FILE *f = fopen(filename, "w");
	if (!f)
	{
		fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
		return -1;
	}
	
	// Write the file header.
	fprintf(f, "%s:%04X\n", PALTEST_MAGIC, PALTEST_VERSION);
	fprintf(f, "PalMode:%s\n", PALTEST_PALMODE_GG);
	
	// Write the palette.
	fprintf(f, "\n");
	for (unsigned i = 0; i <= 0xFFF; i++)
	{
		// Get the Game Gear color components.
		uint8_t r = (i & 0x00F);	r |= (r << 4);
		uint8_t g = ((i >> 4) & 0x00F);	g |= (g << 4);
		uint8_t b = ((i >> 8) & 0x00F);	b |= (b << 4);
		
		// Calculate the scaled RGB colors.
		const uint16_t rgb555 = ((r & 0xF8) << 7) | ((g & 0xF8) << 2) | (b >> 3);
		const uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		const uint32_t rgb888 = (r << 16) | (g << 8) | b;
		
		// Write the palette entry.
		// Format: C:[CRAM]:[RGB555]:[RGB565]:[RGB888]
		// - CRAM: Color RAM value. (16-bit for MD)
		// - RGB555: 15-bit RGB value.
		// - RGB565: 16-bit RGB value.
		// - RGB888: 32-bit RGB value.
		fprintf(f, "C:%04X:%04X:%04X:%06X\n",
			i, rgb555, rgb565, rgb888);
	}
	
	fclose(f);
	return 0;
}

int main(void)
{
	write_paltype_md("PalTest_MD_Raw.txt", CSM_RAW);
	write_paltype_md("PalTest_MD_Full.txt", CSM_FULL);
	write_paltype_md("PalTest_MD_Full_SH.txt", CSM_FULL_SH);
	write_paltype_sms("PalTest_SMS.txt");
	write_paltype_gg("PalTest_GG.txt");
	return EXIT_SUCCESS;
}
