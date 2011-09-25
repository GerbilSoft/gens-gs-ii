/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_Vdp_SpriteMaskTestRom.cpp: Sprite Masking & Overflow Test ROM.     *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
 * Original ROM Copyright (c) 2009 by Nemesis.                             *
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

#include "TestSuite.hpp"

// LibGens VDP.
#include "Vdp/Vdp.hpp"
#include "cpu/M68K_Mem.hpp"
#include "MD/SysVersion.hpp"
#include "Util/byteswap.h"

// Test ROM data.
#include "test_Vdp_SpriteMaskingTestRom_data.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
using namespace std;

// C includes.
#include <unistd.h>

// ZLib.
#define CHUNK 4096
#include <zlib.h>

namespace LibGens { namespace Tests {

class Test_SpriteMaskTestRom : public TestSuite
{
	public:
		int exec(void);
	
	protected:
		Vdp *m_vdp;
		
		enum ScreenMode
		{
			SCREEN_MODE_H32 = 0,
			SCREEN_MODE_H40 = 1,
		};
		
		enum SpriteLimits
		{
			SPRITE_LIMITS_DISABLED = 0,
			SPRITE_LIMITS_ENABLED  = 1,
		};
		
		int loadVRam(ScreenMode screenMode);
		int runTestSection(ScreenMode screenMode, SpriteLimits spriteLimits);
		
		enum SpriteTestResult
		{
			TEST_PASSED = 0,
			TEST_FAILED = 1,
			TEST_UNKNOWN = 2,
		};
		
		enum TestMinMax
		{
			TEST_UNBOUNDED = 0,	// Test is unbounded. (equivalent to TEST_MIN)
			TEST_MIN = 1,		// Test minimum bounds.
			TEST_MAX = 2,		// Test maximum bounds.
		};
		
		/**
		 * Channel value must be < than this number
		 * to be considered "off".
		 */
		static const uint8_t PX_MIN_THRESH = 0x20;
		
		/**
		 * Channel value must be >= than this number
		 * to be considered "on".
		 */
		static const uint8_t PX_MAX_THRESH = 0xE0;
		
		SpriteTestResult checkSpriteTest(int test, TestMinMax testMinMax);
		
		struct TestNames
		{
			const char *name;
			bool isMinMax;
		};
		
		static const TestNames SpriteTestNames[9];
		
		void assertSpriteTest(int test, TestMinMax testMinMax, SpriteTestResult expected, SpriteTestResult actual);
		static void PrintSpriteTestResult(FILE *f, SpriteTestResult result);
};


const Test_SpriteMaskTestRom::TestNames Test_SpriteMaskTestRom::SpriteTestNames[9] =
{
	{"Max Sprites per Line", true},
	{"Max Sprite Dots - Basic", true},
	{"Max Sprite Dots - Complex", true},
	{"Sprite Mask", false},
	{"Sprite Mask in S1", false},
	{"Mask S1 on Dot Overflow", false},
	{"Mask S1, X=1;  S2, X=0", false},
	{"Mask S1, X=40; S2, X=0", false},
	{"Max Sprites per Frame", true},
};


/**
 * Load VRam.
 * @param isH40 If false, load H32; if true, load H40.
 * @return 0 on success; non-zero on error.
 */
int Test_SpriteMaskTestRom::loadVRam(ScreenMode screenMode)
{
	// Based on zlib example code:
	// http://www.zlib.net/zlib_how.html
	int ret;
	z_stream strm;
	
	// VRAM buffer. (slightly more than 64 KB)
	uint8_t out[(64*1024) + 1024];
	const unsigned int out_len = sizeof(out);
	unsigned int out_pos = 0;
	
	// Data to decode.
	uint8_t in[16384];
	unsigned int in_len;
	unsigned int in_pos = 0;
	if (screenMode == SCREEN_MODE_H40)
	{
		in_len = sizeof(test_spritemask_vram_h40);
		memcpy(in, test_spritemask_vram_h40, in_len);
	}
	else
	{
		in_len = sizeof(test_spritemask_vram_h32);
		memcpy(in, test_spritemask_vram_h32, in_len);
	}
	
	// Allocate the zlib inflate state.
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, 15+16);
	if (ret != Z_OK)
		return ret;
	
