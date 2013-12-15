/***************************************************************************
 * mdZ80/tests: Gens Z80 Emulator. (Test Suite)                            *
 * InsnTests.cpp: Instruction tests.                                       *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
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

#include "Util/byteswap.h"

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
using namespace std;

// C includes.
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

// Z80 emulator.
#include "mdZ80.h"
#include "mdZ80_flags.h"

namespace LibGens { namespace Tests {

class InsnTests : public ::testing::Test
{
	protected:
		InsnTests() { }
		virtual ~InsnTests() { }

		virtual void SetUp(void);
		virtual void TearDown(void);

	protected:
		mdZ80_context *z80;

		// Program ROM. (Mapped to $8000)
		static const uint8_t prg_rom[0x8000];

		// Z80 memory access functions.
		static uint8_t z80_ReadB(uint16_t addr);
		static void z80_WriteB(uint16_t addr, uint8_t data);

		// Register state.
		struct RegState {
			uint16_t AF; uint16_t BC;
			uint16_t DE; uint16_t HL;
			uint16_t IX; uint16_t IY;
			uint16_t SP;
			uint16_t AF2; uint16_t BC2;
			uint16_t DE2; uint16_t HL2;
			uint8_t IFF; uint8_t R;
			uint8_t I; uint8_t IM;
		};

		static void RandomizeRegState(RegState *regState);
		void setRegState(const RegState *regState);
		void expectRegState(const RegState *expectedState);

		// Initial register state.
		RegState initRegState;
};

// Z80 memory.
extern "C" uint8_t Ram_Z80[0x2000];
uint8_t Ram_Z80[0x2000];

// Program memory.
#include "InsnTests.prg_rom.inc.h"

/**
 * Set up the Z80 for testing.
 */
void InsnTests::SetUp(void)
{
	z80 = mdZ80_new();

	// Instruction fetch.
	mdZ80_Add_Fetch(z80, 0x00, 0x1F, &Ram_Z80[0]);
	mdZ80_Add_Fetch(z80, 0x20, 0x3F, &Ram_Z80[0]);
	mdZ80_Add_Fetch(z80, 0x80, 0xFF, &prg_rom[0]);

	// Memory access functions.
	mdZ80_Set_ReadB(z80, z80_ReadB);
	mdZ80_Set_WriteB(z80, z80_WriteB);

	// Initialize the CPU state to random values.
	RandomizeRegState(&initRegState);
	setRegState(&initRegState);
}

/**
 * Read Z80 memory.
 * @param addr Address.
 * @return Z80 memory.
 */
uint8_t InsnTests::z80_ReadB(uint16_t addr)
{
	if (addr <= 0x3FFF)
		return Ram_Z80[addr & 0x1FFF];
	else if (addr >= 0x8000)
		return prg_rom[addr & 0x7FFF];
	else
		return 0xFF;
}

/**
 * Write Z80 memory.
 * @param addr Address.
 * @param data Data.
 * @return Z80 memory.
 */
void InsnTests::z80_WriteB(uint16_t addr, uint8_t data)
{
	if (addr <= 0x3FFF)
		Ram_Z80[addr & 0x1FFF] = data;
}

/**
 * Randomize a RegState.
 * @param regState RegState to randomize.
 */
void InsnTests::RandomizeRegState(RegState *regState)
{
	regState->AF = random() & 0xFFFF;
	regState->BC = random() & 0xFFFF;
	regState->DE = random() & 0xFFFF;
	regState->HL = random() & 0xFFFF;
	regState->IX = random() & 0xFFFF;
	regState->IY = random() & 0xFFFF;
	regState->SP = 0x2000;
	regState->AF2 = random() & 0xFFFF;
	regState->BC2 = random() & 0xFFFF;
	regState->DE2 = random() & 0xFFFF;
	regState->HL2 = random() & 0xFFFF;
	regState->IFF = 0;
	regState->R = 0;
	regState->I = 0;
	regState->IM = 1;
}

/**
 * Set the Z80's register state.
 * @param regState RegState.
 */
void InsnTests::setRegState(const RegState *regState)
{
	mdZ80_set_AF(z80, regState->AF);
	mdZ80_set_BC(z80, regState->BC);
	mdZ80_set_DE(z80, regState->DE);
	mdZ80_set_HL(z80, regState->HL);
	mdZ80_set_IX(z80, regState->IX);
	mdZ80_set_IY(z80, regState->IY);
	mdZ80_set_SP(z80, regState->SP);
	mdZ80_set_AF2(z80, regState->AF2);
	mdZ80_set_BC2(z80, regState->BC2);
	mdZ80_set_DE2(z80, regState->DE2);
	mdZ80_set_HL2(z80, regState->HL2);
	mdZ80_set_IFF(z80, regState->IFF);
	mdZ80_set_R(z80, regState->R);
	mdZ80_set_I(z80, regState->I);
	mdZ80_set_IM(z80, regState->IM);
}

/**
 * Check the register state against the Z80.
 * @param expectedState Expected register state.
 */
void InsnTests::expectRegState(const RegState *expectedState)
{
	EXPECT_EQ(expectedState->AF, mdZ80_get_AF(z80));
	EXPECT_EQ(expectedState->BC, mdZ80_get_BC(z80));
	EXPECT_EQ(expectedState->DE, mdZ80_get_DE(z80));
	EXPECT_EQ(expectedState->HL, mdZ80_get_HL(z80));
	EXPECT_EQ(expectedState->IX, mdZ80_get_IX(z80));
	EXPECT_EQ(expectedState->IY, mdZ80_get_IY(z80));
	EXPECT_EQ(expectedState->SP, mdZ80_get_SP(z80));
	EXPECT_EQ(expectedState->AF2, mdZ80_get_AF2(z80));
	EXPECT_EQ(expectedState->BC2, mdZ80_get_BC2(z80));
	EXPECT_EQ(expectedState->DE2, mdZ80_get_DE2(z80));
	EXPECT_EQ(expectedState->HL2, mdZ80_get_HL2(z80));
	EXPECT_EQ(expectedState->IFF, mdZ80_get_IFF(z80));
	EXPECT_EQ(expectedState->R, mdZ80_get_R(z80));
	EXPECT_EQ(expectedState->I, mdZ80_get_I(z80));
	EXPECT_EQ(expectedState->IM, mdZ80_get_IM(z80));
}

/**
 * Tear down the Z80.
 */
void InsnTests::TearDown(void)
{
	mdZ80_free(z80);
}

/** Test cases. **/

TEST_F(InsnTests, INSN_NOP)
{
	const uint16_t initPC = 0x8000;
	const uint16_t endPC = initPC + 2;
	mdZ80_set_PC(z80, initPC);
	mdZ80_exec(z80, 100);
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED);
	EXPECT_EQ(endPC, mdZ80_get_PC(z80));
	expectRegState(&initRegState);
}

} }


int main(int argc, char *argv[])
{
	fprintf(stderr, "mdZ80 test suite: Instruction tests.\n\n");
	srandom(gettimeofday(NULL, NULL));

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
