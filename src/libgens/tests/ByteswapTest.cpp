/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * ByteswapTest.cpp: Byteswapping tests.                                   *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

class ByteswapTest : public ::testing::Test
{
	protected:
		ByteswapTest()
			: ::testing::Test() { }
		virtual ~ByteswapTest() { }

		virtual void SetUp(void) { }
		virtual void TearDown(void) { }

	protected:
		static const char *ByteorderString(int byteorder);
		static int CheckRuntimeByteorder(void);
};

/**
 * Get a string representation of a given Gens byteorder.
 * @param byteorder Gens byteorder.
 * @return String representation.
 */
const char *ByteswapTest::ByteorderString(int byteorder)
{
	switch (byteorder) {
		case GENS_LIL_ENDIAN:
			return "little-endian";
		case GENS_BIG_ENDIAN:
			return "big-endian";
		case GENS_PDP_ENDIAN:
			return "PDP-endian";
		default:
			return "Unknown";
	}
}

/**
 * Check the runtime byteorder.
 * @return Runtime byteorder, or 0 on error.
 */
int ByteswapTest::CheckRuntimeByteorder(void)
{
	// Determine the run-time byte ordering.
	static const uint32_t boCheck_32 = 0x12345678;
	static const uint8_t boCheck_LE[4] = {0x78, 0x56, 0x34, 0x12};
	static const uint8_t boCheck_BE[4] = {0x12, 0x34, 0x56, 0x78};
	static const uint8_t boCheck_PDP[4] = {0x34, 0x12, 0x78, 0x56};

	union DtoB
	{
		uint32_t d;
		uint8_t b[4];
	};
	DtoB byteorder_check;
	byteorder_check.d = boCheck_32;

	if (!memcmp(byteorder_check.b, boCheck_LE, sizeof(byteorder_check.b)))
		return GENS_LIL_ENDIAN;
	else if (!memcmp(byteorder_check.b, boCheck_BE, sizeof(byteorder_check.b)))
		return GENS_BIG_ENDIAN;
	else if (!memcmp(byteorder_check.b, boCheck_PDP, sizeof(byteorder_check.b)))
		return GENS_PDP_ENDIAN;

	// Unknown byteorder.
	return 0;
}

/** Test cases. **/

/**
 * Check that the runtime byteorder matches the compile-time byteorder.
 */
TEST_F(ByteswapTest, checkRuntimeByteorder)
{
	// Print the compile-time byte ordering.
	const int byteorder_compiled = GENS_BYTEORDER;
	const char *byteorder_compiled_str = ByteorderString(byteorder_compiled);
	fprintf(stderr, "Compile-time byteorder: %s\n", byteorder_compiled_str);

	// Determine the run-time byte ordering.
	const int byteorder_runtime = CheckRuntimeByteorder();
	const char *byteorder_runtime_str = ByteorderString(byteorder_runtime);
	fprintf(stderr, "Run-time byteorder: %s\n", byteorder_runtime_str);

	// Make sure the byteorders are equivalent.
	ASSERT_EQ(byteorder_compiled, byteorder_runtime);
}

/**
 * Check little-endian byteswapping macros (non-array).
 */
TEST_F(ByteswapTest, checkLittleEndianMacros)
{
	const uint16_t orig16 = 0x1234;
	const uint32_t orig32 = 0x12345678;
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
	const uint16_t swap16 = 0x1234;
	const uint32_t swap32 = 0x12345678;
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
	const uint16_t swap16 = 0x3412;
	const uint32_t swap32 = 0x78563412;
#endif

	// 16-bit.
	// NOTE: Must use explicit cast to uint16_t to prevent errors.
	// TODO: Add explicit cast in byteswap.h, or use (& 0xFFFF)?
	EXPECT_EQ(swap16, (uint16_t)le16_to_cpu(orig16));
	EXPECT_EQ(swap16, (uint16_t)cpu_to_le16(orig16));

	// 32-bit.
	EXPECT_EQ(swap32, le32_to_cpu(orig32));
	EXPECT_EQ(swap32, cpu_to_le32(orig32));
}

/**
 * Check big-endian byteswapping macros (non-array).
 */
TEST_F(ByteswapTest, checkBigEndianMacros)
{
	const uint16_t orig16 = 0x1234;
	const uint32_t orig32 = 0x12345678;
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
	const uint16_t swap16 = 0x3412;
	const uint32_t swap32 = 0x78563412;
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
	const uint16_t swap16 = 0x1234;
	const uint32_t swap32 = 0x12345678;
#endif

	// 16-bit.
	// NOTE: Must use explicit cast to uint16_t to prevent errors.
	// TODO: Add explicit cast in byteswap.h, or use (& 0xFFFF)?
	EXPECT_EQ(swap16, (uint16_t)be16_to_cpu(orig16));
	EXPECT_EQ(swap16, (uint16_t)cpu_to_be16(orig16));

	// 32-bit.
	EXPECT_EQ(swap32, be32_to_cpu(orig32));
	EXPECT_EQ(swap32, cpu_to_be32(orig32));
}

/**
 * Test 16-bit array byteswapping.
 */
TEST_F(ByteswapTest, checkByteSwap16Array)
{
	uint8_t data[516];
	memcpy(data, ByteswapTest_data_orig, sizeof(data));
	__byte_swap_16_array(data, sizeof(data));
	ASSERT_EQ(0, memcmp(data, ByteswapTest_data_swap16, sizeof(data)));
}

/**
 * Test 16-bit array byteswapping.
 */
TEST_F(ByteswapTest, checkByteSwap32Array)
{
	uint8_t data[516];
	memcpy(data, ByteswapTest_data_orig, sizeof(data));
	__byte_swap_32_array(data, sizeof(data));
	ASSERT_EQ(0, memcmp(data, ByteswapTest_data_swap32, sizeof(data)));
}

} }

int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: Byteswapping tests.\n\n");

	::testing::InitGoogleTest(&argc, argv);
	// NOTE: Array byteswap functions support MMX,
	// so we need to initialize LibGens' CPU_Flags variable.
	LibGens::Init();
	return RUN_ALL_TESTS();
}
