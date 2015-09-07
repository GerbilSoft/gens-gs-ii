/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * Options.cpp: Command line option parser.                                *
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

#include "Options.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cerrno>
#ifndef ECANCELED
#define ECANCELED 158
#endif

// C++ includes.
#include <string>
using std::string;

// popt
#include <popt.h>

namespace GensSdl {

Options::Options()
{
	// Reset the options to the default values.
	reset();
}

/**
 * Reset all options to their default values.
 */
void Options::reset(void)
{
	rom_filename_required = true;

	// TODO: Swap with empty strings?
	rom_filename.clear();
	tmss_rom_filename.clear();
	tmss_enabled = false;

	// Audio options.
	sound_freq = 44100;
	stereo = true;

	// Emulation options.
	sprite_limits = true;
	auto_fix_checksum = true;

	// UI options.
	fps_counter = true;
	auto_pause = false;
	paused_effect = true;
	bpp = 32;

	// Special run modes.
	run_crazy_effect = false;
}

// TODO: Improve these.
static void print_prg_info(void)
{
	fprintf(stderr, "gens-sdl: Gens/GS II basic SDL frontend.\n");
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
}

/**
 * Parse command line arguments using popt.
 * @param ctx gens_args to store arguments in.
 * @param argc
 * @param argv
 * @return 0 if parsed successfully; non-zero on error.
 * Some error codes:
 * - -EINVAL: invalid arguments
 * - -ECANCELED: operation canceled (TODO: define on MSVC?)
 *   - occurs if user specifies something like --help, which exits immediately.
 */
int Options::parse(int argc, const char *argv[])
{
	if (!argv) {
		// Invalid arguments.
		return -EINVAL;
	}

	// Temporary internal option variables.
	struct {
		// popt is a C library, so we have to use
		// C strings here initially. Also, we can't
		// use bool; we have to use int.
		const char *rom_filename;
		const char *tmss_rom_filename;
		// tmss_enabled is implied by tmss_rom_filename.

		// Audio options.
		int sound_freq;
		int stereo;

		// Emulation options.
		int sprite_limits;
		int auto_fix_checksum;

		// UI options.
		int fps_counter;
		int auto_pause;
		int paused_effect;
		int bpp;

		// Special run modes.
		int run_crazy_effect;
	} tmp;

	// Set default values.
	// TODO: Separate function for this?
	memset(&tmp, 0, sizeof(tmp));
	tmp.sound_freq = 44100;
	tmp.stereo = 1;
	tmp.sprite_limits = 1;
	tmp.auto_fix_checksum = 1;
	tmp.fps_counter = 1;
	tmp.paused_effect = 1;
	tmp.bpp = 32;

	// NOTE: rom_filename is provided as a non-option parameter.
	// It will get parsed later.

	// popt: help options table.
	struct poptOption helpOptionsTable[] = {
		{"help", '?', POPT_ARG_NONE, NULL, '?', "Show this help message", NULL},
		{"usage", 0, POPT_ARG_NONE, NULL, 'u', "Display brief usage message", NULL},
		{"version", 'V', POPT_ARG_NONE, NULL, 'V', "Display version information", NULL},
		POPT_TABLEEND
	};

	// popt: audio options table.
	struct poptOption audioOptionsTable[] = {
		{"frequency", 0, POPT_ARG_INT, &tmp.sound_freq, 0,
			"  Audio frequency.", "FREQ"},
		{"mono", 0, POPT_ARG_VAL, &tmp.stereo, 0,
			"  Use monaural audio.", NULL},
		{"stereo", 0, POPT_ARG_VAL, &tmp.stereo, 1,
			"  Use stereo audio.", NULL},
		POPT_TABLEEND
	};

	// popt: emulation options table.
	struct poptOption emulationOptionsTable[] = {
		{"sprite-limits", 0, POPT_ARG_VAL, &tmp.sprite_limits, 1,
			"* Enable sprite limits.", NULL},
		{"no-sprite-limits", 0, POPT_ARG_VAL, &tmp.sprite_limits, 0,
			"  Disable sprite limits.", NULL},
		{"auto-fix-checksum", 0, POPT_ARG_VAL, &tmp.auto_fix_checksum, 1,
			"* Automatically fix checksums.", NULL},
		{"no-auto-fix-checksum", 0, POPT_ARG_VAL, &tmp.auto_fix_checksum, 0,
			"  Don't automatically fix checksums.", NULL},
		POPT_TABLEEND
	};

	// popt: UI options table.
	struct poptOption uiOptionsTable[] = {
		{"fps", 0, POPT_ARG_VAL, &tmp.fps_counter, 1,
			"* Enable the FPS counter.", NULL},
		{"no-fps", 0, POPT_ARG_VAL, &tmp.fps_counter, 0,
			"  Disable the FPS counter.", NULL},
		{"auto-pause", 0, POPT_ARG_VAL, &tmp.auto_pause, 1,
			"* Pause emulator when focus is lost.", NULL},
		{"no-auto-pause", 0, POPT_ARG_VAL, &tmp.auto_pause, 0,
			"  Don't pause emulator when focus is lost.", NULL},
		{"paused-effect", 0, POPT_ARG_VAL, &tmp.paused_effect, 1,
			"* Tint the window when paused.", NULL},
		{"no-paused-effect", 0, POPT_ARG_VAL, &tmp.paused_effect, 0,
			"  Don't tint the window when paused.", NULL},
		{"bpp", 0, POPT_ARG_INT, &tmp.bpp, 0,
			"  Set the internal color depth. (15, 16, 32)", "BPP"},
		POPT_TABLEEND
	};

	// popt: Special run modes table.
	struct poptOption runModesTable[] = {
		{"crazy-effect", 0, POPT_ARG_VAL, &tmp.run_crazy_effect, 1,
			"  Run the \"Crazy\" Effect instead of loading a ROM.", NULL},
		POPT_TABLEEND
	};

	// popt: main options table.
	struct poptOption optionsTable[] = {
		{"tmss-rom", 0, POPT_ARG_STRING, &tmp.tmss_rom_filename, 0,
			"TMSS ROM filename.", "FILENAME"},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, audioOptionsTable, 0,
			"Audio options: (* indicates default)", NULL},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, emulationOptionsTable, 0,
			"Emulation options: (* indicates default)", NULL},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, uiOptionsTable, 0,
			"UI options: (* indicates default)", NULL},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, runModesTable, 0,
			"Special run modes:", NULL},
		{NULL, 0, POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
			"Help options:", NULL},
		POPT_TABLEEND
	};

	// Create the popt context.
	poptContext optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
	poptSetOtherOptionHelp(optCon, "[rom_file]");
	if (rom_filename_required && argc < 2) {
		poptPrintUsage(optCon, stderr, 0);
		return -EINVAL;
	}

	// popt: Alias '-h' to '-?'.
	// NOTE: help_argv must be free()able, so it
	// can't be static or allocated on the stack.
	{
		const char **help_argv = (const char**)malloc(sizeof(const char*) * 2);
		help_argv[0] = "-?";
		help_argv[1] = NULL;
		struct poptAlias help_alias = {NULL, 'h', 1, help_argv};
		poptAddAlias(optCon, help_alias, 0);
	}

	// Process options.
	int c;
	while ((c = poptGetNextOpt(optCon)) >= 0) {
		switch (c) {
			case 'V':
				print_prg_info();
				fputc('\n', stderr);
				print_gpl();
				poptFreeContext(optCon);
				return -ECANCELED;

			case '?':
				print_help(optCon);
				poptFreeContext(optCon);
				return -ECANCELED;

			case 'u':
				poptPrintUsage(optCon, stderr, 0);
				poptFreeContext(optCon);
				return -ECANCELED;

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
		poptFreeContext(optCon);
		return -EINVAL;
	}

	// Process arguments to ensure that they're valid.

	// TMSS ROM filename.
	if (tmp.tmss_rom_filename != nullptr) {
		// TMSS ROM filename was specified.
		this->tmss_rom_filename = string(tmss_rom_filename);
		this->tmss_enabled = true;
	}

	// Get the ROM filename.
	// ROM filename is *not* required here.
	// If the frontend can't run without a ROM,
	// the caller will need to handle it.
	tmp.rom_filename = poptGetArg(optCon);
	if (tmp.rom_filename != nullptr) {
		// ROM filename was specified.
		this->rom_filename = string(tmp.rom_filename);
	} else if (this->rom_filename_required && !tmp.run_crazy_effect) {
		// A ROM is required, but wasn't specified.
		// (If --crazy-effect is specified, this is ignored.)
		fprintf(stderr, "%s: no ROM filename specified\n"
			"Try `%s --help` for more information.\n",
			argv[0], argv[0]);
		poptFreeContext(optCon);
		return -EINVAL;
	}

	// Check if too many filenames were specified.
	if (poptPeekArg(optCon) != NULL) {
		// Too many filenames were specified.
		fprintf(stderr, "%s: too many parameters\n"
			"Try `%s --help` for more information.\n",
			argv[0], argv[0]);
		poptFreeContext(optCon);
		return -EINVAL;
	}

	// Copy other arguments.
	// TODO: Verify that they're valid.

	// Audio options.
	this->sound_freq = tmp.sound_freq;
	this->stereo = !!tmp.stereo;

	// Emulation options.
	this->sprite_limits = !!tmp.sprite_limits;
	this->auto_fix_checksum = !!tmp.auto_fix_checksum;

	// UI options.
	this->fps_counter = !!tmp.fps_counter;
	this->auto_pause = !!tmp.auto_pause;
	this->paused_effect = !!tmp.paused_effect;

	if (tmp.bpp == 15 || tmp.bpp == 16 || tmp.bpp == 32) {
		this->bpp = (uint8_t)tmp.bpp;
	} else {
		// TODO: Show an error.
		this->bpp = 32;
	}

	// Special run modes.
	this->run_crazy_effect = !!tmp.run_crazy_effect;

	// Done parsing arguments.
	poptFreeContext(optCon);
	return 0;
}

}
