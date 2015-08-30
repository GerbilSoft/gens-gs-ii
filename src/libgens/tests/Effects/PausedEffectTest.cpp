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

// C++ includes.
#include <algorithm>

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
			, fb_normal(nullptr)
			, fb_paused(nullptr)
			, fb_test(nullptr) { }
		virtual ~PausedEffectTest() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

		/**
		 * Copy a loaded image to an MdFb in 32-bit color.
		 * Source image must be 32-bit color.
		 * @param fb	[out] Destination MdFb.
		 * @param src	[in] Source img_data.
		 */
		void copyToFb32(MdFb *fb, const Zomg_Img_Data_t *src);

		/**
		 * Compare two framebuffers.
		 * Both framebuffers must have the same color depth.
		 * @param fb_expected	[in] Expected image.
		 * @param fb_actual	[in] Actual image.
		 */
		void compareFb(const MdFb *fb_expected, const MdFb *fb_actual);

	protected:
		Zomg_Img_Data_t img_normal;
		Zomg_Img_Data_t img_paused;

		// Reference framebuffers.
		MdFb *fb_normal;
		MdFb *fb_paused;

		// Test framebuffer.
		// Paused Effect is applied here.
		MdFb *fb_test;
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

	// Allocate the framebuffers.
	fb_normal = new MdFb();
	fb_paused = new MdFb();
	fb_test = new MdFb();

	// Set the framebuffers' color depth.
	// TODO: Parameter.
	fb_normal->setBpp(MdFb::BPP_32);
	fb_normal->setBpp(MdFb::BPP_32);
	fb_normal->setBpp(MdFb::BPP_32);

	// Make sure the images have the correct dimensions.
	EXPECT_EQ(fb_normal->pxPerLine(), (int)img_normal.w);
	EXPECT_EQ(fb_normal->numLines(),  (int)img_normal.h);
	EXPECT_EQ(fb_normal->pxPerLine(), (int)img_paused.w);
	EXPECT_EQ(fb_normal->numLines(),  (int)img_paused.h);

	// Copy the reference images into the framebuffers.
	copyToFb32(fb_normal, &img_normal);
	copyToFb32(fb_paused, &img_paused);
}

/**
 * Tear down the test.
 */
void PausedEffectTest::TearDown(void)
{
	// Free the image data.
	free(img_normal.data);
	free(img_paused.data);

	// Unreference the framebuffers.
	fb_normal->unref();
	fb_paused->unref();
	fb_test->unref();
}

/**
 * Copy a loaded image to an MdFb in 32-bit color.
 * Source image must be 32-bit color.
 * @param fb	[out] Destination MdFb.
 * @param src	[in] Source img_data.
 */
void PausedEffectTest::copyToFb32(MdFb *fb, const Zomg_Img_Data_t *src)
{
	ASSERT_EQ(32, src->bpp);

	// Set the MdFb to 32-bit color.
	fb->setBpp(MdFb::BPP_32);

	// Copy the image to the MdFb.
	const uint32_t *pData = (const uint32_t*)src->data;
	for (int line = 0; line < fb->numLines(); line++) {
		memcpy(fb->lineBuf32(line), pData, src->pitch);
		pData += (src->pitch / 4);
	}
}

/**
 * Compare two framebuffers.
 * Both framebuffers must have the same color depth.
 * @param fb_expected	[in] Expected image.
 * @param fb_actual	[in] Actual image.
 */
void PausedEffectTest::compareFb(const MdFb *fb_expected, const MdFb *fb_actual)
{
	// NOTE: The full pitch is checked, not just the
	// actual image data. (336 of 336)
	ASSERT_EQ(fb_expected->bpp(),       fb_actual->bpp());
	ASSERT_EQ(fb_expected->pxPerLine(), fb_actual->pxPerLine());
	ASSERT_EQ(fb_expected->numLines(),  fb_actual->numLines());

	// Use the minimum pitch between the two framebuffers.
	int pitch = std::min(fb_expected->pxPitch(), fb_actual->pxPitch());

	if (fb_expected->bpp() == MdFb::BPP_32) {
		pitch *= 4;
		for (int line = 0; line < fb_expected->numLines(); line++) {
			EXPECT_EQ(0, memcmp(fb_expected->lineBuf32(line),
					    fb_actual->lineBuf32(line), pitch)) <<
				"Line " << line << " did not match the reference image.";
		}
	} else {
		pitch *= 2;
		for (int line = 0; line < fb_expected->numLines(); line++) {
			EXPECT_EQ(0, memcmp(fb_expected->lineBuf16(line),
					    fb_actual->lineBuf16(line), pitch)) <<
				"Line " << line << " did not match the reference image.";
		}
	}
}

/**
 * Test the Paused Effect in 32-bit color.
 */
TEST_F(PausedEffectTest, do32bit)
{
	// Initialize the test framebuffer with the "normal" image.
	copyToFb32(fb_test, &img_normal);

	// Apply the "paused" effect. (1-FB version)
	// TODO: Test 2-FB version as well.
	PausedEffect::DoPausedEffect(fb_test);

	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test);
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
