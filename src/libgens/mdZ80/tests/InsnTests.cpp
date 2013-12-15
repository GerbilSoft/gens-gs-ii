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

		// Default RAM data.
		static const uint8_t default_ram_data[0x20];

		/**
		 * Get the next HL address.
		 * HL must be within [0x00, 0x1F].
		 */
		static inline uint16_t get_HL_address(void)
		{
			static uint16_t HL_address = 0;
			HL_address++;
			HL_address %= sizeof(default_ram_data);
			return HL_address;
		}

		/**
		 * Get the next IX/IY address.
		 * IX/IY must be within [0x04, 0x1B].
		 */
		static inline uint16_t get_XY_address(void)
		{
			static uint16_t XY_address = 4;
			XY_address++;
			if (XY_address >= (sizeof(default_ram_data)-0x4))
				XY_address = 0x04;
			return XY_address;
		}

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

// Default RAM data. ($0000 - $001F)
const uint8_t InsnTests::default_ram_data[0x20] = {
	'Y','o','u',' ','j','u','s','t',
	' ','l','o','s','t',' ','T','h',
	'e',' ','G','a','m','e','.',' ',
	'R','I','C','K','R','O','L','L'
};

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

	// Copy default RAM data.
	memcpy(&Ram_Z80[0], &default_ram_data[0], sizeof(default_ram_data));
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

/**
 * LD R, R
 * @param Rdest Destination register.
 * @param Rsrc Source register.
 * @param Prefix IX/IY prefix.
 * @param PrgStart Program start address.
 * @param len Program length.
 */
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

/**
 * LD R, N
 * @param Rdest Destination register.
 * @param Prefix IX/IY prefix.
 * @param PrgStart Program start address.
 * @param len Program length.
 * @param expected Expected 'N' value.
 */
