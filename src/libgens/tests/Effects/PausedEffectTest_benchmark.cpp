/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * PausedEffectTest_benchmark.cpp: Paused Effect benchmarks.               *
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

// LibGens, LibCompat
#include "Effects/PausedEffect.hpp"
#include "libcompat/cpuflags.h"

namespace LibGens { namespace Tests {

class PausedEffectTest_benchmark : public PausedEffectTest
{
	protected:
		PausedEffectTest_benchmark()
			: PausedEffectTest() { }
		virtual ~PausedEffectTest_benchmark() { }

	protected:
		// Run benchmark loops 2,000 times.
		static const int BENCHMARK_ITERATIONS = 2000;
};

/**
 * Benchmark the Paused Effect in 15-bit color. (1-FB)
 */
TEST_P(PausedEffectTest_benchmark, do15bit_1FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_15));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb15(fb_test1, &img_normal));

	// Run this test BENCHMARK_ITERATIONS times.
	const uint32_t fb_sz = fb_test1->pxPitch() * fb_test1->numLines() * 2;
	for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
		// Copy the initialized framebuffer to fb_test2.
		memcpy(fb_test2->fb16(), fb_test1->fb16(), fb_sz);
		// Apply the "paused" effect. (1-FB version)
		PausedEffect::DoPausedEffect(fb_test2);
	}
}

/**
 * Benchmark the Paused Effect in 15-bit color. (2-FB)
 */
TEST_P(PausedEffectTest_benchmark, do15bit_2FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_15));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb15(fb_test1, &img_normal));

	// Run this test BENCHMARK_ITERATIONS times.
	for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
		// Apply the "paused" effect. (2-FB version)
		PausedEffect::DoPausedEffect(fb_test2, fb_test1);
	}
}

/**
 * Benchmark the Paused Effect in 16-bit color. (1-FB)
 */
TEST_P(PausedEffectTest_benchmark, do16bit_1FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_16));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb16(fb_test1, &img_normal));

	// Run this test BENCHMARK_ITERATIONS times.
	const uint32_t fb_sz = fb_test1->pxPitch() * fb_test1->numLines() * 2;
	for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
		// Copy the initialized framebuffer to fb_test2.
		memcpy(fb_test2->fb16(), fb_test1->fb16(), fb_sz);
		// Apply the "paused" effect. (1-FB version)
		PausedEffect::DoPausedEffect(fb_test2);
	}
}

/**
 * Benchmark the Paused Effect in 16-bit color. (2-FB)
 */
TEST_P(PausedEffectTest_benchmark, do16bit_2FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_16));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb16(fb_test1, &img_normal));

	// Run this test BENCHMARK_ITERATIONS times.
	for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
		// Apply the "paused" effect. (2-FB version)
		PausedEffect::DoPausedEffect(fb_test2, fb_test1);
	}
}

/**
 * Benchmark the Paused Effect in 32-bit color. (1-FB)
 */
TEST_P(PausedEffectTest_benchmark, do32bit_1FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_32));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb32(fb_test1, &img_normal));

	// Run this test BENCHMARK_ITERATIONS times.
	const uint32_t fb_sz = fb_test1->pxPitch() * fb_test1->numLines() * 4;
	for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
		// Copy the initialized framebuffer to fb_test2.
		memcpy(fb_test2->fb32(), fb_test1->fb32(), fb_sz);
		// Apply the "paused" effect. (1-FB version)
		PausedEffect::DoPausedEffect(fb_test2);
	}
}

/**
 * Benchmark the Paused Effect in 32-bit color. (2-FB)
 */
TEST_P(PausedEffectTest_benchmark, do32bit_2FB)
{
	// Initialize the images.
	ASSERT_NO_FATAL_FAILURE(init(MdFb::BPP_32));
	// Initialize the test framebuffer with the "normal" image.
	ASSERT_NO_FATAL_FAILURE(copyToFb32(fb_test1, &img_normal));

	// Run this test BENCHMARK_ITERATIONS times.
	for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
		// Apply the "paused" effect. (2-FB version)
		PausedEffect::DoPausedEffect(fb_test2, fb_test1);
	}
}

INSTANTIATE_TEST_CASE_P(PausedEffectTest_benchmark_NoFlags, PausedEffectTest_benchmark,
	::testing::Values(EffectTest_flags(0, 0)
));

// NOTE: PausedEffect.cpp only implements MMX/SSE2 using GNU assembler.
#if defined(__GNUC__) && \
    (defined(__i386__) || defined(__amd64__) || defined(__x86_64__))
INSTANTIATE_TEST_CASE_P(PausedEffectTest_benchmark_MMX, PausedEffectTest_benchmark,
	::testing::Values(EffectTest_flags(MDP_CPUFLAG_X86_MMX, 0)
));
INSTANTIATE_TEST_CASE_P(PausedEffectTest_benchmark_SSE2, PausedEffectTest_benchmark,
	::testing::Values(EffectTest_flags(MDP_CPUFLAG_X86_SSE2, MDP_CPUFLAG_X86_SSE2SLOW)
));
#endif

} }
