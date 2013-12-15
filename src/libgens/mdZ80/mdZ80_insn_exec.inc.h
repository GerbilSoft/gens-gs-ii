/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_insn_table.c: Z80 instruction execution.                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2013 by David Korth                                  *
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

/* WARNING: This file should ONLY be included by mdZ80_exec.c::mdZ80_exec()! */
#if !defined(__MDZ80_IN_EXEC) || __MDZ80_IN_EXEC != 19840519
#error *** ERROR: mdZ80_insn_table.inc.h should ONLY be included by mdZ80_exec.c!
#endif

/**
 * Execute the next instruction.
 * @param cycles Number of cycles used in the previous instruction.
 * TODO: odo < 0 is in the original asm - should it be <= 0?
 * NOTE: Interrupts aren't checked here, though an interrupt
 * can't really happen when we're running...
 */
#define NEXT(cycles) \
	do { \
		odo -= cycles; \
		if (odo < 0) { \
			/* Out of cycles. */ \
			goto z80_Exec_Quit; \
		} \
		\
		/* Next instruction. */ \
		uint8_t insn = read_byte_pc(z80); \
		goto *z80_insn_table[insn]; \
	} while (0)

/**
 * Execute the next instruction without checking the number of cycles.
 * Used in Z80I_EI and Z80I_DI.
 */
#define NEXT_NOCHECK(cycles) \
	do { \
		odo -= cycles; \
		\
		/* Next instruction. */ \
		uint8_t insn = read_byte_pc(z80); \
		goto *z80_insn_table[insn]; \
	} while (0)

