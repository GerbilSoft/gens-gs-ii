/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * VdpSpriteMaskingTest.cpp: Sprite Masking & Overflow Test ROM.           *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

// Google Test
#include "gtest/gtest.h"

// LibGens.
#include "lg_main.hpp"

// LibGens VDP.
#include "Vdp/Vdp.hpp"
#include "cpu/M68K_Mem.hpp"
#include "Util/byteswap.h"

// ARRAY_SIZE(x)
#include "macros/common.h"

// Test ROM data.
#include "VdpSpriteMaskingTest_data.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
using namespace std;

// ZLib.
#define CHUNK 4096
#include <zlib.h>

namespace LibGens { namespace Tests {

enum ScreenMode {
	SCREEN_MODE_H32 = 0,
	SCREEN_MODE_H40 = 1,
};

enum SpriteLimits {
	SPRITE_LIMITS_DISABLED = 0,
	SPRITE_LIMITS_ENABLED  = 1,
};

struct VdpSpriteMaskingTest_mode
{
	ScreenMode screenMode;
	SpriteLimits spriteLimits;
	int test;	// Test number. (1-9)

	VdpSpriteMaskingTest_mode()
		: screenMode(SCREEN_MODE_H32)
		, spriteLimits(SPRITE_LIMITS_DISABLED)
		, test(0) { }

	VdpSpriteMaskingTest_mode(ScreenMode screenMode, SpriteLimits spriteLimits, int test)
		: screenMode(screenMode)
		, spriteLimits(spriteLimits)
		, test(test) { }
};

class VdpSpriteMaskingTest : public ::testing::TestWithParam<VdpSpriteMaskingTest_mode>
{
	protected:
		VdpSpriteMaskingTest()
			: m_vdp(nullptr) { }
		virtual ~VdpSpriteMaskingTest() { }

		virtual void SetUp(void);
		virtual void TearDown(void);

	protected:
		Vdp *m_vdp;

		int loadVRam(ScreenMode screenMode);

	public:
		/**
		 * Channel value must be < this number
		 * to be considered "off".
		 */
		static const uint8_t PX_MIN_THRESH = 0x20;

		/**
		 * Channel value must be >= this number
		 * to be considered "on".
		 */
		static const uint8_t PX_MAX_THRESH = 0xE0;

	public:
		enum SpriteTestResult {
			TEST_PASSED = 0,
			TEST_FAILED = 1,
			TEST_UNKNOWN = 2,
		};

		enum TestMinMax {
			TEST_UNBOUNDED = 0,	// Test is unbounded. (equivalent to TEST_MIN)
			TEST_MIN = 1,		// Test minimum bounds.
			TEST_MAX = 2,		// Test maximum bounds.
		};

		struct TestNames {
			const char *name;
			bool isMinMax;
		};

		static const TestNames SpriteTestNames[9];