	// Decompress the stream.
	unsigned int avail_out_before;
	unsigned int avail_out_after;
	do
	{
		if (in_pos >= in_len)
			break;
		strm.avail_in = (in_len - in_pos);
		strm.next_in = &in[in_pos];
		
		// Run inflate() on input until the output buffer is not full.
		do
		{
			avail_out_before = (out_len - out_pos);
			strm.avail_out = avail_out_before;
			strm.next_out = &out[out_pos];
			
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);	// make sure the state isn't clobbered
			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
					// fall through
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					// Error occurred while decoding the stream.
					inflateEnd(&strm);
					fprintf(stderr, "ERR: %d\n", ret);
					return ret;
				default:
					break;
			}
			
			// Increase the output position.
			avail_out_after = (avail_out_before - strm.avail_out);
			out_pos += avail_out_after;
		} while (strm.avail_out == 0 && avail_out_after > 0);
	} while (ret != Z_STREAM_END && avail_out_after > 0);
	
	// Close the stream.
	inflateEnd(&strm);
	
	// If we didn't actually finish reading the compressed data, something went wrong.
	if (ret != Z_STREAM_END)
		return Z_DATA_ERROR;
	
	// VRAM data is 64 KB.
	if (out_pos != 65536)
		return Z_DATA_ERROR;
	
	// First two bytes of both VRAM dumps is 0xDD.
	if (out[0] != 0xDD || out[1] != 0xDD)
		return Z_DATA_ERROR;
	
	// Data was read successfully.
	
	// Byteswap VRam to host-endian.
	be16_to_cpu_array(out, 65536);
	
	// Copy VRam to the VDP.
	memcpy(m_vdp->VRam.u8, out, 65536);
	
	// VRam loaded.
	return 0;
}


/**
 * Check a sprite test using screen scraping.
 * @param test Test number. (1-9)
 * @param testMinMax Select whether to test minimum or maximum bounds.
 * @return Sprite test result.
 */
Test_SpriteMaskTestRom::SpriteTestResult Test_SpriteMaskTestRom::checkSpriteTest(int test, TestMinMax testMinMax)
{
	if (test <= 0 || test > 9)
		return TEST_UNKNOWN;
	
	// X position: min == 216, max == 232
	// Add HPixBegin() for H32 mode.
	const int x = ((testMinMax <= TEST_MIN ? 216 : 232) + m_vdp->GetHPixBegin());
	
	// Y position: 48+8+((test-1)*8)
	const int y = (48 + 8 + ((test-1) * 8));
	
	// Check the pixel color in the framebuffer.
	const uint32_t px = m_vdp->MD_Screen.lineBuf32(y)[x];
	const uint8_t b = (px & 0xFF);
	const uint8_t g = ((px >> 8) & 0xFF);
	const uint8_t r = ((px >> 16) & 0xFF);
	
	// Check for TEST_PASSED.
	if (r < PX_MIN_THRESH && g >= PX_MAX_THRESH && b < PX_MIN_THRESH)
		return TEST_PASSED;
	
	// Check for TEST_FAILED.
	if (r >= PX_MAX_THRESH && g < PX_MIN_THRESH && b < PX_MIN_THRESH)
		return TEST_FAILED;
	
	// Unknown pixel value.
	return TEST_UNKNOWN;
}


/**
 * Print a SpriteTestResult.
 * @param f File to print to.
 * @param result SpriteTestResult.
 */
void Test_SpriteMaskTestRom::PrintSpriteTestResult(FILE *f, SpriteTestResult result)
{
	switch (result)
	{
		case TEST_PASSED:
			PrintPass(f);
			break;
		case TEST_FAILED:
			PrintFail(f);
			break;
		case TEST_UNKNOWN:
		default:
			PrintUnknown(f);
			break;
	}
}


