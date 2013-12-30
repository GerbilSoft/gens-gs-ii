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

	/*! LD R, (HL) */
	0x7E, 0x76,		// $81D7: LD A, (HL)
	0x46, 0x76,		// $81D9: LD B, (HL)
	0x4E, 0x76,		// $81DB: LD C, (HL)
	0x56, 0x76,		// $81DD: LD D, (HL)
	0x5E, 0x76,		// $81DF: LD E, (HL)
	0x66, 0x76,		// $81E1: LD H, (HL)
	0x6E, 0x76,		// $81E3: LD L, (HL)

	/*! LD R, (IX+d) */
	0xDD, 0x7E, 0x00, 0x76,	// $81E5: LD A, (IX+0)
	0xDD, 0x46, 0x01, 0x76,	// $81E9: LD B, (IX+1)
	0xDD, 0x4E, 0x02, 0x76,	// $81ED: LD C, (IX+2)
	0xDD, 0x56, 0x03, 0x76,	// $81F1: LD D, (IX+3)
	0xDD, 0x5E, 0xFF, 0x76,	// $81F5: LD E, (IX-1)
	0xDD, 0x66, 0xFE, 0x76,	// $81F9: LD H, (IX-2)
	0xDD, 0x6E, 0xFD, 0x76,	// $81FD: LD L, (IX-3)

	/*! LD R, (IY+d) */
	0xFD, 0x7E, 0x00, 0x76,	// $8201: LD A, (IY+0)
	0xFD, 0x46, 0x01, 0x76,	// $8205: LD B, (IY+1)
	0xFD, 0x4E, 0x02, 0x76,	// $8209: LD C, (IY+2)
	0xFD, 0x56, 0x03, 0x76,	// $820D: LD D, (IY+3)
	0xFD, 0x5E, 0xFF, 0x76,	// $8211: LD E, (IY-1)
	0xFD, 0x66, 0xFE, 0x76,	// $8215: LD H, (IY-2)
	0xFD, 0x6E, 0xFD, 0x76,	// $8219: LD L, (IY-3)

	/*! LD (HL), R */
	0x77, 0x76,		// $821D: LD (HL), A
	0x70, 0x76,		// $821F: LD (HL), B
	0x71, 0x76,		// $8221: LD (HL), C
	0x72, 0x76,		// $8223: LD (HL), D
	0x73, 0x76,		// $8225: LD (HL), E
	0x74, 0x76,		// $8227: LD (HL), H
	0x75, 0x76,		// $8229: LD (HL), L

	/*! LD (IX+d), R */
	0xDD, 0x77, 0x00, 0x76,	// $822B: LD (IX+0), A
	0xDD, 0x70, 0x01, 0x76,	// $822F: LD (IX+1), B
	0xDD, 0x71, 0x02, 0x76,	// $8233: LD (IX+2), C
	0xDD, 0x72, 0x03, 0x76,	// $8237: LD (IX+3), D
	0xDD, 0x73, 0xFF, 0x76,	// $823B: LD (IX-1), E
	0xDD, 0x74, 0xFE, 0x76,	// $823F: LD (IX-2), H
	0xDD, 0x75, 0xFD, 0x76,	// $8243: LD (IX-3), L

	/*! LD (IY+d), R */
	0xFD, 0x77, 0x00, 0x76,	// $8247: LD (IY+0), A
	0xFD, 0x70, 0x01, 0x76,	// $824B: LD (IY+1), B
	0xFD, 0x71, 0x02, 0x76,	// $824F: LD (IY+2), C
	0xFD, 0x72, 0x03, 0x76,	// $8253: LD (IY+3), D
	0xFD, 0x73, 0xFF, 0x76,	// $8257: LD (IY-1), E
	0xFD, 0x74, 0xFE, 0x76,	// $825B: LD (IY-2), H
	0xFD, 0x75, 0xFD, 0x76,	// $825F: LD (IY-3), L

	/*! Rotate / shift instructions */
	0x07, 0x76,		// $8263: RLCA
	0x17, 0x76,		// $8265: RLA
	0x0F, 0x76,		// $8267: RRCA
	0x1F, 0x76,		// $8269: RRA
};
