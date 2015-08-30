/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * PausedEffectTest.cpp: Paused Effect test.                               *
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

// Google Test
#include "gtest/gtest.h"

// LibGens
#include "lg_main.hpp"
#include "Util/MdFb.hpp"
#include "Effects/PausedEffect.hpp"

// LibZomg
#include "libzomg/PngReader.hpp"
#include "libzomg/img_data.h"
using LibZomg::PngReader;

// LibCompat
#include "libcompat/cpuflags.h"
#include "libcompat/aligned_malloc.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <cstring>

// TODO: More sample data:
// 48,000 Hz @ 50 Hz (960 samples)
// 44,100 Hz @ 60 Hz (735 samples)
// 44,100 Hz @ 50 Hz (882 samples)
// The latter two are important because they test data blocks
// that aren't multiples of 8 or 16 bytes. (MMX/SSE2)

namespace LibGens { namespace Tests {

class PausedEffectTest : public ::testing::Test
{
	protected:
		PausedEffectTest()
			: ::testing::Test()
			, fb(nullptr) { }
		virtual ~PausedEffectTest() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		Zomg_Img_Data_t img_normal;
		Zomg_Img_Data_t img_paused;

		MdFb *fb;
};

/**
 * Set up the test.
 */
void PausedEffectTest::SetUp(void)
{
	// Clear the image data initially.
	memset(&img_normal, 0, sizeof(img_normal));
	memset(&img_paused, 0, sizeof(img_paused));

	// Load the images.
	// TODO: Use a parameter for bpp.
	PngReader reader;
	int ret = reader.readFromFile(&img_normal,
			"PausedEffect.Normal.32.png",
			PngReader::RF_INVERTED_ALPHA);
	ASSERT_EQ(0, ret) << "Error loading \"PausedEffect.Normal.32.png\": " << strerror(-ret);
	ret = reader.readFromFile(&img_paused,
			"PausedEffect.SW.32.png",
			PngReader::RF_INVERTED_ALPHA);
	ASSERT_EQ(0, ret) << "Error loading \"PausedEffect.SW.32.png\": " << strerror(-ret);

	// Allocate an MdFb.
	fb = new MdFb();
	// TODO: Parameter.
	fb->setBpp(MdFb::BPP_32);

	// Make sure the images have the correct dimensions.
	EXPECT_EQ(fb->pxPerLine(), (int)img_normal.w);
	EXPECT_EQ(fb->numLines(),  (int)img_normal.h);
	EXPECT_EQ(fb->pxPerLine(), (int)img_paused.w);
	EXPECT_EQ(fb->numLines(),  (int)img_paused.h);
}

/**
 * Tear down the test.
 */
void PausedEffectTest::TearDown(void)
{
	// Free the image data.
	free(img_normal.data);
	free(img_paused.data);

	// Unreference the MdFb.
	fb->unref();
}

/**
 * Test the Paused Effect in 32-bit color.
 */
TEST_F(PausedEffectTest, do32bit)
{
	// Copy the "normal" image into an MdFb.
	// TODO: Convert to 15/16-bit color if necessary.
	const uint32_t *src = (const uint32_t*)img_normal.data;
	for (int line = 0; line < fb->numLines(); line++) {
		memcpy(fb->lineBuf32(line), src, img_normal.pitch);
		src += (img_normal.pitch / 4);
	}

	// Apply the "paused" effect. (1-FB version)
	// TODO: Test 2-FB version as well.
	PausedEffect::DoPausedEffect(fb);

	// Compare it to the known good "paused" image.
	// NOTE: Only the actual image is checked;
	// the 8px borders are ignored. (320 of 336)
	// TODO: Do we need to clear the high bytes in img_paused?
	src = (const uint32_t*)img_paused.data;
	for (int line = 0; line < fb->numLines(); line++) {
		EXPECT_EQ(0, memcmp(fb->lineBuf32(line), src, img_paused.pitch)) <<
			"Line " << line << " did not match the reference image.";
		src += (img_normal.pitch / 4);
	}
	
#if 0	
	int ret = SoundMgr::writeStereo(buf, samples);
	ASSERT_EQ(samples, ret);

	// Verify the data.
	const int16_t *expected = PausedEffectTest_Output_Stereo;
	for (int i = 0; i < samples*2; i++) {
		EXPECT_EQ(expected[i], buf[i]) <<
			"Output sample " << i << " should be " <<
			std::hex << std::uppercase <<
			std::setfill('0') << std::setw(4) <<
			expected[i] << ", but was " << buf[i];
	}
#endif
}

} }

/**
 * Test suite main function.
 * Called by gtest_main.inc.cpp's main().
 */
static int test_main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: Paused Effect test.\n\n");
	LibGens::Init();
	fflush(nullptr);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

#include "libcompat/tests/gtest_main.inc.cpp"
