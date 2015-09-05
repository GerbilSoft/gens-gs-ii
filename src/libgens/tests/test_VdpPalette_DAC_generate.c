/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_VdpPalette_DAC_generate.c: VdpPalette DAC tests.                   *
 * Data file generator.                                                    *
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

/**
 * TODO:
 * - Add 32X and TMS9918 functions.
 */

#include "test_VdpPalette_DAC.h"

// C includes.
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifndef _MSC_VER
// POSIX: strcasecmp() is in strings.h.
// MSVC has _stricmp(), which is in string.h.
#include <strings.h>
#endif /* _MSC_VER */

// popt
#include <popt.h>

// NOTE: We're not using LibGens defines here.
// test_VdpPalette_generate_DAC.c won't link to LibGens.

// Palette mode. [VdpPalette::PalMode_t]
typedef enum {
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
	unsigned int i;

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
	for (i = 0x0000; i <= 0xFFFF; i++) {
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
	for (i = 0x0000; i <= 0xFFFF; i++) {
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
	for (i = 0x0000; i <= 0xFFFF; i++) {
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
	unsigned int i;

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
	for (i = 0x00; i <= 0xFF; i++) {
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
	unsigned int i;

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
	for (i = 0x0000; i <= 0xFFFF; i++) {
		// Get the Game Gear color components.
		uint8_t r, g, b;
		uint16_t rgb555, rgb565;
		uint32_t rgb888;

		r = (i & 0x00F);	r |= (r << 4);
		g = ((i >> 4) & 0x00F);	g |= (g << 4);
		b = ((i >> 8) & 0x00F);	b |= (b << 4);

		// Calculate the scaled RGB colors.
		rgb555 = ((r & 0xF8) << 7) | ((g & 0xF8) << 2) | (b >> 3);
		rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		rgb888 = (r << 16) | (g << 8) | b;

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

static void print_prg_info(void)
{
	fprintf(stderr, "LibGens: VdpPalette tests: Data file generator.\n");
	// TODO: Version number.
	fprintf(stderr, "Copyright (c) 2011-2015 by David Korth.\n");
}

static void print_gpl(void)
{
	fprintf(stderr,
		"This program is free software; you can redistribute it and/or modify it\n"
		"under the terms of the GNU General Public License as published by the\n"
		"Free Software Foundation; either version 2 of the License, or (at your\n"
		"option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful, but\n"
		"WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License along\n"
		"with this program; if not, write to the Free Software Foundation, Inc.,\n"
		"51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n");
}

static void print_help(const poptContext con)
{
	print_prg_info();
	fputc('\n', stderr);
	// NOTE: poptPrintHelp() only prints the filename portion of argv[0].
	poptPrintHelp(con, stderr, 0);

	fprintf(stderr,
		"\n"
		"Exit status:\n"
		" 0  if OK.\n"
		" 1  if an error occurred.\n");
}

int main(int argc, char *argv[])
{
	// Command line syntax:
	// ./test_VdpPalette_generate --palmode[=PAL] output.filename.txt

	// Options.
	char *pal_mode_str = NULL;              // Palette mode string.
	PalMode_t palMode = PALMODE_MD;         // Palette mode. (Parsed later)
	const char *out_filename = NULL;        // Output filename.

	// popt: help options table.
	struct poptOption helpOptionsTable[] = {
		{"help", '?', POPT_ARG_NONE, NULL, '?', "Show this help message", NULL},
		{"usage", 0, POPT_ARG_NONE, NULL, 'u', "Display brief usage message", NULL},
		{"version", 'V', POPT_ARG_NONE, NULL, 'V', "Display version information", NULL},
		POPT_TABLEEND
	};
	
	// popt: main options table.
	struct poptOption optionsTable[] = {
		{"palmode",  'p', POPT_ARG_STRING, &pal_mode_str, 0,
			"Set the palette mode for the data file.\n"
			"'md', '32x', 'sms', 'gg', 'tms9918'", "PAL"},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
			"Help options:", NULL},
		POPT_TABLEEND
	};

	poptContext optCon;
	int c;
	int ret;

	// TODO: W32U_GetArgvU()
	
	// Initialize the popt context.
	optCon = poptGetContext(NULL, argc, (const char**)argv, optionsTable, 0);

	// popt: Alias '-h' to '-?'.
	// NOTE: help_argv must be free()able, so it
	// can't be static or allocated on the stack.
	{
		const char **help_argv = (const char**)malloc(sizeof(const char*) * 2);
		struct poptAlias help_alias = {NULL, 'h', 1, help_argv};
		help_argv[0] = "-?";
		help_argv[1] = NULL;
		poptAddAlias(optCon, help_alias, 0);
	}
	
	// Process options.
	while ((c = poptGetNextOpt(optCon)) >= 0) {
		switch (c) {
			case 'V':
				print_prg_info();
				fputc('\n', stderr);
				print_gpl();
				return EXIT_SUCCESS;

			case '?':
				print_help(optCon);
				return EXIT_SUCCESS;

			case 'u':
				poptPrintUsage(optCon, stderr, 0);
				return EXIT_SUCCESS;

			default:
				break;
		}
	}

	if (c < -1) {
		// An error occurred during option processing.
		switch (c) {
			case POPT_ERROR_BADOPT:
				// Unrecognized option.
				fprintf(stderr, "%s: unrecognized option '%s'\n"
					"Try `%s --help` for more information.\n",
					argv[0], poptBadOption(optCon, POPT_BADOPTION_NOALIAS), argv[0]);
				break;
			default:
				// Other error.
				fprintf(stderr, "%s: '%s': %s\n"
					"Try `%s --help` for more information.\n",
					argv[0], poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
					poptStrerror(c), argv[0]);
				break;
		}
		return EXIT_FAILURE;
	}

	// Check if a palette mode was specified.
	if (pal_mode_str != NULL) {
		// Parse the palette mode.
		// NOTE: This was originally optional; if no
		// argument was specified, usage information
		// was printed. This isn't needed, since the
		// user can use the help options.
		if (!strcasecmp(pal_mode_str, "MD")) {
			palMode = PALMODE_MD;
		} else if (!strcasecmp(pal_mode_str, "32X")) {
			palMode = PALMODE_32X;
		} else if (!strcasecmp(pal_mode_str, "SMS")) {
			palMode = PALMODE_SMS;
		} else if (!strcasecmp(pal_mode_str, "GG")) {
			palMode = PALMODE_GG;
		} else if (!strcasecmp(pal_mode_str, "TMS9918")) {
			palMode = PALMODE_TMS9918;
		} else {
			// Invalid palette mode.
			// TODO: Distinguish between long and short opts?
			fprintf(stderr, "%s: '%s': invalid palette mode.\n"
				"Valid palette modes are:\n"
				"  'md', `32x', 'sms', 'gg', 'tms9918'\n"
				"Try `%s --help' for more information.\n",
				argv[0], pal_mode_str, argv[0]);
			return EXIT_FAILURE;
		}
	}

	// Get the output filename.
	out_filename = poptGetArg(optCon);
	if (out_filename == NULL) {
		// No output filename specified.
		fprintf(stderr, "%s: no filename specified\n"
			"Try `%s --help` for more information.\n",
			argv[0], argv[0]);
		return EXIT_FAILURE;
	} else if (poptPeekArg(optCon) != NULL) {
		// Too many parameters specified.
		fprintf(stderr, "%s: too many parameters specified\n"
			"Try `%s --help` for more information.\n",
			argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	// Done parsing arguments.
	poptFreeContext(optCon);

	// Write the palette file.
	switch (palMode) {
		case PALMODE_MD:
			ret = write_paltype_md(out_filename);
			break;
		case PALMODE_SMS:
			ret = write_paltype_sms(out_filename);
			break;
		case PALMODE_GG:
			ret = write_paltype_gg(out_filename);
			break;
		case PALMODE_32X:
		case PALMODE_TMS9918:
		default:
			// Unsupported right now.
			fprintf(stderr, "%s: error: palette mode '%s' isn't supported yet.\n",
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