	protected:
		SpriteTestResult checkSpriteTest(int test, TestMinMax testMinMax);
};

const VdpSpriteMaskingTest::TestNames VdpSpriteMaskingTest::SpriteTestNames[9] =
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
 * Formatting function for VdpSpriteMaskingTest.
 * NOTE: This makes use of VdpSpriteMaskingTest::SpriteTestNames[],
 * so it must be defined after VdpSpriteMaskingTest is declared.
 */
inline ::std::ostream& operator<<(::std::ostream& os, const VdpSpriteMaskingTest_mode& mode) {
	return os << "Screen Mode "
		<< (mode.screenMode == SCREEN_MODE_H32 ? "H32" : "H40")
		<< ", Sprite Limits "
		<< (mode.spriteLimits == SPRITE_LIMITS_DISABLED ? "OFF" : "ON")
		<< ", Test "
		<< mode.test
		<< ": "
		<< VdpSpriteMaskingTest::SpriteTestNames[mode.test-1].name;
};


/**
 * Formatting function for SpriteTestResult.
 */
inline ::std::ostream& operator<<(::std::ostream& os, const VdpSpriteMaskingTest::SpriteTestResult& result) {
	switch (result) {
		case VdpSpriteMaskingTest::TEST_PASSED:
			return os << "Passed";
		case VdpSpriteMaskingTest::TEST_FAILED:
			return os << "Failed";
		case VdpSpriteMaskingTest::TEST_UNKNOWN:
		default:
			return os << "Unknown";
	}

	// Should not get here...
	return os << "Unknown";
}

/**
 * Set up the Vdp for testing.
 */
void VdpSpriteMaskingTest::SetUp(void)
{
	// Initialize the VDP.
	m_vdp = new Vdp();
	m_vdp->setNtsc();

	// Set initial registers.
	m_vdp->dbg_setReg(0x00, 0x04);	// Enable the palette. (?)
	m_vdp->dbg_setReg(0x01, 0x44);	// Enable the display, set Mode 5.
	m_vdp->dbg_setReg(0x02, 0x30);	// Set scroll A name table base to 0xC000.
	m_vdp->dbg_setReg(0x04, 0x05);	// Set scroll B name table base to 0xA000.
	m_vdp->dbg_setReg(0x05, 0x70);	// Set the sprite table base to 0xE000.
	m_vdp->dbg_setReg(0x0D, 0x3F);	// Set the HScroll table base to 0xFC00.
	m_vdp->dbg_setReg(0x10, 0x01);	// Set the scroll size to V32 H64.
	m_vdp->dbg_setReg(0x0F, 0x02);	// Set the auto-increment value to 2.

	// Initialize CRam.
	// FIXME: Needs to be byteswapped?
	m_vdp->dbg_writeCRam_16(0, test_spritemask_cram, ARRAY_SIZE(test_spritemask_cram));
	// FIXME: Move MD_Screen out of m_vdp.
	m_vdp->MD_Screen->setBpp(LibGens::MdFb::BPP_32);

	// Initialize VSRam.
	uint16_t vsblock[40];
	memset(vsblock, 0, sizeof(vsblock));
	m_vdp->dbg_writeVSRam_16(0, vsblock, ARRAY_SIZE(vsblock));

	// Determine the parameters for this test.
	VdpSpriteMaskingTest_mode mode = GetParam();

	// Set sprite limits.
	m_vdp->options.spriteLimits =
		(mode.spriteLimits == SPRITE_LIMITS_ENABLED);

	// Set the screen mode.
	const uint8_t reg0C = (mode.screenMode == SCREEN_MODE_H32) ? 0x00 : 0x81;
	m_vdp->dbg_setReg(0x0C, reg0C);

	// Load VRam.
	ASSERT_EQ(0, loadVRam(mode.screenMode))
		<< "Load VRAM for screen mode "
		<< (mode.screenMode == SCREEN_MODE_H32 ? "H32" : "H40");
}

/**
 * Tear down the Vdp.
 */
void VdpSpriteMaskingTest::TearDown(void)
{
	delete m_vdp;
	m_vdp = nullptr;
}

/**
 * Load VRam.
 * @param screenMode Screen mode.
 * @return 0 on success; non-zero on error.
 */
int VdpSpriteMaskingTest::loadVRam(ScreenMode screenMode)
{
	// Based on zlib example code:
	// http://www.zlib.net/zlib_how.html
	int ret;
	z_stream strm;

	// VRAM buffer. (slightly more than 64 KB)
	// TODO: Use H40 or H32 depending on test mode?
	// Then again, it's the same regardless...
	const unsigned int buf_siz = test_spritemask_vram_h40_sz;
	const unsigned int out_len = buf_siz + 64;
	uint8_t *out = (uint8_t*)malloc(out_len);
	unsigned int out_pos = 0;

	// Data to decode.
	unsigned int in_pos = 0;
	const uint8_t *in;
	unsigned int in_len;
	if (screenMode == SCREEN_MODE_H40) {
		in = test_spritemask_vram_h40;
		in_len = sizeof(test_spritemask_vram_h40);
	} else {
		in = test_spritemask_vram_h32;
		in_len = sizeof(test_spritemask_vram_h32);
	}

	// Allocate the zlib inflate state.
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, 15+16);
	if (ret != Z_OK) {
		free(out);
		return ret;
	}

