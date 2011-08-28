/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_VdpPalette.cpp: VdpPalette tests.                                  *
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
#include "Vdp/VdpPalette.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/**
 * Parse a number from a string. (strtol() wrapper)
 * @param str String.
 * @param base Base. (0 for auto-detect)
 * @param pRet Pointer to return value.
 * @return 0 on success; non-zero on error.
 */
static inline int parse_number(const char *str, int base, int* pRet)
{
	char *endptr;
	int ret = strtol(str, &endptr, base);
	
	if (errno != 0 || endptr == str || *endptr != 0x00)
	{
		// Error decoding the string.
		return -1;
	}
	
	// String decoded successfully.
	*pRet = ret;
	return 0;
}

/**
 * Test a palette data file.
 * @param filename Filename.
 * @return 0 on success; non-zero on error.
 */
static int test_file(const char *filename)
{
	// Open the file.
	FILE *f = fopen(filename, "r");
	if (!f)
	{
		fprintf(stderr, "Error opening '%s': %s\n", filename, strerror(errno));
		return EXIT_FAILURE;
	}
	
	// Initialize three VdpPalette objects.
	LibGens::VdpPalette *vdp15 = new LibGens::VdpPalette();
	vdp15->setBpp(LibGens::VdpPalette::BPP_15);
	
	LibGens::VdpPalette *vdp16 = new LibGens::VdpPalette();
	vdp16->setBpp(LibGens::VdpPalette::BPP_16);
	
	LibGens::VdpPalette *vdp32 = new LibGens::VdpPalette();
	vdp32->setBpp(LibGens::VdpPalette::BPP_32);
	
	/**
	 * Default settings:
	 * - PalMode: MD
	 * - ColorScale: Full
	 * - SHMode: Normal
	 */
	vdp15->setPalMode(LibGens::VdpPalette::PALMODE_MD);
	vdp16->setPalMode(LibGens::VdpPalette::PALMODE_MD);
	vdp32->setPalMode(LibGens::VdpPalette::PALMODE_MD);
	
	vdp15->setColorScaleMethod(LibGens::VdpPalette::COLSCALE_FULL);
	vdp16->setColorScaleMethod(LibGens::VdpPalette::COLSCALE_FULL);
	vdp32->setColorScaleMethod(LibGens::VdpPalette::COLSCALE_FULL);
	
	vdp15->setMdShadowHighlight(false);
	vdp16->setMdShadowHighlight(false);
	vdp32->setMdShadowHighlight(false);
	
	// Set to true when PALTEST_MAGIC is read.
	bool isInit = false;
	
	// Read lines.
	char buf[1024];
	const char *token;
	while (!feof(f))
	{
		fgets(buf, sizeof(buf), f);
		buf[sizeof(buf)-1] = 0x00;
		
		// Remove trailing newlines.
		int endpos = (int)strlen(buf) - 1;
		for (; endpos >= 0; endpos--)
		{
			if (buf[endpos] == '\r' || buf[endpos] == '\n')
				buf[endpos] = 0x00;
			else
				break;
		}
		
		// Skip empty lines.
		if (buf[0] == 0x00)
			continue;
		
		// Get the first token.
		token = strtok(buf, ":");
		if (!strcasecmp(token, PALTEST_MAGIC))
		{
			// Header. Check the version.
			token = strtok(NULL, ":");
			int header_version;
			if (parse_number(token, 16, &header_version) != 0)
			{
				fprintf(stderr, "'%s': Invalid file header.\n", filename);
				goto fail;
			}
			
			if (header_version != PALTEST_VERSION)
			{
				fprintf(stderr, "'%s': Incorrect PalTest version. (expected %04X; found %04X)\n",
					filename, header_version, PALTEST_VERSION);
				goto fail;
			}
			
			// Header is valid.
			fprintf(stderr, "Processing file: '%s'\n", filename);
			isInit = true;
		}
		else if (!strcasecmp(token, PALTEST_CMD_PALMODE))
		{
			// Palette mode.
			LibGens::VdpPalette::PalMode_t palMode = LibGens::VdpPalette::PALMODE_MD;
			
			token = strtok(NULL, ":");
			if (!strcasecmp(token, PALTEST_PALMODE_MD))
				palMode = LibGens::VdpPalette::PALMODE_MD;
			/* TODO
			else if (!strcasecmp(token, PALTEST_PALMODE_32X))
				palMode = LibGens::VdpPalette::PALMODE_32X;
			*/
			else if (!strcasecmp(token, PALTEST_PALMODE_SMS))
				palMode = LibGens::VdpPalette::PALMODE_SMS;
			else if (!strcasecmp(token, PALTEST_PALMODE_GG))
				palMode = LibGens::VdpPalette::PALMODE_GG;
			/* TODO
			else if (!strcasecmp(token, PALTEST_PALMODE_TMS9918))
				palMode = LibGens::VdpPalette::PALMODE_TMS9918;
			*/
			else
			{
				fprintf(stderr, "Unsupported PalMode: '%s'\n", token);
				goto fail;
			}
			
			// TODO: Convert PalMode to uppercase?
			fprintf(stderr, "Selected PalMode: '%s'\n", token);
		}
		else
		{
			// Ignore this line.
		}
	}
	
	// TODO: Check if all tests passed.
	fclose(f);
	return EXIT_SUCCESS;
	
fail:
	fclose(f);
	return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		// No filename was specified.
		fprintf(stderr, "Specify a filename to test.\n");
		return EXIT_FAILURE;
	}
	
	return test_file(argv[1]);
}
