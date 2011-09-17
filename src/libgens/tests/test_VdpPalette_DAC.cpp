/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_VdpPalette_DAC.cpp: VdpPalette DAC tests.                          *
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

#include "test_VdpPalette_DAC.h"
#include "Vdp/VdpPalette.hpp"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
using namespace std;

#include <unistd.h>

// Test variables.
static int tests_failed = 0;	// Number of tests failed.
static int tests_total = 0;	// Total number of tests run.

// Test variables. (Current section.)
static int section_failed = 0;	// Number of tests failed.
static int section_total = 0;	// Total number of tests run.

/**
 * New test section.
 * Prints previous section data and clears the test variables.
 */
static void new_section(void)
{
	if (section_total > 0)
	{
		fprintf(stderr, "Section complete. %d/%d tests passed.\n\n",
			(section_total - section_failed), section_total);
	}
	
	section_failed = 0;
	section_total = 0;
}

/**
 * Parse a number from a string. (strtol() wrapper)
 * @param str String.
 * @param base Base. (0 for auto-detect)
 * @param pRet Pointer to return value.
 * @return 0 on success; non-zero on error.
 */
template<typename T>
static inline int parse_number(const char *str, int base, T *pRet)
{
	char *endptr;
	long ret = strtol(str, &endptr, base);
	
	if (errno != 0 || endptr == str || *endptr != 0x00)
	{
		// Error decoding the string.
		return -1;
	}
	
	// String decoded successfully.
	*pRet = (T)ret;
	return 0;
}

// TODO: Templated version!

/**
 * CRam assert for VdpPalette.
 * Increments total test counters.
 * If test fails, error is printed and failed test counters are incremented.
 * @param test_name Test name.
 * @param cram CRam value. (TODO: Only print 2 digits for SMS!)
 * @param expected Expected value.
 * @param actual Actual value.
 * @return Expression value.
 */
template<typename T>
static bool CRam_Assert(const char *test_name, uint16_t cram, T expected, T actual)
{
	// Increment the total test counters.
	tests_total++;
	section_total++;
	
	if (expected == actual)
		return true;
	
	// Assertion failed.
	if (sizeof(T) == 2)
	{
		fprintf(stderr, "[FAIL] %s: [%04X] expected %04X, got %04X\n",
			test_name, cram, expected, actual);
	}
	else
	{
		fprintf(stderr, "[FAIL] %s: [%04X] expected %06X, got %06X\n",
			test_name, cram, expected, actual);
	}
	
	tests_failed++;
	section_failed++;
	return false;
}

/**
 * Test a palette data file.
 * @param filename Filename.
 * @return 0 if tests were run; non-zero if a fatal error occurred.
 */
