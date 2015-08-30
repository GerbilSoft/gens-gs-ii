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

#include "PausedEffectTest.hpp"

// LibGens
#include "lg_main.hpp"
#include "Effects/PausedEffect.hpp"

// LibZomg
#include "libzomg/PngReader.hpp"
using LibZomg::PngReader;

// LibCompat
#include "libcompat/cpuflags.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace LibGens { namespace Tests {

/**
 * Get the base filename for the output reference images.
 * @return Base filename, e.g. "PausedEffect".
 */
const char *PausedEffectTest::baseFilename(void)
{
	return "PausedEffect";
}

/**
 * Test the Paused Effect in 15-bit color. (1-FB)
 */
TEST_P(PausedEffectTest, do15bit_1FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_15));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb15(fb_test1, &img_normal));
	// Apply the "paused" effect. (1-FB version)
	PausedEffect::DoPausedEffect(fb_test1);
	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test1);
}

/**
 * Test the Paused Effect in 15-bit color. (2-FB)
 */
TEST_P(PausedEffectTest, do15bit_2FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_15));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb15(fb_test1, &img_normal));
	// Apply the "paused" effect. (2-FB version)
	PausedEffect::DoPausedEffect(fb_test2, fb_test1);
	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test2);
}

/**
 * Test the Paused Effect in 16-bit color. (1-FB)
 */
TEST_P(PausedEffectTest, do16bit_1FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_16));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb16(fb_test1, &img_normal));
	// Apply the "paused" effect. (1-FB version)
	PausedEffect::DoPausedEffect(fb_test1);
	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test1);
}

/**
 * Test the Paused Effect in 16-bit color. (2-FB)
 */
TEST_P(PausedEffectTest, do16bit_2FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_16));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb16(fb_test1, &img_normal));
	// Apply the "paused" effect. (2-FB version)
	PausedEffect::DoPausedEffect(fb_test2, fb_test1);
	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test2);
}

/**
 * Test the Paused Effect in 32-bit color. (1-FB)
 */
TEST_P(PausedEffectTest, do32bit_1FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_32));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb32(fb_test1, &img_normal));
	// Apply the "paused" effect. (1-FB version)
	PausedEffect::DoPausedEffect(fb_test1);
	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test1);
}

/**
 * Test the Paused Effect in 32-bit color. (2-FB)
 */
TEST_P(PausedEffectTest, do32bit_2FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_32));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb32(fb_test1, &img_normal));
	// Apply the "paused" effect. (2-FB version)
	PausedEffect::DoPausedEffect(fb_test2, fb_test1);
	// Compare it to the known good "paused" image.
	compareFb(fb_paused, fb_test2);
}

INSTANTIATE_TEST_CASE_P(PausedEffectTest_NoFlags, PausedEffectTest,
	::testing::Values(EffectTest_flags(0, 0)
));

// NOTE: PausedEffect.cpp only implements MMX/SSE2 using GNU assembler.
#if defined(__GNUC__) && \
    (defined(__i386__) || defined(__amd64__))
INSTANTIATE_TEST_CASE_P(PausedEffectTest_MMX, PausedEffectTest,
	::testing::Values(EffectTest_flags(MDP_CPUFLAG_X86_MMX, 0)
));
INSTANTIATE_TEST_CASE_P(PausedEffectTest_SSE2, PausedEffectTest,
	::testing::Values(EffectTest_flags(MDP_CPUFLAG_X86_SSE2, MDP_CPUFLAG_X86_SSE2SLOW)
));
#endif

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
