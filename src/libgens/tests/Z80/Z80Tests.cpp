/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * Z80Tests.cpp: ZEXDOC/ZEXALL using a minimal CP/M emulator.              *
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

#include "mdZ80/mdZ80.h"
#include "mdZ80/mdZ80_flags.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
using namespace std;

// NOTE: This test suite uses mdZ80 directly.
// The Z80 class is currently hard-coded for MD only.

// NOTE: Due to limitations in mdZ80 right now,
// the first 8 KB must be RAM, and must be
// accessible as Ram_Z80.
extern "C" {
	uint8_t Ram_Z80[65536];
}

namespace LibGens { namespace Tests {

class Z80Tests : public ::testing::Test
{
	protected:
		Z80Tests()
			: ::testing::Test()
			, m_Z80(nullptr)
			, halt(false) { }
		virtual ~Z80Tests() { }

		virtual void SetUp(void) override;
		virtual void TearDown(void) override;

	protected:
		mdZ80_context *m_Z80;
		static Z80Tests *curZ80Tests;

		// System state.
		bool halt;

		// FIXME: Pass the context in these functions.
		static uint8_t FASTCALL Z80_ReadB_static(uint32_t adr) {
			return curZ80Tests->Z80_ReadB(adr);
		}
		static void FASTCALL Z80_WriteB_static(uint32_t adr, uint8_t data) {
			curZ80Tests->Z80_WriteB(adr, data);
		}

		uint8_t Z80_ReadB(uint32_t adr);
		void Z80_WriteB(uint32_t adr, uint8_t data);

		// FIXME: Pass the context in these functions.
		static uint8_t FASTCALL Z80_InB_static(uint32_t adr) {
			return curZ80Tests->Z80_InB(adr);
		}
		static void FASTCALL Z80_OutB_static(uint32_t adr, uint8_t data) {
			curZ80Tests->Z80_OutB(adr, data);
		}

		uint8_t Z80_InB(uint32_t adr);
		void Z80_OutB(uint32_t adr, uint8_t data);
};

Z80Tests *Z80Tests::curZ80Tests;

void Z80Tests::SetUp()
{
	// Set the current Z80Tests.
	curZ80Tests = this;
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

	// Load BDOS.
	FILE *f = fopen("bdos.bin", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0xF000], 1, 1068, f);
	fclose(f);

	// Initialize the Z80 emulator for CP/M.
	m_Z80 = mdZ80_new();

	// Set instruction fetch handlers.
	mdZ80_Add_Fetch(m_Z80, 0x00, 0xFF, Ram_Z80);
	
	// Set memory read/write handlers.
	mdZ80_Set_ReadB(m_Z80, Z80_ReadB_static);
	mdZ80_Set_WriteB(m_Z80, Z80_WriteB_static);

	// Set I/O handlers.
	mdZ80_Set_In(m_Z80, Z80_InB_static);
	mdZ80_Set_Out(m_Z80, Z80_OutB_static);
}

void Z80Tests::TearDown(void)
{
	mdZ80_free(m_Z80);

	curZ80Tests = nullptr;
	halt = false;
}

// Memory read/write functions.

uint8_t Z80Tests::Z80_ReadB(uint32_t adr)
{
	return Ram_Z80[adr & 0xFFFF];
}

void Z80Tests::Z80_WriteB(uint32_t adr, uint8_t data)
{
	Ram_Z80[adr & 0xFFFF] = data;
}

// I/O functions.

uint8_t Z80Tests::Z80_InB(uint32_t adr)
{
	// No input ports...
	((void)adr);
	return 0xFF;
}

void Z80Tests::Z80_OutB(uint32_t adr, uint8_t data)
{
	switch (adr & 0xFF) {
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
TEST_F(Z80Tests, zexdoc)
{
	// Load ZEXDOC.
	FILE *f = fopen("zexdoc.com", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0x0100], 1, 8585, f);
	fclose(f);

	// Run the Z80 until it's halted.
	// TODO: Add function to mdZ80 to see if HALT instruction was used?
	// For now, rely on port FFh.
	mdZ80_set_PC(m_Z80, 0);

	// TODO: Just check for HALTED status instead of using port 0xFF?
	while (!halt && !(mdZ80_get_Status(m_Z80) & Z80_STATE_HALTED)) {
		z80_Exec(m_Z80, 100);
		mdZ80_clear_odo(m_Z80);
	}
	printf("\n");

	// TODO: If any errors occurred, fail the test.
}

/**
 * Run ZEXALL.
 */
TEST_F(Z80Tests, zexall)
{
	// Load ZEXALL.
	FILE *f = fopen("zexall.com", "rb");
	ASSERT_TRUE(f != nullptr);
	fread(&Ram_Z80[0x0100], 1, 8585, f);
	fclose(f);

	// Run the Z80 until it's halted.
	// TODO: Add function to mdZ80 to see if HALT instruction was used?
	// For now, rely on port FFh.
	mdZ80_set_PC(m_Z80, 0);

	// TODO: Just check for HALTED status instead of using port 0xFF?
	while (!halt && !(mdZ80_get_Status(m_Z80) & Z80_STATE_HALTED)) {
		z80_Exec(m_Z80, 100);
		mdZ80_clear_odo(m_Z80);
	}
	printf("\n");

	// TODO: If any errors occurred, fail the test.
}

} }


int main(int argc, char *argv[])
{
	fprintf(stderr, "LibGens test suite: Z80 tests.\n\n");
	//LibGens::Init(); /* not needed for Z80 tests */
	fflush(nullptr);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