static int test_file(const char *filename)
{
	// Open the file.
	FILE *f = fopen(filename, "r");
	if (!f)
	{
		fprintf(stderr, "Error opening '%s': %s\n", filename, strerror(errno));
		return -1;
	}
	
	// Current Palette Mode.
	LibGens::VdpPalette::PalMode_t palMode = LibGens::VdpPalette::PALMODE_MD;
	const char *palMode_str = PALTEST_PALMODE_MD;
	
	// Current Color Scale Method.
	LibGens::VdpPalette::ColorScaleMethod_t csm = LibGens::VdpPalette::COLSCALE_FULL;
	const char *csm_str = PALTEST_COLORSCALE_FULL;
	
	// Current SHMode. (0x00 == normal, 0x40 == shadow, 0x80 == highlight)
	uint8_t shMode = 0x00;
	const char *shMode_str = PALTEST_SHMODE_NORMAL;
	
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
		if (!fgets(buf, sizeof(buf), f))
			break;
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
			// Format: PalTest:(uint16_t)version
			token = strtok(NULL, ":");
			int header_version;
			if (parse_number(token, 16, &header_version) != 0)
			{
				fprintf(stderr, "'%s': Invalid file header.\n", filename);
				goto fail;
			}
			
			if (header_version != PALTEST_VERSION)
			{
				fprintf(stderr, "'%s': Incorrect PalTest version. (expected %04X; found %04X) [FATAL]\n",
					filename, header_version, PALTEST_VERSION);
				goto fail;
			}
			
			// Header is valid.
			fprintf(stderr, "Processing file: '%s'\n\n", filename);
			isInit = true;
		}
		else if (!strcasecmp(token, PALTEST_CMD_PALMODE))
		{
			if (!isInit)
				goto no_magic;
			new_section();
			
			// Palette mode.
			// Format: PalMode:(string)mode
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
			
			// Convert the selected PalMode to uppercase.
			switch (palMode)
			{
				case LibGens::VdpPalette::PALMODE_MD:
				default:
					palMode_str = PALTEST_PALMODE_MD;
					break;
				/* TODO
				case LibGens::VdpPalette::PALMODE_32X:
					palMode_str = PALTEST_PALMODE_32X;
					break;
				*/
				case LibGens::VdpPalette::PALMODE_SMS:
					palMode_str = PALTEST_PALMODE_SMS;
					break;
				case LibGens::VdpPalette::PALMODE_GG:
					palMode_str = PALTEST_PALMODE_GG;
					break;
				/* TODO
				case LibGens::VdpPalette::PALMODE_TMS9918:
					palMode_str = PALTEST_PALMODE_TMS9918;
					break;
				*/
			}
			
			// Print the selected palette mode.
			fprintf(stderr, "Selected PalMode: '%s'\n", palMode_str);
			
			// Set the palette mode.
			vdp15->setPalMode(palMode);
			vdp16->setPalMode(palMode);
			vdp32->setPalMode(palMode);
		}
		else if (!strcasecmp(token, PALTEST_CMD_COLORSCALE))
		{
			if (!isInit)
				goto no_magic;
			new_section();
			
			// Color Scale Method.
			// Format: ColorScale:(string)csm
			token = strtok(NULL, ":");
			if (!strcasecmp(token, PALTEST_COLORSCALE_RAW))
				csm = LibGens::VdpPalette::COLSCALE_RAW;
			else if (!strcasecmp(token, PALTEST_COLORSCALE_FULL))
				csm = LibGens::VdpPalette::COLSCALE_FULL;
			else if (!strcasecmp(token, PALTEST_COLORSCALE_FULL_SH))
				csm = LibGens::VdpPalette::COLSCALE_FULL_SH;
			else
			{
				fprintf(stderr, "Unsupported ColorScale: '%s'\n", token);
				goto fail;
			}
			
			// Convert the selected ColorScale to uppercase.
			switch (csm)
			{
				case LibGens::VdpPalette::COLSCALE_RAW:
				default:
					csm_str = PALTEST_COLORSCALE_RAW;
					break;
				case LibGens::VdpPalette::COLSCALE_FULL:
					csm_str = PALTEST_COLORSCALE_FULL;
					break;
				case LibGens::VdpPalette::COLSCALE_FULL_SH:
					csm_str = PALTEST_COLORSCALE_FULL_SH;
					break;
			}
			
			// Print the selected color scale method.
			fprintf(stderr, "Selected ColorScale: '%s'\n", csm_str);
			if (palMode != LibGens::VdpPalette::PALMODE_MD)
			{
				// ColorScale is only supported with MD palettes.
				fprintf(stderr, "* WARNING: ColorScale has no effect with PalMode '%s'.\n", palMode_str);
			}
			
			// Set the color scale method.
			vdp15->setColorScaleMethod(csm);
			vdp16->setColorScaleMethod(csm);
			vdp32->setColorScaleMethod(csm);
		}
		else if (!strcasecmp(token, PALTEST_CMD_SHMODE))
		{
			if (!isInit)
				goto no_magic;
			new_section();			
			
			// Shadow/Highlight Mode.
			// Format: SHMode:(string)shmode
			token = strtok(NULL, ":");
			if (!strcasecmp(token, PALTEST_SHMODE_NORMAL))
				shMode = 0x00;
			else if (!strcasecmp(token, PALTEST_SHMODE_SHADOW))
				shMode = 0x40;
			else if (!strcasecmp(token, PALTEST_SHMODE_HIGHLIGHT))
				shMode = 0x80;
			else
			{
				fprintf(stderr, "Unsupported SHMode: '%s'\n", token);
				goto fail;
			}
			
			// Convert the selected SHMode to uppercase.
			switch (shMode)
			{
				case 0x00:
				default:
					shMode_str = PALTEST_SHMODE_NORMAL;
					break;
				case 0x40:
					shMode_str = PALTEST_SHMODE_SHADOW;
					break;
				case 0x80:
					shMode_str = PALTEST_SHMODE_HIGHLIGHT;
					break;
			}
			
			// Print the selected Shadow/Highlight mode.
			fprintf(stderr, "Selected SHMode: '%s'\n", shMode_str);
			if (palMode != LibGens::VdpPalette::PALMODE_MD)
			{
				// ColorScale is only supported with MD palettes.
				fprintf(stderr, "* WARNING: SHMode has no effect with PalMode '%s'.\n", palMode_str);
			}
			
			// Set the Shadow/Highlight mode.
			const bool doSH = (shMode != 0x00);
			vdp15->setMdShadowHighlight(doSH);
			vdp16->setMdShadowHighlight(doSH);
			vdp32->setMdShadowHighlight(doSH);
		}
		else if (!strcasecmp(token, PALTEST_CMD_COLORENTRY))
		{
			// Color entry.
			// Format: C:cram:rgb15:rgb16:rgb32
			// All values are hexadecimal.
			// cram is 16-bit except for SMS, where it's 8-bit.
			// rgb15 and rgb16 are both 16-bit.
			// rgb32 is 32-bit.
			
			uint16_t cram, rgb15, rgb16;
			uint32_t rgb32;
			
			// Parse the data.
			// TODO: Move validation into a common assertion function.
			token = strtok(NULL, ":");
			if (!token || parse_number(token, 16, &cram) != 0)
			{
				fprintf(stderr, "ColorEntry: Invalid CRAM field: '%s'. [FAIL]\n",
					(token ? token : "(null)"));
				tests_failed++;
				tests_total++;
				section_failed++;
				section_total++;
				continue;
			}
			token = strtok(NULL, ":");
			if (!token || parse_number(token, 16, &rgb15) != 0)
			{
				fprintf(stderr, "ColorEntry: Invalid RGB15 field: '%s'. [FAIL]\n",
					(token ? token : "(null)"));
				tests_failed++;
				tests_total++;
				section_failed++;
				section_total++;
				continue;
			}
			token = strtok(NULL, ":");
			if (!token || parse_number(token, 16, &rgb16) != 0)
			{
				fprintf(stderr, "ColorEntry: Invalid RGB16 field: '%s'. [FAIL]\n",
					(token ? token : "(null)"));
				tests_failed++;
				tests_total++;
				section_failed++;
				section_total++;
				continue;
			}
			token = strtok(NULL, ":");
			if (!token || parse_number(token, 16, &rgb32) != 0)
			{
				fprintf(stderr, "ColorEntry: Invalid RGB32 field: '%s'. [FAIL]\n",
					(token ? token : "(null)"));
				tests_failed++;
				tests_total++;
				section_failed++;
				section_total++;
				continue;
			}
			
			// Test the palette entry.
			const uint8_t pal_entry =
				(palMode == LibGens::VdpPalette::PALMODE_MD
					? (shMode + 1)
					: 0x01
					);
			
			if (palMode == LibGens::VdpPalette::PALMODE_SMS)
			{
				// SMS. Write 8-bit data to CRam.
				vdp15->writeCRam_8(0x01, (uint8_t)cram);
				vdp16->writeCRam_8(0x01, (uint8_t)cram);
				vdp32->writeCRam_8(0x01, (uint8_t)cram);
			}
			else
			{
				// Other system. Write 16-bit data to CRam.
				//fprintf(stderr, "WRITING %04X to 0x02\n", cram);
				vdp15->writeCRam_16(0x02, cram);
				vdp16->writeCRam_16(0x02, cram);
				vdp32->writeCRam_16(0x02, cram);
			}
			
			// Update the VdpPalette objects.
			vdp15->update();
			vdp16->update();
			vdp32->update();
			
			// Verify the RGB values.
			CRam_Assert("rgb15", cram, rgb15, vdp15->m_palActive.u16[pal_entry]);
			CRam_Assert("rgb16", cram, rgb16, vdp16->m_palActive.u16[pal_entry]);
			CRam_Assert("rgb32", cram, rgb32, vdp32->m_palActive.u32[pal_entry]);
		}
		else
		{
			// Ignore this line.
		}
	}
	
	// Complete the last section.
	new_section();
	
	fclose(f);
	return 0;

no_magic:
	fprintf(stderr, "'%s': PalTest version line is missing. [FATAL]\n", filename);
fail:
	fclose(f);
	return -1;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		// No filename was specified.
		fprintf(stderr, "Specify a filename to test.\n");
		return EXIT_FAILURE;
	}
	
	int ret = test_file(argv[1]);
	if (ret != 0)
	{
		fprintf(stderr, "Test file aborted due to fatal error.\n");
	}
	else
	{
		fprintf(stderr, "Test file complete. %d/%d tests passed.\n",
			(tests_total - tests_failed), tests_total);
	}
	return ((ret == 0) ? ret : tests_failed);
}
