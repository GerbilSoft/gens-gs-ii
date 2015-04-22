/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * Z80Test.cpp: ZEXDOC/ZEXALL using a minimal CP/M emulator.               *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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

#include "libz80/z80.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
using namespace std;

// NOTE: This test suite uses libz80 directly.
// The Z80 class is currently hard-coded for MD only.

namespace LibGens { namespace Tests {

class Z80Test : public ::testing::Test
{
	protected:
		Z80Test() : m_Z80(nullptr) { }
		virtual ~Z80Test() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		Z80Context *m_Z80;
		static Z80Test *curZ80Test;

		// Z80 RAM.
		uint8_t Ram_Z80[65536];

		// System state.
		bool halt;

		// TODO: Use param as the Z80Test context.
		static uint8_t Z80_ReadB_static(int param, uint16_t address) {
			return curZ80Test->Z80_ReadB(address);
		}
		static void Z80_WriteB_static(int param, uint16_t address, uint8_t data) {
			curZ80Test->Z80_WriteB(address, data);
		}

		uint8_t Z80_ReadB(uint16_t address);
		void Z80_WriteB(uint16_t address, uint8_t data);

		// FIXME: Pass the context in these functions.
		static uint8_t Z80_InB_static(int param, uint16_t address) {
			return curZ80Test->Z80_InB(address);
		}
		static void Z80_OutB_static(int param, uint16_t address, uint8_t data) {
			curZ80Test->Z80_OutB(address, data);
		}

		uint8_t Z80_InB(uint16_t address);
		void Z80_OutB(uint16_t address, uint8_t data);
};

Z80Test *Z80Test::curZ80Test;

void Z80Test::SetUp()
{
	// Set the current Z80Test.
	curZ80Test = this;
	halt = false;

	// Initialize Z80 memory.
	memset(Ram_Z80, 0, sizeof(Ram_Z80));
	// 0: Warm boot code - JP $F000
	Ram_Z80[0] = 0xC3;
	Ram_Z80[1] = 0x00;
	Ram_Z80[2] = 0xF0;
	// 5: SYSCALL - JP $F400
	Ram_Z80[5] = 0xC3;
	Ram_Z80[6] = 0x00;
	Ram_Z80[7] = 0xF0;

	// Load BIOS.
	FILE *f = fopen("bios.bin", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0xF000], 1, 1068, f);
	fclose(f);

	// Initialize the Z80 emulator for CP/M.
	m_Z80 = (Z80Context*)calloc(1, sizeof(Z80Context));

	// Set memory read/write handlers.
	m_Z80->memRead = Z80_ReadB_static;
	m_Z80->memWrite = Z80_WriteB_static;

	// Set I/O handlers.
	m_Z80->ioRead = Z80_InB_static;
	m_Z80->ioWrite = Z80_OutB_static;
}

void Z80Test::TearDown(void)
{
	free(m_Z80);
	m_Z80 = nullptr;

	curZ80Test = nullptr;
	halt = false;
}

// Memory read/write functions.

uint8_t Z80Test::Z80_ReadB(uint16_t address)
{
	return Ram_Z80[address & 0xFFFF];
}

void Z80Test::Z80_WriteB(uint16_t address, uint8_t data)
{
	Ram_Z80[address & 0xFFFF] = data;
}

// I/O functions.

uint8_t Z80Test::Z80_InB(uint16_t address)
{
	// No input ports...
	((void)address);
	return 0xFF;
}

void Z80Test::Z80_OutB(uint16_t address, uint8_t data)
{
	switch (address & 0xFF) {
		case 0x01:
			// stdout
			// TODO: Lines beginning with '*' should be in red and trigger an error.
			fputc(data, stdout);
			break;

		case 0x02:
			// stderr
			// TODO: Make this red?
			fputc(data, stdout);
			break;

		case 0xFF:
			// End of program.
			halt = true;
			break;
	}
}

/** Test cases. **/

/**
 * Run ZEXDOC.
 */
TEST_F(Z80Test, zexdoc)
{
	// Load ZEXDOC.
	FILE *f = fopen("zexdoc.com", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0x0100], 1, 8585, f);
	fclose(f);

	// Reset the Z80.
	Z80RESET(m_Z80);

	// Run the Z80 until it's halted.
	// TODO: Just check for HALTED status instead of using port 0xFF?
	while (!halt && !m_Z80->halted) {
		Z80ExecuteTStates(m_Z80, 100);
	}
	printf("\n");

	// TODO: If any errors occurred, fail the test.
}

/**
 * Run ZEXALL.
 */
TEST_F(Z80Test, zexall)
{
	// Load ZEXALL.
	FILE *f = fopen("zexall.com", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0x0100], 1, 8585, f);
	fclose(f);

	// Reset the Z80.
	Z80RESET(m_Z80);

	// Run the Z80 until it's halted.
	// TODO: Just check for HALTED status instead of using port 0xFF?
	while (!halt && !m_Z80->halted) {
		Z80ExecuteTStates(m_Z80, 100);
	}
	printf("\n");

	// TODO: If any errors occurred, fail the test.
}

} }


int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: Z80 tests.\n\n");

	::testing::InitGoogleTest(&argc, argv);
	//LibGens::Init(); /* not needed for Z80 tests */
	fflush(nullptr);

	int ret = RUN_ALL_TESTS();
	//LibGens::End(); /* not needed for Z80 tests */
	return ret;
}
