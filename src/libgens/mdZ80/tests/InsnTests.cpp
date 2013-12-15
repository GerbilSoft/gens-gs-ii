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
		// TODO: Big-endian support.
		struct RegState {
			union { struct { uint8_t A; uint8_t F; }; uint16_t AF; };
			union { struct { uint8_t C; uint8_t B; }; uint16_t BC; };
			union { struct { uint8_t E; uint8_t D; }; uint16_t DE; };
			union { struct { uint8_t L; uint8_t H; }; uint16_t HL; };
			union { struct { uint8_t IXl; uint8_t IXh; }; uint16_t IX; };
			union { struct { uint8_t IYl; uint8_t IYh; }; uint16_t IY; };
			union { struct { uint8_t SPL; uint8_t SPH; }; uint16_t SP; };
			uint16_t AF2;
			uint16_t BC2;
			uint16_t DE2;
			uint16_t HL2;

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

#define INSN_LD_R_R(Rdest, Rsrc, Prefix, PrgStart, len) \
TEST_F(InsnTests, INSN_ ## Prefix ## LD_ ## Rdest ## _ ## Rsrc) \
{ \
	const uint16_t initPC = (PrgStart); \
	const uint16_t endPC = initPC + (len); \
	mdZ80_set_PC(z80, initPC); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	RegState expectedRegState = initRegState; \
	expectedRegState.Rdest = expectedRegState.Rsrc; \
	expectRegState(&expectedRegState); \
}

/* LD R, R */
#define INSN_LD_R_R_ABCDEHL(Rdest, PrgStart) \
INSN_LD_R_R(Rdest, A, , PrgStart, 2) \
INSN_LD_R_R(Rdest, B, , PrgStart+2, 2) \
INSN_LD_R_R(Rdest, C, , PrgStart+4, 2) \
INSN_LD_R_R(Rdest, D, , PrgStart+6, 2) \
INSN_LD_R_R(Rdest, E, , PrgStart+8, 2) \
INSN_LD_R_R(Rdest, H, , PrgStart+10, 2) \
INSN_LD_R_R(Rdest, L, , PrgStart+12, 2) \

INSN_LD_R_R_ABCDEHL(A, 0x8002)
INSN_LD_R_R_ABCDEHL(B, 0x8010)
INSN_LD_R_R_ABCDEHL(C, 0x801E)
INSN_LD_R_R_ABCDEHL(D, 0x802C)
INSN_LD_R_R_ABCDEHL(E, 0x803A)
INSN_LD_R_R_ABCDEHL(H, 0x8048)
INSN_LD_R_R_ABCDEHL(L, 0x8056)

/* DD (IX): LD R, R */
#define INSN_DD_LD_R_R_ABCDEHL(Rdest, PrgStart) \
INSN_LD_R_R(Rdest, A, DD_, PrgStart, 3) \
INSN_LD_R_R(Rdest, B, DD_, PrgStart+3, 3) \
INSN_LD_R_R(Rdest, C, DD_, PrgStart+6, 3) \
INSN_LD_R_R(Rdest, D, DD_, PrgStart+9, 3) \
INSN_LD_R_R(Rdest, E, DD_, PrgStart+12, 3) \
INSN_LD_R_R(Rdest, IXh, DD_, PrgStart+15, 3) \
INSN_LD_R_R(Rdest, IXl, DD_, PrgStart+18, 3) \

INSN_DD_LD_R_R_ABCDEHL(A, 0x8064)
INSN_DD_LD_R_R_ABCDEHL(B, 0x8079)
INSN_DD_LD_R_R_ABCDEHL(C, 0x808E)
INSN_DD_LD_R_R_ABCDEHL(D, 0x80A3)
INSN_DD_LD_R_R_ABCDEHL(E, 0x80B8)
INSN_DD_LD_R_R_ABCDEHL(IXh, 0x80CD)
INSN_DD_LD_R_R_ABCDEHL(IXl, 0x80E2)

/* FD (IY): LD R, R */
#define INSN_FD_LD_R_R_ABCDEHL(Rdest, PrgStart) \
INSN_LD_R_R(Rdest, A, FD_, PrgStart, 3) \
INSN_LD_R_R(Rdest, B, FD_, PrgStart+3, 3) \
INSN_LD_R_R(Rdest, C, FD_, PrgStart+6, 3) \
INSN_LD_R_R(Rdest, D, FD_, PrgStart+9, 3) \
INSN_LD_R_R(Rdest, E, FD_, PrgStart+12, 3) \
INSN_LD_R_R(Rdest, IYh, FD_, PrgStart+15, 3) \
INSN_LD_R_R(Rdest, IYl, FD_, PrgStart+18, 3) \

INSN_FD_LD_R_R_ABCDEHL(A, 0x80F7)
INSN_FD_LD_R_R_ABCDEHL(B, 0x810C)
INSN_FD_LD_R_R_ABCDEHL(C, 0x8121)
INSN_FD_LD_R_R_ABCDEHL(D, 0x8136)
INSN_FD_LD_R_R_ABCDEHL(E, 0x814B)
INSN_FD_LD_R_R_ABCDEHL(IYh, 0x8160)
INSN_FD_LD_R_R_ABCDEHL(IYl, 0x8175)

#define INSN_LD_RR_NN(Rdest, PrgStart) \
TEST_F(InsnTests, INSN_LD_ ## Rdest ## _NN) \
{ \
	const uint16_t initPC = PrgStart; \
	const uint16_t endPC = initPC + 4; \
	mdZ80_set_PC(z80, initPC); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	RegState expectedRegState = initRegState; \
	expectedRegState.Rdest = 0x1234; \
	expectRegState(&expectedRegState); \
}

//INSN_LD_RR_NN(BC, 0x8002)

} }


int main(int argc, char *argv[])
{
	fprintf(stderr, "mdZ80 test suite: Instruction tests.\n\n");

	// Initialize the Random Number Generator.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srandom(tv.tv_usec);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
