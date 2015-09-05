/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_VdpPalette_DAC_generate.c: VdpPalette DAC tests.                   *
 * Data file generator.                                                    *
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

/**
 * TODO:
 * - Add 32X and TMS9918 functions.
 */

#include "test_VdpPalette_DAC.h"

// C includes.
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>

// getopt_long()
#include <unistd.h>
#include <getopt.h>

// NOTE: We're not using LibGens defines here.
// test_VdpPalette_generate_DAC.c won't link to LibGens.

// Palette mode. [VdpPalette::PalMode_t]
typedef enum
{
	PALMODE_MD,
	PALMODE_32X,
	PALMODE_SMS,
	PALMODE_GG,
	PALMODE_TMS9918,
	
	PALMODE_MAX
} PalMode_t;

// TODO: Combine these functions together.

static int write_paltype_md(const char *filename)
{
	static const uint8_t PalComponent_MD[16] =
		{  0,  18,  36,  54,  72,  91, 109, 127,
		 145, 163, 182, 200, 218, 236, 255, 255};

	FILE *f = fopen(filename, "w");
	if (!f) {
		fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// Write the file header.
	fprintf(f, "%s:%04X\n", PALTEST_MAGIC, PALTEST_VERSION);
	fprintf(f, "%s:%s\n", PALTEST_CMD_PALMODE, PALTEST_PALMODE_MD);

	// Write the normal palette.
	fprintf(f, "\n");
	fprintf(f, "%s:%s\n", PALTEST_CMD_SHMODE, PALTEST_SHMODE_NORMAL);
	for (unsigned int i = 0x0000; i <= 0xFFFF; i++) {
		// Get the MD color components.
		const uint8_t r = (i & 0x00E);
		const uint8_t g = ((i >> 4) & 0x00E);
		const uint8_t b = ((i >> 8) & 0x00E);
		
		// Extract color components for the normal color.
		const uint8_t r_md = PalComponent_MD[r];
		const uint8_t g_md = PalComponent_MD[g];
		const uint8_t b_md = PalComponent_MD[b];
		
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
		fprintf(f, "%s:%04X:%04X:%04X:%06X\n",
			PALTEST_CMD_COLORENTRY, i, rgb555, rgb565, rgb888);
	}

	// Write the shadow palette.
	fprintf(f, "\n");
	fprintf(f, "SHMode:%s\n", PALTEST_SHMODE_SHADOW);
	for (unsigned int i = 0x0000; i <= 0xFFFF; i++) {
		// Get the MD color components.
		const uint8_t r = (i & 0x00E);
		const uint8_t g = ((i >> 4) & 0x00E);
		const uint8_t b = ((i >> 8) & 0x00E);

		// Extract color components for the shadow color.
		const uint8_t r_md = PalComponent_MD[r >> 1];
		const uint8_t g_md = PalComponent_MD[g >> 1];
		const uint8_t b_md = PalComponent_MD[b >> 1];

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
		fprintf(f, "%s:%04X:%04X:%04X:%06X\n",
			PALTEST_CMD_COLORENTRY, i, rgb555, rgb565, rgb888);
	}

	// Write the highlight palette.
	fprintf(f, "\n");
	fprintf(f, "SHMode:%s\n", PALTEST_SHMODE_HIGHLIGHT);
	for (unsigned int i = 0x0000; i <= 0xFFFF; i++) {
		// Get the MD color components.
		const uint8_t r = (i & 0x00E);
		const uint8_t g = ((i >> 4) & 0x00E);
		const uint8_t b = ((i >> 8) & 0x00E);

		// Extract color components for the highlight color.
		const uint8_t r_md = PalComponent_MD[(r >> 1) + 8 - 1];
		const uint8_t g_md = PalComponent_MD[(g >> 1) + 8 - 1];
		const uint8_t b_md = PalComponent_MD[(b >> 1) + 8 - 1];

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
		fprintf(f, "%s:%04X:%04X:%04X:%06X\n",
			PALTEST_CMD_COLORENTRY, i, rgb555, rgb565, rgb888);
	}

	// MD palettes written successfully.
	fclose(f);
	return 0;
}

static int write_paltype_sms(const char *filename)
{
	static const uint8_t PalComponent_SMS[4] = {0x00, 0x55, 0xAA, 0xFF};

	FILE *f = fopen(filename, "w");
	if (!f) {
		fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// Write the file header.
	fprintf(f, "%s:%04X\n", PALTEST_MAGIC, PALTEST_VERSION);
	fprintf(f, "PalMode:%s\n", PALTEST_PALMODE_SMS);

	// Write the SMS palette.
	fprintf(f, "\n");
	for (unsigned int i = 0x00; i <= 0xFF; i++) {
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
	if (!f) {
		fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
		return -1;
	}

	// Write the file header.
	fprintf(f, "%s:%04X\n", PALTEST_MAGIC, PALTEST_VERSION);
	fprintf(f, "PalMode:%s\n", PALTEST_PALMODE_GG);

	// Write the palette.
	fprintf(f, "\n");
	for (unsigned int i = 0x0000; i <= 0xFFFF; i++) {
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

int main(int argc, char *argv[])
{
	static const struct option PalTest_Options[] =
	{
		{"palmode",	1, NULL, 'p'},
		{"help",	0, NULL, 'h'},
		
		{NULL, 0, NULL, 0}
	};
	
	// Command line syntax:
	// ./test_VdpPalette_generate --palmode[=PAL] output.filename.txt
	
	// Configurable options.
	PalMode_t palMode = PALMODE_MD;
	
	// Get options.
	int c;
	int option_index;
	while (1) {
		option_index = -1;
		c = getopt_long(argc, argv, "p:h", PalTest_Options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'p':
				// Palette mode.
				if (!strcasecmp("MD", optarg))
					palMode = PALMODE_MD;
				else if (!strcasecmp("32X", optarg))
					palMode = PALMODE_32X;
				else if (!strcasecmp("SMS", optarg))
					palMode = PALMODE_SMS;
				else if (!strcasecmp("GG", optarg))
					palMode = PALMODE_GG;
				else if (!strcasecmp("TMS9918", optarg))
					palMode = PALMODE_TMS9918;
				else {
					// Invalid palette mode.
					if (option_index < 0)
						fprintf(stderr, "%s: invalid argument `%s' for `-%c'\n",
							argv[0], optarg, c);
					else
						fprintf(stderr, "%s: invalid argument `%s' for `--%s'\n",
							argv[0], optarg, PalTest_Options[option_index].name);

					fprintf(stderr, "Valid arguments are:\n"
							"  - `md', `32x', `sms', `gg', `tms9918'\n");
					fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;

			case 'h':
				// Help.
				fprintf(stderr, "LibGens: VdpPalette tests: Data file generator.\n");
				fprintf(stderr, "Usage: %s [OPTION]... [FILE]\n", argv[0]);
				fprintf(stderr, "Generates a LibGens VdpPalette test data file for use with test_VdpPalette.\n"
						"By default, generates a Mega Drive palette file using Full color scaling.\n"
						"\n"
						"Options:\n"
						"  -p, --palmode[=PAL]        set the palette mode for the data file.\n"
						"                               `md', `32x', `sms', `gg', `tms9918'\n"
						"  -h, --help                 display this help and exit\n"
						"\n"
						"Exit status:\n"
						" 0  if OK.\n"
						" 1  if an error occurred.\n"
						);
				return EXIT_SUCCESS;

			case '?':
			default:
				// Invalid option.
				fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
				return EXIT_FAILURE;
		}
	}

	// Check for an output file.
	if (optind > (argc - 1)) {
		// No output file specified.
		fprintf(stderr, "%s: missing file operand\n", argv[0]);
		fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
		return EXIT_FAILURE;
	} else if (optind < (argc - 1)) {
		// Too many filenames specified.
		fprintf(stderr, "%s: too many filenames specified\n", argv[0]);
		fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
		return EXIT_FAILURE;
	}

	const char *filename = argv[optind];
	int ret;
	switch (palMode) {
		case PALMODE_MD:
			ret = write_paltype_md(filename);
			break;
		case PALMODE_SMS:
			ret = write_paltype_sms(filename);
			break;
		case PALMODE_GG:
			ret = write_paltype_gg(filename);
			break;
		case PALMODE_32X:
		case PALMODE_TMS9918:
		default:
			// Unsupported right now.
			fprintf(stderr, "%s: error: %s palette mode isn't supported yet.\n",
				argv[0], (palMode == PALMODE_32X ? "32X" : "TMS9918"));
			return EXIT_FAILURE;
	}

	// TODO: Print a success/fail message.
	if (ret != 0) {
		// TODO: Print the error message.
		fprintf(stderr, "%s: error %d\n", argv[0], ret);
	}
	return ret;
}