/**
 * Assert a sprite test.
 * @param test Test number.
 * @param testMinMax MIN/MAX/UNBOUNDED.
 * @param expected Expected result.
 * @param actual Actual result.
 */
void Test_SpriteMaskTestRom::assertSpriteTest(int test, TestMinMax testMinMax, SpriteTestResult expected, SpriteTestResult actual)
{
	// NOTE: test is currently unused.
	((void)test);
	
	fprintf(stderr, "   - ");
	if (expected == actual)
	{
		assertPass(NULL);
		PrintPass(stderr);
	}
	else
	{
		assertFail(NULL);
		PrintFail(stderr);
	}
	
	// Bounds string.
	const char *bounds;
	switch (testMinMax)
	{
		case TEST_MIN:
			bounds = "Minimum";
			break;
		case TEST_MAX:
			bounds = "Maximum";
			break;
		case TEST_UNBOUNDED:
		default:
			bounds = "Test";
			break;
	}
	
	// Print the assertion message.
	fprintf(stderr, "%s: expected ", bounds);
	PrintSpriteTestResult(stderr, expected);
	fprintf(stderr, ", got ");
	PrintSpriteTestResult(stderr, actual);
	fprintf(stderr, "\n");
}


/**
 * Run a section of tests.
 * @param screenMode Screen mode. (H32 or H40)
 * @param spriteLimits Indicate if sprite limits are enabled or disabled.
 * @return 0 if tests ran successfully; non-zero if tests could not be run.
 */
int Test_SpriteMaskTestRom::runTestSection(ScreenMode screenMode, SpriteLimits spriteLimits)
{
	int ret;
	
	newSection();
	fprintf(stderr, "Running tests in %s with sprite limits %s:\n",
		((screenMode == SCREEN_MODE_H32) ? "H32 (256x224)" : "H40 (320x224)"),
		((spriteLimits == SPRITE_LIMITS_DISABLED) ? "disabled" : "enabled")
		);
	
	// Load VRam.
	ret = loadVRam(screenMode);
	if (ret != 0)
	{
		// TODO: Assert a failure here.
		return ret;
	}
	
	// Set sprite limits.
	m_vdp->VdpEmuOptions.spriteLimits = (spriteLimits == SPRITE_LIMITS_ENABLED);
	
	// Set the screen mode.
	if (screenMode == SCREEN_MODE_H32)
		m_vdp->Set_Reg(0x0C, 0x00);
	else
		m_vdp->Set_Reg(0x0C, 0x81);
	
	// Set the VRam dirty flag.
	m_vdp->MarkVRamDirty();
	
	// Run the VDP for one frame.
	m_vdp->updateVdpLines(true);
	for (; m_vdp->VDP_Lines.Display.Current < m_vdp->VDP_Lines.Display.Total;
	     m_vdp->VDP_Lines.Display.Current++, m_vdp->VDP_Lines.Visible.Current++)
	{
		m_vdp->Render_Line();
	}
	
	// Verify the results.
	for (int i = 1; i <= 9; i++)
	{
		fprintf(stderr, "%d. %s\n", i, SpriteTestNames[i-1].name);
		
		SpriteTestResult expected, actual;
		
		if (!SpriteTestNames[i-1].isMinMax)
		{
			// Test doesn't have min/max.
			
			// If sprite limits are on, all tests should pass.
			// If sprite limits are off, test 6 should fail; others should pass.
			if (spriteLimits == SPRITE_LIMITS_DISABLED)
				expected = (i == 6 ? TEST_FAILED : TEST_PASSED);
			else
				expected = TEST_PASSED;
			
			actual = checkSpriteTest(i, TEST_UNBOUNDED);
			assertSpriteTest(i, TEST_UNBOUNDED, expected, actual);
		}
		else
		{
			// Test has min/max.
			
			// Run the min test.
			expected = TEST_PASSED;
			actual = checkSpriteTest(i, TEST_MIN);
			assertSpriteTest(i, TEST_MIN, expected, actual);
			
			// Run the max test.
			
			// If sprite limits are on, all tests should passed.
			// If sprite limits are off, tests 1, 2, 3, should fail;
			// test 9 should fail only for H32; others should pass.
			//
			// NOTE: Test 9 doesn't fail in H40 because Gens currently
			// limits sprites to a maximum of 80 regardless of sprite limits.
			// This matches H40's limit. It might change in the future.
			if (spriteLimits == SPRITE_LIMITS_DISABLED)
			{
				if (i == 9 && screenMode == SCREEN_MODE_H32)
					expected = TEST_FAILED;
				else if (i <= 3)
					expected = TEST_FAILED;
				else
					expected = TEST_PASSED;
			}
			else
				expected = TEST_PASSED;
			
			actual = checkSpriteTest(i, TEST_MAX);
			assertSpriteTest(i, TEST_MAX, expected, actual);
		}
	}
	
	// TODO: Assert the test results.
	
	// Tests run successfully.
	return 0;
}


