/***************************************************************************
 * libcompat/tests: Compatibility Library. (Test Suite)                    *
 * ByteswapTest_benchmark.cpp: ByteswapTest benchmarks.                    *
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

// CPU flags and byteswapping macros.
#include "byteswap.h"
#include "cpuflags.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// Test data.
#include "ByteswapTest_data.h"

namespace LibGens { namespace Tests {

struct ByteswapTest_flags {
	uint32_t cpuFlags;
	uint32_t cpuFlags_slow;

	ByteswapTest_flags(uint32_t cpuFlags, uint32_t cpuFlags_slow)
	{
		this->cpuFlags = cpuFlags;
		this->cpuFlags_slow = cpuFlags_slow;
	}
};

class ByteswapTest_benchmark : public ::testing::TestWithParam<ByteswapTest_flags>
{
	protected:
		ByteswapTest_benchmark()
			: ::testing::TestWithParam<ByteswapTest_flags>() { }
		virtual ~ByteswapTest_benchmark() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		// Previous CPU flags.
		uint32_t cpuFlags_old;
};

/**
 * Set up the test.
 */
void ByteswapTest_benchmark::SetUp(void)
{
	// Verify CPU flags.
	ByteswapTest_flags flags = GetParam();
	uint32_t totalFlags = (flags.cpuFlags | flags.cpuFlags_slow);
	if (flags.cpuFlags != 0) {
		ASSERT_NE(0U, CPU_Flags & totalFlags) <<
			"CPU does not support the required flags for this test.";
	}

	// Check if the CPU flag is slow.
	if (CPU_Flags & flags.cpuFlags_slow) {
		if ((CPU_Flags & totalFlags) == flags.cpuFlags_slow) {
			// CPU is always slow.
			printf("WARNING: CPU is known to be slow with this flag.\n"
			       "Don't rely on the benchmark results.\n");
		} else if ((CPU_Flags & totalFlags) == totalFlags) {
			// CPU may be slow.
			printf("WARNING: CPU may be slow with some instructions provided by this flag.\n");
		}
	}

	cpuFlags_old = CPU_Flags;
	CPU_Flags = flags.cpuFlags;
}

/**
 * Tear down the test.
 */
void ByteswapTest_benchmark::TearDown(void)
{
	CPU_Flags = cpuFlags_old;
}

/**
 * Benchmark 16-bit array byteswapping.
 */
TEST_P(ByteswapTest_benchmark, checkByteSwap16Array)
{
	uint8_t data[516];

	// Run this test 10,000,000 times.
	for (int i = 10000000; i > 0; i--) {
		memcpy(data, ByteswapTest_data_orig, sizeof(data));
		__byte_swap_16_array(data, sizeof(data));
	}
}

/**
 * Benchmark 32-bit array byteswapping.
 */
TEST_P(ByteswapTest_benchmark, checkByteSwap32Array)
{
	uint8_t data[516];

	// Run this test 10,000,000 times.
	for (int i = 10000000; i > 0; i--) {
		memcpy(data, ByteswapTest_data_orig, sizeof(data));
		__byte_swap_32_array(data, sizeof(data));
	}
}

INSTANTIATE_TEST_CASE_P(ByteswapTest_benchmark_NoFlags, ByteswapTest_benchmark,
	::testing::Values(ByteswapTest_flags(0, 0)
));

// NOTE: byteswap.c only implements MMX using GNU assembler.
// TODO: Add some flag to disable non-MMX asm optimizations, e.g. 'bswap'.
#if defined(__GNUC__) && \
    (defined(__i386__) || defined(__amd64__))
INSTANTIATE_TEST_CASE_P(ByteswapTest_benchmark_MMX, ByteswapTest_benchmark,
	::testing::Values(ByteswapTest_flags(MDP_CPUFLAG_X86_MMX, 0)
));
INSTANTIATE_TEST_CASE_P(ByteswapTest_benchmark_SSE2, ByteswapTest_benchmark,
	::testing::Values(ByteswapTest_flags(MDP_CPUFLAG_X86_SSE2, MDP_CPUFLAG_X86_SSE2SLOW)
));
#endif

} }