while (1) {
	Z80I_NOP:
		// Do nothing!
		z80->PC++;
		NEXT(4);

	/*! LD: 8-bit */

	// LD R, R		R8 <- R8
	#define Z80I_LD_R_R(Rdest, Rsrc) \
		Z80I_LD_ ## Rdest ## _ ## Rsrc : \
			do { \
				z80->Rdest = z80->Rsrc; \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_LD_R_R(A, A);
	Z80I_LD_R_R(A, B);
	Z80I_LD_R_R(A, C);
	Z80I_LD_R_R(A, D);
	Z80I_LD_R_R(A, E);
	Z80I_LD_R_R(A, H);
	Z80I_LD_R_R(A, L);
	Z80I_LD_R_R(A, IXh);
	Z80I_LD_R_R(A, IXl);
	Z80I_LD_R_R(A, IYh);
	Z80I_LD_R_R(A, IYl);

	Z80I_LD_R_R(B, A);
	Z80I_LD_R_R(B, B);
	Z80I_LD_R_R(B, C);
	Z80I_LD_R_R(B, D);
	Z80I_LD_R_R(B, E);
	Z80I_LD_R_R(B, H);
	Z80I_LD_R_R(B, L);
	Z80I_LD_R_R(B, IXh);
	Z80I_LD_R_R(B, IXl);
	Z80I_LD_R_R(B, IYh);
	Z80I_LD_R_R(B, IYl);

	Z80I_LD_R_R(C, A);
	Z80I_LD_R_R(C, B);
	Z80I_LD_R_R(C, C);
	Z80I_LD_R_R(C, D);
	Z80I_LD_R_R(C, E);
	Z80I_LD_R_R(C, H);
	Z80I_LD_R_R(C, L);
	Z80I_LD_R_R(C, IXh);
	Z80I_LD_R_R(C, IXl);
	Z80I_LD_R_R(C, IYh);
	Z80I_LD_R_R(C, IYl);

	Z80I_LD_R_R(D, A);
	Z80I_LD_R_R(D, B);
	Z80I_LD_R_R(D, C);
	Z80I_LD_R_R(D, D);
	Z80I_LD_R_R(D, E);
	Z80I_LD_R_R(D, H);
	Z80I_LD_R_R(D, L);
	Z80I_LD_R_R(D, IXh);
	Z80I_LD_R_R(D, IXl);
	Z80I_LD_R_R(D, IYh);
	Z80I_LD_R_R(D, IYl);

	Z80I_LD_R_R(E, A);
	Z80I_LD_R_R(E, B);
	Z80I_LD_R_R(E, C);
	Z80I_LD_R_R(E, D);
	Z80I_LD_R_R(E, E);
	Z80I_LD_R_R(E, H);
	Z80I_LD_R_R(E, L);
	Z80I_LD_R_R(E, IXh);
	Z80I_LD_R_R(E, IXl);
	Z80I_LD_R_R(E, IYh);
	Z80I_LD_R_R(E, IYl);

	Z80I_LD_R_R(H, A);
	Z80I_LD_R_R(H, B);
	Z80I_LD_R_R(H, C);
	Z80I_LD_R_R(H, D);
	Z80I_LD_R_R(H, E);
	Z80I_LD_R_R(H, H);
	Z80I_LD_R_R(H, L);
	/* NOTE: Not actually valid...
	Z80I_LD_R_R(H, IXh);
	Z80I_LD_R_R(H, IXl);
	Z80I_LD_R_R(H, IYh);
	Z80I_LD_R_R(H, IYl);
	*/

	Z80I_LD_R_R(L, A);
	Z80I_LD_R_R(L, B);
	Z80I_LD_R_R(L, C);
	Z80I_LD_R_R(L, D);
	Z80I_LD_R_R(L, E);
	Z80I_LD_R_R(L, H);
	Z80I_LD_R_R(L, L);
	/* NOTE: Not actually valid...
	Z80I_LD_R_R(L, IXh);
	Z80I_LD_R_R(L, IXl);
	Z80I_LD_R_R(L, IYh);
	Z80I_LD_R_R(L, IYl);
	*/

	Z80I_LD_R_R(IXh, A);
	Z80I_LD_R_R(IXh, B);
	Z80I_LD_R_R(IXh, C);
	Z80I_LD_R_R(IXh, D);
	Z80I_LD_R_R(IXh, E);
	Z80I_LD_R_R(IXh, L);
	Z80I_LD_R_R(IXh, IXh);
	/* NOTE: Not actually valid...
	Z80I_LD_R_R(IXh, H);
	Z80I_LD_R_R(IXh, IXl);
	Z80I_LD_R_R(IXh, IYh);
	Z80I_LD_R_R(IXh, IYl);
	*/

	Z80I_LD_R_R(IXl, A);
	Z80I_LD_R_R(IXl, B);
	Z80I_LD_R_R(IXl, C);
	Z80I_LD_R_R(IXl, D);
	Z80I_LD_R_R(IXl, E);
	Z80I_LD_R_R(IXl, H);
	Z80I_LD_R_R(IXl, IXl);
	/* NOTE: Not actually valid...
	Z80I_LD_R_R(IXl, L);
	Z80I_LD_R_R(IXl, IXh);
	Z80I_LD_R_R(IXl, IYh);
	Z80I_LD_R_R(IXl, IYl);
	*/

	Z80I_LD_R_R(IYh, A);
	Z80I_LD_R_R(IYh, B);
	Z80I_LD_R_R(IYh, C);
	Z80I_LD_R_R(IYh, D);
	Z80I_LD_R_R(IYh, E);
	Z80I_LD_R_R(IYh, L);
	Z80I_LD_R_R(IYh, IYh);
	/* NOTE: Not actually valid...
	Z80I_LD_R_R(IYh, H);
	Z80I_LD_R_R(IYh, IXh);
	Z80I_LD_R_R(IYh, IXl);
	Z80I_LD_R_R(IYh, IYl);
	*/

	Z80I_LD_R_R(IYl, A);
	Z80I_LD_R_R(IYl, B);
	Z80I_LD_R_R(IYl, C);
	Z80I_LD_R_R(IYl, D);
	Z80I_LD_R_R(IYl, E);
	Z80I_LD_R_R(IYl, H);
	Z80I_LD_R_R(IYl, IYl);
	/* NOTE: Not actually valid...
	Z80I_LD_R_R(IYl, L);
	Z80I_LD_R_R(IYl, IXh);
	Z80I_LD_R_R(IYl, IXl);
	Z80I_LD_R_R(IYl, IYh);
	*/

	// LD R, N		R8 <- imm8
	#define Z80I_LD_R_N(Rdest) \
		Z80I_LD_ ## Rdest ## _N : \
			do { \
				z80->Rdest = read_byte_offset_pc(z80, 1); \
				z80->PC += 2; \
				NEXT(7); \
			} while (0)

	Z80I_LD_R_N(A);
	Z80I_LD_R_N(B);
	Z80I_LD_R_N(C);
	Z80I_LD_R_N(D);
	Z80I_LD_R_N(E);
	Z80I_LD_R_N(H);
	Z80I_LD_R_N(L);
	Z80I_LD_R_N(IXl);
	Z80I_LD_R_N(IXh);
	Z80I_LD_R_N(IYl);
	Z80I_LD_R_N(IYh);

	// LD R, (HL)		R8 <- (HL)
	#define Z80I_LD_R_mHL(Rdest) \
		Z80I_LD_ ## Rdest ## _mHL : \
			do {\
				z80->Rdest = READ_BYTE(z80, z80->HL); \
				z80->PC++; \
				NEXT(7); \
			} while (0)

	Z80I_LD_R_mHL(A);
	Z80I_LD_R_mHL(B);
	Z80I_LD_R_mHL(C);
	Z80I_LD_R_mHL(D);
	Z80I_LD_R_mHL(E);
	Z80I_LD_R_mHL(H);
	Z80I_LD_R_mHL(L);

	// LD R, (XY+d)		R8 <- (XY+d)
	#define Z80I_LD_R_mXYd(Rdest, Ridx) \
		Z80I_LD_ ## Rdest ## _m ## Ridx ## d : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				z80->Rdest = READ_BYTE(z80, z80->Ridx + d); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_LD_R_mXYd(A, IX);
	Z80I_LD_R_mXYd(B, IX);
	Z80I_LD_R_mXYd(C, IX);
	Z80I_LD_R_mXYd(D, IX);
	Z80I_LD_R_mXYd(E, IX);
	Z80I_LD_R_mXYd(H, IX);
	Z80I_LD_R_mXYd(L, IX);

	Z80I_LD_R_mXYd(A, IY);
	Z80I_LD_R_mXYd(B, IY);
	Z80I_LD_R_mXYd(C, IY);
	Z80I_LD_R_mXYd(D, IY);
	Z80I_LD_R_mXYd(E, IY);
	Z80I_LD_R_mXYd(H, IY);
	Z80I_LD_R_mXYd(L, IY);

	// LD (HL), R		(HL) <- R8
	#define Z80I_LD_mHL_R(Rsrc) \
		Z80I_LD_mHL_ ## Rsrc : \
			do {\
				WRITE_BYTE(z80, z80->HL, z80->Rsrc); \
				z80->PC++; \
				NEXT(7); \
			} while (0)

	Z80I_LD_mHL_R(A);
	Z80I_LD_mHL_R(B);
	Z80I_LD_mHL_R(C);
	Z80I_LD_mHL_R(D);
	Z80I_LD_mHL_R(E);
	Z80I_LD_mHL_R(H);
	Z80I_LD_mHL_R(L);

	// LD (XY+d), R8	(XY+d) <- R8
	#define Z80I_LD_mXYd_R(Ridx, Rsrc) \
		Z80I_LD_m ## Ridx ## d_ ## Rsrc : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				WRITE_BYTE(z80, z80->Ridx + d, z80->Rsrc); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_LD_mXYd_R(IX, A);
	Z80I_LD_mXYd_R(IX, B);
	Z80I_LD_mXYd_R(IX, C);
	Z80I_LD_mXYd_R(IX, D);
	Z80I_LD_mXYd_R(IX, E);
	Z80I_LD_mXYd_R(IX, H);
	Z80I_LD_mXYd_R(IX, L);

	Z80I_LD_mXYd_R(IY, A);
	Z80I_LD_mXYd_R(IY, B);
	Z80I_LD_mXYd_R(IY, C);
	Z80I_LD_mXYd_R(IY, D);
	Z80I_LD_mXYd_R(IY, E);
	Z80I_LD_mXYd_R(IY, H);
	Z80I_LD_mXYd_R(IY, L);

	// LD (HL), N		(HL) <- imm8
	Z80I_LD_mHL_N: {
		const uint8_t n = read_byte_offset_pc(z80, 1);
		WRITE_BYTE(z80, z80->HL, n);
		z80->PC += 2;
		NEXT(10);
	}

	// LD (XY+d), N		(XY+d) <- imm8
	#define Z80I_LD_mXYd_N(Ridx) \
		Z80I_LD_m ## Ridx ## d_N : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint8_t n = read_byte_offset_pc(z80, 2); \
				WRITE_BYTE(z80, z80->Ridx + d, n); \
				z80->PC += 3; \
				NEXT(15); \
			} while (0)

	Z80I_LD_mXYd_N(IX);
	Z80I_LD_mXYd_N(IY);

	// LD A, (RR)		A <- (RR)
	// TODO: Combine with Z80I_LD_R_mHL()?
	#define Z80I_LD_A_mRR(Rsrc) \
		Z80I_LD_A_m ## Rsrc : \
			do { \
				z80->A = READ_BYTE(z80, z80->Rsrc); \
				z80->PC++; \
				NEXT(7); \
			} while (0)

	Z80I_LD_A_mRR(BC);
	Z80I_LD_A_mRR(DE);

	// LD A, (imm16)	A <- (imm16)
	Z80I_LD_A_mNN: {
		uint16_t src_addr = read_word_offset_pc(z80, 1);
		z80->A = READ_BYTE(z80, src_addr);
		z80->PC += 3;
		NEXT(13);
	}

	// LD (RR), A		(RR) <- A
	// TODO: Combine with Z80I_LD_mHL_R()?
	#define Z80I_LD_mRR_A(Rdest) \
		Z80I_LD_m ## Rdest ## _A : \
			do { \
				WRITE_BYTE(z80, z80->Rdest, z80->A); \
				z80->PC++; \
				NEXT(7); \
			} while (0)

	Z80I_LD_mRR_A(BC);
	Z80I_LD_mRR_A(DE);

	// LD (imm16), A	(imm16) <- A
	Z80I_LD_mNN_A: {
		uint16_t dest_addr = read_word_offset_pc(z80, 1);
		WRITE_BYTE(z80, dest_addr, z80->A);
		z80->PC += 3;
		NEXT(13);
	}

	/**
	 * Adjust the flags for LD A, IR.
	 * Reference: http://www.z80.info/z80sflag.htm
	 * SZ5H3PNC
	 * SZ503*0-
	 * - SZ53 are set as normal.
	 * - IFF2 is copied to bit 2 (P/V).
	 * - C is not affected.
	 * TODO: Implement lahf "acceleration" on x86/amd64?
	 * TODO: Verify bit 5/3 behavior.
	 * @param mask Mask for z80->A.
	 * On x86, non-0xFF is optimized to 'and', 0xFF is optimized to 'test'.
	 */
	#define Z80X_LD_A_IR_UPDATE_FLAGS(mask) \
		do { \
			z80->A &= mask; \
			/* TODO: 'lahf' on x86 */ \
			z80->F = (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3)) | (z80->F & MDZ80_FLAG_C); \
			if (z80->A == 0) \
				z80->F |= MDZ80_FLAG_Z; \
			else if (z80->A & 0x80) \
				z80->F |= MDZ80_FLAG_S; \
			\
			/* IFF2 is copied to MDZ80_FLAG_P. */ \
			z80->F |= ((z80->IFF & 2) << 1); \
		} while (0)

	// LD A, R		A <- R
	Z80I_LD_A_R: {
		// FIXME: Terrible hack for the R register.
		// Implement a more reliable (but slower) R register later.
		uint32_t tmp = z80->CycleCnt - odo + z80->CycleTD;
		tmp >>= 2;
		uint8_t cl = z80->R;
		z80->A = ((tmp & 0xFF) + cl);
		// NOTE: "and A, 0x7F" would work instead of "test A, A".
		Z80X_LD_A_IR_UPDATE_FLAGS(0x7F);
		z80->PC += 2;
		NEXT(9);
	}

	// LD A, I		A <- I
	Z80I_LD_A_I: {
		z80->A = z80->I;
		Z80X_LD_A_IR_UPDATE_FLAGS(0xFF);
		z80->PC += 2;
		NEXT(9);
	}

	// LD IR, A		I|R <- A
	#define Z80I_LD_IR_A(Rdest) \
		Z80I_LD_ ## Rdest ## _A : \
			do { \
				z80->Rdest = z80->A; \
				z80->PC += 2; \
				NEXT(9); \
			} while (0)

	Z80I_LD_IR_A(I);
	Z80I_LD_IR_A(R);

	/*! LD: 16-bit */

	// LD RR, NN		R16 <- imm16
	// LD R, N		R8 <- imm8
	#define Z80I_LD_RR_NN(Rdest) \
		Z80I_LD_ ## Rdest ## _NN : \
			do { \
				z80->Rdest = read_word_offset_pc(z80, 1); \
				z80->PC += 3; \
				NEXT(10); \
			} while (0)

	Z80I_LD_RR_NN(BC);
	Z80I_LD_RR_NN(DE);
	Z80I_LD_RR_NN(HL);
	Z80I_LD_RR_NN(SP);
	Z80I_LD_RR_NN(IX);
	Z80I_LD_RR_NN(IY);

	// LD HL, (NN)		HL <- (imm16)
	Z80I_LD_HL_mNN: {
		uint16_t src_addr = read_word_offset_pc(z80, 1);
		z80->HL = READ_WORD(z80, src_addr);
		z80->PC += 3;
		NEXT(16);
	}

	// LD RR, (NN)		R16 <- (imm16)
	// NOTE: This includes LD HL, (NN); however, it's a slower
	// version that uses an extra byte.
	#define Z80I_LD_RR_mNN(Rdest, LDsuffix) \
		Z80I_LD ## LDsuffix ## _ ## Rdest ## _mNN : \
			do { \
				uint16_t src_addr = read_word_offset_pc(z80, 2); \
				z80->Rdest = READ_WORD(z80, src_addr); \
				z80->PC += 4; \
				NEXT(20); \
			} while (0)

	Z80I_LD_RR_mNN(BC,);
	Z80I_LD_RR_mNN(DE,);
	Z80I_LD_RR_mNN(HL,2);
	Z80I_LD_RR_mNN(SP,);

	// LD XY, (NN)		IX/IY <- (imm16)
	// TODO: Combine with optimized LD HL, (NN)?
	#define Z80I_LD_XY_mNN(Rdest) \
		Z80I_LD_ ## Rdest ## _mNN : \
			do { \
				uint16_t src_addr = read_word_offset_pc(z80, 1); \
				z80->Rdest = READ_WORD(z80, src_addr); \
				z80->PC += 3; \
				NEXT(16); \
			} while (0)

	Z80I_LD_XY_mNN(IX);
	Z80I_LD_XY_mNN(IY);

	// LD (NN), HL		(imm16) <- HL
	Z80I_LD_mNN_HL: {
		uint16_t dest_addr = read_word_offset_pc(z80, 1);
		WRITE_WORD(z80, dest_addr, z80->HL);
		z80->PC += 3;
		NEXT(16);
	}

	// LD (NN), RR		(imm16) <- R16
	// NOTE: This includes LD (NN), HL; however, it's a slower
	// version that uses an extra byte.
	#define Z80I_LD_mNN_RR(Rsrc, LDsuffix) \
		Z80I_LD ## LDsuffix ## _mNN_ ## Rsrc : \
			do { \
				uint16_t dest_addr = read_word_offset_pc(z80, 2); \
				WRITE_WORD(z80, dest_addr, z80->Rsrc); \
				z80->PC += 4; \
				NEXT(20); \
			} while (0)

	Z80I_LD_mNN_RR(BC,);
	Z80I_LD_mNN_RR(DE,);
	Z80I_LD_mNN_RR(HL,2);
	Z80I_LD_mNN_RR(SP,);

	// LD (NN), XY		(imm16) <- IX/IY
	// TODO: Combine with optimized LD (NN), HL?
	#define Z80I_LD_mNN_XY(Rsrc) \
		Z80I_LD_mNN_ ## Rsrc : \
			do { \
				uint16_t dest_addr = read_word_offset_pc(z80, 1); \
				WRITE_WORD(z80, dest_addr, z80->Rsrc); \
				z80->PC += 3; \
				NEXT(16); \
			} while (0)

	Z80I_LD_mNN_XY(IX);
	Z80I_LD_mNN_XY(IY);

	// LD SP, RR		SP <- R16
	#define Z80I_LD_SP_RR(Rsrc) \
		Z80I_LD_SP_ ## Rsrc : \
			do { \
				z80->SP = z80->Rsrc; \
				z80->PC++; \
				NEXT(6); \
			} while (0)

	Z80I_LD_SP_RR(HL);
	Z80I_LD_SP_RR(IX);
	Z80I_LD_SP_RR(IY);

	// PUSH RR		SP -= 2; (SP) <- R16
	#define Z80I_PUSH_RR(Rsrc) \
		Z80I_PUSH_ ## Rsrc : \
			do { \
				z80->SP -= 2; \
				WRITE_WORD(z80, z80->SP, z80->Rsrc); \
				z80->PC++; \
				NEXT(11); \
			} while (0)

	Z80I_PUSH_RR(AF);
	Z80I_PUSH_RR(BC);
	Z80I_PUSH_RR(DE);
	Z80I_PUSH_RR(HL);
	Z80I_PUSH_RR(IX);
	Z80I_PUSH_RR(IY);

	// POP RR		R16 <- (SP); SP += 2
	#define Z80I_POP_RR(Rdest) \
		Z80I_POP_ ## Rdest : \
			do { \
				z80->Rdest = READ_WORD(z80, z80->SP); \
				z80->SP += 2; \
				z80->PC++; \
				NEXT(10); \
			} while (0)

	Z80I_POP_RR(AF);
	Z80I_POP_RR(BC);
	Z80I_POP_RR(DE);
	Z80I_POP_RR(HL);
	Z80I_POP_RR(IX);
	Z80I_POP_RR(IY);

	/*! Exchange instructions */

	// EX RR, RR		R16 <-> R16
	#define Z80I_EX_RR_RR(Rdest, Rsrc) \
		Z80I_EX_ ## Rdest ## _ ## Rsrc : \
			do { \
				SWAP16(z80->Rdest, z80->Rsrc); \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_EX_RR_RR(DE, HL);
	Z80I_EX_RR_RR(AF, AF2);

	// EXX			BC/DE/HL <-> BC'/DE'/HL'
	Z80I_EXX:
		SWAP16(z80->BC, z80->BC2);
		SWAP16(z80->DE, z80->DE2);
		SWAP16(z80->HL, z80->HL2);
		z80->PC++;
		NEXT(4);

	// EX (SP), DD		(SP) <-> HL|IX|IY
	// TODO: Why DD?
	#define Z80I_EX_mSP_DD(Rdest) \
		Z80I_EX_mSP_ ## Rdest : \
			do { \
				uint16_t sp_data = READ_WORD(z80, z80->SP); \
				WRITE_WORD(z80, z80->SP, z80->Rdest); \
				z80->Rdest = sp_data; \
				z80->PC++; \
				NEXT(19); \
			} while (0)

	Z80I_EX_mSP_DD(HL);
	Z80I_EX_mSP_DD(IX);
	Z80I_EX_mSP_DD(IY);

	// LDI/LDD		(DE++/--) <- (HL++/--); BC--
	// TODO: Verify emulation of flags 3 and 5.
	#define Z80I_LDX(Op, AdrInc) \
		Z80I_LD ## Op : \
			do { \
				const uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				WRITE_BYTE(z80, z80->DE, tmp8); \
				z80->HL += AdrInc; \
				z80->DE += AdrInc; \
				z80->BC--; \
				\
				/* Flags: */ \
				/* - P: set if BC != 0 */ \
				/* - 3: set if (tmp8 + A) & 0x08 (bit 3) */ \
				/* - 5: set if (tmp8 + A) & 0x02 (bit 1) */ \
				z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_C); \
				z80->F |= (z80->BC ? MDZ80_FLAG_P : 0); \
				register const uint8_t n = (tmp8 + z80->A); \
				z80->F |= (n & 0x08);		/* MDZ80_FLAG_3 */ \
				z80->F |= ((n & 0x02) << 4);	/* MDZ80_FLAG_5 */ \
				\
				z80->PC += 2; \
				NEXT(16); \
			} while (0)

	Z80I_LDX(I, 1);
	Z80I_LDX(D, -1);

	// LDIR/LDDR		do { (DE++/--) <- (HL++/--) } while (--BC)
	// TODO: Verify emulation of flags 3 and 5.
	// NOTE: Original ASM mdZ80 used a loop here.
	#define Z80I_LDXR(Op, AdrInc) \
		Z80I_LD ## Op ## R : \
			do { \
				const uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				WRITE_BYTE(z80, z80->DE, tmp8); \
				z80->HL += AdrInc; \
				z80->DE += AdrInc; \
				z80->BC--; \
				\
				/* Flags: */ \
				/* - P: set if BC != 0 */ \
				/* - 3: set if (tmp8 + A) & 0x08 (bit 3) */ \
				/* - 5: set if (tmp8 + A) & 0x02 (bit 1) */ \
				z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_C); \
				register const uint8_t n = (tmp8 + z80->A); \
				z80->F |= (n & 0x08);		/* MDZ80_FLAG_3 */ \
				z80->F |= ((n & 0x02) << 4);	/* MDZ80_FLAG_5 */ \
				\
				if (z80->BC) { \
					z80->F |= MDZ80_FLAG_P; \
					NEXT(21); \
				} else { \
					z80->PC += 2; \
					NEXT(16); \
				} \
			} while (0)

	Z80I_LDXR(I, 1);
	Z80I_LDXR(D, -1);

	// CPI/CPD		A - (HL++/--); BC--
	// TODO: Verify emulation of flags 3 and 5; also S, Z, H, and N.
	#define Z80I_CPX(Op, AdrInc) \
		Z80I_CP ## Op : \
			do { \
				const uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				const uint8_t cmp = (z80->A - tmp8); \
				z80->HL += AdrInc; \
				z80->BC--; \
				\
				/* Flags: (TODO: 'lahf' optimization) */ \
				/* - S: set if cmp is negative */ \
				/* - Z: set if cmp == 0 [A == (HL)] */ \
				/* - H: set if "borrow from bit 4" */ \
				/* - P: set if BC != 0 */ \
				/* - N: set */ \
				/* - C: not affected */ \
				/* - 3: set if (cmp - HF) & 0x08 (bit 3) */ \
				/* - 5: set if (cmp - HF) & 0x02 (bit 1) */ \
				z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_C); \
				z80->F |= (cmp & 0x80);		/* MDZ80_FLAG_S */ \
				z80->F |= (cmp != 0 ? MDZ80_FLAG_Z : 0); \
				z80->F |= (z80->A ^ tmp8 ^ cmp) & MDZ80_FLAG_H; \
				z80->F |= (z80->BC ? MDZ80_FLAG_P : 0); \
				z80->F |= MDZ80_FLAG_N; \
				register const uint8_t n = (z80->A - tmp8 - !!(z80->F & MDZ80_FLAG_H)); \
				z80->F |= (n & 0x08);		/* MDZ80_FLAG_3 */ \
				z80->F |= ((n & 0x02) << 4);	/* MDZ80_FLAG_5 */ \
				\
				z80->PC += 2; \
				NEXT(16); \
			} while (0)

	Z80I_CPX(I, 1);
	Z80I_CPX(D, -1);

	// CPIR/CPDR		do { A - (HL++/--) } while (--BC)
	// TODO: Verify emulation of flags 3 and 5; also S, Z, H, and N.
	// TODO: Verify CPIR/CPDR termination behavior.
	#define Z80I_CPXR(Op, AdrInc) \
		Z80I_CP ## Op ## R : \
			do { \
				const uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				const uint8_t cmp = (z80->A - tmp8); \
				z80->HL += AdrInc; \
				z80->BC--; \
				\
				/* Flags: (TODO: 'lahf' optimization) */ \
				/* - S: set if cmp is negative */ \
				/* - Z: set if cmp == 0 [A == (HL)] */ \
				/* - H: set if "borrow from bit 4" */ \
				/* - P: set if BC != 0 */ \
				/* - N: set */ \
				/* - C: not affected */ \
				/* - 3: set if (cmp - HF) & 0x08 (bit 3) */ \
				/* - 5: set if (cmp - HF) & 0x02 (bit 1) */ \
				z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_C); \
				z80->F |= (cmp & 0x80);		/* MDZ80_FLAG_S */ \
				z80->F |= (cmp != 0 ? MDZ80_FLAG_Z : 0); \
				z80->F |= (z80->A ^ tmp8 ^ cmp) & MDZ80_FLAG_H; \
				z80->F |= (z80->BC ? MDZ80_FLAG_P : 0); \
				z80->F |= MDZ80_FLAG_N; \
				register const uint8_t n = (z80->A - tmp8 - !!(z80->F & MDZ80_FLAG_H)); \
				z80->F |= (n & 0x08);		/* MDZ80_FLAG_3 */ \
				z80->F |= ((n & 0x02) << 4);	/* MDZ80_FLAG_5 */ \
				\
				if (z80->BC && cmp != 0) { \
					NEXT(21); \
				} else { \
					z80->PC += 2; \
					NEXT(18); /* FIXME: Gens says 18; manual says 16... */ \
				} \
			} while (0)

	Z80I_CPXR(I, 1);
	Z80I_CPXR(D, -1);

	/*! Arithmetic instructions (8-bit) */
	// TODO: Test these!

	/**
	 * Perform an 8-bit addition and calculate flags.
	 * TODO: 'lahf' optimization.
	 * @param A Destination. (usually z80->A)
	 * @param d Data to be added.
	 * @param c Carry value.
	 *
	 * Reference: http://www.retrogames.com/cgi-bin/wwwthreads/showpost.pl?Board=retroemuprog&Number=3997&page=&view=&mode=flat&sb=
	 * Flags:
	 * - S: set if r is negative; reset otherwise
	 * - Z: set if r is zero; reset otherwise
	 * - 5: copy of bit 5 from r
	 * - H: set if carry from bit 3; reset otherwise
	 * - 3: copy of bit 3 from r
	 * - P/V: set if overflow; reset otherwise
	 * - N: reset
	 * - C: set if carry from bit 7; reset otherwise
	 */
	#define Z80_ADD8_FLAGS(A, d, c) \
		do { \
			/* S, 5, and 3 are copied directly from the result. */ \
			const uint8_t r = ((A) + (d) + (c)); \
			z80->F = (r & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			if (r == 0) z80->F |= MDZ80_FLAG_Z;			/* Z */ \
			z80->F |= ((A)^(d)^(r)) & MDZ80_FLAG_H;			/* H */ \
			z80->F |= ((((A)^(r))&((d)^(r))) >> 5) & MDZ80_FLAG_P;	/* P/V */ \
			z80->F |= ((A) > r);					/* C */ \
			/* Store the result. */ \
			(A) = r; \
		} while (0)

	// ADD A, R		A <- A + R
	#define Z80I_ADD_R(Rsrc) \
		Z80I_ADD_ ## Rsrc : \
			do {\
				Z80_ADD8_FLAGS(z80->A, z80->Rsrc, 0); \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_ADD_R(A);
	Z80I_ADD_R(B);
	Z80I_ADD_R(C);
	Z80I_ADD_R(D);
	Z80I_ADD_R(E);
	Z80I_ADD_R(H);
	Z80I_ADD_R(L);
	Z80I_ADD_R(IXl);
	Z80I_ADD_R(IXh);
	Z80I_ADD_R(IYl);
	Z80I_ADD_R(IYh);

	// ADC A, R		A <- A + R + c
	#define Z80I_ADC_R(Rsrc) \
		Z80I_ADC_ ## Rsrc : \
			do {\
				Z80_ADD8_FLAGS(z80->A, z80->Rsrc, (z80->F & MDZ80_FLAG_C)); \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_ADC_R(A);
	Z80I_ADC_R(B);
	Z80I_ADC_R(C);
	Z80I_ADC_R(D);
	Z80I_ADC_R(E);
	Z80I_ADC_R(H);
	Z80I_ADC_R(L);
	Z80I_ADC_R(IXl);
	Z80I_ADC_R(IXh);
	Z80I_ADC_R(IYl);
	Z80I_ADC_R(IYh);

	/**
	 * Perform an 8-bit subtraction and calculate flags.
	 * TODO: 'lahf' optimization.
	 * @param A Destination. (usually z80->A)
	 * @param d Data to be added.
	 * @param c Carry value. (Borrow)
	 *
	 * Reference: http://www.retrogames.com/cgi-bin/wwwthreads/showpost.pl?Board=retroemuprog&Number=3997&page=&view=&mode=flat&sb=
	 * Flags:
	 * - S: set if r is negative; reset otherwise
	 * - Z: set if r is zero; reset otherwise
	 * - 5: copy of bit 5 from r
	 * - H: set if borrow from bit 4; reset otherwise
	 * - 3: copy of bit 3 from r
	 * - P/V: set if overflow; reset otherwise
	 * - N: set
	 * - C: set if borrow from bit 8; reset otherwise
	 */
	#define Z80_SUB8_FLAGS(A, d, c) \
		do { \
			/* S, 5, and 3 are copied directly from the result. */ \
			const uint8_t r = ((A) - (d) - (c)); \
			z80->F = (r & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)) | MDZ80_FLAG_N; \
			if (r == 0) z80->F |= MDZ80_FLAG_Z;			/* Z */ \
			z80->F |= ((A)^(d)^(r)) & MDZ80_FLAG_H;			/* H */ \
			z80->F |= ((((A)^(d))&((A)^(r))) >> 5) & MDZ80_FLAG_P;	/* P/V */ \
			z80->F |= ((A) < r);					/* C */ \
			/* Store the result. */ \
			(A) = r; \
		} while (0)

	// SUB A, R		A <- A - R
	#define Z80I_SUB_R(Rsrc) \
		Z80I_SUB_ ## Rsrc : \
			do {\
				Z80_SUB8_FLAGS(z80->A, z80->Rsrc, 0); \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_SUB_R(A);
	Z80I_SUB_R(B);
	Z80I_SUB_R(C);
	Z80I_SUB_R(D);
	Z80I_SUB_R(E);
	Z80I_SUB_R(H);
	Z80I_SUB_R(L);
	Z80I_SUB_R(IXl);
	Z80I_SUB_R(IXh);
	Z80I_SUB_R(IYl);
	Z80I_SUB_R(IYh);

	// SBC A, R		A <- A - R - c
	#define Z80I_SBC_R(Rsrc) \
		Z80I_SBC_ ## Rsrc : \
			do {\
				Z80_SUB8_FLAGS(z80->A, z80->Rsrc, (z80->F & MDZ80_FLAG_C)); \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_SBC_R(A);
	Z80I_SBC_R(B);
	Z80I_SBC_R(C);
	Z80I_SBC_R(D);
	Z80I_SBC_R(E);
	Z80I_SBC_R(H);
	Z80I_SBC_R(L);
	Z80I_SBC_R(IXl);
	Z80I_SBC_R(IXh);
	Z80I_SBC_R(IYl);
	Z80I_SBC_R(IYh);

	// CP R			(null) <- A - R; set flags
	// NOTE: Temp variable 'A' is modified. This may waste CPU
	// cycles if the compiler doesn't optimize it out.
	#define Z80I_CP_R(Rsrc) \
		Z80I_CP_ ## Rsrc : \
			do {\
				register uint8_t A = z80->A; \
				Z80_SUB8_FLAGS(A, z80->Rsrc, 0); \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_CP_R(A);
	Z80I_CP_R(B);
	Z80I_CP_R(C);
	Z80I_CP_R(D);
	Z80I_CP_R(E);
	Z80I_CP_R(H);
	Z80I_CP_R(L);
	Z80I_CP_R(IXl);
	Z80I_CP_R(IXh);
	Z80I_CP_R(IYl);
	Z80I_CP_R(IYh);

	// ADD A, N		A <- A + imm8
	Z80I_ADD_N: {
		const uint8_t d = read_byte_offset_pc(z80, 1);
		Z80_ADD8_FLAGS(z80->A, d, 0);
		z80->PC += 2;
		NEXT(7);
	}

	// ADC A, N		A <- A + imm8 + c
	Z80I_ADC_N: {
		const uint8_t d = read_byte_offset_pc(z80, 1);
		Z80_ADD8_FLAGS(z80->A, d, (z80->F & MDZ80_FLAG_C));
		z80->PC += 2;
		NEXT(7);
	}

	// SUB A, N		A <- A - imm8
	Z80I_SUB_N: {
		const uint8_t d = read_byte_offset_pc(z80, 1);
		Z80_SUB8_FLAGS(z80->A, d, 0);
		z80->PC += 2;
		NEXT(7);
	}

	// SBC A, N		A <- A - imm8 - c
	Z80I_SBC_N: {
		const uint8_t d = read_byte_offset_pc(z80, 1);
		Z80_SUB8_FLAGS(z80->A, d, (z80->F & MDZ80_FLAG_C));
		z80->PC += 2;
		NEXT(7);
	}

	// CP A, N		(null) <- A - N; set flags
	// NOTE: Temp variable 'A' is modified. This may waste CPU
	// cycles if the compiler doesn't optimize it out.
	Z80I_CP_N: {
		const uint8_t d = read_byte_offset_pc(z80, 1);
		register uint8_t A = z80->A;
		Z80_SUB8_FLAGS(A, d, 0);
		z80->PC++;
		NEXT(7);
	}

	// ADD A, (HL)		A <- A + (HL)
	Z80I_ADD_mHL: {
		const uint8_t d = READ_BYTE(z80, z80->HL);
		Z80_ADD8_FLAGS(z80->A, d, 0);
		z80->PC += 2;
		NEXT(7);
	}

	// ADC A, (HL)		A <- A + (HL) + c
	Z80I_ADC_mHL: {
		const uint8_t d = READ_BYTE(z80, z80->HL);
		Z80_ADD8_FLAGS(z80->A, d, (z80->F & MDZ80_FLAG_C));
		z80->PC += 2;
		NEXT(7);
	}

	// SUB A, (HL)		A <- A - (HL)
	Z80I_SUB_mHL: {
		const uint8_t d = READ_BYTE(z80, z80->HL);
		Z80_SUB8_FLAGS(z80->A, d, 0);
		z80->PC += 2;
		NEXT(7);
	}

	// SBC A, (HL)		A <- A - (HL) - c
	Z80I_SBC_mHL: {
		const uint8_t d = READ_BYTE(z80, z80->HL);
		Z80_SUB8_FLAGS(z80->A, d, (z80->F & MDZ80_FLAG_C));
		z80->PC += 2;
		NEXT(7);
	}

	// CP A, (HL)		(null) <- A - (HL); set flags
	// NOTE: Temp variable 'A' is modified. This may waste CPU
	// cycles if the compiler doesn't optimize it out.
	Z80I_CP_mHL: {
		const uint8_t d = READ_BYTE(z80, z80->HL);
		register uint8_t A = z80->A;
		Z80_SUB8_FLAGS(A, d, 0);
		z80->PC++;
		NEXT(7);
	}

	// ADD A, (XY+d)	A <- A + (XY+d)
	// FIXME: Z80 docs say 19 cycles; mdZ80 says 15?
	#define Z80I_ADD_mXYd(Ridx) \
		Z80I_ADD_m ## Ridx ## d : \
			do { \
				const int8_t d_idx = read_sbyte_offset_pc(z80, 1); \
				const uint8_t d = READ_BYTE(z80, z80->Ridx + d_idx); \
				Z80_ADD8_FLAGS(z80->A, d, 0); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_ADD_mXYd(IX);
	Z80I_ADD_mXYd(IY);

	// ADC A, (XY+d)	A <- A + (XY+d) + c
	// FIXME: Z80 docs say 19 cycles; mdZ80 says 15?
	#define Z80I_ADC_mXYd(Ridx) \
		Z80I_ADC_m ## Ridx ## d : \
			do { \
				const int8_t d_idx = read_sbyte_offset_pc(z80, 1); \
				const uint8_t d = READ_BYTE(z80, z80->Ridx + d_idx); \
				Z80_ADD8_FLAGS(z80->A, d, (z80->F & MDZ80_FLAG_C)); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_ADC_mXYd(IX);
	Z80I_ADC_mXYd(IY);

	// SUB A, (XY+d)	A <- A - (XY+d)
	// FIXME: Z80 docs say 19 cycles; mdZ80 says 15?
	#define Z80I_SUB_mXYd(Ridx) \
		Z80I_SUB_m ## Ridx ## d : \
			do { \
				const int8_t d_idx = read_sbyte_offset_pc(z80, 1); \
				const uint8_t d = READ_BYTE(z80, z80->Ridx + d_idx); \
				Z80_SUB8_FLAGS(z80->A, d, 0); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_SUB_mXYd(IX);
	Z80I_SUB_mXYd(IY);

	// SBC A, (XY+d)	A <- A - (XY+d) - c
	// FIXME: Z80 docs say 19 cycles; mdZ80 says 15?
	#define Z80I_SBC_mXYd(Ridx) \
		Z80I_SBC_m ## Ridx ## d : \
			do { \
				const int8_t d_idx = read_sbyte_offset_pc(z80, 1); \
				const uint8_t d = READ_BYTE(z80, z80->Ridx + d_idx); \
				Z80_SUB8_FLAGS(z80->A, d, (z80->F & MDZ80_FLAG_C)); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_SBC_mXYd(IX);
	Z80I_SBC_mXYd(IY);

	// CP A, (XY+d)		(null) <- A - (XY+d); set flags
	// NOTE: Temp variable 'A' is modified. This may waste CPU
	// cycles if the compiler doesn't optimize it out.
	#define Z80I_CP_mXYd(Ridx) \
		Z80I_CP_m ## Ridx ## d : \
			do { \
				const int8_t d_idx = read_sbyte_offset_pc(z80, 1); \
				const uint8_t d = READ_BYTE(z80, z80->Ridx + d_idx); \
				register uint8_t A = z80->A; \
				Z80_SUB8_FLAGS(A, d, 0); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_CP_mXYd(IX);
	Z80I_CP_mXYd(IY);

	/*! Logic instructions (8-bit) */
	// TODO

	/*! INC/DEC instructions (8-bit) */

	#define __MDZ80_IN_INCDEC 19840519
	#include "mdZ80_INC_DEC.inc.h"
	#undef __MDZ80_IN_INCDEC

	// INC/DEC R		R8++/--
	// TODO: Verify emulation of flags 3 and 5.
	#define Z80I_INCDEC_R(Op, ValInc, Rdest) \
		Z80I_ ## Op ## _ ## Rdest : \
			do { \
				z80->F &= MDZ80_FLAG_C; \
				z80->F |= mdZ80_ ## Op ## _Flags_Table[z80->Rdest]; \
				z80->Rdest += ValInc; \
				z80->PC++; \
				NEXT(4); \
			} while (0)

	Z80I_INCDEC_R(INC, 1, A);
	Z80I_INCDEC_R(INC, 1, B);
	Z80I_INCDEC_R(INC, 1, C);
	Z80I_INCDEC_R(INC, 1, D);
	Z80I_INCDEC_R(INC, 1, E);
	Z80I_INCDEC_R(INC, 1, H);
	Z80I_INCDEC_R(INC, 1, L);
	Z80I_INCDEC_R(INC, 1, IXl);
	Z80I_INCDEC_R(INC, 1, IXh);
	Z80I_INCDEC_R(INC, 1, IYl);
	Z80I_INCDEC_R(INC, 1, IYh);

	Z80I_INCDEC_R(DEC, -1, A);
	Z80I_INCDEC_R(DEC, -1, B);
	Z80I_INCDEC_R(DEC, -1, C);
	Z80I_INCDEC_R(DEC, -1, D);
	Z80I_INCDEC_R(DEC, -1, E);
	Z80I_INCDEC_R(DEC, -1, H);
	Z80I_INCDEC_R(DEC, -1, L);
	Z80I_INCDEC_R(DEC, -1, IXl);
	Z80I_INCDEC_R(DEC, -1, IXh);
	Z80I_INCDEC_R(DEC, -1, IYl);
	Z80I_INCDEC_R(DEC, -1, IYh);

	// INC/DEC (HL)		(HL)++/--
	// TODO: Verify emulation of flags 3 and 5.
	#define Z80I_INCDEC_mHL(Op, ValInc) \
		Z80I_ ## Op ## _mHL : \
			do { \
				z80->F &= MDZ80_FLAG_C; \
				uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				z80->F |= mdZ80_ ## Op ## _Flags_Table[tmp8]; \
				tmp8 += ValInc; \
				WRITE_BYTE(z80, z80->HL, tmp8); \
				z80->PC++; \
				NEXT(11); \
			} while (0)

	Z80I_INCDEC_mHL(INC, 1);
	Z80I_INCDEC_mHL(DEC, -1);

	// INC/DEC (XY+d)		(XY+d)++/--
	// TODO: Verify emulation of flags 3 and 5.
	#define Z80I_INCDEC_mXYd(Op, ValInc, Ridx) \
		Z80I_ ## Op ## _m ## Ridx ## d : \
			do { \
				z80->F &= MDZ80_FLAG_C; \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				z80->F |= mdZ80_ ## Op ## _Flags_Table[tmp8]; \
				tmp8 += ValInc; \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 2; \
				NEXT(22); \
			} while (0)

	Z80I_INCDEC_mXYd(INC, 1, IX);
	Z80I_INCDEC_mXYd(INC, 1, IY);
	Z80I_INCDEC_mXYd(DEC, -1, IX);
	Z80I_INCDEC_mXYd(DEC, -1, IY);

	/*! Miscellaneous instructions */

	// DAA				[who knows?]
	Z80I_DAA: {
		#define __MDZ80_IN_DAA 19840519
		#include "mdZ80_DAA.inc.h"
		#undef __MDZ80_IN_DAA
		unsigned int DAA_offset = (z80->AF & 0x3FF);
		DAA_offset |= ((z80->F & MDZ80_FLAG_H) << 6);
		z80->A = mdZ80_DAA_Table[DAA_offset];
		z80->PC++;
		NEXT(4);
	}

	// CPL				A <- ~A
	Z80I_CPL: {
		z80->A = ~z80->A;
		z80->F |= (MDZ80_FLAG_H | MDZ80_FLAG_N);
		z80->PC++;
		NEXT(4);
	}

	// NEG				A <- -A
	// TODO: Set flags. (mdZ80.asm uses 'lahf')
	Z80I_NEG: {
		z80->A = -z80->A;
		// TODO: Set flags.
		z80->PC += 2;
		NEXT(8);
	}

	// CCF				F ^= MDZ80_FLAG_C
	// TODO: Verify emulation of flags 3 and 5.
	/**
	 * Flags:
	 * - S: not modified
	 * - Z: not modified
	 * - 5: copy of bit 5 from A
	 * - H: old C bit
	 * - 3: copy of bit 3 from A
	 * - P/V: not modified
	 * - N: reset
	 * - C: ~C
	 */
	Z80I_CCF: {
		uint8_t flags = (z80->F & (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_P | MDZ80_FLAG_C));
		flags |= ((z80->F & MDZ80_FLAG_C) << 4);
		flags ^= MDZ80_FLAG_C;
		flags |= (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3));
		z80->PC++;
		NEXT(4);
	}

	// SCF				F |= MDZ80_FLAG_C
	// TODO: Verify emulation of flags 3 and 5.
	/**
	 * Flags:
	 * - S: not modified
	 * - Z: not modified
	 * - 5: copy of bit 5 from A
	 * - H: reset
	 * - 3: copy of bit 3 from A
	 * - P/V: not modified
	 * - N: reset
	 * - C: set
	 */
	Z80I_SCF: {
		uint8_t flags = (z80->F & (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_P));
		flags |= MDZ80_FLAG_C;
		flags |= (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3));
		z80->PC++;
		NEXT(4);
	}

	// HALT				HAMMERZEIT!
	Z80I_HALT: {
		z80->Status |= MDZ80_STATUS_HALTED;
		odo = -1;
		z80->PC++;
		goto z80_Exec_Really_Quit;
	}

	// DI				IFF1 = IFF2 = 0
	Z80I_DI: {
		z80->IFF = 0;

		// NOTE: Don't check odo here.
		z80->PC++;
		NEXT_NOCHECK(4);
	}

	// EI				IFF1 = IFF2 = 1
	Z80I_EI: {
		z80->IFF = 3;

		// Force the emulation loop to end early.
		// The remaining cycles will be used at the start
		// of the next mdZ80_exec().
		// This is needed in order to check interrupts.
		z80->CycleSup = odo;
		odo = 0;

		// NOTE: Don't check odo here.
		z80->PC++;
		NEXT_NOCHECK(4);
	}

	// IM0/IM1/IM2			IM = 0|1|2
	#define Z80I_IMX(IntMode) \
		Z80I_IM ## IntMode : \
			do { \
				z80->IM = IntMode; \
				z80->PC += 2; \
				NEXT(8); \
			} while (0)

	Z80I_IMX(0);
	Z80I_IMX(1);
	Z80I_IMX(2);

	/*! Arithmetic instructions (16-bit) */
	// TODO: Test these!

	/**
	 * Perform a 16-bit addition and calculate flags.
	 * TODO: 'lahf' optimization.
	 * @param A Destination.
	 * @param d Data to be added.
	 * @param c Carry value.
	 *
	 * Reference: http://www.retrogames.com/cgi-bin/wwwthreads/showpost.pl?Board=retroemuprog&Number=3997&page=&view=&mode=flat&sb=
	 * Flags:
	 * - S: set if r is negative; reset otherwise
	 * - Z: set if r is zero; reset otherwise
	 * - 5: copy of bit 12 (5+8) from r
	 * - H: set if carry from bit 11 (3+8); reset otherwise
	 * - 3: copy of bit 11 (3+8) from r
	 * - P/V: set if overflow; reset otherwise
	 * - N: reset
	 * - C: set if carry from bit 15; reset otherwise
	 */
	#define Z80_ADD16_FLAGS(A, d, c) \
		do { \
			/* S, 5, and 3 are copied directly from the result. */ \
			const uint16_t r = ((A) + (d) + (c)); \
			const uint8_t AHI = ((A) >> 8); \
			const uint8_t dHI = ((d) >> 8); \
			const uint8_t rHI = (r >> 8); \
			z80->F = (rHI & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			if (r == 0) z80->F |= MDZ80_FLAG_Z;				/* Z */ \
			z80->F |= ((AHI)^(dHI)^(rHI)) & MDZ80_FLAG_H;			/* H */ \
			z80->F |= ((((AHI)^(rHI))&((dHI)^(rHI))) >> 5) & MDZ80_FLAG_P;	/* P/V */ \
			z80->F |= ((AHI) > rHI);					/* C */ \
			/* Store the result. */ \
			(A) = r; \
		} while (0)

	// ADD RR, RR		Rd16 <- Rd16 + Rs16
	#define Z80I_ADD_RR_RR(Rdest, Rsrc) \
		Z80I_ADD_ ## Rdest ## _ ## Rsrc : \
			do {\
				Z80_ADD16_FLAGS(z80->Rdest, z80->Rsrc, 0); \
				z80->PC++; \
				NEXT(11); \
			} while (0)

	Z80I_ADD_RR_RR(HL,BC);
	Z80I_ADD_RR_RR(HL,DE);
	Z80I_ADD_RR_RR(HL,HL);
	Z80I_ADD_RR_RR(HL,SP);
	Z80I_ADD_RR_RR(IX,BC);
	Z80I_ADD_RR_RR(IX,DE);
	Z80I_ADD_RR_RR(IX,IX);
	Z80I_ADD_RR_RR(IX,SP);
	Z80I_ADD_RR_RR(IY,BC);
	Z80I_ADD_RR_RR(IY,DE);
	Z80I_ADD_RR_RR(IY,IY);
	Z80I_ADD_RR_RR(IY,SP);

	// ADC HL, RR		HL <- HL + R16 + c
	#define Z80I_ADC_HL_RR(Rsrc) \
		Z80I_ADC_HL_ ## Rsrc : \
			do {\
				Z80_ADD16_FLAGS(z80->HL, z80->Rsrc, (z80->F & MDZ80_FLAG_C)); \
				z80->PC++; \
				NEXT(15); \
			} while (0)

	Z80I_ADC_HL_RR(BC);
	Z80I_ADC_HL_RR(DE);
	Z80I_ADC_HL_RR(HL);
	Z80I_ADC_HL_RR(SP);

	/**
	 * Perform a 16-bit subtraction and calculate flags.
	 * TODO: 'lahf' optimization.
	 * @param A Destination.
	 * @param d Data to be added.
	 * @param c Carry value. (Borrow)
	 *
	 * Reference: http://www.retrogames.com/cgi-bin/wwwthreads/showpost.pl?Board=retroemuprog&Number=3997&page=&view=&mode=flat&sb=
	 * Flags:
	 * - S: set if r is negative; reset otherwise
	 * - Z: set if r is zero; reset otherwise
	 * - 5: copy of bit 12 (5+8) from r
	 * - H: set if borrow from bit 12 (4+8); reset otherwise
	 * - 3: copy of bit 11 (3+8) from r
	 * - P/V: set if overflow; reset otherwise
	 * - N: set
	 * - C: set if borrow from bit 16; reset otherwise
	 */
	#define Z80_SUB16_FLAGS(A, d, c) \
		do { \
			/* S, 5, and 3 are copied directly from the result. */ \
			const uint16_t r = ((A) - (d) - (c)); \
			const uint8_t AHI = ((A) >> 8); \
			const uint8_t dHI = ((d) >> 8); \
			const uint8_t rHI = (r >> 8); \
			z80->F = (rHI & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)) | MDZ80_FLAG_N; \
			if (r == 0) z80->F |= MDZ80_FLAG_Z;				/* Z */ \
			z80->F |= ((AHI)^(dHI)^(rHI)) & MDZ80_FLAG_H;			/* H */ \
			z80->F |= ((((AHI)^(dHI))&((AHI)^(rHI))) >> 5) & MDZ80_FLAG_P;	/* P/V */ \
			z80->F |= ((AHI) < rHI);					/* C */ \
			/* Store the result. */ \
			(A) = r; \
		} while (0)

	// ADC HL, RR		HL <- HL - R16 - c
	#define Z80I_SBC_HL_RR(Rsrc) \
		Z80I_SBC_HL_ ## Rsrc : \
			do {\
				Z80_SUB16_FLAGS(z80->HL, z80->Rsrc, (z80->F & MDZ80_FLAG_C)); \
				z80->PC++; \
				NEXT(15); \
			} while (0)

	Z80I_SBC_HL_RR(BC);
	Z80I_SBC_HL_RR(DE);
	Z80I_SBC_HL_RR(HL);
	Z80I_SBC_HL_RR(SP);

	// INC/DEC RR		R16++/--
	#define Z80I_INCDEC_RR(Op, ValInc, Rdest) \
		Z80I_ ## Op ## _ ## Rdest : \
			do { \
				z80->Rdest++; \
				z80->PC++; \
				NEXT(6); \
			} while (0)

	Z80I_INCDEC_RR(INC, 1, BC);
	Z80I_INCDEC_RR(INC, 1, DE);
	Z80I_INCDEC_RR(INC, 1, HL);
	Z80I_INCDEC_RR(INC, 1, IX);
	Z80I_INCDEC_RR(INC, 1, IY);
	Z80I_INCDEC_RR(INC, 1, SP);

	Z80I_INCDEC_RR(DEC, -1, BC);
	Z80I_INCDEC_RR(DEC, -1, DE);
	Z80I_INCDEC_RR(DEC, -1, HL);
	Z80I_INCDEC_RR(DEC, -1, IX);
	Z80I_INCDEC_RR(DEC, -1, IY);
	Z80I_INCDEC_RR(DEC, -1, SP);

	/*! Rotate and shift instructions */

	// RLCA			Rotate A left; copy sign bit to CF
	// Flags: --503-0C
	// TODO: Verify emulation of flags 3 and 5.
	// TODO: Verify carry emulation.
	Z80I_RLCA: {
		z80->A = (z80->A << 1) | (z80->A >> 7);
		z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_P);
		z80->F |= (z80->A & 0x01); /* MDZ80_FLAG_C */
		z80->F |= (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3));
		z80->PC++;
		NEXT(4);
	}

	// RLA			Rotate A left through CF
	// Flags: --503-0C
	// TODO: Verify emulation of flags 3 and 5.
	// TODO: Verify carry emulation.
	Z80I_RLA: {
		const unsigned int tmp = (z80->A << 1) | (z80->F & MDZ80_FLAG_C);
		z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_P);
		z80->A = (tmp & 0xFF);
		z80->F |= (tmp >> 8); /* MDZ80_FLAG_C */
		z80->F |= (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3));
		z80->PC++;
		NEXT(4);
	}

	// RRCA			Rotate A right; copy sign bit to CF
	// Flags: --503-0C
	// TODO: Verify emulation of flags 3 and 5.
	// TODO: Verify carry emulation.
	Z80I_RRCA: {
		z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_P);
		z80->F |= (z80->A & 0x01);	/* MDZ80_FLAG_C */
		z80->A = (z80->A >> 1) | (z80->A << 7);
		z80->F |= (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3));
		z80->PC++;
		NEXT(4);
	}

	// RRA			Rotate A right through CF
	// Flags: --503-0C
	// TODO: Verify emulation of flags 3 and 5.
	// TODO: Verify carry emulation.
	Z80I_RRA: {
		const uint8_t tmp = (z80->A >> 1) | ((z80->F & MDZ80_FLAG_C) << 7); \
		z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z | MDZ80_FLAG_P); \
		z80->F |= (z80->A & MDZ80_FLAG_C); \
		z80->A = tmp; \
		z80->F |= (z80->A & (MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
		z80->PC++;
		NEXT(4);
	}

	/**
	 * Z80U_OP_RLC: Rotate Dest left; copy sign bit to CF.
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_RLC(Dest) \
		do { \
			Dest = (Dest << 1) | (Dest >> 7); \
			z80->F = (Dest & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			z80->F |= (Dest >> 7);	/* MDZ80_FLAG_C */ \
			if (Dest) z80->F |= MDZ80_FLAG_Z; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	/**
	 * Z80U_OP_RL: Rotate Dest left through CF.
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_RL(Dest) \
		do { \
			const unsigned int tmp = (Dest << 1) | (z80->F & MDZ80_FLAG_C); \
			Dest = (tmp & 0xFF); \
			z80->F = (Dest & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			z80->F |= (tmp >> 8); /* MDZ80_FLAG_C */ \
			if (Dest) z80->F |= MDZ80_FLAG_Z; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	/**
	 * Z80U_OP_RRC: Rotate Dest right; copy sign bit to CF.
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_RRC(Dest) \
		do { \
			z80->F = (Dest & 0x01);	/* MDZ80_FLAG_C */ \
			Dest = (Dest >> 1) | (Dest << 7); \
			z80->F |= (Dest & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			if (Dest) z80->F |= MDZ80_FLAG_Z; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	/**
	 * Z80U_OP_RR: Rotate Dest right through CF.
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_RR(Dest) \
		do { \
			const uint8_t tmp = (Dest >> 1) | ((z80->F & MDZ80_FLAG_C) << 7); \
			z80->F = (Dest & 0x01);	/* MDZ80_FLAG_C */ \
			Dest = tmp; \
			z80->F |= (Dest & (MDZ80_FLAG_S | MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			if (Dest) z80->F |= MDZ80_FLAG_Z; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	/**
	 * Z80I_ROT_R: Rotate R8 using the given operation.
	 * This is a Z80 extended instruction. (CB-prefix)
	 * Flags: SZ503P0C
	 * @param Op Rotate operation.
	 * @param Rdest Destination register.
	 */
	#define Z80I_ROT_R(Op, Rdest) \
		Z80I_ ## Op ## _ ## Rdest : \
			do { \
				Z80U_OP_ ## Op(z80->Rdest); \
				z80->PC += 2; \
				NEXT(8); \
			} while (0)

	Z80I_ROT_R(RLC, A);
	Z80I_ROT_R(RLC, B);
	Z80I_ROT_R(RLC, C);
	Z80I_ROT_R(RLC, D);
	Z80I_ROT_R(RLC, E);
	Z80I_ROT_R(RLC, H);
	Z80I_ROT_R(RLC, L);

	Z80I_ROT_R(RL, A);
	Z80I_ROT_R(RL, B);
	Z80I_ROT_R(RL, C);
	Z80I_ROT_R(RL, D);
	Z80I_ROT_R(RL, E);
	Z80I_ROT_R(RL, H);
	Z80I_ROT_R(RL, L);

	Z80I_ROT_R(RRC, A);
	Z80I_ROT_R(RRC, B);
	Z80I_ROT_R(RRC, C);
	Z80I_ROT_R(RRC, D);
	Z80I_ROT_R(RRC, E);
	Z80I_ROT_R(RRC, H);
	Z80I_ROT_R(RRC, L);

	Z80I_ROT_R(RR, A);
	Z80I_ROT_R(RR, B);
	Z80I_ROT_R(RR, C);
	Z80I_ROT_R(RR, D);
	Z80I_ROT_R(RR, E);
	Z80I_ROT_R(RR, H);
	Z80I_ROT_R(RR, L);

	/**
	 * Z80U_OP_SLA: Shift Left (arithmetic)
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_SLA(Dest) \
		do { \
			const uint8_t tmp = (Dest << 1); \
			z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z); \
			z80->F |= (Dest >> 8);	/* MDZ80_FLAG_C */ \
			z80->F |= (tmp & (MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			Dest = tmp; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	/**
	 * Z80U_OP_SLL: Shift Left (logical)
	 * NOTE: Same as SLA, but sets bit 0. [Undocumented instruction!]
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_SLL(Dest) \
		do { \
			const uint8_t tmp = ((Dest << 1) + 1); \
			z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z); \
			z80->F |= (Dest >> 8);	/* MDZ80_FLAG_C */ \
			z80->F |= (tmp & (MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			Dest = tmp; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	Z80I_ROT_R(SLA, A);
	Z80I_ROT_R(SLA, B);
	Z80I_ROT_R(SLA, C);
	Z80I_ROT_R(SLA, D);
	Z80I_ROT_R(SLA, E);
	Z80I_ROT_R(SLA, H);
	Z80I_ROT_R(SLA, L);

	Z80I_ROT_R(SLL, A);
	Z80I_ROT_R(SLL, B);
	Z80I_ROT_R(SLL, C);
	Z80I_ROT_R(SLL, D);
	Z80I_ROT_R(SLL, E);
	Z80I_ROT_R(SLL, H);
	Z80I_ROT_R(SLL, L);

	/**
	 * Z80U_OP_SRA: Shift Right (arithmetic)
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_SRA(Dest) \
		do { \
			const int8_t Orig = *((int8_t*)&(Dest)); \
			const int8_t tmp = (Orig >> 1); \
			z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z); \
			z80->F |= (Orig & 1);	/* MDZ80_FLAG_C */ \
			z80->F |= (tmp & (MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			Dest = *((uint8_t*)&tmp); \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	/**
	 * Z80U_OP_SRL: Shift Right (logical)
	 * @param Dest Destination.
	 * Flags: SZ503P0C
	 * TODO: Verify emulation of flags 3 and 5.
	 * TODO: Verify carry emulation.
	 */
	#define Z80U_OP_SRL(Dest) \
		do { \
			const uint8_t tmp = (Dest >> 1); \
			z80->F &= (MDZ80_FLAG_S | MDZ80_FLAG_Z); \
			z80->F |= (Dest >> 8);	/* MDZ80_FLAG_C */ \
			z80->F |= (tmp & (MDZ80_FLAG_5 | MDZ80_FLAG_3)); \
			Dest = tmp; \
			z80->F |= calc_parity_flag(Dest); \
		} while (0)

	Z80I_ROT_R(SRA, A);
	Z80I_ROT_R(SRA, B);
	Z80I_ROT_R(SRA, C);
	Z80I_ROT_R(SRA, D);
	Z80I_ROT_R(SRA, E);
	Z80I_ROT_R(SRA, H);
	Z80I_ROT_R(SRA, L);

	Z80I_ROT_R(SRL, A);
	Z80I_ROT_R(SRL, B);
	Z80I_ROT_R(SRL, C);
	Z80I_ROT_R(SRL, D);
	Z80I_ROT_R(SRL, E);
	Z80I_ROT_R(SRL, H);
	Z80I_ROT_R(SRL, L);

	/**
	 * Z80I_ROT_mHL: Rotate (HL) using the given operation.
	 * This is a Z80 extended instruction. (CB-prefix)
	 * @param Op Rotate operation.
	 * Flags: SZ503P0C
	 */
	#define Z80I_ROT_mHL(Op) \
		Z80I_ ## Op ## _mHL : \
			do { \
				uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				Z80U_OP_ ## Op(tmp8); \
				WRITE_BYTE(z80, z80->HL, tmp8); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_ROT_mHL(RLC);
	Z80I_ROT_mHL(RL);
	Z80I_ROT_mHL(RRC);
	Z80I_ROT_mHL(RR);
	Z80I_ROT_mHL(SLA);
	Z80I_ROT_mHL(SLL);
	Z80I_ROT_mHL(SRA);
	Z80I_ROT_mHL(SRL);

	/**
	 * Z80I_ROT_mXYd: Rotate (XY+d) using the given operation.
	 * This is a Z80 extended instruction. (DDCB/FDCB-prefix)
	 * @param Op Rotate operation.
	 * @param Ridx Index register.
	 * Flags: SZ503P0C
	 */
	#define Z80I_ROT_mXYd(Op, Ridx) \
		Z80I_ ## Op ## _m ## Ridx ## d : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				Z80U_OP_ ## Op(tmp8); \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 2; \
				NEXT(23); \
			} while (0)

	/**
	 * Z80I_ROT_mXYd_R: Rotate (XY+d) using the given operation.
	 * Result is also copied to R8.
	 * This is a Z80 extended instruction. (DDCB/FDCB-prefix)
	 * @param Op Rotate operation.
	 * @param Ridx Index register.
	 * Flags: SZ503P0C
	 */
	#define Z80I_ROT_mXYd_R(Op, Ridx, Rdest) \
		Z80I_ ## Op ## _m ## Ridx ## d_ ## Rdest : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				Z80U_OP_ ## Op(tmp8); \
				z80->Rdest = tmp8; \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 2; \
				NEXT(23); \
			} while (0)

	Z80I_ROT_mXYd(RLC, IX);
	Z80I_ROT_mXYd_R(RLC, IX, A);
	Z80I_ROT_mXYd_R(RLC, IX, B);
	Z80I_ROT_mXYd_R(RLC, IX, C);
	Z80I_ROT_mXYd_R(RLC, IX, D);
	Z80I_ROT_mXYd_R(RLC, IX, E);
	Z80I_ROT_mXYd_R(RLC, IX, H);
	Z80I_ROT_mXYd_R(RLC, IX, L);

	Z80I_ROT_mXYd(RLC, IY);
	Z80I_ROT_mXYd_R(RLC, IY, A);
	Z80I_ROT_mXYd_R(RLC, IY, B);
	Z80I_ROT_mXYd_R(RLC, IY, C);
	Z80I_ROT_mXYd_R(RLC, IY, D);
	Z80I_ROT_mXYd_R(RLC, IY, E);
	Z80I_ROT_mXYd_R(RLC, IY, H);
	Z80I_ROT_mXYd_R(RLC, IY, L);

	Z80I_ROT_mXYd(RL, IX);
	Z80I_ROT_mXYd_R(RL, IX, A);
	Z80I_ROT_mXYd_R(RL, IX, B);
	Z80I_ROT_mXYd_R(RL, IX, C);
	Z80I_ROT_mXYd_R(RL, IX, D);
	Z80I_ROT_mXYd_R(RL, IX, E);
	Z80I_ROT_mXYd_R(RL, IX, H);
	Z80I_ROT_mXYd_R(RL, IX, L);

	Z80I_ROT_mXYd(RL, IY);
	Z80I_ROT_mXYd_R(RL, IY, A);
	Z80I_ROT_mXYd_R(RL, IY, B);
	Z80I_ROT_mXYd_R(RL, IY, C);
	Z80I_ROT_mXYd_R(RL, IY, D);
	Z80I_ROT_mXYd_R(RL, IY, E);
	Z80I_ROT_mXYd_R(RL, IY, H);
	Z80I_ROT_mXYd_R(RL, IY, L);

	Z80I_ROT_mXYd(RRC, IX);
	Z80I_ROT_mXYd_R(RRC, IX, A);
	Z80I_ROT_mXYd_R(RRC, IX, B);
	Z80I_ROT_mXYd_R(RRC, IX, C);
	Z80I_ROT_mXYd_R(RRC, IX, D);
	Z80I_ROT_mXYd_R(RRC, IX, E);
	Z80I_ROT_mXYd_R(RRC, IX, H);
	Z80I_ROT_mXYd_R(RRC, IX, L);

	Z80I_ROT_mXYd(RRC, IY);
	Z80I_ROT_mXYd_R(RRC, IY, A);
	Z80I_ROT_mXYd_R(RRC, IY, B);
	Z80I_ROT_mXYd_R(RRC, IY, C);
	Z80I_ROT_mXYd_R(RRC, IY, D);
	Z80I_ROT_mXYd_R(RRC, IY, E);
	Z80I_ROT_mXYd_R(RRC, IY, H);
	Z80I_ROT_mXYd_R(RRC, IY, L);

	Z80I_ROT_mXYd(RR, IX);
	Z80I_ROT_mXYd_R(RR, IX, A);
	Z80I_ROT_mXYd_R(RR, IX, B);
	Z80I_ROT_mXYd_R(RR, IX, C);
	Z80I_ROT_mXYd_R(RR, IX, D);
	Z80I_ROT_mXYd_R(RR, IX, E);
	Z80I_ROT_mXYd_R(RR, IX, H);
	Z80I_ROT_mXYd_R(RR, IX, L);

	Z80I_ROT_mXYd(RR, IY);
	Z80I_ROT_mXYd_R(RR, IY, A);
	Z80I_ROT_mXYd_R(RR, IY, B);
	Z80I_ROT_mXYd_R(RR, IY, C);
	Z80I_ROT_mXYd_R(RR, IY, D);
	Z80I_ROT_mXYd_R(RR, IY, E);
	Z80I_ROT_mXYd_R(RR, IY, H);
	Z80I_ROT_mXYd_R(RR, IY, L);

	Z80I_ROT_mXYd(SLA, IX);
	Z80I_ROT_mXYd_R(SLA, IX, A);
	Z80I_ROT_mXYd_R(SLA, IX, B);
	Z80I_ROT_mXYd_R(SLA, IX, C);
	Z80I_ROT_mXYd_R(SLA, IX, D);
	Z80I_ROT_mXYd_R(SLA, IX, E);
	Z80I_ROT_mXYd_R(SLA, IX, H);
	Z80I_ROT_mXYd_R(SLA, IX, L);

	Z80I_ROT_mXYd(SLA, IY);
	Z80I_ROT_mXYd_R(SLA, IY, A);
	Z80I_ROT_mXYd_R(SLA, IY, B);
	Z80I_ROT_mXYd_R(SLA, IY, C);
	Z80I_ROT_mXYd_R(SLA, IY, D);
	Z80I_ROT_mXYd_R(SLA, IY, E);
	Z80I_ROT_mXYd_R(SLA, IY, H);
	Z80I_ROT_mXYd_R(SLA, IY, L);

	Z80I_ROT_mXYd(SLL, IX);
	Z80I_ROT_mXYd_R(SLL, IX, A);
	Z80I_ROT_mXYd_R(SLL, IX, B);
	Z80I_ROT_mXYd_R(SLL, IX, C);
	Z80I_ROT_mXYd_R(SLL, IX, D);
	Z80I_ROT_mXYd_R(SLL, IX, E);
	Z80I_ROT_mXYd_R(SLL, IX, H);
	Z80I_ROT_mXYd_R(SLL, IX, L);

	Z80I_ROT_mXYd(SLL, IY);
	Z80I_ROT_mXYd_R(SLL, IY, A);
	Z80I_ROT_mXYd_R(SLL, IY, B);
	Z80I_ROT_mXYd_R(SLL, IY, C);
	Z80I_ROT_mXYd_R(SLL, IY, D);
	Z80I_ROT_mXYd_R(SLL, IY, E);
	Z80I_ROT_mXYd_R(SLL, IY, H);
	Z80I_ROT_mXYd_R(SLL, IY, L);

	Z80I_ROT_mXYd(SRA, IX);
	Z80I_ROT_mXYd_R(SRA, IX, A);
	Z80I_ROT_mXYd_R(SRA, IX, B);
	Z80I_ROT_mXYd_R(SRA, IX, C);
	Z80I_ROT_mXYd_R(SRA, IX, D);
	Z80I_ROT_mXYd_R(SRA, IX, E);
	Z80I_ROT_mXYd_R(SRA, IX, H);
	Z80I_ROT_mXYd_R(SRA, IX, L);

	Z80I_ROT_mXYd(SRA, IY);
	Z80I_ROT_mXYd_R(SRA, IY, A);
	Z80I_ROT_mXYd_R(SRA, IY, B);
	Z80I_ROT_mXYd_R(SRA, IY, C);
	Z80I_ROT_mXYd_R(SRA, IY, D);
	Z80I_ROT_mXYd_R(SRA, IY, E);
	Z80I_ROT_mXYd_R(SRA, IY, H);
	Z80I_ROT_mXYd_R(SRA, IY, L);

	Z80I_ROT_mXYd(SRL, IX);
	Z80I_ROT_mXYd_R(SRL, IX, A);
	Z80I_ROT_mXYd_R(SRL, IX, B);
	Z80I_ROT_mXYd_R(SRL, IX, C);
	Z80I_ROT_mXYd_R(SRL, IX, D);
	Z80I_ROT_mXYd_R(SRL, IX, E);
	Z80I_ROT_mXYd_R(SRL, IX, H);
	Z80I_ROT_mXYd_R(SRL, IX, L);

	Z80I_ROT_mXYd(SRL, IY);
	Z80I_ROT_mXYd_R(SRL, IY, A);
	Z80I_ROT_mXYd_R(SRL, IY, B);
	Z80I_ROT_mXYd_R(SRL, IY, C);
	Z80I_ROT_mXYd_R(SRL, IY, D);
	Z80I_ROT_mXYd_R(SRL, IY, E);
	Z80I_ROT_mXYd_R(SRL, IY, H);
	Z80I_ROT_mXYd_R(SRL, IY, L);

	/*! Bit operation instructions */

	// BIT B, R		Test bit B in register R
	#define Z80I_BITb_R(BitID, Rdest, FlagsToSet) \
		Z80I_BIT ## BitID ## _ ## Rdest : \
			do { \
				z80->F &= MDZ80_FLAG_C; \
				const uint8_t bitValue = (1 << BitID); \
				if (z80->Rdest & bitValue) { \
					z80->F |= (FlagsToSet | MDZ80_FLAG_H); \
				} else { \
					z80->F |= (MDZ80_FLAG_Z | MDZ80_FLAG_H | MDZ80_FLAG_P); \
				} \
				z80->PC += 2; \
				NEXT(8); \
			} while (0)

	Z80I_BITb_R(0, A, 0);
	Z80I_BITb_R(1, A, 0);
	Z80I_BITb_R(2, A, 0);
	Z80I_BITb_R(3, A, MDZ80_FLAG_3);
	Z80I_BITb_R(4, A, 0);
	Z80I_BITb_R(5, A, MDZ80_FLAG_5);
	Z80I_BITb_R(6, A, 0);
	Z80I_BITb_R(7, A, MDZ80_FLAG_S);

	Z80I_BITb_R(0, B, 0);
	Z80I_BITb_R(1, B, 0);
	Z80I_BITb_R(2, B, 0);
	Z80I_BITb_R(3, B, MDZ80_FLAG_3);
	Z80I_BITb_R(4, B, 0);
	Z80I_BITb_R(5, B, MDZ80_FLAG_5);
	Z80I_BITb_R(6, B, 0);
	Z80I_BITb_R(7, B, MDZ80_FLAG_S);

	Z80I_BITb_R(0, C, 0);
	Z80I_BITb_R(1, C, 0);
	Z80I_BITb_R(2, C, 0);
	Z80I_BITb_R(3, C, MDZ80_FLAG_3);
	Z80I_BITb_R(4, C, 0);
	Z80I_BITb_R(5, C, MDZ80_FLAG_5);
	Z80I_BITb_R(6, C, 0);
	Z80I_BITb_R(7, C, MDZ80_FLAG_S);

	Z80I_BITb_R(0, D, 0);
	Z80I_BITb_R(1, D, 0);
	Z80I_BITb_R(2, D, 0);
	Z80I_BITb_R(3, D, MDZ80_FLAG_3);
	Z80I_BITb_R(4, D, 0);
	Z80I_BITb_R(5, D, MDZ80_FLAG_5);
	Z80I_BITb_R(6, D, 0);
	Z80I_BITb_R(7, D, MDZ80_FLAG_S);

	Z80I_BITb_R(0, E, 0);
	Z80I_BITb_R(1, E, 0);
	Z80I_BITb_R(2, E, 0);
	Z80I_BITb_R(3, E, MDZ80_FLAG_3);
	Z80I_BITb_R(4, E, 0);
	Z80I_BITb_R(5, E, MDZ80_FLAG_5);
	Z80I_BITb_R(6, E, 0);
	Z80I_BITb_R(7, E, MDZ80_FLAG_S);

	Z80I_BITb_R(0, H, 0);
	Z80I_BITb_R(1, H, 0);
	Z80I_BITb_R(2, H, 0);
	Z80I_BITb_R(3, H, MDZ80_FLAG_3);
	Z80I_BITb_R(4, H, 0);
	Z80I_BITb_R(5, H, MDZ80_FLAG_5);
	Z80I_BITb_R(6, H, 0);
	Z80I_BITb_R(7, H, MDZ80_FLAG_S);

	Z80I_BITb_R(0, L, 0);
	Z80I_BITb_R(1, L, 0);
	Z80I_BITb_R(2, L, 0);
	Z80I_BITb_R(3, L, MDZ80_FLAG_3);
	Z80I_BITb_R(4, L, 0);
	Z80I_BITb_R(5, L, MDZ80_FLAG_5);
	Z80I_BITb_R(6, L, 0);
	Z80I_BITb_R(7, L, MDZ80_FLAG_S);

	// BIT B, (HL)		Test bit B in (HL)
	// FIXME: Flags 3 and 5 are NOT set according to BitID.
	// http://www.myquest.nl/z80undocumented/z80-documented-v0.91.pdf
	#define Z80I_BITb_mHL(BitID, FlagsToSet) \
		Z80I_BIT ## BitID ## _mHL : \
			do { \
				z80->F &= MDZ80_FLAG_C; \
				const uint8_t bitValue = (1 << BitID); \
				const uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				if (tmp8 & bitValue) { \
					z80->F |= (FlagsToSet | MDZ80_FLAG_H); \
				} else { \
					z80->F |= (MDZ80_FLAG_Z | MDZ80_FLAG_H | MDZ80_FLAG_P); \
				} \
				z80->PC += 2; \
				NEXT(12); \
			} while (0)

	Z80I_BITb_mHL(0, 0);
	Z80I_BITb_mHL(1, 0);
	Z80I_BITb_mHL(2, 0);
	Z80I_BITb_mHL(3, MDZ80_FLAG_3);
	Z80I_BITb_mHL(4, 0);
	Z80I_BITb_mHL(5, MDZ80_FLAG_5);
	Z80I_BITb_mHL(6, 0);
	Z80I_BITb_mHL(7, MDZ80_FLAG_S);

	// BIT B, (XY+d)	Test bit B in (XY+d)
	// NOTE: Flags 3 and 5 should be pulled from the high byte of (XY+d).
	// http://www.myquest.nl/z80undocumented/z80-documented-v0.91.pdf
	// TODO: Verify emulation of flags 3 and 5.
	#define Z80I_BITb_mXYd(BitID, Ridx) \
		Z80I_BIT ## BitID ## _m ## Ridx ## d : \
			do { \
				z80->F &= MDZ80_FLAG_C; \
				const uint8_t bitValue = (1 << BitID); \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				const uint8_t tmp8 = READ_BYTE(z80, addr); \
				if (tmp8 & bitValue) { \
					switch (BitID) { \
						case 7: /* MDZ80_FLAG_S */ \
							z80->F |= (MDZ80_FLAG_S | MDZ80_FLAG_H); \
							break; \
						case 5: /* MDZ80_FLAG_5 */ \
						case 3: /* MDZ80_FLAG_3 */ \
							z80->F |= (((addr >> 8) & bitValue) | MDZ80_FLAG_H); \
							break; \
						default: \
							z80->F |= MDZ80_FLAG_H; \
							break; \
					} \
				} else { \
					z80->F |= (MDZ80_FLAG_Z | MDZ80_FLAG_H | MDZ80_FLAG_P); \
				} \
				z80->PC += 3; \
				NEXT(16); \
			} while (0)

	Z80I_BITb_mXYd(0, IX);
	Z80I_BITb_mXYd(1, IX);
	Z80I_BITb_mXYd(2, IX);
	Z80I_BITb_mXYd(3, IX);
	Z80I_BITb_mXYd(4, IX);
	Z80I_BITb_mXYd(5, IX);
	Z80I_BITb_mXYd(6, IX);
	Z80I_BITb_mXYd(7, IX);

	Z80I_BITb_mXYd(0, IY);
	Z80I_BITb_mXYd(1, IY);
	Z80I_BITb_mXYd(2, IY);
	Z80I_BITb_mXYd(3, IY);
	Z80I_BITb_mXYd(4, IY);
	Z80I_BITb_mXYd(5, IY);
	Z80I_BITb_mXYd(6, IY);
	Z80I_BITb_mXYd(7, IY);

	// TODO: BIT B, (IX+d) -> R

	// SET B, R		R |= (1 << B)
	#define Z80I_SETb_R(BitID, Rdest) \
		Z80I_SET ## BitID ## _ ## Rdest : \
			do { \
				z80->Rdest |= (1 << BitID); \
				z80->PC += 2; \
				NEXT(8); \
			} while (0)

	Z80I_SETb_R(0, A);
	Z80I_SETb_R(1, A);
	Z80I_SETb_R(2, A);
	Z80I_SETb_R(3, A);
	Z80I_SETb_R(4, A);
	Z80I_SETb_R(5, A);
	Z80I_SETb_R(6, A);
	Z80I_SETb_R(7, A);

	Z80I_SETb_R(0, B);
	Z80I_SETb_R(1, B);
	Z80I_SETb_R(2, B);
	Z80I_SETb_R(3, B);
	Z80I_SETb_R(4, B);
	Z80I_SETb_R(5, B);
	Z80I_SETb_R(6, B);
	Z80I_SETb_R(7, B);

	Z80I_SETb_R(0, C);
	Z80I_SETb_R(1, C);
	Z80I_SETb_R(2, C);
	Z80I_SETb_R(3, C);
	Z80I_SETb_R(4, C);
	Z80I_SETb_R(5, C);
	Z80I_SETb_R(6, C);
	Z80I_SETb_R(7, C);

	Z80I_SETb_R(0, D);
	Z80I_SETb_R(1, D);
	Z80I_SETb_R(2, D);
	Z80I_SETb_R(3, D);
	Z80I_SETb_R(4, D);
	Z80I_SETb_R(5, D);
	Z80I_SETb_R(6, D);
	Z80I_SETb_R(7, D);

	Z80I_SETb_R(0, E);
	Z80I_SETb_R(1, E);
	Z80I_SETb_R(2, E);
	Z80I_SETb_R(3, E);
	Z80I_SETb_R(4, E);
	Z80I_SETb_R(5, E);
	Z80I_SETb_R(6, E);
	Z80I_SETb_R(7, E);

	Z80I_SETb_R(0, H);
	Z80I_SETb_R(1, H);
	Z80I_SETb_R(2, H);
	Z80I_SETb_R(3, H);
	Z80I_SETb_R(4, H);
	Z80I_SETb_R(5, H);
	Z80I_SETb_R(6, H);
	Z80I_SETb_R(7, H);

	Z80I_SETb_R(0, L);
	Z80I_SETb_R(1, L);
	Z80I_SETb_R(2, L);
	Z80I_SETb_R(3, L);
	Z80I_SETb_R(4, L);
	Z80I_SETb_R(5, L);
	Z80I_SETb_R(6, L);
	Z80I_SETb_R(7, L);

	// RES B, R		R &= ~(1 << B)
	#define Z80I_RESb_R(BitID, Rdest) \
		Z80I_RES ## BitID ## _ ## Rdest : \
			do { \
				z80->Rdest &= ~(1 << BitID); \
				z80->PC += 2; \
				NEXT(8); \
			} while (0)

	Z80I_RESb_R(0, A);
	Z80I_RESb_R(1, A);
	Z80I_RESb_R(2, A);
	Z80I_RESb_R(3, A);
	Z80I_RESb_R(4, A);
	Z80I_RESb_R(5, A);
	Z80I_RESb_R(6, A);
	Z80I_RESb_R(7, A);

	Z80I_RESb_R(0, B);
	Z80I_RESb_R(1, B);
	Z80I_RESb_R(2, B);
	Z80I_RESb_R(3, B);
	Z80I_RESb_R(4, B);
	Z80I_RESb_R(5, B);
	Z80I_RESb_R(6, B);
	Z80I_RESb_R(7, B);

	Z80I_RESb_R(0, C);
	Z80I_RESb_R(1, C);
	Z80I_RESb_R(2, C);
	Z80I_RESb_R(3, C);
	Z80I_RESb_R(4, C);
	Z80I_RESb_R(5, C);
	Z80I_RESb_R(6, C);
	Z80I_RESb_R(7, C);

	Z80I_RESb_R(0, D);
	Z80I_RESb_R(1, D);
	Z80I_RESb_R(2, D);
	Z80I_RESb_R(3, D);
	Z80I_RESb_R(4, D);
	Z80I_RESb_R(5, D);
	Z80I_RESb_R(6, D);
	Z80I_RESb_R(7, D);

	Z80I_RESb_R(0, E);
	Z80I_RESb_R(1, E);
	Z80I_RESb_R(2, E);
	Z80I_RESb_R(3, E);
	Z80I_RESb_R(4, E);
	Z80I_RESb_R(5, E);
	Z80I_RESb_R(6, E);
	Z80I_RESb_R(7, E);

	Z80I_RESb_R(0, H);
	Z80I_RESb_R(1, H);
	Z80I_RESb_R(2, H);
	Z80I_RESb_R(3, H);
	Z80I_RESb_R(4, H);
	Z80I_RESb_R(5, H);
	Z80I_RESb_R(6, H);
	Z80I_RESb_R(7, H);

	Z80I_RESb_R(0, L);
	Z80I_RESb_R(1, L);
	Z80I_RESb_R(2, L);
	Z80I_RESb_R(3, L);
	Z80I_RESb_R(4, L);
	Z80I_RESb_R(5, L);
	Z80I_RESb_R(6, L);
	Z80I_RESb_R(7, L);

	// SET B, (HL)		(HL) |= (1 << B)
	#define Z80I_SETb_mHL(BitID) \
		Z80I_SET ## BitID ## _mHL : \
			do { \
				uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				tmp8 |= (1 << BitID); \
				WRITE_BYTE(z80, z80->HL, tmp8); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_SETb_mHL(0);
	Z80I_SETb_mHL(1);
	Z80I_SETb_mHL(2);
	Z80I_SETb_mHL(3);
	Z80I_SETb_mHL(4);
	Z80I_SETb_mHL(5);
	Z80I_SETb_mHL(6);
	Z80I_SETb_mHL(7);

	// RES B, (HL)		(HL) &= ~(1 << B)
	#define Z80I_RESb_mHL(BitID) \
		Z80I_RES ## BitID ## _mHL : \
			do { \
				uint8_t tmp8 = READ_BYTE(z80, z80->HL); \
				tmp8 &= ~(1 << BitID); \
				WRITE_BYTE(z80, z80->HL, tmp8); \
				z80->PC += 2; \
				NEXT(15); \
			} while (0)

	Z80I_RESb_mHL(0);
	Z80I_RESb_mHL(1);
	Z80I_RESb_mHL(2);
	Z80I_RESb_mHL(3);
	Z80I_RESb_mHL(4);
	Z80I_RESb_mHL(5);
	Z80I_RESb_mHL(6);
	Z80I_RESb_mHL(7);

	// SET B, (XY+d)	(XY+d) |= (1 << B)
	#define Z80I_SETb_mXYd(BitID, Ridx) \
		Z80I_SET ## BitID ## _m ## Ridx ## d : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				tmp8 |= (1 << BitID); \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 3; \
				NEXT(19); \
			} while (0)

	// SET B, (XY+d)->R	(XY+d) |= (1 << B); R8 <- (XY+d)
	#define Z80I_SETb_mXYd_R(BitID, Ridx, Rdest) \
		Z80I_SET ## BitID ## _m ## Ridx ## d_ ## Rdest : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				tmp8 |= (1 << BitID); \
				z80->Rdest = tmp8; \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 3; \
				NEXT(19); \
			} while (0)

	Z80I_SETb_mXYd(0, IX);
	Z80I_SETb_mXYd_R(0, IX, A);
	Z80I_SETb_mXYd_R(0, IX, B);
	Z80I_SETb_mXYd_R(0, IX, C);
	Z80I_SETb_mXYd_R(0, IX, D);
	Z80I_SETb_mXYd_R(0, IX, E);
	Z80I_SETb_mXYd_R(0, IX, H);
	Z80I_SETb_mXYd_R(0, IX, L);

	Z80I_SETb_mXYd(0, IY);
	Z80I_SETb_mXYd_R(0, IY, A);
	Z80I_SETb_mXYd_R(0, IY, B);
	Z80I_SETb_mXYd_R(0, IY, C);
	Z80I_SETb_mXYd_R(0, IY, D);
	Z80I_SETb_mXYd_R(0, IY, E);
	Z80I_SETb_mXYd_R(0, IY, H);
	Z80I_SETb_mXYd_R(0, IY, L);

	Z80I_SETb_mXYd(1, IX);
	Z80I_SETb_mXYd_R(1, IX, A);
	Z80I_SETb_mXYd_R(1, IX, B);
	Z80I_SETb_mXYd_R(1, IX, C);
	Z80I_SETb_mXYd_R(1, IX, D);
	Z80I_SETb_mXYd_R(1, IX, E);
	Z80I_SETb_mXYd_R(1, IX, H);
	Z80I_SETb_mXYd_R(1, IX, L);

	Z80I_SETb_mXYd(1, IY);
	Z80I_SETb_mXYd_R(1, IY, A);
	Z80I_SETb_mXYd_R(1, IY, B);
	Z80I_SETb_mXYd_R(1, IY, C);
	Z80I_SETb_mXYd_R(1, IY, D);
	Z80I_SETb_mXYd_R(1, IY, E);
	Z80I_SETb_mXYd_R(1, IY, H);
	Z80I_SETb_mXYd_R(1, IY, L);

	Z80I_SETb_mXYd(2, IX);
	Z80I_SETb_mXYd_R(2, IX, A);
	Z80I_SETb_mXYd_R(2, IX, B);
	Z80I_SETb_mXYd_R(2, IX, C);
	Z80I_SETb_mXYd_R(2, IX, D);
	Z80I_SETb_mXYd_R(2, IX, E);
	Z80I_SETb_mXYd_R(2, IX, H);
	Z80I_SETb_mXYd_R(2, IX, L);

	Z80I_SETb_mXYd(2, IY);
	Z80I_SETb_mXYd_R(2, IY, A);
	Z80I_SETb_mXYd_R(2, IY, B);
	Z80I_SETb_mXYd_R(2, IY, C);
	Z80I_SETb_mXYd_R(2, IY, D);
	Z80I_SETb_mXYd_R(2, IY, E);
	Z80I_SETb_mXYd_R(2, IY, H);
	Z80I_SETb_mXYd_R(2, IY, L);

	Z80I_SETb_mXYd(3, IX);
	Z80I_SETb_mXYd_R(3, IX, A);
	Z80I_SETb_mXYd_R(3, IX, B);
	Z80I_SETb_mXYd_R(3, IX, C);
	Z80I_SETb_mXYd_R(3, IX, D);
	Z80I_SETb_mXYd_R(3, IX, E);
	Z80I_SETb_mXYd_R(3, IX, H);
	Z80I_SETb_mXYd_R(3, IX, L);

	Z80I_SETb_mXYd(3, IY);
	Z80I_SETb_mXYd_R(3, IY, A);
	Z80I_SETb_mXYd_R(3, IY, B);
	Z80I_SETb_mXYd_R(3, IY, C);
	Z80I_SETb_mXYd_R(3, IY, D);
	Z80I_SETb_mXYd_R(3, IY, E);
	Z80I_SETb_mXYd_R(3, IY, H);
	Z80I_SETb_mXYd_R(3, IY, L);

	Z80I_SETb_mXYd(4, IX);
	Z80I_SETb_mXYd_R(4, IX, A);
	Z80I_SETb_mXYd_R(4, IX, B);
	Z80I_SETb_mXYd_R(4, IX, C);
	Z80I_SETb_mXYd_R(4, IX, D);
	Z80I_SETb_mXYd_R(4, IX, E);
	Z80I_SETb_mXYd_R(4, IX, H);
	Z80I_SETb_mXYd_R(4, IX, L);

	Z80I_SETb_mXYd(4, IY);
	Z80I_SETb_mXYd_R(4, IY, A);
	Z80I_SETb_mXYd_R(4, IY, B);
	Z80I_SETb_mXYd_R(4, IY, C);
	Z80I_SETb_mXYd_R(4, IY, D);
	Z80I_SETb_mXYd_R(4, IY, E);
	Z80I_SETb_mXYd_R(4, IY, H);
	Z80I_SETb_mXYd_R(4, IY, L);

	Z80I_SETb_mXYd(5, IX);
	Z80I_SETb_mXYd_R(5, IX, A);
	Z80I_SETb_mXYd_R(5, IX, B);
	Z80I_SETb_mXYd_R(5, IX, C);
	Z80I_SETb_mXYd_R(5, IX, D);
	Z80I_SETb_mXYd_R(5, IX, E);
	Z80I_SETb_mXYd_R(5, IX, H);
	Z80I_SETb_mXYd_R(5, IX, L);

	Z80I_SETb_mXYd(5, IY);
	Z80I_SETb_mXYd_R(5, IY, A);
	Z80I_SETb_mXYd_R(5, IY, B);
	Z80I_SETb_mXYd_R(5, IY, C);
	Z80I_SETb_mXYd_R(5, IY, D);
	Z80I_SETb_mXYd_R(5, IY, E);
	Z80I_SETb_mXYd_R(5, IY, H);
	Z80I_SETb_mXYd_R(5, IY, L);

	Z80I_SETb_mXYd(6, IX);
	Z80I_SETb_mXYd_R(6, IX, A);
	Z80I_SETb_mXYd_R(6, IX, B);
	Z80I_SETb_mXYd_R(6, IX, C);
	Z80I_SETb_mXYd_R(6, IX, D);
	Z80I_SETb_mXYd_R(6, IX, E);
	Z80I_SETb_mXYd_R(6, IX, H);
	Z80I_SETb_mXYd_R(6, IX, L);

	Z80I_SETb_mXYd(6, IY);
	Z80I_SETb_mXYd_R(6, IY, A);
	Z80I_SETb_mXYd_R(6, IY, B);
	Z80I_SETb_mXYd_R(6, IY, C);
	Z80I_SETb_mXYd_R(6, IY, D);
	Z80I_SETb_mXYd_R(6, IY, E);
	Z80I_SETb_mXYd_R(6, IY, H);
	Z80I_SETb_mXYd_R(6, IY, L);

	Z80I_SETb_mXYd(7, IX);
	Z80I_SETb_mXYd_R(7, IX, A);
	Z80I_SETb_mXYd_R(7, IX, B);
	Z80I_SETb_mXYd_R(7, IX, C);
	Z80I_SETb_mXYd_R(7, IX, D);
	Z80I_SETb_mXYd_R(7, IX, E);
	Z80I_SETb_mXYd_R(7, IX, H);
	Z80I_SETb_mXYd_R(7, IX, L);

	Z80I_SETb_mXYd(7, IY);
	Z80I_SETb_mXYd_R(7, IY, A);
	Z80I_SETb_mXYd_R(7, IY, B);
	Z80I_SETb_mXYd_R(7, IY, C);
	Z80I_SETb_mXYd_R(7, IY, D);
	Z80I_SETb_mXYd_R(7, IY, E);
	Z80I_SETb_mXYd_R(7, IY, H);
	Z80I_SETb_mXYd_R(7, IY, L);

	// RES B, (XY+d)	(XY+d) &= ~(1 << B)
	#define Z80I_RESb_mXYd(BitID, Ridx) \
		Z80I_RES ## BitID ## _m ## Ridx ## d : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				tmp8 &= ~(1 << BitID); \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 3; \
				NEXT(19); \
			} while (0)

	// RES B, (XY+d)->R	(XY+d) &= ~(1 << B); R8 <- (XY+d)
	#define Z80I_RESb_mXYd_R(BitID, Ridx, Rdest) \
		Z80I_RES ## BitID ## _m ## Ridx ## d_ ## Rdest : \
			do { \
				const int8_t d = read_sbyte_offset_pc(z80, 1); \
				const uint16_t addr = (z80->Ridx + d); \
				uint8_t tmp8 = READ_BYTE(z80, addr); \
				tmp8 &= ~(1 << BitID); \
				z80->Rdest = tmp8; \
				WRITE_BYTE(z80, addr, tmp8); \
				z80->PC += 3; \
				NEXT(19); \
			} while (0)

	Z80I_RESb_mXYd(0, IX);
	Z80I_RESb_mXYd_R(0, IX, A);
	Z80I_RESb_mXYd_R(0, IX, B);
	Z80I_RESb_mXYd_R(0, IX, C);
	Z80I_RESb_mXYd_R(0, IX, D);
	Z80I_RESb_mXYd_R(0, IX, E);
	Z80I_RESb_mXYd_R(0, IX, H);
	Z80I_RESb_mXYd_R(0, IX, L);

	Z80I_RESb_mXYd(0, IY);
	Z80I_RESb_mXYd_R(0, IY, A);
	Z80I_RESb_mXYd_R(0, IY, B);
	Z80I_RESb_mXYd_R(0, IY, C);
	Z80I_RESb_mXYd_R(0, IY, D);
	Z80I_RESb_mXYd_R(0, IY, E);
	Z80I_RESb_mXYd_R(0, IY, H);
	Z80I_RESb_mXYd_R(0, IY, L);

	Z80I_RESb_mXYd(1, IX);
	Z80I_RESb_mXYd_R(1, IX, A);
	Z80I_RESb_mXYd_R(1, IX, B);
	Z80I_RESb_mXYd_R(1, IX, C);
	Z80I_RESb_mXYd_R(1, IX, D);
	Z80I_RESb_mXYd_R(1, IX, E);
	Z80I_RESb_mXYd_R(1, IX, H);
	Z80I_RESb_mXYd_R(1, IX, L);

	Z80I_RESb_mXYd(1, IY);
	Z80I_RESb_mXYd_R(1, IY, A);
	Z80I_RESb_mXYd_R(1, IY, B);
	Z80I_RESb_mXYd_R(1, IY, C);
	Z80I_RESb_mXYd_R(1, IY, D);
	Z80I_RESb_mXYd_R(1, IY, E);
	Z80I_RESb_mXYd_R(1, IY, H);
	Z80I_RESb_mXYd_R(1, IY, L);

	Z80I_RESb_mXYd(2, IX);
	Z80I_RESb_mXYd_R(2, IX, A);
	Z80I_RESb_mXYd_R(2, IX, B);
	Z80I_RESb_mXYd_R(2, IX, C);
	Z80I_RESb_mXYd_R(2, IX, D);
	Z80I_RESb_mXYd_R(2, IX, E);
	Z80I_RESb_mXYd_R(2, IX, H);
	Z80I_RESb_mXYd_R(2, IX, L);

	Z80I_RESb_mXYd(2, IY);
	Z80I_RESb_mXYd_R(2, IY, A);
	Z80I_RESb_mXYd_R(2, IY, B);
	Z80I_RESb_mXYd_R(2, IY, C);
	Z80I_RESb_mXYd_R(2, IY, D);
	Z80I_RESb_mXYd_R(2, IY, E);
	Z80I_RESb_mXYd_R(2, IY, H);
	Z80I_RESb_mXYd_R(2, IY, L);

	Z80I_RESb_mXYd(3, IX);
	Z80I_RESb_mXYd_R(3, IX, A);
	Z80I_RESb_mXYd_R(3, IX, B);
	Z80I_RESb_mXYd_R(3, IX, C);
	Z80I_RESb_mXYd_R(3, IX, D);
	Z80I_RESb_mXYd_R(3, IX, E);
	Z80I_RESb_mXYd_R(3, IX, H);
	Z80I_RESb_mXYd_R(3, IX, L);

	Z80I_RESb_mXYd(3, IY);
	Z80I_RESb_mXYd_R(3, IY, A);
	Z80I_RESb_mXYd_R(3, IY, B);
	Z80I_RESb_mXYd_R(3, IY, C);
	Z80I_RESb_mXYd_R(3, IY, D);
	Z80I_RESb_mXYd_R(3, IY, E);
	Z80I_RESb_mXYd_R(3, IY, H);
	Z80I_RESb_mXYd_R(3, IY, L);

	Z80I_RESb_mXYd(4, IX);
	Z80I_RESb_mXYd_R(4, IX, A);
	Z80I_RESb_mXYd_R(4, IX, B);
	Z80I_RESb_mXYd_R(4, IX, C);
	Z80I_RESb_mXYd_R(4, IX, D);
	Z80I_RESb_mXYd_R(4, IX, E);
	Z80I_RESb_mXYd_R(4, IX, H);
	Z80I_RESb_mXYd_R(4, IX, L);

	Z80I_RESb_mXYd(4, IY);
	Z80I_RESb_mXYd_R(4, IY, A);
	Z80I_RESb_mXYd_R(4, IY, B);
	Z80I_RESb_mXYd_R(4, IY, C);
	Z80I_RESb_mXYd_R(4, IY, D);
	Z80I_RESb_mXYd_R(4, IY, E);
	Z80I_RESb_mXYd_R(4, IY, H);
	Z80I_RESb_mXYd_R(4, IY, L);

	Z80I_RESb_mXYd(5, IX);
	Z80I_RESb_mXYd_R(5, IX, A);
	Z80I_RESb_mXYd_R(5, IX, B);
	Z80I_RESb_mXYd_R(5, IX, C);
	Z80I_RESb_mXYd_R(5, IX, D);
	Z80I_RESb_mXYd_R(5, IX, E);
	Z80I_RESb_mXYd_R(5, IX, H);
	Z80I_RESb_mXYd_R(5, IX, L);

	Z80I_RESb_mXYd(5, IY);
	Z80I_RESb_mXYd_R(5, IY, A);
	Z80I_RESb_mXYd_R(5, IY, B);
	Z80I_RESb_mXYd_R(5, IY, C);
	Z80I_RESb_mXYd_R(5, IY, D);
	Z80I_RESb_mXYd_R(5, IY, E);
	Z80I_RESb_mXYd_R(5, IY, H);
	Z80I_RESb_mXYd_R(5, IY, L);

	Z80I_RESb_mXYd(6, IX);
	Z80I_RESb_mXYd_R(6, IX, A);
	Z80I_RESb_mXYd_R(6, IX, B);
	Z80I_RESb_mXYd_R(6, IX, C);
	Z80I_RESb_mXYd_R(6, IX, D);
	Z80I_RESb_mXYd_R(6, IX, E);
	Z80I_RESb_mXYd_R(6, IX, H);
	Z80I_RESb_mXYd_R(6, IX, L);

	Z80I_RESb_mXYd(6, IY);
	Z80I_RESb_mXYd_R(6, IY, A);
	Z80I_RESb_mXYd_R(6, IY, B);
	Z80I_RESb_mXYd_R(6, IY, C);
	Z80I_RESb_mXYd_R(6, IY, D);
	Z80I_RESb_mXYd_R(6, IY, E);
	Z80I_RESb_mXYd_R(6, IY, H);
	Z80I_RESb_mXYd_R(6, IY, L);

	Z80I_RESb_mXYd(7, IX);
	Z80I_RESb_mXYd_R(7, IX, A);
	Z80I_RESb_mXYd_R(7, IX, B);
	Z80I_RESb_mXYd_R(7, IX, C);
	Z80I_RESb_mXYd_R(7, IX, D);
	Z80I_RESb_mXYd_R(7, IX, E);
	Z80I_RESb_mXYd_R(7, IX, H);
	Z80I_RESb_mXYd_R(7, IX, L);

	Z80I_RESb_mXYd(7, IY);
	Z80I_RESb_mXYd_R(7, IY, A);
	Z80I_RESb_mXYd_R(7, IY, B);
	Z80I_RESb_mXYd_R(7, IY, C);
	Z80I_RESb_mXYd_R(7, IY, D);
	Z80I_RESb_mXYd_R(7, IY, E);
	Z80I_RESb_mXYd_R(7, IY, H);
	Z80I_RESb_mXYd_R(7, IY, L);

	/*! Jump instructions */

	// JP (NN)		Jump to imm16
	Z80I_JP_NN: {
		const uint16_t addr = read_word_offset_pc(z80, 1);
		set_pc(z80, addr);
		NEXT(10);
	}

	// JPcc (NN)		Conditional jump to imm16
	// TODO: Possible performance optimization: Put NEXT(10) in both branches?
	#define Z80I_JPcc_NN(cond, ncond, invert) \
		Z80I_JP ## ncond ## _NN : \
			do { \
				if (invert(z80->F & MDZ80_FLAG_ ## cond)) { \
					const uint16_t addr = read_word_offset_pc(z80, 1); \
					set_pc(z80, addr); \
				} else { \
					z80->PC += 3; \
				} \
				NEXT(10); \
			} while (0)

	Z80I_JPcc_NN(Z,Z,);
	Z80I_JPcc_NN(Z,NZ,!);
	Z80I_JPcc_NN(C,C,);
	Z80I_JPcc_NN(C,NC,!);
	Z80I_JPcc_NN(P,P,);
	Z80I_JPcc_NN(P,NP,!);
	Z80I_JPcc_NN(S,S,);
	Z80I_JPcc_NN(S,NS,!);

	// JR N			Relative jump by N bytes
	Z80I_JR_N: {
		const int8_t d = read_sbyte_offset_pc(z80, 1);
		z80->PC += (2 + d);
		NEXT(12);
	}

	// JRcc N		Conditional relative jump by N bytes
	#define Z80I_JRcc_N(cond, ncond, invert) \
		Z80I_JR ## ncond ## _N : \
			do { \
				if (invert(z80->F & MDZ80_FLAG_ ## cond)) { \
					const int8_t d = read_sbyte_offset_pc(z80, 1); \
					z80->PC += (2 + d); \
					NEXT(12); \
				} else { \
					z80->PC += 2; \
					NEXT(7); \
				} \
			} while (0)

	Z80I_JRcc_N(Z,Z,);
	Z80I_JRcc_N(Z,NZ,!);
	Z80I_JRcc_N(C,C,);
	Z80I_JRcc_N(C,NC,!);

	// JP RR		Jump to address in R16
	#define Z80I_JP_RR(Rsrc) \
		Z80I_JP_ ## Rsrc : \
			do { \
				set_pc(z80, z80->Rsrc); \
				NEXT(4); \
			} while (0)

	Z80I_JP_RR(HL);
	Z80I_JP_RR(IX);
	Z80I_JP_RR(IY);

	// DJNZ N		Decrement B; relative jump by N bytes if B != 0
	Z80I_DJNZ: {
		z80->B--;
		if (z80->B != 0) {
			// Not zero. Do the branch.
			const int8_t d = read_sbyte_offset_pc(z80, 1);
			z80->PC += (2 + d);
			NEXT(13);
		} else {
			// Zero! No branch.
			z80->PC += 2;
			NEXT(10);
		}
	}

	/*! Call/Return instructions */

	// CALL_NN		CALL NN
	Z80I_CALL_NN: {
		/* Push the current PC onto the stack. */
		const uint16_t pc = ((z80->PC + 3) - z80->BasePC);
		z80->SP -= 2;
		WRITE_WORD(z80, z80->SP, pc);
		/* Jump to the new address. */
		const uint16_t addr = read_word_offset_pc(z80, 1);
		set_pc(z80, addr);
		NEXT(17);
	}

	// CALLcc_NN		if (cc) CALL NN
	#define Z80I_CALLcc_NN(cond, ncond, invert) \
		Z80I_CALL ## ncond ## _NN : \
			do { \
				if (invert(z80->F & MDZ80_FLAG_ ## cond)) { \
					/* Push the current PC onto the stack. */ \
					const uint16_t pc = ((z80->PC + 3) - z80->BasePC); \
					z80->SP -= 2; \
					WRITE_WORD(z80, z80->SP, pc); \
					/* Jump to the new address. */ \
					const uint16_t addr = read_word_offset_pc(z80, 1); \
					set_pc(z80, addr); \
					NEXT(17); \
				} else { \
					z80->PC += 3; \
					NEXT(10); \
				} \
			} while (0)

	Z80I_CALLcc_NN(Z,Z,);
	Z80I_CALLcc_NN(Z,NZ,!);
	Z80I_CALLcc_NN(C,C,);
	Z80I_CALLcc_NN(C,NC,!);
	Z80I_CALLcc_NN(P,P,);
	Z80I_CALLcc_NN(P,NP,!);
	Z80I_CALLcc_NN(S,S,);
	Z80I_CALLcc_NN(S,NS,!);

	// RET			RET
	Z80I_RET: {
		/* Get the previous PC from the stack. */
		const uint16_t addr = READ_WORD(z80, z80->SP);
		z80->SP += 2;
		/* Jump to the previous address. */
		set_pc(z80, addr);
		NEXT(10);
	}

	// RETcc		if (cc) RET
	#define Z80I_RETcc(cond, ncond, invert) \
		Z80I_RET ## ncond : \
			do { \
				if (invert(z80->F & MDZ80_FLAG_ ## cond)) { \
					/* Get the previous PC from the stack. */ \
					const uint16_t addr = READ_WORD(z80, z80->SP); \
					z80->SP += 2; \
					/* Jump to the previous address. */ \
					set_pc(z80, addr); \
					NEXT(17); \
				} else { \
					z80->PC++; \
					NEXT(5); \
				} \
			} while (0)

	Z80I_RETcc(Z,Z,);
	Z80I_RETcc(Z,NZ,!);
	Z80I_RETcc(C,C,);
	Z80I_RETcc(C,NC,!);
	Z80I_RETcc(P,P,);
	Z80I_RETcc(P,NP,!);
	Z80I_RETcc(S,S,);
	Z80I_RETcc(S,NS,!);

	// RETI/RETN		RET from interrupt
	// NOTE: These instructions are identical.
	// RETN is a different opcode to make it
	// easier for devices to recognize it.
	Z80I_RETI:
	Z80I_RETN: {
		/* Get the previous PC from the stack. */
		const uint16_t addr = READ_WORD(z80, z80->SP);
		z80->SP += 2;
		/* Copy IFF2 to IFF1. */
		z80->IFF &= ~1;
		z80->IFF |= ((z80->IFF >> 1) & 1);
		/* Jump to the previous address. */
		set_pc(z80, addr);
		NEXT(14);
	}

	// RST N		CALL the specified interrupt vector
	// NOTE: This does NOT modify IFF!
	Z80I_RST: {
		/* Push the current PC onto the stack. */
		const uint16_t pc = ((z80->PC + 3) - z80->BasePC);
		z80->SP -= 2;
		WRITE_WORD(z80, z80->SP, pc);
		/* Get the interrupt vector address. */
		const uint16_t addr = (read_byte_pc(z80) & 0x38);
		/* Jump to the interrupt vector. */
		set_pc(z80, addr);
		NEXT(11);
	}

	/*! Input/Output instructions */

	// IN A, (N)		A <- IN(A<<8|N)
	Z80I_IN_mN: {
		const uint16_t io_addr = ((z80->A << 8) | read_byte_offset_pc(z80, 1));
		z80->A = DO_IN(z80, io_addr);
		z80->PC += 2;
		NEXT(11);
	}

	// IN R, (BC)		R8 <- IN(BC)
	// TODO: Set flags!
	#define Z80I_IN_R_mBC(Rdest) \
		Z80I_IN_ ## Rdest ## _mBC : \
			do { \
				z80->Rdest = DO_IN(z80, z80->BC); \
				/* TODO: Set flags! */ \
				z80->PC += 2; \
				NEXT(12); \
			} while (0)

	Z80I_IN_R_mBC(A);
	Z80I_IN_R_mBC(B);
	Z80I_IN_R_mBC(C);
	Z80I_IN_R_mBC(D);
	Z80I_IN_R_mBC(E);
	Z80I_IN_R_mBC(H);
	Z80I_IN_R_mBC(L);

	// IN F, (BC)		F <- flags after IN(BC)
	// TODO: Set flags!
	Z80I_IN_F_mBC: {
		const uint8_t in_data = DO_IN(z80, z80->BC);
		/* TODO: Set flags! */
		z80->PC += 2;
		NEXT(12);
	}

	// TODO: INX / INXR

	// OUT (N), A		OUT(A<<8|N) <- A
	Z80I_OUT_mN: {
		const uint16_t io_addr = ((z80->A << 8) | read_byte_offset_pc(z80, 1));
		DO_OUT(z80, io_addr, z80->A);
		z80->PC += 2;
		NEXT(11);
	}

	// OUT (BC), R		OUT(BC) <- R8
	#define Z80I_OUT_mBC_R(Rsrc) \
		Z80I_OUT_mBC_ ## Rsrc : \
			do { \
				DO_OUT(z80, z80->BC, z80->Rsrc); \
				z80->PC += 2; \
				NEXT(12); \
			} while (0)

	Z80I_OUT_mBC_R(A);
	Z80I_OUT_mBC_R(B);
	Z80I_OUT_mBC_R(C);
	Z80I_OUT_mBC_R(D);
	Z80I_OUT_mBC_R(E);
	Z80I_OUT_mBC_R(H);
	Z80I_OUT_mBC_R(L);

	// OUT (BC), 0		OUT(BC) <- 0
	// NOTE: This would've been OUT (BC), F, but the
	// Z80 doesn't actually support that operation.
	Z80I_OUT_mBC_0: {
		DO_OUT(z80, z80->BC, 0);
		z80->PC += 2;
		NEXT(12);
	}

	// TODO: OUTX/OUTXR

	/*! Instruction prefixes. */

	// CB instruction prefix
	PREFIX_CB: {
		const uint8_t CB_opcode = read_byte_offset_pc(z80, 1);
		goto *z80_insn_table_CB[CB_opcode];
	}

	// ED instruction prefix
	PREFIX_ED: {
		const uint8_t ED_opcode = read_byte_offset_pc(z80, 1);
		goto *z80_insn_table_ED[ED_opcode];
	}

	// DD instruction prefix
	PREFIX_DD: {
		const uint8_t DD_opcode = read_byte_offset_pc(z80, 1);
		odo -= 4;
		z80->PC++;
		goto *z80_insn_table_DD[DD_opcode];
	}

	// DDCB instruction prefix
	PREFIX_DDCB: {
		const uint8_t DDCB_opcode = read_byte_offset_pc(z80, 2);
		goto *z80_insn_table_DDCB[DDCB_opcode];
	}

	// FD instruction prefix
	PREFIX_FD: {
		const uint8_t FD_opcode = read_byte_offset_pc(z80, 1);
		odo -= 4;
		z80->PC++;
		goto *z80_insn_table_FD[FD_opcode];
	}

	// FDCB instruction prefix
	PREFIX_FDCB: {
		const uint8_t FDCB_opcode = read_byte_offset_pc(z80, 2);
		goto *z80_insn_table_FDCB[FDCB_opcode];
	}

	// Unimplemented opcodes
	// FIXME: Implement these!

	// Logic instructions (8-bit)
	Z80I_AND_A: Z80I_AND_B: Z80I_AND_C: Z80I_AND_D:
	Z80I_AND_E: Z80I_AND_H: Z80I_AND_L: Z80I_AND_mHL:
	Z80I_AND_IXl: Z80I_AND_IXh: Z80I_AND_IYh: Z80I_AND_IYl:
	Z80I_AND_N: Z80I_AND_mIXd: Z80I_AND_mIYd:

	Z80I_OR_A: Z80I_OR_B: Z80I_OR_C: Z80I_OR_D:
	Z80I_OR_E: Z80I_OR_H: Z80I_OR_L: Z80I_OR_mHL:
	Z80I_OR_IXl: Z80I_OR_IXh: Z80I_OR_IYh: Z80I_OR_IYl:
	Z80I_OR_N: Z80I_OR_mIXd: Z80I_OR_mIYd:

	Z80I_XOR_A: Z80I_XOR_B: Z80I_XOR_C: Z80I_XOR_D:
	Z80I_XOR_E: Z80I_XOR_H: Z80I_XOR_L: Z80I_XOR_mHL:
	Z80I_XOR_IXl: Z80I_XOR_IXh: Z80I_XOR_IYh: Z80I_XOR_IYl:
	Z80I_XOR_N: Z80I_XOR_mIXd: Z80I_XOR_mIYd:

	// Rotate and shift instructions
	Z80I_RLD:  Z80I_RRD:

	// Input/Output instructions
	Z80I_INI:  Z80I_IND:  Z80I_INIR: Z80I_INDR:
	Z80I_OUTI: Z80I_OUTD: Z80I_OTIR: Z80I_OTDR:

		// Handle this as a NOP for now.
		goto Z80I_NOP;
}