/**
 * Test the Sprite Masking & Overflow Test ROM.
 * @return 0 on success; negative on fatal error; positive if tests failed.
 */
int Test_SpriteMaskTestRom::exec(void)
{
	fprintf(stderr, "LibGens: VDP Sprite Masking & Overflow Test ROM.\n"
			"Original ROM (c) 2009 by Nemesis.\n\n");
	
	// Initialize the VDP.
	m_vdp = new Vdp();
	
	// Set initial registers.
	m_vdp->Set_Reg(0x00, 0x04);	// Enable the palette. (?)
	m_vdp->Set_Reg(0x01, 0x44);	// Enable the display, set Mode 5.
	m_vdp->Set_Reg(0x02, 0x30);	// Set scroll A name table base to 0xC000.
	m_vdp->Set_Reg(0x04, 0x05);	// Set scroll B name table base to 0xA000.
	m_vdp->Set_Reg(0x05, 0x70);	// Set the sprite table base to 0xE000.
	m_vdp->Set_Reg(0x0D, 0x3F);	// Set the HScroll table base to 0xFC00.
	m_vdp->Set_Reg(0x10, 0x01);	// Set the scroll size to V32 H64.
	m_vdp->Set_Reg(0x0F, 0x02);	// Set the auto-increment value to 2.
	
	// Initialize CRam.
	LibGens::VdpPalette *palette = &m_vdp->m_palette;
	palette->setBpp(LibGens::VdpPalette::BPP_32);
	for (size_t i = 0; i < (sizeof(test_spritemask_cram)/sizeof(test_spritemask_cram[0])); i++)
	{
		palette->writeCRam_16((i<<1), test_spritemask_cram[i]);
	}
	
	// Initialize VSRam.
	memset(m_vdp->VSRam.u16, 0x00, sizeof(m_vdp->VSRam.u16));
	
	// Run four test sections.
	if (runTestSection(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED) != 0)
		goto fail;
	if (runTestSection(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED) != 0)
		goto fail;
	if (runTestSection(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED) != 0)
		goto fail;
	if (runTestSection(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED) != 0)
		goto fail;
	
	// Tests are complete.
	// TODO: Print class name.
	testsCompleted();
	return testsFailed();

fail:
	// Tests are complete.
	// TODO: Indicate fatal errors.
	// TODO: Print class name.
	testsCompleted();
	return -1;
}

} }

int main(void)
{
	// TODO: Remove M68K_Mem::ms_SysVersion dependency from LibGens::Vdp.
	LibGens::M68K_Mem::ms_SysVersion.setRegion(LibGens::SysVersion::REGION_US_NTSC);
	
	LibGens::Tests::Test_SpriteMaskTestRom spriteMaskTest;
	int ret = spriteMaskTest.exec();
	return ((ret == 0) ? ret : spriteMaskTest.testsFailed());
}