#define INSN_LD_R_N(Rdest, Prefix, PrgStart, len, expected) \
TEST_F(InsnTests, INSN_ ## Prefix ## LD_ ## Rdest ## _N) \
{ \
	const uint16_t initPC = (PrgStart); \
	const uint16_t endPC = initPC + (len); \
	mdZ80_set_PC(z80, initPC); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	RegState expectedRegState = initRegState; \
	expectedRegState.Rdest = expected; \
	expectRegState(&expectedRegState); \
}

/* LD R, N */
INSN_LD_R_N(A, , 0x818A, 3, 0x12)
INSN_LD_R_N(B, , 0x818D, 3, 0x34)
INSN_LD_R_N(C, , 0x8190, 3, 0x56)
INSN_LD_R_N(D, , 0x8193, 3, 0x78)
INSN_LD_R_N(E, , 0x8196, 3, 0x9A)
INSN_LD_R_N(H, , 0x8199, 3, 0xBC)
INSN_LD_R_N(L, , 0x819C, 3, 0xDE)

/* DD (IX): LD R, N */
INSN_LD_R_N(A, DD_, 0x819F, 4, 0x12)
INSN_LD_R_N(B, DD_, 0x81A3, 4, 0x34)
INSN_LD_R_N(C, DD_, 0x81A7, 4, 0x56)
INSN_LD_R_N(D, DD_, 0x81AB, 4, 0x78)
INSN_LD_R_N(E, DD_, 0x81AF, 4, 0x9A)
INSN_LD_R_N(IXh, DD_, 0x81B3, 4, 0xBC)
INSN_LD_R_N(IXl, DD_, 0x81B7, 4, 0xDE)

/* FD (IY): LD R, N */
INSN_LD_R_N(A, FD_, 0x81BB, 4, 0x12)
INSN_LD_R_N(B, FD_, 0x81BF, 4, 0x34)
INSN_LD_R_N(C, FD_, 0x81C3, 4, 0x56)
INSN_LD_R_N(D, FD_, 0x81C7, 4, 0x78)
INSN_LD_R_N(E, FD_, 0x81CB, 4, 0x9A)
INSN_LD_R_N(IYh, FD_, 0x81CF, 4, 0xBC)
INSN_LD_R_N(IYl, FD_, 0x81D3, 4, 0xDE)

/**
 * LD R, (HL)
 * @param Rdest Destination register.
 * @param PrgStart Program start address.
 * @param len Program length.
 */
#define INSN_LD_R_mHL(Rdest, PrgStart, len) \
TEST_F(InsnTests, INSN_ ## LD_ ## Rdest ## _mHL) \
{ \
	const uint16_t initPC = (PrgStart); \
	const uint16_t endPC = initPC + (len); \
	mdZ80_set_PC(z80, initPC); \
	uint16_t HL_addr = get_HL_address(); \
	initRegState.HL = (HL_addr); \
	mdZ80_set_HL(z80, HL_addr); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	RegState expectedRegState = initRegState; \
	expectedRegState.Rdest = default_ram_data[HL_addr]; \
	expectRegState(&expectedRegState); \
}

/* LD R, (HL) */
INSN_LD_R_mHL(A, 0x81D7, 2)
INSN_LD_R_mHL(B, 0x81D9, 2)
INSN_LD_R_mHL(C, 0x81DB, 2)
INSN_LD_R_mHL(D, 0x81DD, 2)
INSN_LD_R_mHL(E, 0x81DF, 2)
INSN_LD_R_mHL(H, 0x81E1, 2)
INSN_LD_R_mHL(L, 0x81E3, 2)

/**
 * LD R, (XY+d)
 * @param Rdest Destination register.
 * @param Ridx Index register.
 * @param disp Displacement.
 * @param PrgStart Program start address.
 * @param len Program length.
 */
#define INSN_LD_R_mXYd(Rdest, Ridx, disp, PrgStart, len) \
TEST_F(InsnTests, INSN_LD_ ## Rdest ## _m ## Ridx ## d) \
{ \
	const uint16_t initPC = (PrgStart); \
	const uint16_t endPC = initPC + (len); \
	mdZ80_set_PC(z80, initPC); \
	uint16_t XY_addr = get_XY_address(); \
	initRegState.Ridx = XY_addr; \
	mdZ80_set_ ## Ridx(z80, XY_addr); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	RegState expectedRegState = initRegState; \
	expectedRegState.Rdest = default_ram_data[XY_addr + disp]; \
	expectRegState(&expectedRegState); \
}

/* LD R, (IX+d) */
INSN_LD_R_mXYd(A, IX,  0, 0x81E5, 4)
INSN_LD_R_mXYd(B, IX,  1, 0x81E9, 4)
INSN_LD_R_mXYd(C, IX,  2, 0x81ED, 4)
INSN_LD_R_mXYd(D, IX,  3, 0x81F1, 4)
INSN_LD_R_mXYd(E, IX, -1, 0x81F5, 4)
INSN_LD_R_mXYd(H, IX, -2, 0x81F9, 4)
INSN_LD_R_mXYd(L, IX, -3, 0x81FD, 4)

/* LD R, (IY+d) */
INSN_LD_R_mXYd(A, IY,  0, 0x8201, 4)
INSN_LD_R_mXYd(B, IY,  1, 0x8205, 4)
INSN_LD_R_mXYd(C, IY,  2, 0x8209, 4)
INSN_LD_R_mXYd(D, IY,  3, 0x820D, 4)
INSN_LD_R_mXYd(E, IY, -1, 0x8211, 4)
INSN_LD_R_mXYd(H, IY, -2, 0x8215, 4)
INSN_LD_R_mXYd(L, IY, -3, 0x8219, 4)

/**
 * LD (HL), R
 * @param Rsrc Source register.
 * @param PrgStart Program start address.
 * @param len Program length.
 */
#define INSN_LD_mHL_R(Rsrc, PrgStart, len) \
TEST_F(InsnTests, INSN_ ## LD_mHL_ ## Rsrc) \
{ \
	const uint16_t initPC = (PrgStart); \
	const uint16_t endPC = initPC + (len); \
	mdZ80_set_PC(z80, initPC); \
	uint16_t HL_addr = get_HL_address(); \
	initRegState.HL = (HL_addr); \
	mdZ80_set_HL(z80, HL_addr); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	EXPECT_EQ(initRegState.Rsrc, Ram_Z80[HL_addr]); \
	RegState expectedRegState = initRegState; \
	expectRegState(&expectedRegState); \
}

/* LD (HL), R */
INSN_LD_mHL_R(A, 0x821D, 2)
INSN_LD_mHL_R(B, 0x821F, 2)
INSN_LD_mHL_R(C, 0x8221, 2)
INSN_LD_mHL_R(D, 0x8223, 2)
INSN_LD_mHL_R(E, 0x8225, 2)
INSN_LD_mHL_R(H, 0x8227, 2)
INSN_LD_mHL_R(L, 0x8229, 2)

/**
 * LD (XY+d), R
 * @param Ridx Index register.
 * @param disp Displacement.
 * @param Rsrc Destination register.
 * @param PrgStart Program start address.
 * @param len Program length.
 */
#define INSN_LD_mXYd_R(Ridx, disp, Rsrc, PrgStart, len) \
TEST_F(InsnTests, INSN_LD_m_ ## Ridx ## d_ ## Rsrc) \
{ \
	const uint16_t initPC = (PrgStart); \
	const uint16_t endPC = initPC + (len); \
	mdZ80_set_PC(z80, initPC); \
	uint16_t XY_addr = get_XY_address(); \
	initRegState.Ridx = XY_addr; \
	mdZ80_set_ ## Ridx(z80, XY_addr); \
	mdZ80_exec(z80, 100); \
	\
	EXPECT_TRUE(mdZ80_get_Status(z80) & MDZ80_STATUS_HALTED); \
	EXPECT_EQ(endPC, mdZ80_get_PC(z80)); \
	EXPECT_EQ(initRegState.Rsrc, Ram_Z80[XY_addr + disp]); \
	RegState expectedRegState = initRegState; \
	expectRegState(&expectedRegState); \
}

/* LD (IX+d), R */
INSN_LD_mXYd_R(IX,  0, A, 0x822B, 4)
INSN_LD_mXYd_R(IX,  1, B, 0x822F, 4)
INSN_LD_mXYd_R(IX,  2, C, 0x8233, 4)
INSN_LD_mXYd_R(IX,  3, D, 0x8237, 4)
INSN_LD_mXYd_R(IX, -1, E, 0x823B, 4)
INSN_LD_mXYd_R(IX, -2, H, 0x823F, 4)
INSN_LD_mXYd_R(IX, -3, L, 0x8243, 4)

/* LD (IY+d), R */
INSN_LD_mXYd_R(IY,  0, A, 0x8247, 4)
INSN_LD_mXYd_R(IY,  1, B, 0x824B, 4)
INSN_LD_mXYd_R(IY,  2, C, 0x824F, 4)
INSN_LD_mXYd_R(IY,  3, D, 0x8253, 4)
INSN_LD_mXYd_R(IY, -1, E, 0x8257, 4)
INSN_LD_mXYd_R(IY, -2, H, 0x825B, 4)
INSN_LD_mXYd_R(IY, -3, L, 0x825F, 4)

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