	// Decompress the stream.
	unsigned int avail_out_before;
	unsigned int avail_out_after;
	do {
		if (in_pos >= in_len)
			break;
		strm.avail_in = (in_len - in_pos);
		strm.next_in = &in[in_pos];

		// Run inflate() on input until the output buffer is not full.
		do {
			avail_out_before = (out_len - out_pos);
			strm.avail_out = avail_out_before;
			strm.next_out = &out[out_pos];

			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);	// make sure the state isn't clobbered
			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
					// fall through
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
				case Z_STREAM_ERROR:
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
	if (ret != Z_STREAM_END) {
		free(out);
		return Z_DATA_ERROR;
	}

	// VRAM data is 64 KB.
	if (out_pos != buf_siz) {
		free(out);
		return Z_DATA_ERROR;
	}

	// First two bytes of both VRAM dumps is 0xDD.
	if (out[0] != 0xDD || out[1] != 0xDD) {
		free(out);
		return Z_DATA_ERROR;
	}

	// Data was read successfully.

	// Byteswap VRam to host-endian.
	be16_to_cpu_array(out, out_pos);

	// Copy VRam to the VDP.
	m_vdp->dbg_writeVRam_16(0, (uint16_t*)out, out_pos);

	// VRam loaded.
	free(out);
	return 0;
}

/**
 * Check a sprite test using screen scraping.
 * @param test Test number. (1-9)
 * @param testMinMax Select whether to test minimum or maximum bounds.
 * @return Sprite test result.
 */
VdpSpriteMaskingTest::SpriteTestResult VdpSpriteMaskingTest::checkSpriteTest(int test, TestMinMax testMinMax)
{
	if (test < 1 || test > 9)
		return TEST_UNKNOWN;

	// X position: min == 216, max == 232
	// Add HPixBegin() for H32 mode.
	const int x = ((testMinMax <= TEST_MIN ? 216 : 232) + m_vdp->getHPixBegin());

	// Y position: 48+8+((test-1)*8)
	const int y = (48 + 8 + ((test-1) * 8));

	// Check the pixel color in the framebuffer.
	const uint32_t px = m_vdp->MD_Screen->lineBuf32(y)[x];
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
 * Run a sprite test.
 */
TEST_P(VdpSpriteMaskingTest, spriteMaskingTest)
{
	VdpSpriteMaskingTest_mode mode = GetParam();

	// Run the VDP for one frame.
	m_vdp->updateVdpLines(true);
	for (; m_vdp->VDP_Lines.currentLine < m_vdp->VDP_Lines.totalDisplayLines;
	     m_vdp->VDP_Lines.currentLine++)
	{
		m_vdp->renderLine();
	}

	// Check the test.
	SpriteTestResult expected, actual;

	if (!SpriteTestNames[mode.test-1].isMinMax) {
		// Test doesn't have min/max.
		
		// If sprite limits are on, all tests should pass.
		// If sprite limits are off, test 6 should fail; others should pass.
		if (mode.spriteLimits == SPRITE_LIMITS_DISABLED)
			expected = (mode.test == 6 ? TEST_FAILED : TEST_PASSED);
		else
			expected = TEST_PASSED;

		actual = checkSpriteTest(mode.test, TEST_UNBOUNDED);
		EXPECT_EQ(expected, actual)
			<< "Test " << mode.test << ": " << SpriteTestNames[mode.test-1].name;
	} else {
		// Test has min/max.

		// Run the min test.
		expected = TEST_PASSED;
		actual = checkSpriteTest(mode.test, TEST_MIN);
		EXPECT_EQ(expected, actual)
			<< "Test " << mode.test << ": " << SpriteTestNames[mode.test-1].name << " (min)";

		// Run the max test.

		// If sprite limits are on, all tests should passed.
		// If sprite limits are off, tests 1, 2, 3, should fail;
		// others should pass.
		//
		// NOTE: Test 9 previously failed in H32 because Gens allowed
		// up to 80 sprites per frame as a hard limit. The SAT cache
		// uses the sprite table mask, so it's impossible to write to
		// any sprite over 63 in H32 mode now.
		if (mode.spriteLimits == SPRITE_LIMITS_DISABLED) {
			if (mode.test <= 3)
				expected = TEST_FAILED;
			else
				expected = TEST_PASSED;
		} else {
			expected = TEST_PASSED;
		}

		actual = checkSpriteTest(mode.test, TEST_MAX);
		EXPECT_EQ(expected, actual)
			<< "Test " << mode.test << ": " << SpriteTestNames[mode.test-1].name << " (max)";
	}
}

// Test cases.
// NOTE: Test case numbers start with 0 in Google Test.
// TODO: Add a dummy test 0?

// Screen mode H32, no sprite limit.
INSTANTIATE_TEST_CASE_P(ScreenH32NoLimit, VdpSpriteMaskingTest,
	::testing::Values(
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 1),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 2),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 3),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 4),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 5),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 6),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 7),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 8),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_DISABLED, 9)
		));

// Screen mode H32, sprite limit.
INSTANTIATE_TEST_CASE_P(ScreenH32SpriteLimit, VdpSpriteMaskingTest,
	::testing::Values(
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 1),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 2),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 3),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 4),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 5),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 6),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 7),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 8),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H32, SPRITE_LIMITS_ENABLED, 9)
		));

// Screen mode H40, no sprite limit.
INSTANTIATE_TEST_CASE_P(ScreenH40NoLimit, VdpSpriteMaskingTest,
	::testing::Values(
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 1),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 2),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 3),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 4),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 5),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 6),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 7),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 8),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_DISABLED, 9)
		));

// Screen mode H40, sprite limit.
INSTANTIATE_TEST_CASE_P(ScreenH40SpriteLimit, VdpSpriteMaskingTest,
	::testing::Values(
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 1),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 2),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 3),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 4),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 5),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 6),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 7),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 8),
		VdpSpriteMaskingTest_mode(SCREEN_MODE_H40, SPRITE_LIMITS_ENABLED, 9)
		));

} }

int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: VDP Sprite Masking & Overflow tests.\n\n");

	::testing::InitGoogleTest(&argc, argv);
	LibGens::Init();
	fprintf(stderr, "\n");
	fprintf(stderr, "LibGens: VDP Sprite Masking & Overflow Test ROM.\n"
			"Original ROM (c) 2009 by Nemesis.\n\n");
	fflush(nullptr);

	int ret = RUN_ALL_TESTS();
	LibGens::End();
	return ret;
}
