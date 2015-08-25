/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
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

#include "lg_main.hpp"
#include "Util/byteswap.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>

// Test data.
#include "ByteswapTest_data.h"

namespace LibGens { namespace Tests {

class ByteswapTest_benchmark : public ::testing::Test
{
	protected:
		ByteswapTest_benchmark()
			: ::testing::Test() { }
		virtual ~ByteswapTest_benchmark() { }
};

/**
 * Benchmark 16-bit array byteswapping.
 */
TEST_F(ByteswapTest_benchmark, checkByteSwap16Array)
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
TEST_F(ByteswapTest_benchmark, checkByteSwap32Array)
{
	uint8_t data[516];

	// Run this test 10,000,000 times.
	for (int i = 10000000; i > 0; i--) {
		memcpy(data, ByteswapTest_data_orig, sizeof(data));
		__byte_swap_32_array(data, sizeof(data));
	}
}

} }
