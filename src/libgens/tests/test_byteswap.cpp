/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * test_byteswap.cpp: Byteswapping tests.                                  *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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
#include "Util/byteswap.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
using namespace std;

// C includes.
#include <unistd.h>

namespace LibGens { namespace Tests {

class Test_Byteswap : public TestSuite
{
	public:
		int exec(void);
	
	protected:
		const char *byteorderString(int byteorder);
		
		union DtoB
		{
			uint32_t d;
			uint8_t b[4];
		};
		
		int checkRuntimeByteorder(void);
};

// TODO: Templated version!

/**
 * Get a string representation of a given Gens byteorder.
 * @param byteorder Gens byteorder.
 * @return String representation.
 */
const char *Test_Byteswap::byteorderString(int byteorder)
{
	switch (byteorder)
	{
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
int Test_Byteswap::checkRuntimeByteorder(void)
{
	// Determine the run-time byte ordering.
	static const uint32_t boCheck_32 = 0x12345678;
	static const uint8_t boCheck_LE[4] = {0x78, 0x56, 0x34, 0x12};
	static const uint8_t boCheck_BE[4] = {0x12, 0x34, 0x56, 0x78};
	static const uint8_t boCheck_PDP[4] = {0x34, 0x12, 0x78, 0x56};
	
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


/**
 * Test the LibGens byteswapping code.
 * @return 0 on success; negative on fatal error; positive if tests failed.
 */
int Test_Byteswap::exec(void)
{
	fprintf(stderr, "LibGens: Byteswap test suite.\n\n");
	
	// Print the compile-time byte ordering.
	const int byteorder_compiled = GENS_BYTEORDER;
	const char *byteorder_compiled_str = byteorderString(byteorder_compiled);
	fprintf(stderr, "Compile-time byte ordering: %s\n", byteorder_compiled_str);
	
	// Determine the run-time byte ordering.
	const int byteorder_runtime = checkRuntimeByteorder();
	const char *byteorder_runtime_str = byteorderString(byteorder_runtime);
	fprintf(stderr, "Run-time byte ordering: %s\n", byteorder_runtime_str);
	
	// Make sure the byteorders are equivalent.
	if (!assertEquals("Byteorder", (uint16_t)byteorder_compiled, (uint16_t)byteorder_runtime))
	{
		// Byteorders do not match. Print a warning.
		PrintWarn(stderr);
		fprintf(stderr, "Remaining tests will probably fail due to byteorder mismatch.\n");
	}
	
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

int main(int argc, char *argv[])
{
	LibGens::Tests::Test_Byteswap byteswapTest;
	int ret = byteswapTest.exec();
	return ((ret == 0) ? ret : byteswapTest.testsFailed());
}
