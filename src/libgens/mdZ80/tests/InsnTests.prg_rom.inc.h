/***************************************************************************
 * mdZ80/tests: Gens Z80 Emulator. (Test Suite)                            *
 * InsnTests.prg_rom.inc.h: Program ROM for instruction tests.             *
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

/**
 * Program ROM. (Mapped to $8000)
 * All tests are followed by HALT.
 */
const uint8_t InsnTests::prg_rom[0x8000] = {
	// $8000: NOP
	0x00, 0x76,

	/*! LD R, R */
	#define PRG_LD_R_R(base) \
	((base)+7), 0x76, ((base)+0), 0x76, ((base)+1), 0x76, ((base)+2), 0x76, \
	((base)+3), 0x76, ((base)+4), 0x76, ((base)+5), 0x76
	PRG_LD_R_R(0x78),	// $8002: LD A, [ABCDEHL]
	PRG_LD_R_R(0x40),	// $8010: LD B, [ABCDEHL]
	PRG_LD_R_R(0x48),	// $801E: LD C, [ABCDEHL]
	PRG_LD_R_R(0x50),	// $802C: LD D, [ABCDEHL]
	PRG_LD_R_R(0x58),	// $803A: LD E, [ABCDEHL]
	PRG_LD_R_R(0x60),	// $8048: LD H, [ABCDEHL]
	PRG_LD_R_R(0x68),	// $8056: LD L, [ABCDEHL]

	/*! (DD) LD R, R */
	#define PRG_DD_LD_R_R(base) \
	0xDD, ((base)+7), 0x76, 0xDD, ((base)+0), 0x76, \
	0xDD, ((base)+1), 0x76, 0xDD, ((base)+2), 0x76, \
	0xDD, ((base)+3), 0x76, 0xDD, ((base)+4), 0x76, \
	0xDD, ((base)+5), 0x76
	PRG_DD_LD_R_R(0x78),	// $8064: LD A, [ABCDE/IXh/IXl]
	PRG_DD_LD_R_R(0x40),	// $8079: LD B, [ABCDE/IXh/IXl]
	PRG_DD_LD_R_R(0x48),	// $808E: LD C, [ABCDE/IXh/IXl]
	PRG_DD_LD_R_R(0x50),	// $80A3: LD D, [ABCDE/IXh/IXl]
	PRG_DD_LD_R_R(0x58),	// $80B8: LD E, [ABCDE/IXh/IXl]
	PRG_DD_LD_R_R(0x60),	// $80CD: LD IXh, [ABCDE/IXh/IXl]
	PRG_DD_LD_R_R(0x68),	// $80E2: LD IXl, [ABCDE/IXh/IXl]

	/*! (FD) LD R, R */
	#define PRG_FD_LD_R_R(base) \
	0xFD, ((base)+7), 0x76, 0xFD, ((base)+0), 0x76, \
	0xFD, ((base)+1), 0x76, 0xFD, ((base)+2), 0x76, \
	0xFD, ((base)+3), 0x76, 0xFD, ((base)+4), 0x76, \
	0xFD, ((base)+5), 0x76
	PRG_FD_LD_R_R(0x78),	// $80F7: LD A, [ABCDE/IYh/IYl]
	PRG_FD_LD_R_R(0x40),	// $810C: LD B, [ABCDE/IYh/IYl]
	PRG_FD_LD_R_R(0x48),	// $8121: LD C, [ABCDE/IYh/IYl]
	PRG_FD_LD_R_R(0x50),	// $8136: LD D, [ABCDE/IYh/IYl]
	PRG_FD_LD_R_R(0x58),	// $814B: LD E, [ABCDE/IYh/IYl]
	PRG_FD_LD_R_R(0x60),	// $8160: LD IYh, [ABCDE/IYh/IYl]
	PRG_FD_LD_R_R(0x68),	// $8175: LD IYl, [ABCDE/IYh/IYl]

	/*! LD R, N */
	0x3E, 0x12, 0x76,	// $818A: LD A, $12
	0x06, 0x34, 0x76,	// $818D: LD B, $34
	0x0E, 0x56, 0x76,	// $8190: LD C, $56
	0x16, 0x78, 0x76,	// $8193: LD D, $78
	0x1E, 0x9A, 0x76,	// $8196: LD E, $9A
	0x26, 0xBC, 0x76,	// $8199: LD H, $BC
	0x2E, 0xDE, 0x76,	// $819C: LD L, $DE

	/*! (DD) LD R, N */
	0xDD, 0x3E, 0x12, 0x76,	// $819F: LD A, $12
	0xDD, 0x06, 0x34, 0x76,	// $81A3: LD B, $34
	0xDD, 0x0E, 0x56, 0x76,	// $81A7: LD C, $56
	0xDD, 0x16, 0x78, 0x76,	// $81AB: LD D, $78
	0xDD, 0x1E, 0x9A, 0x76,	// $81AF: LD E, $9A
	0xDD, 0x26, 0xBC, 0x76, // $81B3: LD IXh, $BC
	0xDD, 0x2E, 0xDE, 0x76,	// $81B7: LD IXl, $DE

	/*! (FD) LD R, N */
	0xFD, 0x3E, 0x12, 0x76,	// $81BB: LD A, $12
	0xFD, 0x06, 0x34, 0x76,	// $81BF: LD B, $34
	0xFD, 0x0E, 0x56, 0x76,	// $81C3: LD C, $56
	0xFD, 0x16, 0x78, 0x76,	// $81C7: LD D, $78
	0xFD, 0x1E, 0x9A, 0x76,	// $81CB: LD E, $9A
	0xFD, 0x26, 0xBC, 0x76, // $81CF: LD IYh, $BC
	0xFD, 0x2E, 0xDE, 0x76,	// $81D3: LD IYl, $DE

	// $8002: LD BC, NN
	//0x01, 0x34, 0x12, 0x76,
};
