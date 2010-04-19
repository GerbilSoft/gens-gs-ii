;****************************************
;
; Z80 emulator 0.99
; Copyright 2002 Stéphane Dallongeville
; Used for the Genesis emulation in Gens
;
;****************************************


;********************************************************************
;
; Register Usage Description
; --------------------------
;
; EAX: AH = F, AL = A, bits 16-31 = 0
; EBX: BH = H, BL = L, bits 16-31 = 0
; ECX: Main address  (fastcall convention)
; EDX: Main data     (fastcall convention)
;      Also used for instruction decoding
; EBP: Z80 context pointer
; ESI: PC (+ base offset)
; EDI: Cycle counter
;
;********************************************************************


%ifidn	__OUTPUT_FORMAT__, elf
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, elf32
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, elf64
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, win32
	%define	__OBJ_WIN32
	%define __PLATFORM_WINDOWS
	%define	.rodata	.rdata
%elifidn __OUTPUT_FORMAT__, win64
	%define	__OBJ_WIN64
	%define __PLATFORM_WINDOWS
	%define	.rodata	.rdata
%elifidn __OUTPUT_FORMAT__, macho
	%define	__OBJ_MACHO
%endif

%ifdef __OBJ_ELF
	; Mark the stack as non-executable on ELF.
	section .note.GNU-stack noalloc noexec nowrite progbits
%endif


;**************************
;
; Some usefull ASM macros
;
;**************************


; function are declared in fastcall when number of arguments < 3
; and in cdecl in other case

%ifndef __PLATFORM_WINDOWS
%macro DECLF 1-2

%if %0 > 1
	global %1%2
%endif
	global _%1
	global %1

%if %0 > 1
	%1%2:
%endif
	_%1:
	%1:

%endmacro
%else
%macro DECLF 1-2

%if %0 > 1
	global @%1@%2
%endif
	global _%1
	global %1

%if %0 > 1
	@%1@%2:
%endif
	_%1:
	%1:

%endmacro
%endif

;*******************
;
; Data declaration
;
;*******************

section .bss align=64

%define Z80_SAFE   0
%define GENS_OPT   1
%define GENS_LOG   0

%if (GENS_OPT == 1)
	%ifdef __OBJ_ELF
		%define _Ram_Z80 Ram_Z80
	%endif
	extern _Ram_Z80			; Gens stuff
%endif

%if (GENS_LOG == 1)
	extern @z80log@4
%endif

	
%macro Z80_CONTEXT 0
		
		.AF:
		.A:	resb 1		; swapped because flags in AH
		.F:	resb 1		;
		.FXYW:	resb 1
		.FXYB:	resb 1
		
		.BC:
		.C:	resb 1
		.B:	resb 1
			resw 1
		
		.DE:
		.E:	resb 1
		.D:	resb 1
			resw 1
		
		.HL:
		.L:	resb 1
		.H:	resb 1
			resw 1
		
		.IX:
		.IXL:	resb 1
		.IXH:	resb 1
			resw 1
		
		.IY:
		.IYL:	resb 1
		.IYH:	resb 1
			resw 1
		
		.PC:			; PC == BasePC + Z80 PC
		.PCL:	resb 1
		.PCH:	resb 1
			resw 1
		
		.SP:
		.SPL:	resb 1
		.SPH:	resb 1
			resw 1
		
		.AF2:
		.A2:	resb 1
		.F2:	resb 1
		.FXYW2:	resb 1
		.FXYB2:	resb 1
		
		.BC2:
		.C2:	resb 1
		.B2:	resb 1
			resw 1
		
		.DE2:
		.E2:	resb 1
		.D2:	resb 1
			resw 1
		
		.HL2:
		.L2:	resb 1
		.H2:	resb 1
			resw 1
		
		.IFF:
		.IFF1:	resb 1
		.IFF2:	resb 1
			resw 1
		
		.R:
		.R1:	resb 1
		.R2:	resb 1
			resw 1
		
		.I:		resb 1
		.IM:		resb 1
		.IntVect:	resb 1
		.IntLine:	resb 1
		
		.Status:	resd 1
		.BasePC:	resd 1	; Pointer to x86 memory location where Z80 RAM starts.
		.TmpSav:	resd 1
				resd 1
		
		.CycleCnt:	resd 1
		.CycleTD:	resd 1
		.CycleIO:	resd 1
		.CycleSup:	resd 1
		
		.Reset_Size:
		
		.ReadB:		resd 0x100
		.WriteB:	resd 0x100
		.Fetch:		resd 0x100
		.ReadW:		resd 0x100
		.WriteW:	resd 0x100
		
		.IN:		resd 1
		.OUT:		resd 1
		.RetIC:		resd 1
		.IntAckC:	resd 1
		
		.Init_Size:
		
%endmacro
	
	
	struc Z80
		
		Z80_CONTEXT
		
	endstruc
	
	
	; Symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define	_M_Z80		M_Z80
		%define	_mdZ80_def_mem	mdZ80_def_mem
	%endif
	global _M_Z80
	_M_Z80:
		Z80_CONTEXT

	alignb 32
	
	global _mdZ80_def_mem
	_mdZ80_def_mem:
		resb 0x10000
	

; External read-only data symbols.

section .rodata align=64
	
	; External symbol redefines for ELF.
	%ifdef __OBJ_ELF
		%define	_mdZ80_DAA_Table	mdZ80_DAA_Table
		%define	_mdZ80_INC_Flags_Table	mdZ80_INC_Flags_Table
		%define	_mdZ80_DEC_Flags_Table	mdZ80_DEC_Flags_Table
	%endif
	
	extern _mdZ80_DAA_Table
	extern _mdZ80_INC_Flags_Table
	extern _mdZ80_DEC_Flags_Table

section .text align=64


;******************************************************
;
; Constantes & macros definitions for code generation
;
;******************************************************


%define zA	al
%define zhAF	al
%define zF	ah
%define zlAF	ah
%define zAF	ax

%define zFXY	byte [ebp + Z80.FXYB]
%define zbFXY	byte [ebp + Z80.FXYB]
%define zwFXY	word [ebp + Z80.FXYW]

%define zB	byte [ebp + Z80.B]
%define zhBC	byte [ebp + Z80.B]
%define zC	byte [ebp + Z80.C]
%define zlBC	byte [ebp + Z80.C]
%define zBC	word [ebp + Z80.BC]
%define zxBC	dword [ebp + Z80.BC]

%define zD	byte [ebp + Z80.D]
%define zhDE	byte [ebp + Z80.D]
%define zE	byte [ebp + Z80.E]
%define zlDE	byte [ebp + Z80.E]
%define zDE	word [ebp + Z80.DE]
%define zxDE	dword [ebp + Z80.DE]

%define zH	bh
%define zhHL	bh
%define zL	bl
%define zlHL	bl
%define zHL	bx
%define zxHL	ebx

%define zA2	byte [ebp + Z80.A2]
%define zhAF2	byte [ebp + Z80.A2]
%define zF2	byte [ebp + Z80.F2]
%define zlAF2	byte [ebp + Z80.F2]
%define zAF2	word [ebp + Z80.AF2]

%define zFXY2	byte [ebp + Z80.FXYB2]

%define zB2	byte [ebp + Z80.B2]
%define zhBC2	byte [ebp + Z80.B2]
%define zC2	byte [ebp + Z80.C2]
%define zlBC2	byte [ebp + Z80.C2]
%define zBC2	word [ebp + Z80.BC2]
%define zxBC2	dword [ebp + Z80.BC2]

%define zD2	byte [ebp + Z80.D2]
%define zhDE2	byte [ebp + Z80.D2]
%define zE2	byte [ebp + Z80.E2]
%define zlDE2	byte [ebp + Z80.E2]
%define zDE2	word [ebp + Z80.DE2]
%define zxDE2	dword [ebp + Z80.DE2]

%define zH2	byte [ebp + Z80.H2]
%define zhHL2	byte [ebp + Z80.H2]
%define zL2	byte [ebp + Z80.L2]
%define zlHL2	byte [ebp + Z80.L2]
%define zHL2	word [ebp + Z80.HL2]
%define zxHL2	dword [ebp + Z80.HL2]

%define zlIX	byte [ebp + Z80.IXL]
%define zhIX	byte [ebp + Z80.IXH]
%define zIX	word [ebp + Z80.IX]
%define zxIX	dword [ebp + Z80.IX]

%define zlIY	byte [ebp + Z80.IYL]
%define zhIY	byte [ebp + Z80.IYH]
%define zIY	word [ebp + Z80.IY]
%define zxIY	dword [ebp + Z80.IY]

%define zPC	si
%define zxPC	esi

%define zlSP	byte [ebp + Z80.SPL]
%define zhSP	byte [ebp + Z80.SPH]
%define zSP	word [ebp + Z80.SP]
%define zxSP	dword [ebp + Z80.SP]

%define zI	byte [ebp + Z80.I]
%define zIM	byte [ebp + Z80.IM]
%define zR	byte [ebp + Z80.R]

%define zIFF	word [ebp + Z80.IFF]
%define zIFF1	byte [ebp + Z80.IFF2]
%define zIFF2	byte [ebp + Z80.IFF2]
%define zxIFF	dword [ebp + Z80.IFF]


%define FLAG_C	0x01
%define FLAG_N	0x02
%define FLAG_P	0x04
%define FLAG_X	0x08
%define FLAG_H	0x10
%define FLAG_Y	0x20
%define FLAG_Z	0x40
%define FLAG_S	0x80


%define Z80_RUNNING	0x01
%define Z80_HALTED	0x02
%define Z80_FAULTED	0x10


; SAVE_AF / SAVE_A / SAVE_F
; =========================
;
; Save AF/A/F/L before modification of AX
; by a call to an external read/write function

%macro SAVE_AF 0
	mov word [ebp + Z80.AF], zAF
%endmacro

%macro SAVE_A 0
	mov byte [ebp + Z80.A], zA
%endmacro

%macro SAVE_F 0
	mov byte [ebp + Z80.F], zF
%endmacro


; RELOAD_AF / RELOAD_A / RELOAD_F
; ===============================
;
; Reload AF/A/F in AX after a call to
; an external read/write function

%macro RELOAD_AF 0
	mov zAF, word [ebp + Z80.AF]
%endmacro

%macro RELOAD_A 0
	mov zA, byte [ebp + Z80.A]
%endmacro

%macro RELOAD_F 0
	mov zF, byte [ebp + Z80.F]
%endmacro


; NEXT cycle
; ==========
;
; End execution of an instruction and go to the next one
; (called only during emulation loop)

%macro NEXT 1
	
	movzx	edx, byte [zxPC]
	sub	edi, byte %1
	js	near z80_Exec_Quit
	
%if (GENS_LOG == 1)
	push	eax
	push	ecx
	push	edx
	mov	ecx, zxPC
	sub	ecx, [ebp + Z80.BasePC]
	call	@z80log@4
	pop	edx
	pop	ecx
	pop	eax
%endif
	
	jmp	[OP_Table + edx * 4]
	
%endmacro


; READ_BYTE dest
; ==============
;
; Read data from memory :
; - Address where read in ECX
; - Data returned in dest or DL if not dest

%macro READ_BYTE 0-1

%if (GENS_OPT == 1)
	cmp	ecx, 0x3FFF
	ja	short %%IO
	
	and	ecx, 0x1FFF
%ifidn %1, A
	mov	al, [_Ram_Z80 + ecx]
%elif %0 > 0
	mov	dl, [_Ram_Z80 + ecx]
	mov	z%1, dl
%else
	mov	dl, [_Ram_Z80 + ecx]
%endif
	jmp	short %%End
	
align 16
	
%%IO
%endif
 	mov	[ebp + Z80.CycleIO], edi
	movzx	edi, ch
%ifnidn %1, A
	SAVE_A
%endif
	SAVE_F
	call	[ebp + Z80.ReadB + edi * 4]
%ifnidn %1, A
	%if %0 > 0
		mov	z%1, al
	%else
		mov	dl, al
	%endif
%endif
	mov	edi, [ebp + Z80.CycleIO]
%ifnidn %1, A
	RELOAD_A
%endif
	RELOAD_F
	
%if (GENS_OPT == 1)

align 16
%%End
%endif

%endmacro


; WRITE_BYTE src
; ==============
;
; Write data to memory :
; - Address where write in ECX
; - Data to write in src or DL if not src


%macro WRITE_BYTE 0-1

%if (GENS_OPT == 1)
	cmp	ecx, 0x3FFF
	ja	short %%IO
	
	and	ecx, 0x1FFF
%if %0 > 0
	mov	dl, z%1
%endif
	mov	[_Ram_Z80 + ecx], dl
	jmp	short %%End
	
align 16
	
%%IO
%endif
	mov	[ebp + Z80.CycleIO], edi
%if %0 > 0
	mov	dl, z%1
%endif
	movzx	edi, ch
	SAVE_AF
	call	[ebp + Z80.WriteB + edi * 4]
	mov	edi, [ebp + Z80.CycleIO]
	RELOAD_AF
	
%if (GENS_OPT == 1)
align 16

%%End
%endif

%endmacro


; READ_WORD dest
; ==============
;
; Read data from memory :
; - Address where read in ECX
; - Data returned in dest or DX if not dest


%macro READ_WORD 0-1

%if (GENS_OPT == 1)
	cmp	ecx, 0x3FFF
	ja	short %%IO
	
	and	ecx, 0x1FFF
%ifidn %1, AF
	mov	ax, [_Ram_Z80 + ecx]
%elif %0 > 0
	mov	dx, [_Ram_Z80 + ecx]
	mov	z%1, dx
%else
	mov	dx, [_Ram_Z80 + ecx]
%endif
	jmp	short %%End
	
align 16

%%IO
%endif
	mov	[ebp + Z80.CycleIO], edi
	movzx	edi, ch
%if %0 > 0
	%ifidn %1, AF
		call	[ebp + Z80.ReadW + edi * 4]
		mov	edi, [ebp + Z80.CycleIO]
	%else
		SAVE_AF
		call	[ebp + Z80.ReadW + edi * 4]
		mov	z%1, ax
		mov	edi, [ebp + Z80.CycleIO]
		RELOAD_AF
	%endif
%else
	SAVE_AF
	call	[ebp + Z80.ReadW + edi * 4]
	mov	dx, ax
	mov	edi, [ebp + Z80.CycleIO]
	RELOAD_AF
%endif

%if (GENS_OPT == 1)
align 16

%%End
%endif

%endmacro


; WRITE_WORD src
; ==============
;
; Write data to memory :
; - Address where write in ECX
; - Data to write in src or DX if not src


%macro WRITE_WORD 0-1

%if (GENS_OPT == 1)
	cmp	ecx, 0x3FFF
	ja	short %%IO
	
	and	ecx, 0x1FFF
%if %0 > 0
	mov	dx, z%1
%endif
	mov	[_Ram_Z80 + ecx], dx
	jmp	short %%End
	
align 16
	
%%IO
%endif
	mov	[ebp + Z80.CycleIO], edi
%if %0 > 0
	mov	dx, z%1
%endif
	movzx	edi, ch
	SAVE_AF
	call	[ebp + Z80.WriteW + edi * 4]
	mov	edi, [ebp + Z80.CycleIO]
	RELOAD_AF
	
%if (GENS_OPT == 1)
align 16

%%End
%endif

%endmacro


; DO_IN dest
; ==========
;
; Read data from port:
; - Address where read in ECX
; - Data returned in dest or DL if not dest


%macro DO_IN 0-1

%if (GENS_OPT == 1)
	xor	dl, dl
%else
%ifnidn %1, A
	SAVE_A
%endif
	SAVE_F
	mov	[ebp + Z80.CycleIO], edi
	call	[ebp + Z80.IN]
%ifnidn %1, A
	%if %0 > 0
		mov	z%1, al
	%else
		mov	dl, al
	%endif
%endif
	mov	edi, [ebp + Z80.CycleIO]
%ifnidn %1, A
	RELOAD_A
%endif
	RELOAD_F
%endif

%endmacro


; DO_OUT src
; ==========
;
; Write data to port:
; - Address where write in ECX
; - data to write in src or DL if not src


%macro DO_OUT 0-1

%if (GENS_OPT == 1)
%else
%if %0 > 0
	mov	dl, z%1
%endif
	mov	[ebp + Z80.CycleIO], edi
	SAVE_AF
	call	[ebp + Z80.OUT]
	mov	edi, [ebp + Z80.CycleIO]
	RELOAD_AF
%endif

%endmacro


; REBASE_PC
; =========
;
; IN: Unbased PC in ESI
; OUT: Based PC in ESI

; EDX modified by the macro

%macro REBASE_PC 0
	mov	edx, esi
	shr	edx, 8
	mov	edx, [ebp + Z80.Fetch + edx * 4]
	add	esi, edx
	mov	[ebp + Z80.BasePC], edx
%endmacro


; CHECK_INT
; =========
;
; EDX modified by the macro

%macro CHECK_INT 0
	
	mov	dl, [ebp + Z80.IntLine]
	mov	dh, zIFF1
	test	dl, dl
	jz	short %%No_Int
	js	short %%NMI
	
	test	dl, dh
	jz	short %%No_Int
	
	call	_do_INT
	jmp	short %%No_Int
	
align 8
	
%%NMI
	call	_do_NMI
	
align 16
	
%%No_Int

%endmacro



; ***********************
;
; Real code start here
;
; ***********************


; Internals core functions
; ------------------------


align 16

_do_NMI:
	mov	ecx, zxSP
	mov	edx, zxPC
	sub	ecx, byte 2
	sub	edx, [ebp + Z80.BasePC]
	and	ecx, 0xFFFF
	mov	zxSP, ecx
	WRITE_WORD
	mov	dl, [ebp + Z80.IntLine]
	mov	dh, [ebp + Z80.Status]
	mov	zIFF1, 0
	and	dl, ~0x80
	and	dh, ~Z80_HALTED
	mov	zxPC, 0x66
	mov	[ebp + Z80.IntLine], dl
	mov	[ebp + Z80.Status], dh
	REBASE_PC
	ret


align 16

_do_INT:
	mov	ecx, zxSP
	mov	edx, zxPC
	sub	ecx, byte 2
	sub	edx, [ebp + Z80.BasePC]
	and	ecx, 0xFFFF
	mov	zxSP, ecx
	WRITE_WORD
	mov	dl, [ebp + Z80.Status]
	mov	dh, [ebp + Z80.IntLine]
	xor	ecx, ecx
	and	dl, ~Z80_HALTED
	and	dh, 0x80
	mov	zxIFF, ecx
	mov	[ebp + Z80.Status], dl
	mov	[ebp + Z80.IntLine], dh
	mov	dl, [ebp + Z80.IM]
	
.IM0:
	cmp	dl, 0
	jne	short .IM1
	
	mov	dl, [ebp + Z80.IntVect]
	sub	edi, 13
	sub	dl, 0xC7	; assume we have a RST instruction
	movzx	zxPC, dl
	REBASE_PC
	ret
	
align 16
	
.IM1:
	cmp	dl, 1
	jne	short .IM2
	
	sub	edi, 13
	mov	zxPC, 0x38
	REBASE_PC
	ret
	
align 16
	
.IM2:
	movzx	edx, zI
	movzx	ecx, byte [ebp + Z80.IntVect]
	shl	edx, 8
	sub	edi, 19
	or	ecx, edx
	READ_WORD
	movzx	zxPC, dx
	REBASE_PC
	ret


; Most important instruction :)
; -----------------------------


align 16

Z80I_NOP:
	inc	zxPC
	NEXT 4


; Load 8 bits instruction
; -----------------------


; LD_R_R dest, src		R8 <- R8

%macro LD_R_R 2

align 16

Z80I_LD_%1_%2:

%ifnidn %1, %2
	%ifidn %1, A
		inc zxPC
		mov z%1, z%2
	%elifidn %1, H
		inc zxPC
		mov z%1, z%2
	%elifidn %1, L
		inc zxPC
		mov z%1, z%2
	%elifidn %2, A
		inc zxPC
		mov z%1, z%2
	%elifidn %2, H
		inc zxPC
		mov z%1, z%2
	%elifidn %2, L
		inc zxPC
		mov z%1, z%2
	%else
		mov dl, z%2
		inc zxPC
		mov z%1, dl
	%endif
%else
	inc zxPC
%endif
	NEXT 4

%endmacro


LD_R_R A, A
LD_R_R A, B
LD_R_R A, C
LD_R_R A, D
LD_R_R A, E
LD_R_R A, H
LD_R_R A, L
LD_R_R A, hIX
LD_R_R A, lIX
LD_R_R A, hIY
LD_R_R A, lIY

LD_R_R B, A
LD_R_R B, B
LD_R_R B, C
LD_R_R B, D
LD_R_R B, E
LD_R_R B, H
LD_R_R B, L
LD_R_R B, hIX
LD_R_R B, lIX
LD_R_R B, hIY
LD_R_R B, lIY

LD_R_R C, A
LD_R_R C, B
LD_R_R C, C
LD_R_R C, D
LD_R_R C, E
LD_R_R C, H
LD_R_R C, L
LD_R_R C, hIX
LD_R_R C, lIX
LD_R_R C, hIY
LD_R_R C, lIY

LD_R_R D, A
LD_R_R D, B
LD_R_R D, C
LD_R_R D, D
LD_R_R D, E
LD_R_R D, H
LD_R_R D, L
LD_R_R D, hIX
LD_R_R D, lIX
LD_R_R D, hIY
LD_R_R D, lIY

LD_R_R E, A
LD_R_R E, B
LD_R_R E, C
LD_R_R E, D
LD_R_R E, E
LD_R_R E, H
LD_R_R E, L
LD_R_R E, hIX
LD_R_R E, lIX
LD_R_R E, hIY
LD_R_R E, lIY

LD_R_R H, A
LD_R_R H, B
LD_R_R H, C
LD_R_R H, D
LD_R_R H, E
LD_R_R H, H
LD_R_R H, L
LD_R_R H, hIX
LD_R_R H, lIX
LD_R_R H, hIY
LD_R_R H, lIY

LD_R_R L, A
LD_R_R L, B
LD_R_R L, C
LD_R_R L, D
LD_R_R L, E
LD_R_R L, H
LD_R_R L, L
LD_R_R L, hIX
LD_R_R L, lIX
LD_R_R L, hIY
LD_R_R L, lIY

LD_R_R hIX, A
LD_R_R hIX, B
LD_R_R hIX, C
LD_R_R hIX, D
LD_R_R hIX, E
LD_R_R hIX, H
LD_R_R hIX, L
LD_R_R hIX, hIX
LD_R_R hIX, lIX
LD_R_R hIX, hIY
LD_R_R hIX, lIY

LD_R_R lIX, A
LD_R_R lIX, B
LD_R_R lIX, C
LD_R_R lIX, D
LD_R_R lIX, E
LD_R_R lIX, H
LD_R_R lIX, L
LD_R_R lIX, hIX
LD_R_R lIX, lIX
LD_R_R lIX, hIY
LD_R_R lIX, lIY

LD_R_R hIY, A
LD_R_R hIY, B
LD_R_R hIY, C
LD_R_R hIY, D
LD_R_R hIY, E
LD_R_R hIY, H
LD_R_R hIY, L
LD_R_R hIY, hIX
LD_R_R hIY, lIX
LD_R_R hIY, hIY
LD_R_R hIY, lIY

LD_R_R lIY, A
LD_R_R lIY, B
LD_R_R lIY, C
LD_R_R lIY, D
LD_R_R lIY, E
LD_R_R lIY, H
LD_R_R lIY, L
LD_R_R lIY, hIX
LD_R_R lIY, lIX
LD_R_R lIY, hIY
LD_R_R lIY, lIY


; LD_R_N dest			R8 <- imm8

%macro LD_R_N 1

align 16

Z80I_LD_%1_N:

%ifidn %1, A
	mov z%1, byte [zxPC + 1]
	add zxPC, byte 2
%elifidn %1, H
	mov z%1, byte [zxPC + 1]
	add zxPC, byte 2
%elifidn %1, L
	mov z%1, byte [zxPC + 1]
	add zxPC, byte 2
%else
	mov dl, byte [zxPC + 1]
	add zxPC, byte 2
	mov z%1, dl
%endif
	NEXT 7

%endmacro


LD_R_N A
LD_R_N B
LD_R_N C
LD_R_N D
LD_R_N E
LD_R_N H
LD_R_N L
LD_R_N hIX
LD_R_N lIX
LD_R_N hIY
LD_R_N lIY


; LD_R_mHL dest			R8 <- (HL)

%macro LD_R_mHL 1

align 16

Z80I_LD_%1_mHL:

	mov ecx, zxHL
	inc zxPC
	READ_BYTE %1
	NEXT 7

%endmacro


LD_R_mHL A
LD_R_mHL B
LD_R_mHL C
LD_R_mHL D
LD_R_mHL E
LD_R_mHL H
LD_R_mHL L


; LD_R_mXYd				R8 <- (XY+d)

%macro LD_R_mXYd 2

align 16

Z80I_LD_%1_m%2d:

%ifidn %2, IX
	%define rind zxIX
%elifidn %2, IY
	%define rind zxIY
%endif

	mov	ecx, rind
	movsx	edx, byte [zxPC + 1]
	add	ecx, edx
	add	zxPC, byte 2
	and	ecx, 0xFFFF
	READ_BYTE %1
	NEXT 15

%endmacro


LD_R_mXYd A, IX
LD_R_mXYd B, IX
LD_R_mXYd C, IX
LD_R_mXYd D, IX
LD_R_mXYd E, IX
LD_R_mXYd H, IX
LD_R_mXYd L, IX

LD_R_mXYd A, IY
LD_R_mXYd B, IY
LD_R_mXYd C, IY
LD_R_mXYd D, IY
LD_R_mXYd E, IY
LD_R_mXYd H, IY
LD_R_mXYd L, IY


; LD_mHL_R src			(HL) <- R8

%macro LD_mHL_R 1

align 16

Z80I_LD_mHL_%1:
	mov ecx, zxHL
	inc zxPC
	WRITE_BYTE %1
	NEXT 7

%endmacro


LD_mHL_R A
LD_mHL_R B
LD_mHL_R C
LD_mHL_R D
LD_mHL_R E
LD_mHL_R H
LD_mHL_R L


; LD_mXYd_R src			(XY+d) <- R8

%macro LD_mXYd_R 2

align 16

Z80I_LD_m%2d_%1:
	mov	edx, zx%2
	movsx	ecx, byte [zxPC + 1]
	add	ecx, edx
	add	zxPC, byte 2
	and	ecx, 0xFFFF
	WRITE_BYTE %1
	NEXT 15

%endmacro

LD_mXYd_R A, IX
LD_mXYd_R B, IX
LD_mXYd_R C, IX
LD_mXYd_R D, IX
LD_mXYd_R E, IX
LD_mXYd_R H, IX
LD_mXYd_R L, IX

LD_mXYd_R A, IY
LD_mXYd_R B, IY
LD_mXYd_R C, IY
LD_mXYd_R D, IY
LD_mXYd_R E, IY
LD_mXYd_R H, IY
LD_mXYd_R L, IY


align 16

Z80I_LD_mHL_N:
	mov dl, [zxPC + 1]
	mov ecx, zxHL
	add zxPC, byte 2
	WRITE_BYTE
	NEXT 10


; LD_mXYd_N src			(XY+d) <- imm8

%macro LD_mXYd_N 1

align 16

Z80I_LD_m%1d_N:
	mov	edx, zx%1
	movsx	ecx, byte [zxPC + 1]
	add	ecx, edx
	add	zxPC, byte 3
	and	ecx, 0xFFFF
	mov	dl, [zxPC - 1]
	WRITE_BYTE
	NEXT 15

%endmacro


LD_mXYd_N IX
LD_mXYd_N IY


align 16

Z80I_LD_A_mBC:
	mov ecx, zxBC
	inc zxPC
	READ_BYTE A
	NEXT 7


align 16

Z80I_LD_A_mDE:
	mov ecx, zxDE
	inc zxPC
	READ_BYTE A
	NEXT 7


align 16

Z80I_LD_A_mNN:
	movzx	edx, byte [zxPC + 2]
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 3
	or	ecx, edx
	READ_BYTE A
	NEXT 13


align 16

Z80I_LD_mBC_A:
	mov ecx, zxBC
	inc zxPC
	WRITE_BYTE A
	NEXT 7


align 16

Z80I_LD_mDE_A:
	mov ecx, zxDE
	mov dl, zA
	inc zxPC
	WRITE_BYTE
	NEXT 7


align 16

Z80I_LD_mNN_A:
	movzx	edx, byte [zxPC + 2]
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 3
	or	ecx, edx
	mov	dl, zA
	WRITE_BYTE
	NEXT 13


; LD_A_IR src			A <- I/R

%macro LD_A_IR 1

align 16

Z80I_LD_A_%1:

%ifidn %1, R
	mov	edx, [ebp + Z80.CycleCnt]
	mov	ecx, [ebp + Z80.CycleTD]
	sub	edx, edi
	add	edx, ecx
	shr	edx, 2
	mov	cl, zR
	mov	zA, dl
	and	zF, FLAG_C
	add	zA, cl
	mov	dl, zF
	and	zA, 0x7F
%else
	mov	zA, z%1
	and	zF, FLAG_C
	test	zA, zA
	mov	dl, zF
%endif
	lahf
	mov	dh, zIFF2
	mov	zFXY, zA
	or	dl, dh
	and	zF, FLAG_S | FLAG_Z
	add	zxPC, byte 2
	or	zF, dl
	NEXT 9

%endmacro


LD_A_IR I
LD_A_IR R


align 16

Z80I_LD_I_A:
	add zxPC, byte 2
	mov zI, zA
	NEXT 9


align 16

Z80I_LD_R_A:
	add zxPC, byte 2
	mov zR, zA
	NEXT 9



; Load 16 bits instruction
; ------------------------


; LD_RR_NN dest			R16 <- imm16

%macro LD_RR_NN 1

align 16

Z80I_LD_%1_NN:

%ifidn %1, HL
	mov	zL, byte [zxPC + 1]
	mov	zH, byte [zxPC + 2]
	add	zxPC, byte 3
%else
	mov	dl, byte [zxPC + 1]
	mov	dh, byte [zxPC + 2]
	add	zxPC, byte 3
	mov	zl%1, dl
	mov	zh%1, dh
%endif
	NEXT 10

%endmacro


LD_RR_NN BC
LD_RR_NN DE
LD_RR_NN HL
LD_RR_NN SP
LD_RR_NN IX
LD_RR_NN IY


align 16

Z80I_LD_HL_mNN:
	movzx	edx, byte [zxPC + 2]
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 3
	or	ecx, edx
	READ_WORD
	mov	zHL, dx
	NEXT 16


; LD_RR_mNN dest			R16 <- (imm16)

%macro LD_RR_mNN 1

align 16

%ifidn %1, HL
Z80I_LD2_HL_mNN:
%else
Z80I_LD_%1_mNN:
%endif
	movzx	edx, byte [zxPC + 3]
	movzx	ecx, byte [zxPC + 2]
	shl	edx, 8
	add	zxPC, byte 4
	or	ecx, edx
	READ_WORD
	mov	z%1, dx
	NEXT 20

%endmacro


LD_RR_mNN BC
LD_RR_mNN DE
LD_RR_mNN HL
LD_RR_mNN SP


; LD_XY_mNN dest			IX/IY <- (imm16)

%macro LD_XY_mNN 1

align 16

Z80I_LD_%1_mNN:
	movzx	edx, byte [zxPC + 2]
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 3
	or	ecx, edx
	READ_WORD
	mov	z%1, dx
	NEXT 16

%endmacro


LD_XY_mNN IX
LD_XY_mNN IY


align 16

Z80I_LD_mNN_HL:
	movzx	edx, byte [zxPC + 2]
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 3
	or	ecx, edx
	mov	dx, zHL
	WRITE_WORD
	NEXT 16


; LD_mNN_RR dest			(imm16) <- R16

%macro LD_mNN_RR 1

align 16

%ifidn %1, HL
Z80I_LD2_mNN_HL:
%else
Z80I_LD_mNN_%1:
%endif
	movzx	edx, byte [zxPC + 3]
	movzx	ecx, byte [zxPC + 2]
	shl	edx, 8
	add	zxPC, byte 4
	or	ecx, edx
	mov	dx, z%1
	WRITE_WORD
	NEXT 20

%endmacro


LD_mNN_RR BC
LD_mNN_RR DE
LD_mNN_RR HL
LD_mNN_RR SP


; LD_mNN_XY dest			(imm16) <- IX/IY

%macro LD_mNN_XY 1

align 16

Z80I_LD_mNN_%1:

	movzx	edx, byte [zxPC + 2]
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 3
	or	ecx, edx
	mov	dx, z%1
	WRITE_WORD
	NEXT 16

%endmacro


LD_mNN_XY IX
LD_mNN_XY IY


; LD_SP_RR dest			SP <- R16

%macro LD_SP_RR 1

align 16

Z80I_LD_SP_%1:

%ifidn %1, HL
	inc	zxPC
	mov	zSP, zHL
%else
	mov	dx, z%1
	inc	zxPC
	mov	zSP, dx
%endif
	NEXT 6

%endmacro


LD_SP_RR HL
LD_SP_RR IX
LD_SP_RR IY


; PUSH_RR dest			PUSH R16

%macro PUSH_RR 1

align 16

Z80I_PUSH_%1:
%ifidn %1, AF
	mov	ecx, zxSP
	inc	zxPC
	mov	dl, zF
	mov	dh, zFXY
	and	dl, ~(FLAG_X | FLAG_Y)
	and	dh, FLAG_X | FLAG_Y
	sub	ecx, byte 2
	or	dl, dh
	mov	dh, zA
	and	ecx, 0xFFFF
	mov	zxSP, ecx
	WRITE_WORD
%else
	mov	ecx, zxSP
	sub	ecx, byte 2
	inc	zxPC
	and	ecx, 0xFFFF
	mov	zxSP, ecx
	WRITE_WORD %1
%endif
	NEXT 11

%endmacro


PUSH_RR AF
PUSH_RR BC
PUSH_RR DE
PUSH_RR HL
PUSH_RR IX
PUSH_RR IY


; POP_RR dest			POP R16

%macro POP_RR 1

align 16

Z80I_POP_%1:
	mov	ecx, zxSP
	inc	zxPC
%ifidn %1, AF
	READ_WORD
	mov	zF, dl
	mov	ecx, zxSP
	mov	zFXY, dl
	add	ecx, byte 2
	mov	zA, dh
	and	ecx, 0xFFFF
	mov	zxSP, ecx
%else
	READ_WORD %1
	mov	ecx, zxSP
	add	ecx, byte 2
	and	ecx, 0xFFFF
	mov	zxSP, ecx
%endif
	NEXT 10

%endmacro


POP_RR AF
POP_RR BC
POP_RR DE
POP_RR HL
POP_RR IX
POP_RR IY


; Exchange, block transfert/search instruction
; --------------------------------------------


align 16

Z80I_EX_DE_HL:
	mov	edx, zxHL
	inc	zxPC
	mov	zxHL, zxDE
	mov	zxDE, edx
	NEXT 4


align 16

Z80I_EX_AF_AF2:
	mov	dx, zAF
	mov	cl, zFXY
	mov	ch, zFXY2
	inc	zxPC
	mov	zAF, zAF2
	mov	zAF2, dx
	mov	zFXY, ch
	mov	zFXY2, cl
	NEXT 4


align 16

Z80I_EXX:
	mov	ecx, zxBC2
	mov	edx, zxBC
	mov	zxBC, ecx
	mov	zxBC2, edx
	inc	zxPC
	mov	ecx, zxDE2
	mov	edx, zxDE
	mov	zxDE, ecx
	mov	zxDE2, edx
	mov	ecx, zxHL2
	mov	edx, zxHL
	mov	zxHL, ecx
	mov	zxHL2, edx
	NEXT 4


; EX_mSP_DD reg16		(SP) <-> R16

%macro EX_mSP_DD 1

align 16

Z80I_EX_mSP_%1:
	mov	ecx, zxSP
	inc	zxPC
	READ_WORD
	mov	cx, z%1
	mov	z%1, dx
	mov	dx, cx
	mov	ecx, zxSP
	WRITE_WORD
	NEXT 19

%endmacro


EX_mSP_DD HL
EX_mSP_DD IX
EX_mSP_DD IY


; LDX I/D				(DE++/--) <- (HL++/--), BC--
;
; TODO : emulate flag X and flag Y

%macro LDX 1

align 16

Z80I_LD%1:
	mov	ecx, zxHL
	add	zxPC, byte 2
	READ_BYTE
	mov	ecx, zxDE
	WRITE_BYTE
	and	zF, FLAG_S | FLAG_Z | FLAG_C
	mov	edx, zxBC
	mov	ecx, zxDE
%ifidn %1, I
	inc	zxHL
	dec	edx
	inc	ecx
%else
	dec	zxHL
	dec	edx
	dec	ecx
%endif
	and	zxHL, 0xFFFF
	and	ecx, 0xFFFF
	and	edx, 0xFFFF
	jz	short %%BC_zero
	
	or	zF, FLAG_P

%%BC_zero
	mov	zxDE, ecx
	mov	zxBC, edx
	NEXT 16

%endmacro


LDX I
LDX D


; LDXR I/D				do { (DE++/--) <- (HL++/--) } while(--BC)
;
; TODO : emulate flag X and flag Y

%macro LDXR 1

align 16

Z80I_LD%1R:
%%Loop
	mov	ecx, zxHL
	READ_BYTE
	mov	ecx, zxDE
	WRITE_BYTE
	mov	edx, zxBC
	mov	ecx, zxDE
%ifidn %1, I
	inc	zxHL
	dec	edx
	inc	ecx
%else
	dec	zxHL
	dec	edx
	dec	ecx
%endif
	and	zxHL, 0xFFFF
	and	ecx, 0xFFFF
	and	edx, 0xFFFF
	mov	zxDE, ecx
	mov	zxBC, edx
	jz	short %%End
	
	sub	edi, byte 21
	jns	near %%Loop
	
	and	zF, FLAG_S | FLAG_Z | FLAG_C
	jmp	z80_Exec_Really_Quit

align 16

%%End
	add	zxPC, byte 2
	and	zF, FLAG_S | FLAG_Z | FLAG_C
	NEXT 16

%endmacro


LDXR I
LDXR D


; CPX I/D				A - (HL++/--), BC--
;
; TODO : emulate flag X and flag Y

%macro CPX 1

align 16

Z80I_CP%1:
	mov	ecx, zxHL
	add	zxPC, byte 2
	READ_BYTE
	and	zF, FLAG_C
%ifidn %1, I
	inc	zxHL
%else
	dec	zxHL
%endif
	mov	dh, zF
	cmp	zA, dl
	lahf
	mov	ecx, zxBC
	and	zF, FLAG_S | FLAG_Z | FLAG_H | FLAG_N
	and	zxHL, 0xFFFF
	dec	ecx
	or	zF, dh
	and	ecx, 0xFFFF
	jz	short %%BC_zero
	
	or	zF, FLAG_P

%%BC_zero
	mov	zxBC, ecx
	NEXT 16

%endmacro


CPX I
CPX D


; CPXR I/D				do { A - (HL++/--) } while(--BC)
;
; TODO : emulate flag X and flag Y

%macro CPXR 1

align 16

Z80I_CP%1R:
	and	zF, FLAG_C
%%Loop
	mov	ecx, zxHL
	READ_BYTE
	mov	ecx, zxBC
%ifidn %1, I
	inc	zxHL
%else
	dec	zxHL
%endif
	dec	ecx
	and	zxHL, 0xFFFF
	and	ecx, 0xFFFF
	jz	short %%End_BC_zero
	
	mov	zxBC, ecx
	cmp	zA, dl
	je	short %%End_A_equal_mHL
	
	sub	edi, byte 21
	jns	short %%Loop
	
	cmp	zA, dl
	mov	dh, zF
	lahf
	or	dh, FLAG_P
	and	zF, FLAG_S | FLAG_Z | FLAG_H | FLAG_N
	or	zF, dh
	jmp	z80_Exec_Really_Quit

align 16

%%End_A_equal_mHL
	mov	dh, zF
	cmp	zA, dl
	lahf
	or	dh, FLAG_P
	and	zF, FLAG_S | FLAG_Z | FLAG_H | FLAG_N
	add	zxPC, byte 2
	or	zF, dh
	NEXT 18

align 16

%%End_BC_zero
	mov dh, zF
	cmp zA, dl
	lahf
	mov zxBC, ecx
	and zF, FLAG_S | FLAG_Z | FLAG_H | FLAG_N
	add zxPC, byte 2
	or zF, dh
	NEXT 18

%endmacro


CPXR I
CPXR D


; Arithmetic 8 bits instruction
; -----------------------------


; ARITH_A_R				 A <- A op R8

%macro ARITH_A_R 2

align 32

%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C
%define OPX %1

%ifidn %1, SUB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, SBC
	%define OPX SBB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, CP
	%define OPX CMP
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%endif

Z80I_%1_%2:
%ifnidn %2, A
	%ifnidn %2, H
		%ifnidn %2, L
			mov dl, z%2
		%endif
	%endif
%endif
%ifidn %1, ADC
	shr zF, 1
	lea zxPC, [zxPC + 1]
%elifidn %1, SBC
	shr zF, 1
	lea zxPC, [zxPC + 1]
%else
	inc zxPC
%endif
%ifnidn %2, A
	%ifnidn %2, H
		%ifnidn %2, L
			OPX zA, dl
		%else
			OPX zA, z%2
		%endif
	%else
		OPX zA, z%2
	%endif
%else
	OPX zA, z%2
%endif
	lahf
%ifidn %1, CP
	mov zFXY, dl
%else
	mov zFXY, zA
%endif
	jo short %%over

	and zF, OPFLAG
	NEXT 4

align 16

%%over
	and zF, OPFLAG
	or zF, FLAG_P
	NEXT 4

%endmacro


ARITH_A_R ADD, A
ARITH_A_R ADD, B
ARITH_A_R ADD, C
ARITH_A_R ADD, D
ARITH_A_R ADD, E
ARITH_A_R ADD, H
ARITH_A_R ADD, L
ARITH_A_R ADD, lIX
ARITH_A_R ADD, hIX
ARITH_A_R ADD, lIY
ARITH_A_R ADD, hIY

ARITH_A_R ADC, A
ARITH_A_R ADC, B
ARITH_A_R ADC, C
ARITH_A_R ADC, D
ARITH_A_R ADC, E
ARITH_A_R ADC, H
ARITH_A_R ADC, L
ARITH_A_R ADC, lIX
ARITH_A_R ADC, hIX
ARITH_A_R ADC, lIY
ARITH_A_R ADC, hIY

ARITH_A_R SUB, A
ARITH_A_R SUB, B
ARITH_A_R SUB, C
ARITH_A_R SUB, D
ARITH_A_R SUB, E
ARITH_A_R SUB, H
ARITH_A_R SUB, L
ARITH_A_R SUB, lIX
ARITH_A_R SUB, hIX
ARITH_A_R SUB, lIY
ARITH_A_R SUB, hIY

ARITH_A_R SBC, A
ARITH_A_R SBC, B
ARITH_A_R SBC, C
ARITH_A_R SBC, D
ARITH_A_R SBC, E
ARITH_A_R SBC, H
ARITH_A_R SBC, L
ARITH_A_R SBC, lIX
ARITH_A_R SBC, hIX
ARITH_A_R SBC, lIY
ARITH_A_R SBC, hIY

ARITH_A_R CP, A
ARITH_A_R CP, B
ARITH_A_R CP, C
ARITH_A_R CP, D
ARITH_A_R CP, E
ARITH_A_R CP, H
ARITH_A_R CP, L
ARITH_A_R CP, lIX
ARITH_A_R CP, hIX
ARITH_A_R CP, lIY
ARITH_A_R CP, hIY


; ARITH_A_N				 A <- A op imm8

%macro ARITH_A_N 1

align 16

%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C
%define OPX %1

%ifidn %1, SUB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, SBC
	%define OPX SBB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, CP
	%define OPX CMP
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%endif

Z80I_%1_N:
	mov dl, [zxPC + 1]
	add zxPC, byte 2
%ifidn %1, ADC
	shr zF, 1
%elifidn %1, SBC
	shr zF, 1
%endif
	OPX zA, dl
	lahf
%ifidn %1, CP
	mov zFXY, dl
%else
	mov zFXY, zA
%endif
	jo short %%over

	and zF, OPFLAG
	NEXT 7

align 16

%%over
	and zF, OPFLAG
	or zF, FLAG_P
	NEXT 7

%endmacro


ARITH_A_N ADD
ARITH_A_N ADC
ARITH_A_N SUB
ARITH_A_N SBC
ARITH_A_N CP


; ARITH_A_mHL			 A <- A op (HL)

%macro ARITH_A_mHL 1

align 16

%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C
%define OPX %1

%ifidn %1, SUB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, SBC
	%define OPX SBB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, CP
	%define OPX CMP
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%endif

Z80I_%1_mHL:
	mov ecx, zxHL
	inc zxPC
	READ_BYTE
%ifidn %1, ADC
	shr zF, 1
%elifidn %1, SBC
	shr zF, 1
%endif
	OPX zA, dl
	lahf
%ifidn %1, CP
	mov zFXY, dl
%else
	mov zFXY, zA
%endif
	jo short %%over

	and zF, OPFLAG
	NEXT 7

align 16

%%over
	and zF, OPFLAG
	or zF, FLAG_P
	NEXT 7

%endmacro


ARITH_A_mHL ADD
ARITH_A_mHL ADC
ARITH_A_mHL SUB
ARITH_A_mHL SBC
ARITH_A_mHL CP


; ARITH_A_mXYd			 A <- A op (XY+d)

%macro ARITH_A_mXYd 2

align 16

%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C
%define OPX %1

%ifidn %1, SUB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, SBC
	%define OPX SBB
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%elifidn %1, CP
	%define OPX CMP
	%define OPFLAG FLAG_S | FLAG_Z | FLAG_H | FLAG_C | FLAG_N
%endif

Z80I_%1_m%2d:
	mov edx, zx%2
	movsx ecx, byte [zxPC + 1]
	add ecx, edx
	add zxPC, byte 2
	and ecx, 0xFFFF
	READ_BYTE
%ifidn %1, ADC
	shr zF, 1
%elifidn %1, SBC
	shr zF, 1
%endif
	OPX zA, dl
	lahf
%ifidn %1, CP
	mov zFXY, dl
%else
	mov zFXY, zA
%endif
	jo short %%over

	and zF, OPFLAG
	NEXT 15

align 16

%%over
	and zF, OPFLAG
	or zF, FLAG_P
	NEXT 15

%endmacro


ARITH_A_mXYd ADD, IX
ARITH_A_mXYd ADC, IX
ARITH_A_mXYd SUB, IX
ARITH_A_mXYd SBC, IX
ARITH_A_mXYd CP, IX

ARITH_A_mXYd ADD, IY
ARITH_A_mXYd ADC, IY
ARITH_A_mXYd SUB, IY
ARITH_A_mXYd SBC, IY
ARITH_A_mXYd CP, IY


; Logic 8 bits instruction
; ------------------------


; LOGIC_A_R				 A <- A op R8

%macro LOGIC_A_R 2

align 16

Z80I_%1_%2:
%ifidn %2, A
	inc zxPC
	%1 zA, z%2
%elifidn %2, H
	inc zxPC
	%1 zA, z%2
%elifidn %2, L
	inc zxPC
	%1 zA, z%2
%else
	mov dl, z%2
	inc zxPC
	%1 zA, dl
%endif
	lahf
%ifidn %1, AND
	and zF, ~(FLAG_N | FLAG_C)
	mov zFXY, zA
	or zF, FLAG_H
%else
	mov zFXY, zA
	and zF, ~(FLAG_N | FLAG_C | FLAG_H)
%endif
	NEXT 4

%endmacro


LOGIC_A_R AND, A
LOGIC_A_R AND, B
LOGIC_A_R AND, C
LOGIC_A_R AND, D
LOGIC_A_R AND, E
LOGIC_A_R AND, H
LOGIC_A_R AND, L
LOGIC_A_R AND, lIX
LOGIC_A_R AND, hIX
LOGIC_A_R AND, lIY
LOGIC_A_R AND, hIY

LOGIC_A_R OR, A
LOGIC_A_R OR, B
LOGIC_A_R OR, C
LOGIC_A_R OR, D
LOGIC_A_R OR, E
LOGIC_A_R OR, H
LOGIC_A_R OR, L
LOGIC_A_R OR, lIX
LOGIC_A_R OR, hIX
LOGIC_A_R OR, lIY
LOGIC_A_R OR, hIY

LOGIC_A_R XOR, A
LOGIC_A_R XOR, B
LOGIC_A_R XOR, C
LOGIC_A_R XOR, D
LOGIC_A_R XOR, E
LOGIC_A_R XOR, H
LOGIC_A_R XOR, L
LOGIC_A_R XOR, lIX
LOGIC_A_R XOR, hIX
LOGIC_A_R XOR, lIY
LOGIC_A_R XOR, hIY


; LOGIC_A_N				 A <- A op imm8

%macro LOGIC_A_N 1

align 16

Z80I_%1_N:
	mov	dl, [zxPC + 1]
	add	zxPC, byte 2
	%1 zA, dl
	lahf
%ifidn %1, AND
	and	zF, ~(FLAG_N | FLAG_C)
	mov	zFXY, zA
	or	zF, FLAG_H
%else
	mov	zFXY, zA
	and	zF, ~(FLAG_N | FLAG_C | FLAG_H)
%endif
	NEXT 7

%endmacro


LOGIC_A_N AND
LOGIC_A_N OR
LOGIC_A_N XOR


; LOGIC_A_mHL			 A <- A op (HL)

%macro LOGIC_A_mHL 1

align 16

Z80I_%1_mHL:
	mov	ecx, zxHL
	inc	zxPC
	READ_BYTE
	%1 zA, dl
	lahf
%ifidn %1, AND
	and	zF, ~(FLAG_N | FLAG_C)
	mov	zFXY, zA
	or	zF, FLAG_H
%else
	mov	zFXY, zA
	and	zF, ~(FLAG_N | FLAG_C | FLAG_H)
%endif
	NEXT 7

%endmacro


LOGIC_A_mHL AND
LOGIC_A_mHL OR
LOGIC_A_mHL XOR


; LOGIC_A_mXYd			 A <- A op (XY+d)

%macro LOGIC_A_mXYd 2

align 16

Z80I_%1_m%2d:
	mov	edx, zx%2
	movsx	ecx, byte [zxPC + 1]
	add	ecx, edx
	add	zxPC, byte 2
	and	ecx, 0xFFFF
	READ_BYTE
	%1 zA, dl
	lahf
%ifidn %1, AND
	and	zF, ~(FLAG_N | FLAG_C)
	mov	zFXY, zA
	or	zF, FLAG_H
%else
	mov	zFXY, zA
	and	zF, ~(FLAG_N | FLAG_C | FLAG_H)
%endif
	NEXT 15

%endmacro


LOGIC_A_mXYd AND, IX
LOGIC_A_mXYd AND, IY
LOGIC_A_mXYd OR, IX
LOGIC_A_mXYd OR, IY
LOGIC_A_mXYd XOR, IX
LOGIC_A_mXYd XOR, IY


; INC/DEC 8 bits instruction
; --------------------------


; INCDEC_R				 R8 <- R8 +/- 1

%macro INCDEC_R 2

align 16

Z80I_%1_%2:
%ifnidn %2, A
	%ifnidn %2, H
		%ifnidn %2, L
			%define rd dl
			mov dl, z%2
		%else
			%define rd z%2
		%endif
	%else
		%define rd z%2
	%endif
%else
	%define rd z%2
%endif
	and	zF, FLAG_C
	inc	zxPC
	movzx	ecx, rd
	%1 rd
	mov	dh, [_mdZ80_%1_Flags_Table + ecx]
	mov	zFXY, rd
	or	zF, dh
%ifidn rd, dl
	mov	z%2, rd
%endif
	NEXT 4

%endmacro


INCDEC_R INC, A
INCDEC_R INC, B
INCDEC_R INC, C
INCDEC_R INC, D
INCDEC_R INC, E
INCDEC_R INC, H
INCDEC_R INC, L
INCDEC_R INC, lIX
INCDEC_R INC, hIX
INCDEC_R INC, lIY
INCDEC_R INC, hIY

INCDEC_R DEC, A
INCDEC_R DEC, B
INCDEC_R DEC, C
INCDEC_R DEC, D
INCDEC_R DEC, E
INCDEC_R DEC, H
INCDEC_R DEC, L
INCDEC_R DEC, lIX
INCDEC_R DEC, hIX
INCDEC_R DEC, lIY
INCDEC_R DEC, hIY


; INCDEC_mHL				 (HL) <- (HL) +/- 1

%macro INCDEC_mHL 1

align 16

Z80I_%1_mHL:
	mov	ecx, zxHL
	inc	zxPC
	READ_BYTE
	and	zF, FLAG_C
	movzx	ecx, dl
	%1 dl
	mov	dh, [_mdZ80_%1_Flags_Table + ecx]
	mov	zFXY, dl
	mov	ecx, zxHL
	or	zF, dh
	WRITE_BYTE
	NEXT 11

%endmacro


INCDEC_mHL INC
INCDEC_mHL DEC


; INCDEC_mXYd				 (XY+d) <- (XY+d) +/- 1

%macro INCDEC_mXYd 2

align 16

Z80I_%1_m%2d:
	mov	edx, zx%2
	movsx	ecx, byte [zxPC + 1]
	add	ecx, edx
	add	zxPC, byte 2
	and	ecx, 0xFFFF
	push	ecx
	READ_BYTE
	and	zF, FLAG_C
	movzx	ecx, dl
	%1 dl
	mov	dh, [_mdZ80_%1_Flags_Table + ecx]
	mov	zFXY, dl
	pop	ecx
	or	zF, dh
	WRITE_BYTE
	NEXT 22

%endmacro


INCDEC_mXYd INC, IX
INCDEC_mXYd INC, IY
INCDEC_mXYd DEC, IX
INCDEC_mXYd DEC, IY




; Misc instruction
; ----------------


align 16

Z80I_DAA:
	mov	edx, eax
	and	eax, 0x3FF
	and	edx, 0x1000
	movzx	eax, word [_mdZ80_DAA_Table + eax * 2 + edx]
	inc	zxPC
	mov	zFXY, zF
	NEXT 4


align 16

Z80I_CPL:
	inc	zxPC
	not	zA
	or	zF, FLAG_H | FLAG_N
	mov	zFXY, zA
	NEXT 4


align 16

Z80I_NEG:
	add	zxPC, byte 2
	neg	zA
	lahf
	mov	zFXY, zA
	jo	short .over
	
	and	zF, ~FLAG_P
	NEXT 8

align 16

.over
	or	zF, FLAG_P
	NEXT 8


align 16

Z80I_CCF:
	mov	dh, zF
	xor	zF, FLAG_C
	and	dh, FLAG_C
	and	zF, FLAG_S | FLAG_Z | FLAG_P | FLAG_C
	shl	dh, 4
	inc	zxPC
	or	zF, dh
	NEXT 4


align 16

Z80I_SCF:
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	inc	zxPC
	mov	zFXY, zA
	or	zF, FLAG_C
	NEXT 4


align 16

Z80I_HALT:
	mov	edx, [ebp + Z80.Status]
	or	edi, byte -1
	or	edx, Z80_HALTED
	inc	zxPC
	mov	[ebp + Z80.Status], edx
	jmp	z80_Exec_Really_Quit


align 16

Z80I_DI:
	;xor	ecx, ecx
	;inc	zxPC
	;mov	zxIFF, ecx
	;NEXT 4
	
	movzx	edx, byte [zxPC + 1]
	xor	ecx, ecx
	inc	zxPC
	mov	zxIFF, ecx
	sub	edi, byte 4
	
%if (GENS_LOG == 1)
	push	eax
	push	ecx
	push	edx
	mov	ecx, zxPC
	sub	ecx, [ebp + Z80.BasePC]
	call	@z80log@4
	pop	edx
	pop	ecx
	pop	eax
%endif
	
	jmp	[OP_Table + edx * 4]


align 16

Z80I_EI:
	movzx	edx, byte [zxPC + 1]
	mov	[ebp + Z80.CycleSup], edi	; we will check for interrupt after the next instruction
	inc	zxPC
	mov	ecx, FLAG_P | (FLAG_P << 8)
	xor	edi, edi
	mov	zxIFF, ecx
	sub	edi, byte 4
	
%if (GENS_LOG == 1)
	push	eax
	push	ecx
	push	edx
	mov	ecx, zxPC
	sub	ecx, [ebp + Z80.BasePC]
	call	@z80log@4
	pop	edx
	pop	ecx
	pop	eax
%endif
	
	jmp	[OP_Table + edx * 4]


align 16

Z80I_IM0:
	add	zxPC, byte 2
	mov	zIM, byte 0
	NEXT 8


align 16

Z80I_IM1:
	add	zxPC, byte 2
	mov	zIM, byte 1
	NEXT 8


align 16

Z80I_IM2:
	add	zxPC, byte 2
	mov	zIM, byte 2
	NEXT 8


; Arithmetic 16 bits instruction
; ------------------------------


; ADD_RR_RR				R16 <- R16 + R16

%macro ADD_RR_RR 2

%define rdl zl%1
%define rdh zh%1
%define rsl zl%2
%define rsh zh%2

align 16

Z80I_ADD_%1_%2:
%ifnidn %1, HL
	%define rdl dl
	%define rdh dh
	mov	dl, zl%1
	mov	dh, zh%1
%endif
	inc	zxPC
%ifnidn %2, HL
	%define rsl cl
	%define rsh ch
	mov	cl, zl%2
	mov	ch, zh%2
%endif
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	add	rdl, rsl
	adc	rdh, rsh
	mov	cl, zF
	lahf
	and	zF, ~(FLAG_S | FLAG_Z | FLAG_P | FLAG_N)
%ifidn rdl, dl
	mov	zl%1, rdl
	mov	zh%1, rdh
%endif
	or	zF, cl
	mov	zFXY, rdh
	NEXT 11

%endmacro


ADD_RR_RR HL, BC
ADD_RR_RR HL, DE
ADD_RR_RR HL, HL
ADD_RR_RR HL, SP

ADD_RR_RR IX, BC
ADD_RR_RR IX, DE
ADD_RR_RR IX, IX
ADD_RR_RR IX, SP

ADD_RR_RR IY, BC
ADD_RR_RR IY, DE
ADD_RR_RR IY, IY
ADD_RR_RR IY, SP


; ADC_HL_RR				HL <- HL + R16 + Carry

%macro ADC_HL_RR 1

%define rsl zl%1
%define rsh zh%1

align 16

Z80I_ADC_HL_%1:
%ifnidn %1, HL
	%define rsl cl
	%define rsh ch
	mov	cl, zl%1
	mov	ch, zh%1
%endif
	shr	zF, 1
	lea	zxPC, [zxPC + 2]
	adc	zlHL, rsl
	adc	zhHL, rsh
	lahf
	seto	dl
	mov	dh, zlHL
	and	zF, ~(FLAG_Z | FLAG_P | FLAG_N)
	or	dh, zhHL
	setz	dh
	shl	dl, 2
	mov	zFXY, zhHL
	shl	dh, 6
	or	zF, dl
	or	zF, dh
	NEXT 15

%endmacro


ADC_HL_RR BC
ADC_HL_RR DE
ADC_HL_RR HL
ADC_HL_RR SP


; SBC_HL_RR				HL <- HL - R16 - Carry

%macro SBC_HL_RR 1

%define rsl zl%1
%define rsh zh%1

align 16

Z80I_SBC_HL_%1:
%ifnidn %1, HL
	%define rsl cl
	%define rsh ch
	mov	cl, zl%1
	mov	ch, zh%1
%endif
	shr	zF, 1
	lea	zxPC, [zxPC + 2]
	sbb	zlHL, rsl
	sbb	zhHL, rsh
	lahf
	seto	dl
	mov	dh, zlHL
	and	zF, ~(FLAG_Z | FLAG_P)
	or	dh, zhHL
	mov	zFXY, zhHL
	setz	dh
	shl	dl, 2
	or	zF, FLAG_N
	shl	dh, 6
	or	zF, dl
	or	zF, dh
	NEXT 15

%endmacro


SBC_HL_RR BC
SBC_HL_RR DE
SBC_HL_RR HL
SBC_HL_RR SP


; INCDEC_RR				 R16 <- R16 +/- 1

%macro INCDEC_RR 2

align 16

Z80I_%1_%2:
	%1 z%2
	inc zxPC
	NEXT 6

%endmacro

INCDEC_RR INC, BC
INCDEC_RR INC, DE
INCDEC_RR INC, HL
INCDEC_RR INC, IX
INCDEC_RR INC, IY
INCDEC_RR INC, SP

INCDEC_RR DEC, BC
INCDEC_RR DEC, DE
INCDEC_RR DEC, HL
INCDEC_RR DEC, IX
INCDEC_RR DEC, IY
INCDEC_RR DEC, SP


; Rotate and shift instruction
; ----------------------------


align 16

Z80I_RLCA:
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	rol	zA, 1
	lea	zxPC, [zxPC + 1]
	adc	zF, 0
	mov	zFXY, zA
	NEXT 4


align 16

Z80I_RLA:
	and	zF, FLAG_S | FLAG_Z | FLAG_P | FLAG_C
	inc	zxPC
	shr	zF, 1
	rcl	zA, 1
	adc	zF, zF
	mov	zFXY, zA
	NEXT 4


align 16

Z80I_RRCA:
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	ror	zA, 1
	lea	zxPC, [zxPC + 1]
	adc	zF, 0
	mov	zFXY, zA
	NEXT 4


align 16

Z80I_RRA:
	and	zF, FLAG_S | FLAG_Z | FLAG_P | FLAG_C
	inc	zxPC
	shr	zF, 1
	rcr	zA, 1
	adc	zF, zF
	mov	zFXY, zA
	NEXT 4


; ROT_R					op R8

%macro ROT_R 2

align 16

Z80I_%1_%2:
%ifnidn %2, A
	%ifnidn %2, H
		%ifnidn %2, L
			%define reg dl
			mov dl, z%2
		%else
			%define reg z%2
		%endif
	%else
		%define reg z%2
	%endif
%else
	%define reg z%2
%endif
%ifidn %1, RLC
	add zxPC, byte 2
	rol reg, 1
	%define sft 0
%elifidn %1, RL
	shr zF, 1
	lea zxPC, [zxPC + 2]
	rcl reg, 1
	%define sft 0
%elifidn %1, RRC
	add zxPC, byte 2
	ror reg, 1
	%define sft 0
%elifidn %1, RR
	shr zF, 1
	lea zxPC, [zxPC + 2]
	rcr reg, 1
	%define sft 0
%elifidn %1, SLA
	add zxPC, byte 2
	add reg, reg
	%define sft 1
%elifidn %1, SLL
	add zxPC, byte 2
	add reg, reg
	%define sft 1
%elifidn %1, SRA
	add zxPC, byte 2
	sar reg, 1
	%define sft 1
%elifidn %1, SRL
	add zxPC, byte 2
	shr reg, 1
	%define sft 1
%endif
%if (sft == 0)
	setc dh
	test reg, reg
	%ifidn reg, dl
		mov z%2, reg
	%endif
	lahf
	and zF, FLAG_Z | FLAG_S | FLAG_P
	mov zFXY, reg
	or zF, dh
%else
	lahf
	%ifidn reg, dl
		mov z%2, reg
	%endif
	mov zFXY, reg
	and zF, FLAG_S | FLAG_Z | FLAG_P | FLAG_C
	%ifidn %1, SLL
		inc reg
		or zF, FLAG_P
	%endif
%endif

	NEXT 8

%endmacro


ROT_R RLC, A
ROT_R RLC, B
ROT_R RLC, C
ROT_R RLC, D
ROT_R RLC, E
ROT_R RLC, H
ROT_R RLC, L

ROT_R RL, A
ROT_R RL, B
ROT_R RL, C
ROT_R RL, D
ROT_R RL, E
ROT_R RL, H
ROT_R RL, L

ROT_R RRC, A
ROT_R RRC, B
ROT_R RRC, C
ROT_R RRC, D
ROT_R RRC, E
ROT_R RRC, H
ROT_R RRC, L

ROT_R RR, A
ROT_R RR, B
ROT_R RR, C
ROT_R RR, D
ROT_R RR, E
ROT_R RR, H
ROT_R RR, L

ROT_R SLA, A
ROT_R SLA, B
ROT_R SLA, C
ROT_R SLA, D
ROT_R SLA, E
ROT_R SLA, H
ROT_R SLA, L

ROT_R SLL, A
ROT_R SLL, B
ROT_R SLL, C
ROT_R SLL, D
ROT_R SLL, E
ROT_R SLL, H
ROT_R SLL, L

ROT_R SRA, A
ROT_R SRA, B
ROT_R SRA, C
ROT_R SRA, D
ROT_R SRA, E
ROT_R SRA, H
ROT_R SRA, L

ROT_R SRL, A
ROT_R SRL, B
ROT_R SRL, C
ROT_R SRL, D
ROT_R SRL, E
ROT_R SRL, H
ROT_R SRL, L


; ROT_mHL				op (HL)

%macro ROT_mHL 1

align 16

Z80I_%1_mHL:
	mov ecx, zxHL
	add zxPC, byte 2
	READ_BYTE
%ifidn %1, RLC
	rol dl, 1
	mov ecx, zxHL
	%define sft 0
%elifidn %1, RL
	shr zF, 1
	mov ecx, zxHL
	rcl dl, 1
	%define sft 0
%elifidn %1, RRC
	ror dl, 1
	mov ecx, zxHL
	%define sft 0
%elifidn %1, RR
	shr zF, 1
	mov ecx, zxHL
	rcr dl, 1
	%define sft 0
%elifidn %1, SLA
	add dl, dl
	mov ecx, zxHL
	%define sft 1
%elifidn %1, SLL
	add dl, dl
	mov ecx, zxHL
	%define sft 1
%elifidn %1, SRA
	sar dl, 1
	mov ecx, zxHL
	%define sft 1
%elifidn %1, SRL
	shr dl, 1
	mov ecx, zxHL
	%define sft 1
%endif
%if (sft == 0)
	setc dh
	test dl, dl
	lahf
	and zF, FLAG_Z | FLAG_S | FLAG_P
	mov zFXY, dl
	or zF, dh
%else
	lahf
	mov zFXY, dl
	and zF, FLAG_S | FLAG_Z | FLAG_P | FLAG_C
	%ifidn %1, SLL
		inc dl
		or zF, FLAG_P
	%endif
%endif
	WRITE_BYTE
	NEXT 15

%endmacro


ROT_mHL RLC
ROT_mHL RL
ROT_mHL RRC
ROT_mHL RR
ROT_mHL SLA
ROT_mHL SLL
ROT_mHL SRA
ROT_mHL SRL


; ROT_mXYd_R			op (XY+d), R

%macro ROT_mXYd_R 2-3

align 16

%if %0 > 2
Z80I_%1_m%2d_%3:
%else
Z80I_%1_m%2d:
%endif
	mov edx, zx%2
	movsx ecx, byte [zxPC + 1]
	add ecx, edx
	add zxPC, byte 3
	and ecx, 0xFFFF
	push ecx
	READ_BYTE
%ifidn %1, RLC
	rol dl, 1
	pop ecx
	%define sft 0
%elifidn %1, RL
	shr zF, 1
	pop ecx
	rcl dl, 1
	%define sft 0
%elifidn %1, RRC
	ror dl, 1
	pop ecx
	%define sft 0
%elifidn %1, RR
	shr zF, 1
	pop ecx
	rcr dl, 1
	%define sft 0
%elifidn %1, SLA
	add dl, dl
	pop ecx
	%define sft 1
%elifidn %1, SLL
	add dl, dl
	pop ecx
	%define sft 1
%elifidn %1, SRA
	sar dl, 1
	pop ecx
	%define sft 1
%elifidn %1, SRL
	shr dl, 1
	pop ecx
	%define sft 1
%endif
%if (sft == 0)
	setc dh
	test dl, dl
	%if %0 > 2
		mov z%3, dl
	%endif
	lahf
	and zF, FLAG_Z | FLAG_S | FLAG_P
	mov zFXY, dl
	or zF, dh
%else
	lahf
	mov zFXY, dl
	and zF, FLAG_S | FLAG_Z | FLAG_P | FLAG_C
	%ifidn %1, SLL
		inc dl
		or zF, FLAG_P
	%endif
	%if %0 > 2
		mov z%3, dl
	%endif
%endif
	WRITE_BYTE
	NEXT 23

%endmacro


ROT_mXYd_R RLC, IX
ROT_mXYd_R RLC, IX, A
ROT_mXYd_R RLC, IX, B
ROT_mXYd_R RLC, IX, C
ROT_mXYd_R RLC, IX, D
ROT_mXYd_R RLC, IX, E
ROT_mXYd_R RLC, IX, H
ROT_mXYd_R RLC, IX, L

ROT_mXYd_R RLC, IY
ROT_mXYd_R RLC, IY, A
ROT_mXYd_R RLC, IY, B
ROT_mXYd_R RLC, IY, C
ROT_mXYd_R RLC, IY, D
ROT_mXYd_R RLC, IY, E
ROT_mXYd_R RLC, IY, H
ROT_mXYd_R RLC, IY, L

ROT_mXYd_R RL, IX
ROT_mXYd_R RL, IX, A
ROT_mXYd_R RL, IX, B
ROT_mXYd_R RL, IX, C
ROT_mXYd_R RL, IX, D
ROT_mXYd_R RL, IX, E
ROT_mXYd_R RL, IX, H
ROT_mXYd_R RL, IX, L

ROT_mXYd_R RL, IY
ROT_mXYd_R RL, IY, A
ROT_mXYd_R RL, IY, B
ROT_mXYd_R RL, IY, C
ROT_mXYd_R RL, IY, D
ROT_mXYd_R RL, IY, E
ROT_mXYd_R RL, IY, H
ROT_mXYd_R RL, IY, L

ROT_mXYd_R RRC, IX
ROT_mXYd_R RRC, IX, A
ROT_mXYd_R RRC, IX, B
ROT_mXYd_R RRC, IX, C
ROT_mXYd_R RRC, IX, D
ROT_mXYd_R RRC, IX, E
ROT_mXYd_R RRC, IX, H
ROT_mXYd_R RRC, IX, L

ROT_mXYd_R RRC, IY
ROT_mXYd_R RRC, IY, A
ROT_mXYd_R RRC, IY, B
ROT_mXYd_R RRC, IY, C
ROT_mXYd_R RRC, IY, D
ROT_mXYd_R RRC, IY, E
ROT_mXYd_R RRC, IY, H
ROT_mXYd_R RRC, IY, L

ROT_mXYd_R RR, IX
ROT_mXYd_R RR, IX, A
ROT_mXYd_R RR, IX, B
ROT_mXYd_R RR, IX, C
ROT_mXYd_R RR, IX, D
ROT_mXYd_R RR, IX, E
ROT_mXYd_R RR, IX, H
ROT_mXYd_R RR, IX, L

ROT_mXYd_R RR, IY
ROT_mXYd_R RR, IY, A
ROT_mXYd_R RR, IY, B
ROT_mXYd_R RR, IY, C
ROT_mXYd_R RR, IY, D
ROT_mXYd_R RR, IY, E
ROT_mXYd_R RR, IY, H
ROT_mXYd_R RR, IY, L

ROT_mXYd_R SLA, IX
ROT_mXYd_R SLA, IX, A
ROT_mXYd_R SLA, IX, B
ROT_mXYd_R SLA, IX, C
ROT_mXYd_R SLA, IX, D
ROT_mXYd_R SLA, IX, E
ROT_mXYd_R SLA, IX, H
ROT_mXYd_R SLA, IX, L

ROT_mXYd_R SLA, IY
ROT_mXYd_R SLA, IY, A
ROT_mXYd_R SLA, IY, B
ROT_mXYd_R SLA, IY, C
ROT_mXYd_R SLA, IY, D
ROT_mXYd_R SLA, IY, E
ROT_mXYd_R SLA, IY, H
ROT_mXYd_R SLA, IY, L

ROT_mXYd_R SLL, IX
ROT_mXYd_R SLL, IX, A
ROT_mXYd_R SLL, IX, B
ROT_mXYd_R SLL, IX, C
ROT_mXYd_R SLL, IX, D
ROT_mXYd_R SLL, IX, E
ROT_mXYd_R SLL, IX, H
ROT_mXYd_R SLL, IX, L

ROT_mXYd_R SLL, IY
ROT_mXYd_R SLL, IY, A
ROT_mXYd_R SLL, IY, B
ROT_mXYd_R SLL, IY, C
ROT_mXYd_R SLL, IY, D
ROT_mXYd_R SLL, IY, E
ROT_mXYd_R SLL, IY, H
ROT_mXYd_R SLL, IY, L

ROT_mXYd_R SRA, IX
ROT_mXYd_R SRA, IX, A
ROT_mXYd_R SRA, IX, B
ROT_mXYd_R SRA, IX, C
ROT_mXYd_R SRA, IX, D
ROT_mXYd_R SRA, IX, E
ROT_mXYd_R SRA, IX, H
ROT_mXYd_R SRA, IX, L

ROT_mXYd_R SRA, IY
ROT_mXYd_R SRA, IY, A
ROT_mXYd_R SRA, IY, B
ROT_mXYd_R SRA, IY, C
ROT_mXYd_R SRA, IY, D
ROT_mXYd_R SRA, IY, E
ROT_mXYd_R SRA, IY, H
ROT_mXYd_R SRA, IY, L

ROT_mXYd_R SRL, IX
ROT_mXYd_R SRL, IX, A
ROT_mXYd_R SRL, IX, B
ROT_mXYd_R SRL, IX, C
ROT_mXYd_R SRL, IX, D
ROT_mXYd_R SRL, IX, E
ROT_mXYd_R SRL, IX, H
ROT_mXYd_R SRL, IX, L

ROT_mXYd_R SRL, IY
ROT_mXYd_R SRL, IY, A
ROT_mXYd_R SRL, IY, B
ROT_mXYd_R SRL, IY, C
ROT_mXYd_R SRL, IY, D
ROT_mXYd_R SRL, IY, E
ROT_mXYd_R SRL, IY, H
ROT_mXYd_R SRL, IY, L


align 16

Z80I_RLD:
	mov	ecx, zxHL
	add	zxPC, byte 2
	READ_BYTE
	mov	dh, zF
	mov	ch, dl
	mov	cl, zA
	and	dh, FLAG_C
	shr	ch, 4
	and	cl, 0xF0
	shl	dl, 4
	and	ch, 0x0F
	and	zA, 0xF0
	or	dl, cl
	or	zA, ch
	lahf
	mov	ecx, zxHL
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	mov	zFXY, zA
	or	zF, dh
	WRITE_BYTE
	NEXT 18


align 16

Z80I_RRD:
	mov	ecx, zxHL
	add	zxPC, byte 2
	READ_BYTE
	mov	dh, zF
	mov	ch, dl
	mov	cl, zA
	and	dh, FLAG_C
	shl	cl, 4
	and	ch, 0x0F
	shr	dl, 4
	and	cl, 0xF0
	and	zA, 0xF0
	or	dl, cl
	or	zA, ch
	lahf
	mov	ecx, zxHL
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	mov	zFXY, zA
	or	zF, dh
	WRITE_BYTE
	NEXT 18


; Bits operation instruction
; --------------------------


; BITb_R				ZF <- !Rb

%macro BITb_R 2

align 16

Z80I_BIT%1_%2:
%ifnidn %2, A
	%ifnidn %2, H
		%ifnidn %2, L
			%define reg dl
			mov dl, z%2
		%else
			%define reg z%2
		%endif
	%else
		%define reg z%2
	%endif
%else
	%define reg z%2
%endif
	and zF, FLAG_C
	test reg, (1 << %1)
	lea zxPC, [zxPC + 2]
	jz short %%zero

%if (%1 = 7)
	mov zFXY, 0
	or zF, FLAG_S | FLAG_H
%elif (%1 = 5)
	mov zFXY, reg
	or zF, FLAG_Y | FLAG_H
%elif (%1 = 3)
	mov zFXY, reg
	or zF, FLAG_X | FLAG_H
%else
	mov zFXY, 0
	or zF, FLAG_H
%endif
	NEXT 8

align 16

%%zero
	mov zFXY, 0
	or zF, FLAG_Z | FLAG_H | FLAG_P
	NEXT 8

%endmacro


BITb_R 0, A
BITb_R 1, A
BITb_R 2, A
BITb_R 3, A
BITb_R 4, A
BITb_R 5, A
BITb_R 6, A
BITb_R 7, A

BITb_R 0, B
BITb_R 1, B
BITb_R 2, B
BITb_R 3, B
BITb_R 4, B
BITb_R 5, B
BITb_R 6, B
BITb_R 7, B

BITb_R 0, C
BITb_R 1, C
BITb_R 2, C
BITb_R 3, C
BITb_R 4, C
BITb_R 5, C
BITb_R 6, C
BITb_R 7, C

BITb_R 0, D
BITb_R 1, D
BITb_R 2, D
BITb_R 3, D
BITb_R 4, D
BITb_R 5, D
BITb_R 6, D
BITb_R 7, D

BITb_R 0, E
BITb_R 1, E
BITb_R 2, E
BITb_R 3, E
BITb_R 4, E
BITb_R 5, E
BITb_R 6, E
BITb_R 7, E

BITb_R 0, H
BITb_R 1, H
BITb_R 2, H
BITb_R 3, H
BITb_R 4, H
BITb_R 5, H
BITb_R 6, H
BITb_R 7, H

BITb_R 0, L
BITb_R 1, L
BITb_R 2, L
BITb_R 3, L
BITb_R 4, L
BITb_R 5, L
BITb_R 6, L
BITb_R 7, L


; BITb_mHL				ZF <- !(HL)b

%macro BITb_mHL 1

align 16

Z80I_BIT%1_mHL:
	mov ecx, zxHL
	and zF, FLAG_C
	READ_BYTE
	test dl, (1 << %1)
	lea zxPC, [zxPC + 2]
	jz short %%zero

%if (%1 = 7)
	mov zFXY, 0
	or zF, FLAG_S | FLAG_H
%elif (%1 = 5)
	mov zFXY, ch
	or zF, FLAG_Y | FLAG_H
%elif (%1 = 3)
	mov zFXY, ch
	or zF, FLAG_X | FLAG_H
%else
	mov zFXY, 0
	or zF, FLAG_H
%endif
	NEXT 12

align 16

%%zero
	mov zFXY, 0
	or zF, FLAG_Z | FLAG_H | FLAG_P
	NEXT 12

%endmacro


BITb_mHL 0
BITb_mHL 1
BITb_mHL 2
BITb_mHL 3
BITb_mHL 4
BITb_mHL 5
BITb_mHL 6
BITb_mHL 7


; BITb_mXYd				ZF <- !(XY+d)b

%macro BITb_mXYd 2

align 16

Z80I_BIT%1_m%2d:
	mov edx, zx%2
	movsx ecx, byte [zxPC + 1]
	add ecx, edx
	and zF, FLAG_C
	and ecx, 0xFFFF
	READ_BYTE
	test dl, (1 << %1)
	lea zxPC, [zxPC + 3]
	jz short %%zero

%if (%1 == 7)
	mov zFXY, 0
	or zF, FLAG_S | FLAG_H
%elif (%1 == 5)
	mov zFXY, ch
	or zF, FLAG_Y | FLAG_H
%elif (%1 == 3)
	mov zFXY, ch
	or zF, FLAG_X | FLAG_H
%else
	mov zFXY, 0
	or zF, FLAG_H
%endif
	NEXT 16

align 16

%%zero
	mov zFXY, 0
	or zF, FLAG_Z | FLAG_H | FLAG_P
	NEXT 16

%endmacro


BITb_mXYd 0, IX
BITb_mXYd 1, IX
BITb_mXYd 2, IX
BITb_mXYd 3, IX
BITb_mXYd 4, IX
BITb_mXYd 5, IX
BITb_mXYd 6, IX
BITb_mXYd 7, IX

BITb_mXYd 0, IY
BITb_mXYd 1, IY
BITb_mXYd 2, IY
BITb_mXYd 3, IY
BITb_mXYd 4, IY
BITb_mXYd 5, IY
BITb_mXYd 6, IY
BITb_mXYd 7, IY


; SETRESb_R				Rb <- 1/0

%macro SETRESb_R 3

align 16

Z80I_%1%2_%3:
%ifidn %1, SET
	or z%3, (1 << %2)
%else
	and z%3, ~(1 << %2)
%endif
	add zxPC, byte 2
	NEXT 8

%endmacro


SETRESb_R SET, 0, A
SETRESb_R SET, 1, A
SETRESb_R SET, 2, A
SETRESb_R SET, 3, A
SETRESb_R SET, 4, A
SETRESb_R SET, 5, A
SETRESb_R SET, 6, A
SETRESb_R SET, 7, A

SETRESb_R RES, 0, A
SETRESb_R RES, 1, A
SETRESb_R RES, 2, A
SETRESb_R RES, 3, A
SETRESb_R RES, 4, A
SETRESb_R RES, 5, A
SETRESb_R RES, 6, A
SETRESb_R RES, 7, A

SETRESb_R SET, 0, B
SETRESb_R SET, 1, B
SETRESb_R SET, 2, B
SETRESb_R SET, 3, B
SETRESb_R SET, 4, B
SETRESb_R SET, 5, B
SETRESb_R SET, 6, B
SETRESb_R SET, 7, B

SETRESb_R RES, 0, B
SETRESb_R RES, 1, B
SETRESb_R RES, 2, B
SETRESb_R RES, 3, B
SETRESb_R RES, 4, B
SETRESb_R RES, 5, B
SETRESb_R RES, 6, B
SETRESb_R RES, 7, B

SETRESb_R SET, 0, C
SETRESb_R SET, 1, C
SETRESb_R SET, 2, C
SETRESb_R SET, 3, C
SETRESb_R SET, 4, C
SETRESb_R SET, 5, C
SETRESb_R SET, 6, C
SETRESb_R SET, 7, C

SETRESb_R RES, 0, C
SETRESb_R RES, 1, C
SETRESb_R RES, 2, C
SETRESb_R RES, 3, C
SETRESb_R RES, 4, C
SETRESb_R RES, 5, C
SETRESb_R RES, 6, C
SETRESb_R RES, 7, C

SETRESb_R SET, 0, D
SETRESb_R SET, 1, D
SETRESb_R SET, 2, D
SETRESb_R SET, 3, D
SETRESb_R SET, 4, D
SETRESb_R SET, 5, D
SETRESb_R SET, 6, D
SETRESb_R SET, 7, D

SETRESb_R RES, 0, D
SETRESb_R RES, 1, D
SETRESb_R RES, 2, D
SETRESb_R RES, 3, D
SETRESb_R RES, 4, D
SETRESb_R RES, 5, D
SETRESb_R RES, 6, D
SETRESb_R RES, 7, D

SETRESb_R SET, 0, E
SETRESb_R SET, 1, E
SETRESb_R SET, 2, E
SETRESb_R SET, 3, E
SETRESb_R SET, 4, E
SETRESb_R SET, 5, E
SETRESb_R SET, 6, E
SETRESb_R SET, 7, E

SETRESb_R RES, 0, E
SETRESb_R RES, 1, E
SETRESb_R RES, 2, E
SETRESb_R RES, 3, E
SETRESb_R RES, 4, E
SETRESb_R RES, 5, E
SETRESb_R RES, 6, E
SETRESb_R RES, 7, E

SETRESb_R SET, 0, H
SETRESb_R SET, 1, H
SETRESb_R SET, 2, H
SETRESb_R SET, 3, H
SETRESb_R SET, 4, H
SETRESb_R SET, 5, H
SETRESb_R SET, 6, H
SETRESb_R SET, 7, H

SETRESb_R RES, 0, H
SETRESb_R RES, 1, H
SETRESb_R RES, 2, H
SETRESb_R RES, 3, H
SETRESb_R RES, 4, H
SETRESb_R RES, 5, H
SETRESb_R RES, 6, H
SETRESb_R RES, 7, H

SETRESb_R SET, 0, L
SETRESb_R SET, 1, L
SETRESb_R SET, 2, L
SETRESb_R SET, 3, L
SETRESb_R SET, 4, L
SETRESb_R SET, 5, L
SETRESb_R SET, 6, L
SETRESb_R SET, 7, L

SETRESb_R RES, 0, L
SETRESb_R RES, 1, L
SETRESb_R RES, 2, L
SETRESb_R RES, 3, L
SETRESb_R RES, 4, L
SETRESb_R RES, 5, L
SETRESb_R RES, 6, L
SETRESb_R RES, 7, L


; SETRESb_mHL			(HL)b <- 1/0

%macro SETRESb_mHL 2

align 16

Z80I_%1%2_mHL:
	mov ecx, zxHL
	add zxPC, byte 2
	READ_BYTE
	mov ecx, zxHL
%ifidn %1, SET
	or dl, (1 << %2)
%else
	and dl, ~(1 << %2)
%endif
	WRITE_BYTE
	NEXT 15

%endmacro


SETRESb_mHL SET, 0
SETRESb_mHL SET, 1
SETRESb_mHL SET, 2
SETRESb_mHL SET, 3
SETRESb_mHL SET, 4
SETRESb_mHL SET, 5
SETRESb_mHL SET, 6
SETRESb_mHL SET, 7

SETRESb_mHL RES, 0
SETRESb_mHL RES, 1
SETRESb_mHL RES, 2
SETRESb_mHL RES, 3
SETRESb_mHL RES, 4
SETRESb_mHL RES, 5
SETRESb_mHL RES, 6
SETRESb_mHL RES, 7


; SETRESb_mXYd_R		(XY+d)b, R <- 1/0

%macro SETRESb_mXYd_R 3-4

align 16

%if %0 > 3
Z80I_%1%2_m%3d_%4:
%else
Z80I_%1%2_m%3d:
%endif
	mov edx, zx%3
	movsx ecx, byte [zxPC + 1]
	add ecx, edx
	add zxPC, byte 3
	and ecx, 0xFFFF
	push ecx
	READ_BYTE
%if %0 < 4
	pop ecx
%endif
%ifidn %1, SET
	or dl, (1 << %2)
%else
	and dl, ~(1 << %2)
%endif
%if %0 > 3
	pop ecx
	mov z%4, dl
%endif
	WRITE_BYTE
	NEXT 19

%endmacro


SETRESb_mXYd_R SET, 0, IX
SETRESb_mXYd_R SET, 0, IX, A
SETRESb_mXYd_R SET, 0, IX, B
SETRESb_mXYd_R SET, 0, IX, C
SETRESb_mXYd_R SET, 0, IX, D
SETRESb_mXYd_R SET, 0, IX, E
SETRESb_mXYd_R SET, 0, IX, H
SETRESb_mXYd_R SET, 0, IX, L

SETRESb_mXYd_R SET, 1, IX
SETRESb_mXYd_R SET, 1, IX, A
SETRESb_mXYd_R SET, 1, IX, B
SETRESb_mXYd_R SET, 1, IX, C
SETRESb_mXYd_R SET, 1, IX, D
SETRESb_mXYd_R SET, 1, IX, E
SETRESb_mXYd_R SET, 1, IX, H
SETRESb_mXYd_R SET, 1, IX, L

SETRESb_mXYd_R SET, 2, IX
SETRESb_mXYd_R SET, 2, IX, A
SETRESb_mXYd_R SET, 2, IX, B
SETRESb_mXYd_R SET, 2, IX, C
SETRESb_mXYd_R SET, 2, IX, D
SETRESb_mXYd_R SET, 2, IX, E
SETRESb_mXYd_R SET, 2, IX, H
SETRESb_mXYd_R SET, 2, IX, L

SETRESb_mXYd_R SET, 3, IX
SETRESb_mXYd_R SET, 3, IX, A
SETRESb_mXYd_R SET, 3, IX, B
SETRESb_mXYd_R SET, 3, IX, C
SETRESb_mXYd_R SET, 3, IX, D
SETRESb_mXYd_R SET, 3, IX, E
SETRESb_mXYd_R SET, 3, IX, H
SETRESb_mXYd_R SET, 3, IX, L

SETRESb_mXYd_R SET, 4, IX
SETRESb_mXYd_R SET, 4, IX, A
SETRESb_mXYd_R SET, 4, IX, B
SETRESb_mXYd_R SET, 4, IX, C
SETRESb_mXYd_R SET, 4, IX, D
SETRESb_mXYd_R SET, 4, IX, E
SETRESb_mXYd_R SET, 4, IX, H
SETRESb_mXYd_R SET, 4, IX, L

SETRESb_mXYd_R SET, 5, IX
SETRESb_mXYd_R SET, 5, IX, A
SETRESb_mXYd_R SET, 5, IX, B
SETRESb_mXYd_R SET, 5, IX, C
SETRESb_mXYd_R SET, 5, IX, D
SETRESb_mXYd_R SET, 5, IX, E
SETRESb_mXYd_R SET, 5, IX, H
SETRESb_mXYd_R SET, 5, IX, L

SETRESb_mXYd_R SET, 6, IX
SETRESb_mXYd_R SET, 6, IX, A
SETRESb_mXYd_R SET, 6, IX, B
SETRESb_mXYd_R SET, 6, IX, C
SETRESb_mXYd_R SET, 6, IX, D
SETRESb_mXYd_R SET, 6, IX, E
SETRESb_mXYd_R SET, 6, IX, H
SETRESb_mXYd_R SET, 6, IX, L

SETRESb_mXYd_R SET, 7, IX
SETRESb_mXYd_R SET, 7, IX, A
SETRESb_mXYd_R SET, 7, IX, B
SETRESb_mXYd_R SET, 7, IX, C
SETRESb_mXYd_R SET, 7, IX, D
SETRESb_mXYd_R SET, 7, IX, E
SETRESb_mXYd_R SET, 7, IX, H
SETRESb_mXYd_R SET, 7, IX, L

SETRESb_mXYd_R RES, 0, IX
SETRESb_mXYd_R RES, 0, IX, A
SETRESb_mXYd_R RES, 0, IX, B
SETRESb_mXYd_R RES, 0, IX, C
SETRESb_mXYd_R RES, 0, IX, D
SETRESb_mXYd_R RES, 0, IX, E
SETRESb_mXYd_R RES, 0, IX, H
SETRESb_mXYd_R RES, 0, IX, L

SETRESb_mXYd_R RES, 1, IX
SETRESb_mXYd_R RES, 1, IX, A
SETRESb_mXYd_R RES, 1, IX, B
SETRESb_mXYd_R RES, 1, IX, C
SETRESb_mXYd_R RES, 1, IX, D
SETRESb_mXYd_R RES, 1, IX, E
SETRESb_mXYd_R RES, 1, IX, H
SETRESb_mXYd_R RES, 1, IX, L

SETRESb_mXYd_R RES, 2, IX
SETRESb_mXYd_R RES, 2, IX, A
SETRESb_mXYd_R RES, 2, IX, B
SETRESb_mXYd_R RES, 2, IX, C
SETRESb_mXYd_R RES, 2, IX, D
SETRESb_mXYd_R RES, 2, IX, E
SETRESb_mXYd_R RES, 2, IX, H
SETRESb_mXYd_R RES, 2, IX, L

SETRESb_mXYd_R RES, 3, IX
SETRESb_mXYd_R RES, 3, IX, A
SETRESb_mXYd_R RES, 3, IX, B
SETRESb_mXYd_R RES, 3, IX, C
SETRESb_mXYd_R RES, 3, IX, D
SETRESb_mXYd_R RES, 3, IX, E
SETRESb_mXYd_R RES, 3, IX, H
SETRESb_mXYd_R RES, 3, IX, L

SETRESb_mXYd_R RES, 4, IX
SETRESb_mXYd_R RES, 4, IX, A
SETRESb_mXYd_R RES, 4, IX, B
SETRESb_mXYd_R RES, 4, IX, C
SETRESb_mXYd_R RES, 4, IX, D
SETRESb_mXYd_R RES, 4, IX, E
SETRESb_mXYd_R RES, 4, IX, H
SETRESb_mXYd_R RES, 4, IX, L

SETRESb_mXYd_R RES, 5, IX
SETRESb_mXYd_R RES, 5, IX, A
SETRESb_mXYd_R RES, 5, IX, B
SETRESb_mXYd_R RES, 5, IX, C
SETRESb_mXYd_R RES, 5, IX, D
SETRESb_mXYd_R RES, 5, IX, E
SETRESb_mXYd_R RES, 5, IX, H
SETRESb_mXYd_R RES, 5, IX, L

SETRESb_mXYd_R RES, 6, IX
SETRESb_mXYd_R RES, 6, IX, A
SETRESb_mXYd_R RES, 6, IX, B
SETRESb_mXYd_R RES, 6, IX, C
SETRESb_mXYd_R RES, 6, IX, D
SETRESb_mXYd_R RES, 6, IX, E
SETRESb_mXYd_R RES, 6, IX, H
SETRESb_mXYd_R RES, 6, IX, L

SETRESb_mXYd_R RES, 7, IX
SETRESb_mXYd_R RES, 7, IX, A
SETRESb_mXYd_R RES, 7, IX, B
SETRESb_mXYd_R RES, 7, IX, C
SETRESb_mXYd_R RES, 7, IX, D
SETRESb_mXYd_R RES, 7, IX, E
SETRESb_mXYd_R RES, 7, IX, H
SETRESb_mXYd_R RES, 7, IX, L

SETRESb_mXYd_R SET, 0, IY
SETRESb_mXYd_R SET, 0, IY, A
SETRESb_mXYd_R SET, 0, IY, B
SETRESb_mXYd_R SET, 0, IY, C
SETRESb_mXYd_R SET, 0, IY, D
SETRESb_mXYd_R SET, 0, IY, E
SETRESb_mXYd_R SET, 0, IY, H
SETRESb_mXYd_R SET, 0, IY, L

SETRESb_mXYd_R SET, 1, IY
SETRESb_mXYd_R SET, 1, IY, A
SETRESb_mXYd_R SET, 1, IY, B
SETRESb_mXYd_R SET, 1, IY, C
SETRESb_mXYd_R SET, 1, IY, D
SETRESb_mXYd_R SET, 1, IY, E
SETRESb_mXYd_R SET, 1, IY, H
SETRESb_mXYd_R SET, 1, IY, L

SETRESb_mXYd_R SET, 2, IY
SETRESb_mXYd_R SET, 2, IY, A
SETRESb_mXYd_R SET, 2, IY, B
SETRESb_mXYd_R SET, 2, IY, C
SETRESb_mXYd_R SET, 2, IY, D
SETRESb_mXYd_R SET, 2, IY, E
SETRESb_mXYd_R SET, 2, IY, H
SETRESb_mXYd_R SET, 2, IY, L

SETRESb_mXYd_R SET, 3, IY
SETRESb_mXYd_R SET, 3, IY, A
SETRESb_mXYd_R SET, 3, IY, B
SETRESb_mXYd_R SET, 3, IY, C
SETRESb_mXYd_R SET, 3, IY, D
SETRESb_mXYd_R SET, 3, IY, E
SETRESb_mXYd_R SET, 3, IY, H
SETRESb_mXYd_R SET, 3, IY, L

SETRESb_mXYd_R SET, 4, IY
SETRESb_mXYd_R SET, 4, IY, A
SETRESb_mXYd_R SET, 4, IY, B
SETRESb_mXYd_R SET, 4, IY, C
SETRESb_mXYd_R SET, 4, IY, D
SETRESb_mXYd_R SET, 4, IY, E
SETRESb_mXYd_R SET, 4, IY, H
SETRESb_mXYd_R SET, 4, IY, L

SETRESb_mXYd_R SET, 5, IY
SETRESb_mXYd_R SET, 5, IY, A
SETRESb_mXYd_R SET, 5, IY, B
SETRESb_mXYd_R SET, 5, IY, C
SETRESb_mXYd_R SET, 5, IY, D
SETRESb_mXYd_R SET, 5, IY, E
SETRESb_mXYd_R SET, 5, IY, H
SETRESb_mXYd_R SET, 5, IY, L

SETRESb_mXYd_R SET, 6, IY
SETRESb_mXYd_R SET, 6, IY, A
SETRESb_mXYd_R SET, 6, IY, B
SETRESb_mXYd_R SET, 6, IY, C
SETRESb_mXYd_R SET, 6, IY, D
SETRESb_mXYd_R SET, 6, IY, E
SETRESb_mXYd_R SET, 6, IY, H
SETRESb_mXYd_R SET, 6, IY, L

SETRESb_mXYd_R SET, 7, IY
SETRESb_mXYd_R SET, 7, IY, A
SETRESb_mXYd_R SET, 7, IY, B
SETRESb_mXYd_R SET, 7, IY, C
SETRESb_mXYd_R SET, 7, IY, D
SETRESb_mXYd_R SET, 7, IY, E
SETRESb_mXYd_R SET, 7, IY, H
SETRESb_mXYd_R SET, 7, IY, L

SETRESb_mXYd_R RES, 0, IY
SETRESb_mXYd_R RES, 0, IY, A
SETRESb_mXYd_R RES, 0, IY, B
SETRESb_mXYd_R RES, 0, IY, C
SETRESb_mXYd_R RES, 0, IY, D
SETRESb_mXYd_R RES, 0, IY, E
SETRESb_mXYd_R RES, 0, IY, H
SETRESb_mXYd_R RES, 0, IY, L

SETRESb_mXYd_R RES, 1, IY
SETRESb_mXYd_R RES, 1, IY, A
SETRESb_mXYd_R RES, 1, IY, B
SETRESb_mXYd_R RES, 1, IY, C
SETRESb_mXYd_R RES, 1, IY, D
SETRESb_mXYd_R RES, 1, IY, E
SETRESb_mXYd_R RES, 1, IY, H
SETRESb_mXYd_R RES, 1, IY, L

SETRESb_mXYd_R RES, 2, IY
SETRESb_mXYd_R RES, 2, IY, A
SETRESb_mXYd_R RES, 2, IY, B
SETRESb_mXYd_R RES, 2, IY, C
SETRESb_mXYd_R RES, 2, IY, D
SETRESb_mXYd_R RES, 2, IY, E
SETRESb_mXYd_R RES, 2, IY, H
SETRESb_mXYd_R RES, 2, IY, L

SETRESb_mXYd_R RES, 3, IY
SETRESb_mXYd_R RES, 3, IY, A
SETRESb_mXYd_R RES, 3, IY, B
SETRESb_mXYd_R RES, 3, IY, C
SETRESb_mXYd_R RES, 3, IY, D
SETRESb_mXYd_R RES, 3, IY, E
SETRESb_mXYd_R RES, 3, IY, H
SETRESb_mXYd_R RES, 3, IY, L

SETRESb_mXYd_R RES, 4, IY
SETRESb_mXYd_R RES, 4, IY, A
SETRESb_mXYd_R RES, 4, IY, B
SETRESb_mXYd_R RES, 4, IY, C
SETRESb_mXYd_R RES, 4, IY, D
SETRESb_mXYd_R RES, 4, IY, E
SETRESb_mXYd_R RES, 4, IY, H
SETRESb_mXYd_R RES, 4, IY, L

SETRESb_mXYd_R RES, 5, IY
SETRESb_mXYd_R RES, 5, IY, A
SETRESb_mXYd_R RES, 5, IY, B
SETRESb_mXYd_R RES, 5, IY, C
SETRESb_mXYd_R RES, 5, IY, D
SETRESb_mXYd_R RES, 5, IY, E
SETRESb_mXYd_R RES, 5, IY, H
SETRESb_mXYd_R RES, 5, IY, L

SETRESb_mXYd_R RES, 6, IY
SETRESb_mXYd_R RES, 6, IY, A
SETRESb_mXYd_R RES, 6, IY, B
SETRESb_mXYd_R RES, 6, IY, C
SETRESb_mXYd_R RES, 6, IY, D
SETRESb_mXYd_R RES, 6, IY, E
SETRESb_mXYd_R RES, 6, IY, H
SETRESb_mXYd_R RES, 6, IY, L

SETRESb_mXYd_R RES, 7, IY
SETRESb_mXYd_R RES, 7, IY, A
SETRESb_mXYd_R RES, 7, IY, B
SETRESb_mXYd_R RES, 7, IY, C
SETRESb_mXYd_R RES, 7, IY, D
SETRESb_mXYd_R RES, 7, IY, E
SETRESb_mXYd_R RES, 7, IY, H
SETRESb_mXYd_R RES, 7, IY, L


; Jump instruction
; ----------------


align 16

Z80I_JP_NN:
	movzx	edx, byte [zxPC + 2]
	movzx	zxPC, byte [zxPC + 1]
	shl	edx, 8
	or	zxPC, edx
	REBASE_PC
	NEXT 10


; JPcc_NN				if (cc) PC <- NN

%macro JPcc_NN 1-2

align 16

Z80I_JP%2%1_NN:
	test	zF, FLAG_%1
	j%2z	short %%dont_take_it

	movzx	edx, byte [zxPC + 2]
	movzx	zxPC, byte [zxPC + 1]
	shl	edx, 8
	or	zxPC, edx
	REBASE_PC
	NEXT 10

align 16

%%dont_take_it
	add	zxPC, 3
	NEXT 10

%endmacro


JPcc_NN Z
JPcc_NN Z, N
JPcc_NN C
JPcc_NN C, N
JPcc_NN P
JPcc_NN P, N
JPcc_NN S
JPcc_NN S, N


align 16

Z80I_JR_N:
	movsx	edx, byte [zxPC + 1]
	add	zxPC, 2
	add	zxPC, edx
	NEXT 12


; JRcc_N				if (cc) PC <- PC + N

%macro JRcc_N 1-2

align 16

Z80I_JR%2%1_N:
	test	zF, FLAG_%1
	j%2z	short %%dont_take_it

	movsx	edx, byte [zxPC + 1]
	add	zxPC, 2
	add	zxPC, edx
	NEXT 12

align 16

%%dont_take_it
	add	zxPC, 2
	NEXT 7

%endmacro


JRcc_N Z
JRcc_N Z, N
JRcc_N C
JRcc_N C, N


; JP_RR				PC <- RR

%macro JP_RR 1

align 16

Z80I_JP_%1:
	mov zxPC, zx%1
	REBASE_PC
	NEXT 4

%endmacro


JP_RR HL
JP_RR IX
JP_RR IY


align 16

Z80I_DJNZ:
	mov	dl, zB
	movsx	ecx, byte [zxPC + 1]
	dec	dl
	jz	short .dont_take_it
	
	add zxPC, byte 2
	mov	zB, dl
	add	zxPC, ecx
	NEXT 13

align 16

.dont_take_it:
	add	zxPC, byte 2
	mov	zB, dl
	NEXT 10


; Call/Return instruction
; -----------------------


; CALLcc_NN				if (cc) CALL NN

%macro CALLcc_NN 0-2

align 16

Z80I_CALL%2%1_NN:
%if %0 > 0
	test	zF, FLAG_%1
%endif
	mov	ecx, zxSP
%if %0 > 0
	j%2z	near %%dont_take_it
%endif

	sub	ecx, byte 2
	lea	edx, [zxPC + 3]
	and	ecx, 0xFFFF
	sub	edx, [ebp + Z80.BasePC]
	mov	zxSP, ecx
	WRITE_WORD
	movzx	edx, byte [zxPC + 2]
	movzx	zxPC, byte [zxPC + 1]
	shl	edx, 8
	or	zxPC, edx
	REBASE_PC
	NEXT 17

%if %0 > 0
align 16

%%dont_take_it
	add	zxPC, 3
	NEXT 10
%endif

%endmacro


CALLcc_NN
CALLcc_NN Z
CALLcc_NN Z, N
CALLcc_NN C
CALLcc_NN C, N
CALLcc_NN P
CALLcc_NN P, N
CALLcc_NN S
CALLcc_NN S, N


; RETcc					if (cc) RET

%macro RETcc 0-2

align 16

Z80I_RET%2%1:
%if %0 > 0
	test	zF, FLAG_%1
%endif
	mov	ecx, zxSP
%if %0 > 0
	j%2z short %%dont_take_it
%endif

	READ_WORD
	mov	ecx, zxSP
	movzx	zxPC, dx
	add	ecx, byte 2
	and	ecx, 0xFFFF
	mov	zxSP, ecx
	REBASE_PC
%if %0 > 0
	NEXT 17
%else
	NEXT 10
%endif

%if %0 > 0
align 16

%%dont_take_it
	inc zxPC
	NEXT 5
%endif

%endmacro


RETcc
RETcc Z
RETcc Z, N
RETcc C
RETcc C, N
RETcc P
RETcc P, N
RETcc S
RETcc S, N


align 16

Z80I_RETI:
Z80I_RETN:
	mov	ecx, zxSP
	READ_WORD
	mov	ecx, zxSP
	movzx	zxPC, dx
	add	ecx, byte 2
	mov	dl, zIFF2
	and	ecx, 0xFFFF
	mov	zIFF1, dl
	mov	zxSP, ecx
	REBASE_PC
	NEXT 14


align 16

Z80I_RST:
	mov	ecx, zxSP
	lea	edx, [zxPC + 1]
	sub	ecx, byte 2
	movzx	zxPC, byte [zxPC]
	and	ecx, 0xFFFF
	sub	edx, [ebp + Z80.BasePC]
	and	zxPC, 0x38
	mov	zxSP, ecx
	WRITE_WORD
	REBASE_PC
	NEXT 11


; Input/Output instruction
; ------------------------


align 16

Z80I_IN_mN:
	movzx	edx, zA
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 2
	or	ecx, edx
	DO_IN A
	NEXT 11


; IN_R_mBC					R8 <- (BC)

%macro IN_R_mBC 0-1

align 16

%if %0 > 0
Z80I_IN_%1_mBC:
%else
Z80I_IN_F_mBC:
%endif
	mov	ecx, zxBC
	add	zxPC, byte 2
	DO_IN
	and	zF, FLAG_C
	test	dl, dl
	mov	dh, zF
%if %0 > 0
	mov	z%1, dl
%endif
	lahf
	and	zF, FLAG_S | FLAG_Z | FLAG_P
	mov	zFXY, dl
	or	zF, dh
	NEXT 12

%endmacro


IN_R_mBC
IN_R_mBC A
IN_R_mBC B
IN_R_mBC C
IN_R_mBC D
IN_R_mBC E
IN_R_mBC H
IN_R_mBC L


; INX I/D				(HL++) <- (C), B--

%macro INX 1

align 16

Z80I_IN%1:
	movzx	ecx, zC
	add	zxPC, byte 2
	DO_IN
	mov	ecx, zxHL
	mov	zF, dl			; all flags are modified anyway
%ifidn %1, I
	inc	zxHL
%else
	dec	zxHL
%endif
	WRITE_BYTE
	mov	dh, zC
	and	zxHL, 0xFFFF
	mov	dl, zB
	inc	dh
	dec	dl
	mov	cl, zF
	lahf
	and	zF, FLAG_S | FLAG_Z | FLAG_P		; FLAG_P is weird here
	add	dh, cl
	mov	zB, dl
	jnc	short .no_carry

	or	zF, FLAG_H | FLAG_C

.no_carry:
	shr	cl, 7
	add	cl, cl
	or	zF, cl
	NEXT 16

%endmacro


INX I
INX D


; INXR I/D				do { (HL++) <- (C) } while(B--)

%macro INXR 1

align 16

Z80I_IN%1R:
%%Loop
	movzx	ecx, zC
	DO_IN
	mov	ecx, zxHL
	mov	zF, dl
	WRITE_BYTE
%ifidn %1, I
	inc	zxHL
%else
	dec	zxHL
%endif
	mov	dl, zB
	and	zxHL, 0xFFFF
	dec	dl
	jz	short %%End
	
	sub	edi, byte 21
	mov	zB, dl
	jns	short %%Loop
	
	mov	cl, zF
	mov	dh, zC
	mov	zF, dl
	inc	dh
	and	zF, FLAG_S
	add	dh, cl
	mov	zFXY, dl
	jnc	short %%no_carry1
	
	or	zF, FLAG_H | FLAG_C

%%no_carry1
	shr	cl, 7
	add	cl, cl
	or	zF, cl
	jmp	z80_Exec_Really_Quit

align 16

%%End
	mov	cl, zF
	mov	dh, zC
	mov	zF, FLAG_Z | FLAG_P
	inc	dh
	mov	zFXY, 0
	add	dh, cl
	jnc	short %%no_carry2
	
	or	zF, FLAG_H | FLAG_C

%%no_carry2
	shr	cl, 7
	add	zxPC, byte 2
	add	cl, cl
	mov	zB, dl
	or	zF, cl
	NEXT 16

%endmacro


INXR I
INXR D


align 16

Z80I_OUT_mN:
	movzx	edx, zA
	movzx	ecx, byte [zxPC + 1]
	shl	edx, 8
	add	zxPC, byte 2
	or	ecx, edx
	mov	dl, zA
	DO_OUT
	NEXT 11


; OUT_mBC_R				(BC) <- R8

%macro OUT_mBC_R 0-1

align 16

%if %0 > 0
Z80I_OUT_mBC_%1:
%else
Z80I_OUT_mBC_0:
%endif
	mov ecx, zxBC
	add zxPC, byte 2
%if %0 > 0
	mov dl, z%1
%else
	xor dl, dl
%endif
	DO_OUT
	NEXT 12

%endmacro


OUT_mBC_R
OUT_mBC_R A
OUT_mBC_R B
OUT_mBC_R C
OUT_mBC_R D
OUT_mBC_R E
OUT_mBC_R H
OUT_mBC_R L


; OUTX I/D				(C) <- (HL++), B--

%macro OUTX 1

align 16

Z80I_OUT%1:
	mov	ecx, zxHL
	add	zxPC, byte 2
	READ_BYTE
	movzx	ecx, zC
	mov	zF, dl			; all flags are modified anyway
%ifidn %1, I
	inc	zxHL
%else
	dec	zxHL
%endif
	DO_OUT
	mov	dl, zB
	and	zxHL, 0xFFFF
	dec	dl
	mov	cl, zF
	lahf
	mov	dh, zL
	and	zF, FLAG_S | FLAG_Z | FLAG_P		; FLAG_P is weird here
	add	dh, cl
	mov	zB, dl
	jnc	short .no_carry
	
	or	zF, FLAG_H | FLAG_C

.no_carry:
	shr	cl, 7
	add	cl, cl
	or	zF, cl
	NEXT 16

%endmacro


OUTX I
OUTX D


; OUTXR I/D				do { (HL++) <- (C) } while(B--)

%macro OUTXR 1

align 16

Z80I_OT%1R:
%%Loop
	mov	ecx, zxHL
	READ_BYTE
	movzx	ecx, zC
	mov	zF, dl			; all flags are modified anyway
%ifidn %1, I
	inc	zxHL
%else
	dec	zxHL
%endif
	DO_OUT
	mov	dl, zB
	and	zxHL, 0xFFFF
	dec	dl
	jz	short %%End
	
	sub	edi, byte 21
	mov	zB, dl
	jns	short %%Loop
	
	mov	cl, zF
	mov	zF, dl
	mov	dh, zL
	and	zF, FLAG_S
	add	dh, cl
	mov	zFXY, dl
	jnc	short %%no_carry1
	
	or	zF, FLAG_H | FLAG_C

%%no_carry1
	shr	cl, 7
	add	cl, cl
	or	zF, cl
	jmp	z80_Exec_Really_Quit

align 16

%%End
	mov	cl, zF
	mov	dh, zL
	mov	zF, FLAG_Z | FLAG_P
	add	dh, cl
	mov	zFXY, 0
	jnc	short %%no_carry2
	
	or	zF, FLAG_H | FLAG_C

%%no_carry2
	shr	cl, 7
	add	zxPC, byte 2
	add	cl, cl
	mov	zB, dl
	or	zF, cl
	NEXT 16

%endmacro


OUTXR I
OUTXR D


align 16

PREFIXE_CB:
	movzx	edx, byte [zxPC + 1]
	jmp	[CB_Table + edx * 4]


align 16

PREFIXE_ED:
	movzx	edx, byte [zxPC + 1]
	jmp	[ED_Table + edx * 4]


align 16

PREFIXE_DD:
	movzx	edx, byte [zxPC + 1]
	sub	edi, 4
	inc	zxPC
	jmp	[DD_Table + edx * 4]


align 16

PREFIXE_DDCB:
	movzx	edx, byte [zxPC + 2]
	jmp	[DDCB_Table + edx * 4]


align 16

PREFIXE_FD:
	movzx	edx, byte [zxPC + 1]
	sub	edi, 4
	inc	zxPC
	jmp	[FD_Table + edx * 4]


align 16

PREFIXE_FDCB:
	movzx	edx, byte [zxPC + 2]
	jmp	[FDCB_Table + edx * 4]



;*******************
;
; Publics functions
;
;*******************


; External symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_mdZ80_def_ReadB	mdZ80_def_ReadB
	%define	_mdZ80_def_In		mdZ80_def_In
	%define	_mdZ80_def_ReadW	mdZ80_def_ReadW
	%define	_mdZ80_def_WriteB	mdZ80_def_WriteB
	%define	_mdZ80_def_Out		mdZ80_def_Out
	%define	_mdZ80_def_WriteW	mdZ80_def_WriteW
%endif

; Fastcall symbol redefines for Win32.
%ifdef __OBJ_WIN32
	%define	_mdZ80_def_ReadB	@mdZ80_def_ReadB@4
	%define	_mdZ80_def_In		@mdZ80_def_In@4
	%define	_mdZ80_def_ReadW	@mdZ80_def_ReadW@4
	%define	_mdZ80_def_WriteB	@mdZ80_def_WriteB@8
	%define	_mdZ80_def_Out		@mdZ80_def_Out@8
	%define	_mdZ80_def_WriteW	@mdZ80_def_WriteW@8
%endif

align 16

global _mdZ80_def_ReadB
global _mdZ80_def_In
_mdZ80_def_ReadB:
_mdZ80_def_In:
	mov al, [_mdZ80_def_mem + ecx]
	ret


align 16

global _mdZ80_def_ReadW
_mdZ80_def_ReadW:
	mov ax, [_mdZ80_def_mem + ecx]
	ret


align 16

global _mdZ80_def_WriteB
global _mdZ80_def_Out
_mdZ80_def_WriteB:
_mdZ80_def_Out:
	mov [_mdZ80_def_mem + ecx], dl
	ret

align 16

global _mdZ80_def_WriteW
_mdZ80_def_WriteW:
	mov [_mdZ80_def_mem + ecx], dx
	ret


%macro ADD_HANDLER 1

align 16

DECLF z80_Add_%1
	
	push	ecx
	push	edx
	push	ebp
	
	mov ebp, [esp + 16]		; context
	mov ecx, [esp + 20]		; low adr
	mov edx, [esp + 24]		; high adr
	mov eax, [esp + 28]		; *func
	
	and	ecx, 0xFF
	and	edx, 0xFF
	
%ifidn %1, Fetch
	shl	ecx, 8
	sub	eax, ecx
	shr	ecx, 8
%endif
	
	cmp	ecx, edx
	ja	short %%end
	
%%Loop
	mov	[ebp + Z80.%1 + ecx * 4], eax
	inc	ecx
	cmp	ecx, edx
	jbe	short %%Loop
	
%%end
	pop	ebp
	pop	edx
	pop	ecx
	ret

%endmacro


; UINT32 z80_Add_ReadB(Z80_CONTEXT *z80, UINT32 low_adr, UINT32 high_adr, Z80_RB *Func)
;
; RETURN: 0

	ADD_HANDLER ReadB


; UINT32 z80_Add_ReadW(Z80_CONTEXT *z80, UINT32 low_adr, UINT32 high_adr, Z80_RW *Func)
;
; RETURN: 0

	ADD_HANDLER ReadW


; UINT32 z80_Add_WriteB(Z80_CONTEXT *z80, UINT32 low_adr, UINT32 high_adr, Z80_WB *Func)
;
; RETURN: 0

	ADD_HANDLER WriteB


; UINT32 z80_Add_WriteW(Z80_CONTEXT *z80, UINT32 low_adr, UINT32 high_adr, Z80_WW *Func)
;
; RETURN: 0

	ADD_HANDLER WriteW


; UINT32 z80_Add_Fetch(Z80_CONTEXT *z80, UINT32 low_adr, UINT32 high_adr, UINT8 *Region)
;
; RETURN: 0

	ADD_HANDLER Fetch


align 16

; UINT32 FASTCALL z80_Exec(Z80_CONTEXT *z80, UINT32 odo)
; ecx = context pointer
; edx = odometer to raise
;
; RETURN:
; 0  -> ok
; !0 -> error (status returned) or no cycle to do (-1)

DECLF z80_Exec, 8
	sub	edx, [ecx + Z80.CycleCnt]
	jbe	near z80_Cycles_Already_done
	
	push	ebx
	push	edi
	push	esi
	push	ebp
	
	mov	zxPC, [ecx + Z80.PC]
	mov	edi, edx
	mov	ebp, ecx
	dec	edi
	mov	zAF, [ebp + Z80.AF]			; need to be here
	
	CHECK_INT
	
	mov	edx, [ebp + Z80.Status]
	xor	ecx, ecx
	test	edx, Z80_HALTED | Z80_FAULTED | Z80_RUNNING
	jnz	near z80_Cannot_Run
	
	or	edx, Z80_RUNNING
	mov	[ebp + Z80.CycleSup], ecx
	mov	[ebp + Z80.Status], edx
	mov	[ebp + Z80.CycleTD], edi
	movzx	edx, byte [zxPC]
	mov	zxHL, [ebp + Z80.HL]
	
%if (GENS_LOG == 1)
	push	eax
	push	ecx
	push	edx
	mov	ecx, zxPC
	sub	ecx, [ebp + Z80.BasePC]
	call	@z80log@4
	pop	edx
	pop	ecx
	pop	eax
%endif
	
	jmp	dword [OP_Table + edx * 4]

align 16

z80_Exec_Quit:
	mov	edx, [ebp + Z80.CycleSup]
	xor	ecx, ecx
	add	edi, edx
	mov	[ebp + Z80.CycleSup], ecx
	jns	short z80_Exec_Interrupt_Happened

z80_Exec_Really_Quit:
	mov	[ebp + Z80.AF], zAF
	mov	[ebp + Z80.HL], zxHL
	mov	eax, [ebp + Z80.CycleTD]
	xor	ecx, ecx
	mov	ebx, [ebp + Z80.CycleCnt]
	mov	cl, [ebp + Z80.Status]
	sub	eax, edi
%if (Z80_SAFE = 1)
	mov	edx, [ebp + Z80.BasePC]
%endif
	mov	[ebp + Z80.PC], esi
	and	cl, ~Z80_RUNNING
%if (Z80_SAFE = 1)
	sub	esi, edx
%endif
	add	eax, ebx
%if (Z80_SAFE = 1)
	test	esi, 0xFFFF0000
	jz	short .OK
	
	or	cl, Z80_FAULTED
	jmp	short .OK

align 16

.OK:
%endif
	mov	[ebp + Z80.CycleCnt], eax
	mov	[ebp + Z80.Status], cl
	xor	eax, eax

z80_Cannot_Run:
	test	edx, Z80_HALTED
	mov	ecx, [ebp + Z80.CycleCnt]
	jz	short .not_halted
	
	add	ecx, edi

.not_halted:
	mov	[ebp + Z80.CycleCnt], ecx
	pop	ebp
	pop	esi
	pop	edi
	pop	ebx
	ret

align 16

z80_Cycles_Already_done:
	or eax, byte -1
	ret

align 16

z80_Exec_Interrupt_Happened:
	CHECK_INT
	movzx	edx, byte [zxPC]

%if (GENS_LOG == 1)
	push	eax
	push	ecx
	push	edx
	mov	ecx, zxPC
	sub	ecx, [ebp + Z80.BasePC]
	call	@z80log@4
	pop	edx
	pop	ecx
	pop	eax
%endif
	
	jmp	dword [OP_Table + edx * 4]


;*********************
;
; Tables declaration 
;
;*********************


section .rodata align=64
	
	OP_Table:
		dd	Z80I_NOP, Z80I_LD_BC_NN, Z80I_LD_mBC_A, Z80I_INC_BC		; 00-03
		dd	Z80I_INC_B, Z80I_DEC_B, Z80I_LD_B_N, Z80I_RLCA			; 04-07
		dd	Z80I_EX_AF_AF2, Z80I_ADD_HL_BC, Z80I_LD_A_mBC, Z80I_DEC_BC	; 08-0B
		dd	Z80I_INC_C, Z80I_DEC_C, Z80I_LD_C_N, Z80I_RRCA			; 0C-0F
		dd	Z80I_DJNZ, Z80I_LD_DE_NN, Z80I_LD_mDE_A, Z80I_INC_DE		; 10-13
		dd	Z80I_INC_D, Z80I_DEC_D, Z80I_LD_D_N, Z80I_RLA			; 14-17
		dd	Z80I_JR_N, Z80I_ADD_HL_DE, Z80I_LD_A_mDE, Z80I_DEC_DE		; 18-1B
		dd	Z80I_INC_E, Z80I_DEC_E, Z80I_LD_E_N, Z80I_RRA			; 1C-1F
		dd	Z80I_JRNZ_N, Z80I_LD_HL_NN, Z80I_LD_mNN_HL, Z80I_INC_HL		; 20-23
		dd	Z80I_INC_H, Z80I_DEC_H, Z80I_LD_H_N, Z80I_DAA			; 24-27
		dd	Z80I_JRZ_N, Z80I_ADD_HL_HL, Z80I_LD_HL_mNN, Z80I_DEC_HL		; 28-2B
		dd	Z80I_INC_L, Z80I_DEC_L, Z80I_LD_L_N, Z80I_CPL			; 2C-2F
		dd	Z80I_JRNC_N, Z80I_LD_SP_NN, Z80I_LD_mNN_A, Z80I_INC_SP		; 30-33
		dd	Z80I_INC_mHL, Z80I_DEC_mHL, Z80I_LD_mHL_N, Z80I_SCF		; 34-37
		dd	Z80I_JRC_N, Z80I_ADD_HL_SP, Z80I_LD_A_mNN, Z80I_DEC_SP		; 38-3B
		dd	Z80I_INC_A, Z80I_DEC_A, Z80I_LD_A_N, Z80I_CCF			; 3C-3F
		dd	Z80I_LD_B_B, Z80I_LD_B_C, Z80I_LD_B_D, Z80I_LD_B_E		; 40-43
		dd	Z80I_LD_B_H, Z80I_LD_B_L, Z80I_LD_B_mHL, Z80I_LD_B_A		; 44-47
		dd	Z80I_LD_C_B, Z80I_LD_C_C, Z80I_LD_C_D, Z80I_LD_C_E		; 48-4B
		dd	Z80I_LD_C_H, Z80I_LD_C_L, Z80I_LD_C_mHL, Z80I_LD_C_A		; 4C-4F
		dd	Z80I_LD_D_B, Z80I_LD_D_C, Z80I_LD_D_D, Z80I_LD_D_E		; 50-53
		dd	Z80I_LD_D_H, Z80I_LD_D_L, Z80I_LD_D_mHL, Z80I_LD_D_A		; 54-57
		dd	Z80I_LD_E_B, Z80I_LD_E_C, Z80I_LD_E_D, Z80I_LD_E_E		; 58-5B
		dd	Z80I_LD_E_H, Z80I_LD_E_L, Z80I_LD_E_mHL, Z80I_LD_E_A		; 5C-5F
		dd	Z80I_LD_H_B, Z80I_LD_H_C, Z80I_LD_H_D, Z80I_LD_H_E		; 60-63
		dd	Z80I_LD_H_H, Z80I_LD_H_L, Z80I_LD_H_mHL, Z80I_LD_H_A		; 64-67
		dd	Z80I_LD_L_B, Z80I_LD_L_C, Z80I_LD_L_D, Z80I_LD_L_E		; 68-6B
		dd	Z80I_LD_L_H, Z80I_LD_L_L, Z80I_LD_L_mHL, Z80I_LD_L_A		; 6C-6F
		dd	Z80I_LD_mHL_B, Z80I_LD_mHL_C, Z80I_LD_mHL_D, Z80I_LD_mHL_E	; 70-73
		dd	Z80I_LD_mHL_H, Z80I_LD_mHL_L, Z80I_HALT, Z80I_LD_mHL_A		; 74-77
		dd	Z80I_LD_A_B, Z80I_LD_A_C, Z80I_LD_A_D, Z80I_LD_A_E		; 78-7B
		dd	Z80I_LD_A_H, Z80I_LD_A_L, Z80I_LD_A_mHL, Z80I_LD_A_A		; 7C-7F
		dd	Z80I_ADD_B, Z80I_ADD_C, Z80I_ADD_D, Z80I_ADD_E			; 80-83
		dd	Z80I_ADD_H, Z80I_ADD_L, Z80I_ADD_mHL, Z80I_ADD_A		; 84-87
		dd	Z80I_ADC_B, Z80I_ADC_C, Z80I_ADC_D, Z80I_ADC_E			; 88-8B
		dd	Z80I_ADC_H, Z80I_ADC_L, Z80I_ADC_mHL, Z80I_ADC_A		; 8C-8F
		dd	Z80I_SUB_B, Z80I_SUB_C, Z80I_SUB_D, Z80I_SUB_E			; 90-83
		dd	Z80I_SUB_H, Z80I_SUB_L, Z80I_SUB_mHL, Z80I_SUB_A		; 94-87
		dd	Z80I_SBC_B, Z80I_SBC_C, Z80I_SBC_D, Z80I_SBC_E			; 98-8B
		dd	Z80I_SBC_H, Z80I_SBC_L, Z80I_SBC_mHL, Z80I_SBC_A		; 9C-8F
		dd	Z80I_AND_B, Z80I_AND_C, Z80I_AND_D, Z80I_AND_E			; A0-A3
		dd	Z80I_AND_H, Z80I_AND_L, Z80I_AND_mHL, Z80I_AND_A		; A4-A7
		dd	Z80I_XOR_B, Z80I_XOR_C, Z80I_XOR_D, Z80I_XOR_E			; A8-AB
		dd	Z80I_XOR_H, Z80I_XOR_L, Z80I_XOR_mHL, Z80I_XOR_A		; AC-AF
		dd	Z80I_OR_B, Z80I_OR_C, Z80I_OR_D, Z80I_OR_E			; B0-B3
		dd	Z80I_OR_H, Z80I_OR_L, Z80I_OR_mHL, Z80I_OR_A			; B4-B7
		dd	Z80I_CP_B, Z80I_CP_C, Z80I_CP_D, Z80I_CP_E			; B8-BB
		dd	Z80I_CP_H, Z80I_CP_L, Z80I_CP_mHL, Z80I_CP_A			; BC-BF
		dd	Z80I_RETNZ, Z80I_POP_BC, Z80I_JPNZ_NN, Z80I_JP_NN		; C0-C3
		dd	Z80I_CALLNZ_NN, Z80I_PUSH_BC, Z80I_ADD_N, Z80I_RST		; C4-C7
		dd	Z80I_RETZ, Z80I_RET, Z80I_JPZ_NN, PREFIXE_CB			; C8-CB
		dd	Z80I_CALLZ_NN, Z80I_CALL_NN, Z80I_ADC_N, Z80I_RST		; CC-CF
		dd	Z80I_RETNC, Z80I_POP_DE, Z80I_JPNC_NN, Z80I_OUT_mN		; D0-D3
		dd	Z80I_CALLNC_NN, Z80I_PUSH_DE, Z80I_SUB_N, Z80I_RST		; D4-D7
		dd	Z80I_RETC, Z80I_EXX, Z80I_JPC_NN, Z80I_IN_mN			; D8-DB
		dd	Z80I_CALLC_NN, PREFIXE_DD, Z80I_SBC_N, Z80I_RST			; DC-DF
		dd	Z80I_RETNP, Z80I_POP_HL, Z80I_JPNP_NN, Z80I_EX_mSP_HL		; E0-E3
		dd	Z80I_CALLNP_NN, Z80I_PUSH_HL, Z80I_AND_N, Z80I_RST		; E4-E7
		dd	Z80I_RETP, Z80I_JP_HL, Z80I_JPP_NN, Z80I_EX_DE_HL		; E8-EB
		dd	Z80I_CALLP_NN, PREFIXE_ED, Z80I_XOR_N, Z80I_RST			; EC-EF
		dd	Z80I_RETNS, Z80I_POP_AF, Z80I_JPNS_NN, Z80I_DI			; F0-F3
		dd	Z80I_CALLNS_NN, Z80I_PUSH_AF, Z80I_OR_N, Z80I_RST		; F4-F7
		dd	Z80I_RETS, Z80I_LD_SP_HL, Z80I_JPS_NN, Z80I_EI			; F8-FB
		dd	Z80I_CALLS_NN, PREFIXE_FD, Z80I_CP_N, Z80I_RST			; FC-FF
	
	CB_Table:
		dd	Z80I_RLC_B, Z80I_RLC_C, Z80I_RLC_D, Z80I_RLC_E			; 00-03
		dd	Z80I_RLC_H, Z80I_RLC_L, Z80I_RLC_mHL, Z80I_RLC_A		; 04-07
		dd	Z80I_RRC_B, Z80I_RRC_C, Z80I_RRC_D, Z80I_RRC_E			; 08-0B
		dd	Z80I_RRC_H, Z80I_RRC_L, Z80I_RRC_mHL, Z80I_RRC_A		; 0C-0F
		dd	Z80I_RL_B, Z80I_RL_C, Z80I_RL_D, Z80I_RL_E			; 10-13
		dd	Z80I_RL_H, Z80I_RL_L, Z80I_RL_mHL, Z80I_RL_A			; 14-17
		dd	Z80I_RR_B, Z80I_RR_C, Z80I_RR_D, Z80I_RR_E			; 18-1B
		dd	Z80I_RR_H, Z80I_RR_L, Z80I_RR_mHL, Z80I_RR_A			; 1C-1F
		dd	Z80I_SLA_B, Z80I_SLA_C, Z80I_SLA_D, Z80I_SLA_E			; 20-23
		dd	Z80I_SLA_H, Z80I_SLA_L, Z80I_SLA_mHL, Z80I_SLA_A		; 24-27
		dd	Z80I_SRA_B, Z80I_SRA_C, Z80I_SRA_D, Z80I_SRA_E			; 28-2B
		dd	Z80I_SRA_H, Z80I_SRA_L, Z80I_SRA_mHL, Z80I_SRA_A		; 2C-2F
		dd	Z80I_SLL_B, Z80I_SLL_C, Z80I_SLL_D, Z80I_SLL_E			; 30-33
		dd	Z80I_SLL_H, Z80I_SLL_L, Z80I_SLL_mHL, Z80I_SLL_A		; 34-37
		dd	Z80I_SRL_B, Z80I_SRL_C, Z80I_SRL_D, Z80I_SRL_E			; 38-3B
		dd	Z80I_SRL_H, Z80I_SRL_L, Z80I_SRL_mHL, Z80I_SRL_A		; 3C-3F
		dd	Z80I_BIT0_B, Z80I_BIT0_C, Z80I_BIT0_D, Z80I_BIT0_E		; 40-43
		dd	Z80I_BIT0_H, Z80I_BIT0_L, Z80I_BIT0_mHL, Z80I_BIT0_A		; 44-47
		dd	Z80I_BIT1_B, Z80I_BIT1_C, Z80I_BIT1_D, Z80I_BIT1_E		; 48-4B
		dd	Z80I_BIT1_H, Z80I_BIT1_L, Z80I_BIT1_mHL, Z80I_BIT1_A		; 4C-4F
		dd	Z80I_BIT2_B, Z80I_BIT2_C, Z80I_BIT2_D, Z80I_BIT2_E		; 50-53
		dd	Z80I_BIT2_H, Z80I_BIT2_L, Z80I_BIT2_mHL, Z80I_BIT2_A		; 54-57
		dd	Z80I_BIT3_B, Z80I_BIT3_C, Z80I_BIT3_D, Z80I_BIT3_E		; 58-5B
		dd	Z80I_BIT3_H, Z80I_BIT3_L, Z80I_BIT3_mHL, Z80I_BIT3_A		; 5C-5F
		dd	Z80I_BIT4_B, Z80I_BIT4_C, Z80I_BIT4_D, Z80I_BIT4_E		; 60-63
		dd	Z80I_BIT4_H, Z80I_BIT4_L, Z80I_BIT4_mHL, Z80I_BIT4_A		; 64-67
		dd	Z80I_BIT5_B, Z80I_BIT5_C, Z80I_BIT5_D, Z80I_BIT5_E		; 68-6B
		dd	Z80I_BIT5_H, Z80I_BIT5_L, Z80I_BIT5_mHL, Z80I_BIT5_A		; 6C-6F
		dd	Z80I_BIT6_B, Z80I_BIT6_C, Z80I_BIT6_D, Z80I_BIT6_E		; 70-73
		dd	Z80I_BIT6_H, Z80I_BIT6_L, Z80I_BIT6_mHL, Z80I_BIT6_A		; 74-77
		dd	Z80I_BIT7_B, Z80I_BIT7_C, Z80I_BIT7_D, Z80I_BIT7_E		; 78-7B
		dd	Z80I_BIT7_H, Z80I_BIT7_L, Z80I_BIT7_mHL, Z80I_BIT7_A		; 7C-7F
		dd	Z80I_RES0_B, Z80I_RES0_C, Z80I_RES0_D, Z80I_RES0_E		; 80-83
		dd	Z80I_RES0_H, Z80I_RES0_L, Z80I_RES0_mHL, Z80I_RES0_A		; 84-87
		dd	Z80I_RES1_B, Z80I_RES1_C, Z80I_RES1_D, Z80I_RES1_E		; 88-8B
		dd	Z80I_RES1_H, Z80I_RES1_L, Z80I_RES1_mHL, Z80I_RES1_A		; 8C-8F
		dd	Z80I_RES2_B, Z80I_RES2_C, Z80I_RES2_D, Z80I_RES2_E		; 90-93
		dd	Z80I_RES2_H, Z80I_RES2_L, Z80I_RES2_mHL, Z80I_RES2_A		; 94-97
		dd	Z80I_RES3_B, Z80I_RES3_C, Z80I_RES3_D, Z80I_RES3_E		; 98-9B
		dd	Z80I_RES3_H, Z80I_RES3_L, Z80I_RES3_mHL, Z80I_RES3_A		; 9C-9F
		dd	Z80I_RES4_B, Z80I_RES4_C, Z80I_RES4_D, Z80I_RES4_E		; A0-A3
		dd	Z80I_RES4_H, Z80I_RES4_L, Z80I_RES4_mHL, Z80I_RES4_A		; A4-A7
		dd	Z80I_RES5_B, Z80I_RES5_C, Z80I_RES5_D, Z80I_RES5_E		; A8-AB
		dd	Z80I_RES5_H, Z80I_RES5_L, Z80I_RES5_mHL, Z80I_RES5_A		; AC-AF
		dd	Z80I_RES6_B, Z80I_RES6_C, Z80I_RES6_D, Z80I_RES6_E		; B0-B3
		dd	Z80I_RES6_H, Z80I_RES6_L, Z80I_RES6_mHL, Z80I_RES6_A		; B4-B7
		dd	Z80I_RES7_B, Z80I_RES7_C, Z80I_RES7_D, Z80I_RES7_E		; B8-BB
		dd	Z80I_RES7_H, Z80I_RES7_L, Z80I_RES7_mHL, Z80I_RES7_A		; BC-BF
		dd	Z80I_SET0_B, Z80I_SET0_C, Z80I_SET0_D, Z80I_SET0_E		; C0-C3
		dd	Z80I_SET0_H, Z80I_SET0_L, Z80I_SET0_mHL, Z80I_SET0_A		; C4-C7
		dd	Z80I_SET1_B, Z80I_SET1_C, Z80I_SET1_D, Z80I_SET1_E		; C8-CB
		dd	Z80I_SET1_H, Z80I_SET1_L, Z80I_SET1_mHL, Z80I_SET1_A		; CC-CF
		dd	Z80I_SET2_B, Z80I_SET2_C, Z80I_SET2_D, Z80I_SET2_E		; D0-D3
		dd	Z80I_SET2_H, Z80I_SET2_L, Z80I_SET2_mHL, Z80I_SET2_A		; D4-D7
		dd	Z80I_SET3_B, Z80I_SET3_C, Z80I_SET3_D, Z80I_SET3_E		; D8-DB
		dd	Z80I_SET3_H, Z80I_SET3_L, Z80I_SET3_mHL, Z80I_SET3_A		; DC-DF
		dd	Z80I_SET4_B, Z80I_SET4_C, Z80I_SET4_D, Z80I_SET4_E		; E0-E3
		dd	Z80I_SET4_H, Z80I_SET4_L, Z80I_SET4_mHL, Z80I_SET4_A		; E4-E7
		dd	Z80I_SET5_B, Z80I_SET5_C, Z80I_SET5_D, Z80I_SET5_E		; E8-EB
		dd	Z80I_SET5_H, Z80I_SET5_L, Z80I_SET5_mHL, Z80I_SET5_A		; EC-EF
		dd	Z80I_SET6_B, Z80I_SET6_C, Z80I_SET6_D, Z80I_SET6_E		; F0-F3
		dd	Z80I_SET6_H, Z80I_SET6_L, Z80I_SET6_mHL, Z80I_SET6_A		; F4-F7
		dd	Z80I_SET7_B, Z80I_SET7_C, Z80I_SET7_D, Z80I_SET7_E		; F8-FB
		dd	Z80I_SET7_H, Z80I_SET7_L, Z80I_SET7_mHL, Z80I_SET7_A		; FC-FF
	
	DD_Table:
		dd	Z80I_NOP, Z80I_LD_BC_NN, Z80I_LD_mBC_A, Z80I_INC_BC		; 00-03
		dd	Z80I_INC_B, Z80I_DEC_B, Z80I_LD_B_N, Z80I_RLCA			; 04-07
		dd	Z80I_EX_AF_AF2, Z80I_ADD_IX_BC, Z80I_LD_A_mBC, Z80I_DEC_BC	; 08-0B
		dd	Z80I_INC_C, Z80I_DEC_C, Z80I_LD_C_N, Z80I_RRCA			; 0C-0F
		dd	Z80I_DJNZ, Z80I_LD_DE_NN, Z80I_LD_mDE_A, Z80I_INC_DE		; 10-13
		dd	Z80I_INC_D, Z80I_DEC_D, Z80I_LD_D_N, Z80I_RLA			; 14-17
		dd	Z80I_JR_N, Z80I_ADD_IX_DE, Z80I_LD_A_mDE, Z80I_DEC_DE		; 18-1B
		dd	Z80I_DEC_E, Z80I_INC_E, Z80I_LD_E_N, Z80I_RRA			; 1C-1F
		dd	Z80I_JRNZ_N, Z80I_LD_IX_NN, Z80I_LD_mNN_IX, Z80I_INC_IX		; 20-23
		dd	Z80I_INC_hIX, Z80I_DEC_hIX, Z80I_LD_hIX_N, Z80I_DAA		; 24-27
		dd	Z80I_JRZ_N, Z80I_ADD_IX_IX, Z80I_LD_IX_mNN, Z80I_DEC_IX		; 28-2B
		dd	Z80I_INC_lIX, Z80I_DEC_lIX, Z80I_LD_lIX_N, Z80I_CPL		; 2C-2F
		dd	Z80I_JRNC_N, Z80I_LD_SP_NN, Z80I_LD_mNN_A, Z80I_INC_SP		; 30-33
		dd	Z80I_INC_mIXd, Z80I_DEC_mIXd, Z80I_LD_mIXd_N, Z80I_SCF		; 34-37
		dd	Z80I_JRC_N, Z80I_ADD_IX_SP, Z80I_LD_A_mNN, Z80I_DEC_SP		; 38-3B
		dd	Z80I_INC_A, Z80I_DEC_A, Z80I_LD_A_N, Z80I_CCF			; 3C-3F
		dd	Z80I_LD_B_B, Z80I_LD_B_C, Z80I_LD_B_D, Z80I_LD_B_E		; 40-43
		dd	Z80I_LD_B_hIX, Z80I_LD_B_lIX, Z80I_LD_B_mIXd, Z80I_LD_B_A	; 44-47
		dd	Z80I_LD_C_B, Z80I_LD_C_C, Z80I_LD_C_D, Z80I_LD_C_E		; 48-4B
		dd	Z80I_LD_C_hIX, Z80I_LD_C_lIX, Z80I_LD_C_mIXd, Z80I_LD_C_A	; 4C-4F
		dd	Z80I_LD_D_B, Z80I_LD_D_C, Z80I_LD_D_D, Z80I_LD_D_E		; 50-53
		dd	Z80I_LD_D_hIX, Z80I_LD_D_lIX, Z80I_LD_D_mIXd, Z80I_LD_D_A	; 54-57
		dd	Z80I_LD_E_B, Z80I_LD_E_C, Z80I_LD_E_D, Z80I_LD_E_E		; 58-5B
		dd	Z80I_LD_E_hIX, Z80I_LD_E_lIX, Z80I_LD_E_mIXd, Z80I_LD_E_A	; 5C-5F
		dd	Z80I_LD_hIX_B, Z80I_LD_hIX_C, Z80I_LD_hIX_D, Z80I_LD_hIX_E	; 60-63
		dd	Z80I_LD_hIX_hIX, Z80I_LD_hIX_L, Z80I_LD_H_mIXd, Z80I_LD_hIX_A	; 64-67
		dd	Z80I_LD_lIX_B, Z80I_LD_lIX_C, Z80I_LD_lIX_D, Z80I_LD_lIX_E	; 68-6B
		dd	Z80I_LD_lIX_H, Z80I_LD_lIX_lIX, Z80I_LD_L_mIXd, Z80I_LD_lIX_A	; 6C-6F
		dd	Z80I_LD_mIXd_B, Z80I_LD_mIXd_C, Z80I_LD_mIXd_D, Z80I_LD_mIXd_E	; 70-73
		dd	Z80I_LD_mIXd_H, Z80I_LD_mIXd_L, Z80I_HALT, Z80I_LD_mIXd_A	; 74-77
		dd	Z80I_LD_A_B, Z80I_LD_A_C, Z80I_LD_A_D, Z80I_LD_A_E		; 78-7B
		dd	Z80I_LD_A_hIX, Z80I_LD_A_lIX, Z80I_LD_A_mIXd, Z80I_LD_A_A	; 7C-7F
		dd	Z80I_ADD_B, Z80I_ADD_C, Z80I_ADD_D, Z80I_ADD_E			; 80-83
		dd	Z80I_ADD_hIX, Z80I_ADD_lIX, Z80I_ADD_mIXd, Z80I_ADD_A		; 84-87
		dd	Z80I_ADC_B, Z80I_ADC_C, Z80I_ADC_D, Z80I_ADC_E			; 88-8B
		dd	Z80I_ADC_hIX, Z80I_ADC_lIX, Z80I_ADC_mIXd, Z80I_ADC_A		; 8C-8F
		dd	Z80I_SUB_B, Z80I_SUB_C, Z80I_SUB_D, Z80I_SUB_E			; 90-83
		dd	Z80I_SUB_hIX, Z80I_SUB_lIX, Z80I_SUB_mIXd, Z80I_SUB_A		; 94-87
		dd	Z80I_SBC_B, Z80I_SBC_C, Z80I_SBC_D, Z80I_SBC_E			; 98-8B
		dd	Z80I_SBC_hIX, Z80I_SBC_lIX, Z80I_SBC_mIXd, Z80I_SBC_A		; 9C-8F
		dd	Z80I_AND_B, Z80I_AND_C, Z80I_AND_D, Z80I_AND_E			; A0-A3
		dd	Z80I_AND_hIX, Z80I_AND_lIX, Z80I_AND_mIXd, Z80I_AND_A		; A4-A7
		dd	Z80I_XOR_B, Z80I_XOR_C, Z80I_XOR_D, Z80I_XOR_E			; A8-AB
		dd	Z80I_XOR_hIX, Z80I_XOR_lIX, Z80I_XOR_mIXd, Z80I_XOR_A		; AC-AF
		dd	Z80I_OR_B, Z80I_OR_C, Z80I_OR_D, Z80I_OR_E			; B0-B3
		dd	Z80I_OR_hIX, Z80I_OR_lIX, Z80I_OR_mIXd, Z80I_OR_A		; B4-B7
		dd	Z80I_CP_B, Z80I_CP_C, Z80I_CP_D, Z80I_CP_E			; B8-BB
		dd	Z80I_CP_hIX, Z80I_CP_lIX, Z80I_CP_mIXd, Z80I_CP_A		; BC-BF
		dd	Z80I_RETNZ, Z80I_POP_BC, Z80I_JPNZ_NN, Z80I_JP_NN		; C0-C3
		dd	Z80I_CALLNZ_NN, Z80I_PUSH_BC, Z80I_ADD_N, Z80I_RST		; C4-C7
		dd	Z80I_RETZ, Z80I_RET, Z80I_JPZ_NN, PREFIXE_DDCB			; C8-CB
		dd	Z80I_CALLZ_NN, Z80I_CALL_NN, Z80I_ADC_N, Z80I_RST		; CC-CF
		dd	Z80I_RETNC, Z80I_POP_DE, Z80I_JPNC_NN, Z80I_OUT_mN		; D0-D3
		dd	Z80I_CALLNC_NN, Z80I_PUSH_DE, Z80I_SUB_N, Z80I_RST		; D4-D7
		dd	Z80I_RETC, Z80I_EXX, Z80I_JPC_NN, Z80I_IN_mN			; D8-DB
		dd	Z80I_CALLC_NN, PREFIXE_DD, Z80I_SBC_N, Z80I_RST			; DC-DF
		dd	Z80I_RETNP, Z80I_POP_IX, Z80I_JPNP_NN, Z80I_EX_mSP_IX		; E0-E3
		dd	Z80I_CALLNP_NN, Z80I_PUSH_IX, Z80I_AND_N, Z80I_RST		; E4-E7
		dd	Z80I_RETP, Z80I_JP_IX, Z80I_JPP_NN, Z80I_EX_DE_HL		; E8-EB
		dd	Z80I_CALLP_NN, PREFIXE_ED, Z80I_XOR_N, Z80I_RST			; EC-EF
		dd	Z80I_RETNS, Z80I_POP_AF, Z80I_JPNS_NN, Z80I_DI			; F0-F3
		dd	Z80I_CALLNS_NN, Z80I_PUSH_AF, Z80I_OR_N, Z80I_RST		; F4-F7
		dd	Z80I_RETS, Z80I_LD_SP_IX, Z80I_JPS_NN, Z80I_EI			; F8-FB
		dd	Z80I_CALLNS_NN, PREFIXE_FD, Z80I_CP_N, Z80I_RST			; FC-FF
	
	ED_Table:
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 00-03
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 04-07
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 08-0B
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 0C-0F
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 10-13
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 14-17
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 18-1B
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 1C-1F
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 20-23
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 24-27
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 28-2B
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 2C-2F
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 30-33
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 34-37
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 38-3B
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 3C-3F
		dd	Z80I_IN_B_mBC, Z80I_OUT_mBC_B, Z80I_SBC_HL_BC, Z80I_LD_mNN_BC	; 40-43
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM0, Z80I_LD_I_A			; 44-47
		dd	Z80I_IN_C_mBC, Z80I_OUT_mBC_C, Z80I_ADC_HL_BC, Z80I_LD_BC_mNN	; 48-4B
		dd	Z80I_NEG, Z80I_RETI, Z80I_IM0, Z80I_LD_R_A			; 4C-4F
		dd	Z80I_IN_D_mBC, Z80I_OUT_mBC_D, Z80I_SBC_HL_DE, Z80I_LD_mNN_DE	; 50-53
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM1, Z80I_LD_A_I			; 54-57
		dd	Z80I_IN_E_mBC, Z80I_OUT_mBC_E, Z80I_ADC_HL_DE, Z80I_LD_DE_mNN	; 58-5B
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM2, Z80I_LD_A_R			; 5C-5F
		dd	Z80I_IN_H_mBC, Z80I_OUT_mBC_H, Z80I_SBC_HL_HL, Z80I_LD2_mNN_HL	; 60-63
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM0, Z80I_RRD				; 64-67
		dd	Z80I_IN_L_mBC, Z80I_OUT_mBC_L, Z80I_ADC_HL_HL, Z80I_LD2_HL_mNN	; 68-6B
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM0, Z80I_RLD				; 6C-6F
		dd	Z80I_IN_F_mBC, Z80I_OUT_mBC_0, Z80I_SBC_HL_SP, Z80I_LD_mNN_SP	; 70-73
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM1, Z80I_NOP				; 74-77
		dd	Z80I_IN_A_mBC, Z80I_OUT_mBC_A, Z80I_ADC_HL_SP, Z80I_LD_SP_mNN	; 78-7B
		dd	Z80I_NEG, Z80I_RETN, Z80I_IM2, Z80I_NOP				; 7C-7F
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 80-83
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 84-87
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 88-8B
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 8C-8F
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 90-93
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 94-97
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 98-9B
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; 9C-9F
		dd	Z80I_LDI, Z80I_CPI, Z80I_INI, Z80I_OUTI				; A0-A3
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; A4-A7
		dd	Z80I_LDD, Z80I_CPD, Z80I_IND, Z80I_OUTD				; A8-AB
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; AC-AF
		dd	Z80I_LDIR, Z80I_CPIR, Z80I_INIR, Z80I_OTIR			; B0-B3
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; B4-B7
		dd	Z80I_LDDR, Z80I_CPDR, Z80I_INDR, Z80I_OTDR			; B8-BB
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; BC-BF
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; C0-C3
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; C4-C7
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; C8-CB
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; CC-CF
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; D0-D3
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; D4-D7
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; D8-DB
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; DC-DF
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; E0-E3
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; E4-E7
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; E8-EB
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; EC-EF
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; F0-F3
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; F4-F7
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; F8-FB
		dd	Z80I_NOP, Z80I_NOP, Z80I_NOP, Z80I_NOP				; FC-FF
	
	FD_Table:
		dd	Z80I_NOP, Z80I_LD_BC_NN, Z80I_LD_mBC_A, Z80I_INC_BC		; 00-03
		dd	Z80I_INC_B, Z80I_DEC_B, Z80I_LD_B_N, Z80I_RLCA			; 04-07
		dd	Z80I_EX_AF_AF2, Z80I_ADD_IY_BC, Z80I_LD_A_mBC, Z80I_DEC_BC	; 08-0B
		dd	Z80I_INC_C, Z80I_DEC_C, Z80I_LD_C_N, Z80I_RRCA			; 0C-0F
		dd	Z80I_DJNZ, Z80I_LD_DE_NN, Z80I_LD_mDE_A, Z80I_INC_DE		; 10-13
		dd	Z80I_INC_D, Z80I_DEC_D, Z80I_LD_D_N, Z80I_RLA			; 14-17
		dd	Z80I_JR_N, Z80I_ADD_IY_DE, Z80I_LD_A_mDE, Z80I_DEC_DE		; 18-1B
		dd	Z80I_DEC_E, Z80I_INC_E, Z80I_LD_E_N, Z80I_RRA			; 1C-1F
		dd	Z80I_JRNZ_N, Z80I_LD_IY_NN, Z80I_LD_mNN_IY, Z80I_INC_IY		; 20-23
		dd	Z80I_INC_hIY, Z80I_DEC_hIY, Z80I_LD_hIY_N, Z80I_DAA		; 24-27
		dd	Z80I_JRZ_N, Z80I_ADD_IY_IY, Z80I_LD_IY_mNN, Z80I_DEC_IY		; 28-2B
		dd	Z80I_INC_lIY, Z80I_DEC_lIY, Z80I_LD_lIY_N, Z80I_CPL		; 2C-2F
		dd	Z80I_JRNC_N, Z80I_LD_SP_NN, Z80I_LD_mNN_A, Z80I_INC_SP		; 30-33
		dd	Z80I_INC_mIYd, Z80I_DEC_mIYd, Z80I_LD_mIYd_N, Z80I_SCF		; 34-37
		dd	Z80I_JRC_N, Z80I_ADD_IY_SP, Z80I_LD_A_mNN, Z80I_DEC_SP		; 38-3B
		dd	Z80I_INC_A, Z80I_DEC_A, Z80I_LD_A_N, Z80I_CCF			; 3C-3F
		dd	Z80I_LD_B_B, Z80I_LD_B_C, Z80I_LD_B_D, Z80I_LD_B_E		; 40-43
		dd	Z80I_LD_B_hIY, Z80I_LD_B_lIY, Z80I_LD_B_mIYd, Z80I_LD_B_A	; 44-47
		dd	Z80I_LD_C_B, Z80I_LD_C_C, Z80I_LD_C_D, Z80I_LD_C_E		; 48-4B
		dd	Z80I_LD_C_hIY, Z80I_LD_C_lIY, Z80I_LD_C_mIYd, Z80I_LD_C_A	; 4C-4F
		dd	Z80I_LD_D_B, Z80I_LD_D_C, Z80I_LD_D_D, Z80I_LD_D_E		; 50-53
		dd	Z80I_LD_D_hIY, Z80I_LD_D_lIY, Z80I_LD_D_mIYd, Z80I_LD_D_A	; 54-57
		dd	Z80I_LD_E_B, Z80I_LD_E_C, Z80I_LD_E_D, Z80I_LD_E_E		; 58-5B
		dd	Z80I_LD_E_hIY, Z80I_LD_E_lIY, Z80I_LD_E_mIYd, Z80I_LD_E_A	; 5C-5F
		dd	Z80I_LD_hIY_B, Z80I_LD_hIY_C, Z80I_LD_hIY_D, Z80I_LD_hIY_E	; 60-63
		dd	Z80I_LD_hIY_hIY, Z80I_LD_hIY_L, Z80I_LD_H_mIYd, Z80I_LD_hIY_A	; 64-67
		dd	Z80I_LD_lIY_B, Z80I_LD_lIY_C, Z80I_LD_lIY_D, Z80I_LD_lIY_E	; 68-6B
		dd	Z80I_LD_lIY_H, Z80I_LD_lIY_lIY, Z80I_LD_L_mIYd, Z80I_LD_lIY_A	; 6C-6F
		dd	Z80I_LD_mIYd_B, Z80I_LD_mIYd_C, Z80I_LD_mIYd_D, Z80I_LD_mIYd_E	; 70-73
		dd	Z80I_LD_mIYd_H, Z80I_LD_mIYd_L, Z80I_HALT, Z80I_LD_mIYd_A	; 74-77
		dd	Z80I_LD_A_B, Z80I_LD_A_C, Z80I_LD_A_D, Z80I_LD_A_E		; 78-7B
		dd	Z80I_LD_A_hIY, Z80I_LD_A_lIY, Z80I_LD_A_mIYd, Z80I_LD_A_A	; 7C-7F
		dd	Z80I_ADD_B, Z80I_ADD_C, Z80I_ADD_D, Z80I_ADD_E			; 80-83
		dd	Z80I_ADD_hIY, Z80I_ADD_lIY, Z80I_ADD_mIYd, Z80I_ADD_A		; 84-87
		dd	Z80I_ADC_B, Z80I_ADC_C, Z80I_ADC_D, Z80I_ADC_E			; 88-8B
		dd	Z80I_ADC_hIY, Z80I_ADC_lIY, Z80I_ADC_mIYd, Z80I_ADC_A		; 8C-8F
		dd	Z80I_SUB_B, Z80I_SUB_C, Z80I_SUB_D, Z80I_SUB_E			; 90-83
		dd	Z80I_SUB_hIY, Z80I_SUB_lIY, Z80I_SUB_mIYd, Z80I_SUB_A		; 94-87
		dd	Z80I_SBC_B, Z80I_SBC_C, Z80I_SBC_D, Z80I_SBC_E			; 98-8B
		dd	Z80I_SBC_hIY, Z80I_SBC_lIY, Z80I_SBC_mIYd, Z80I_SBC_A		; 9C-8F
		dd	Z80I_AND_B, Z80I_AND_C, Z80I_AND_D, Z80I_AND_E			; A0-A3
		dd	Z80I_AND_hIY, Z80I_AND_lIY, Z80I_AND_mIYd, Z80I_AND_A		; A4-A7
		dd	Z80I_XOR_B, Z80I_XOR_C, Z80I_XOR_D, Z80I_XOR_E			; A8-AB
		dd	Z80I_XOR_hIY, Z80I_XOR_lIY, Z80I_XOR_mIYd, Z80I_XOR_A		; AC-AF
		dd	Z80I_OR_B, Z80I_OR_C, Z80I_OR_D, Z80I_OR_E			; B0-B3
		dd	Z80I_OR_hIY, Z80I_OR_lIY, Z80I_OR_mIYd, Z80I_OR_A		; B4-B7
		dd	Z80I_CP_B, Z80I_CP_C, Z80I_CP_D, Z80I_CP_E			; B8-BB
		dd	Z80I_CP_hIY, Z80I_CP_lIY, Z80I_CP_mIYd, Z80I_CP_A		; BC-BF
		dd	Z80I_RETNZ, Z80I_POP_BC, Z80I_JPNZ_NN, Z80I_JP_NN		; C0-C3
		dd	Z80I_CALLNZ_NN, Z80I_PUSH_BC, Z80I_ADD_N, Z80I_RST		; C4-C7
		dd	Z80I_RETZ, Z80I_RET, Z80I_JPZ_NN, PREFIXE_FDCB			; C8-CB
		dd	Z80I_CALLZ_NN, Z80I_CALL_NN, Z80I_ADC_N, Z80I_RST		; CC-CF
		dd	Z80I_RETNC, Z80I_POP_DE, Z80I_JPNC_NN, Z80I_OUT_mN		; D0-D3
		dd	Z80I_CALLNC_NN, Z80I_PUSH_DE, Z80I_SUB_N, Z80I_RST		; D4-D7
		dd	Z80I_RETC, Z80I_EXX, Z80I_JPC_NN, Z80I_IN_mN			; D8-DB
		dd	Z80I_CALLC_NN, PREFIXE_DD, Z80I_SBC_N, Z80I_RST			; DC-DF
		dd	Z80I_RETNP, Z80I_POP_IY, Z80I_JPNP_NN, Z80I_EX_mSP_IY		; E0-E3
		dd	Z80I_CALLNP_NN, Z80I_PUSH_IY, Z80I_AND_N, Z80I_RST		; E4-E7
		dd	Z80I_RETP, Z80I_JP_IY, Z80I_JPP_NN, Z80I_EX_DE_HL		; E8-EB
		dd	Z80I_CALLP_NN, PREFIXE_ED, Z80I_XOR_N, Z80I_RST			; EC-EF
		dd	Z80I_RETNS, Z80I_POP_AF, Z80I_JPNS_NN, Z80I_DI			; F0-F3
		dd	Z80I_CALLNS_NN, Z80I_PUSH_AF, Z80I_OR_N, Z80I_RST		; F4-F7
		dd	Z80I_RETS, Z80I_LD_SP_IY, Z80I_JPS_NN, Z80I_EI			; F8-FB
		dd	Z80I_CALLNS_NN, PREFIXE_FD, Z80I_CP_N, Z80I_RST			; FC-FF
	
	DDCB_Table:
		dd	Z80I_RLC_mIXd_B, Z80I_RLC_mIXd_C, Z80I_RLC_mIXd_D, Z80I_RLC_mIXd_E	; 00-03
		dd	Z80I_RLC_mIXd_H, Z80I_RLC_mIXd_L, Z80I_RLC_mIXd, Z80I_RLC_mIXd_A	; 04-07
		dd	Z80I_RRC_mIXd_B, Z80I_RRC_mIXd_C, Z80I_RRC_mIXd_D, Z80I_RRC_mIXd_E	; 08-0B
		dd	Z80I_RRC_mIXd_H, Z80I_RRC_mIXd_L, Z80I_RRC_mIXd, Z80I_RRC_mIXd_A	; 0C-0F
		dd	Z80I_RL_mIXd_B, Z80I_RL_mIXd_C, Z80I_RL_mIXd_D, Z80I_RL_mIXd_E		; 10-13
		dd	Z80I_RL_mIXd_H, Z80I_RL_mIXd_L, Z80I_RL_mIXd, Z80I_RL_mIXd_A		; 14-17
		dd	Z80I_RR_mIXd_B, Z80I_RR_mIXd_C, Z80I_RR_mIXd_D, Z80I_RR_mIXd_E		; 18-1B
		dd	Z80I_RR_mIXd_H, Z80I_RR_mIXd_L, Z80I_RR_mIXd, Z80I_RR_mIXd_A		; 1C-1F
		dd	Z80I_SLA_mIXd_B, Z80I_SLA_mIXd_C, Z80I_SLA_mIXd_D, Z80I_SLA_mIXd_E	; 20-23
		dd	Z80I_SLA_mIXd_H, Z80I_SLA_mIXd_L, Z80I_SLA_mIXd, Z80I_SLA_mIXd_A	; 24-27
		dd	Z80I_SRA_mIXd_B, Z80I_SRA_mIXd_C, Z80I_SRA_mIXd_D, Z80I_SRA_mIXd_E	; 28-2B
		dd	Z80I_SRA_mIXd_H, Z80I_SRA_mIXd_L, Z80I_SRA_mIXd, Z80I_SRA_mIXd_A	; 2C-2F
		dd	Z80I_SLL_mIXd_B, Z80I_SLL_mIXd_C, Z80I_SLL_mIXd_D, Z80I_SLL_mIXd_E	; 30-33
		dd	Z80I_SLL_mIXd_H, Z80I_SLL_mIXd_L, Z80I_SLL_mIXd, Z80I_SLL_mIXd_A	; 34-37
		dd	Z80I_SRL_mIXd_B, Z80I_SRL_mIXd_C, Z80I_SRL_mIXd_D, Z80I_SRL_mIXd_E	; 38-3B
		dd	Z80I_SRL_mIXd_H, Z80I_SRL_mIXd_L, Z80I_SRL_mIXd, Z80I_SRL_mIXd_A	; 3C-3F
		dd	Z80I_BIT0_B, Z80I_BIT0_C, Z80I_BIT0_D, Z80I_BIT0_E			; 40-43
		dd	Z80I_BIT0_H, Z80I_BIT0_L, Z80I_BIT0_mIXd, Z80I_BIT0_A			; 44-47
		dd	Z80I_BIT1_B, Z80I_BIT1_C, Z80I_BIT1_D, Z80I_BIT1_E			; 48-4B
		dd	Z80I_BIT1_H, Z80I_BIT1_L, Z80I_BIT1_mIXd, Z80I_BIT1_A			; 4C-4F
		dd	Z80I_BIT2_B, Z80I_BIT2_C, Z80I_BIT2_D, Z80I_BIT2_E			; 50-53
		dd	Z80I_BIT2_H, Z80I_BIT2_L, Z80I_BIT2_mIXd, Z80I_BIT2_A			; 54-57
		dd	Z80I_BIT3_B, Z80I_BIT3_C, Z80I_BIT3_D, Z80I_BIT3_E			; 58-5B
		dd	Z80I_BIT3_H, Z80I_BIT3_L, Z80I_BIT3_mIXd, Z80I_BIT3_A			; 5C-5F
		dd	Z80I_BIT4_B, Z80I_BIT4_C, Z80I_BIT4_D, Z80I_BIT4_E			; 60-63
		dd	Z80I_BIT4_H, Z80I_BIT4_L, Z80I_BIT4_mIXd, Z80I_BIT4_A			; 64-67
		dd	Z80I_BIT5_B, Z80I_BIT5_C, Z80I_BIT5_D, Z80I_BIT5_E			; 68-6B
		dd	Z80I_BIT5_H, Z80I_BIT5_L, Z80I_BIT5_mIXd, Z80I_BIT5_A			; 6C-6F
		dd	Z80I_BIT6_B, Z80I_BIT6_C, Z80I_BIT6_D, Z80I_BIT6_E			; 70-73
		dd	Z80I_BIT6_H, Z80I_BIT6_L, Z80I_BIT6_mIXd, Z80I_BIT6_A			; 74-77
		dd	Z80I_BIT7_B, Z80I_BIT7_C, Z80I_BIT7_D, Z80I_BIT7_E			; 78-7B
		dd	Z80I_BIT7_H, Z80I_BIT7_L, Z80I_BIT7_mIXd, Z80I_BIT7_A			; 7C-7F
		dd	Z80I_RES0_mIXd_B, Z80I_RES0_mIXd_C, Z80I_RES0_mIXd_D, Z80I_RES0_mIXd_E	; 80-83
		dd	Z80I_RES0_mIXd_H, Z80I_RES0_mIXd_L, Z80I_RES0_mIXd, Z80I_RES0_mIXd_A	; 84-87
		dd	Z80I_RES1_mIXd_B, Z80I_RES1_mIXd_C, Z80I_RES1_mIXd_D, Z80I_RES1_mIXd_E	; 88-8B
		dd	Z80I_RES1_mIXd_H, Z80I_RES1_mIXd_L, Z80I_RES1_mIXd, Z80I_RES1_mIXd_A	; 8C-8F
		dd	Z80I_RES2_mIXd_B, Z80I_RES2_mIXd_C, Z80I_RES2_mIXd_D, Z80I_RES2_mIXd_E	; 90-93
		dd	Z80I_RES2_mIXd_H, Z80I_RES2_mIXd_L, Z80I_RES2_mIXd, Z80I_RES2_mIXd_A	; 94-97
		dd	Z80I_RES3_mIXd_B, Z80I_RES3_mIXd_C, Z80I_RES3_mIXd_D, Z80I_RES3_mIXd_E	; 98-9B
		dd	Z80I_RES3_mIXd_H, Z80I_RES3_mIXd_L, Z80I_RES3_mIXd, Z80I_RES3_mIXd_A	; 9C-9F
		dd	Z80I_RES4_mIXd_B, Z80I_RES4_mIXd_C, Z80I_RES4_mIXd_D, Z80I_RES4_mIXd_E	; A0-A3
		dd	Z80I_RES4_mIXd_H, Z80I_RES4_mIXd_L, Z80I_RES4_mIXd, Z80I_RES4_mIXd_A	; A4-A7
		dd	Z80I_RES5_mIXd_B, Z80I_RES5_mIXd_C, Z80I_RES5_mIXd_D, Z80I_RES5_mIXd_E	; A8-AB
		dd	Z80I_RES5_mIXd_H, Z80I_RES5_mIXd_L, Z80I_RES5_mIXd, Z80I_RES5_mIXd_A	; AC-AF
		dd	Z80I_RES6_mIXd_B, Z80I_RES6_mIXd_C, Z80I_RES6_mIXd_D, Z80I_RES6_mIXd_E	; B0-B3
		dd	Z80I_RES6_mIXd_H, Z80I_RES6_mIXd_L, Z80I_RES6_mIXd, Z80I_RES6_mIXd_A	; B4-B7
		dd	Z80I_RES7_mIXd_B, Z80I_RES7_mIXd_C, Z80I_RES7_mIXd_D, Z80I_RES7_mIXd_E	; B8-BB
		dd	Z80I_RES7_mIXd_H, Z80I_RES7_mIXd_L, Z80I_RES7_mIXd, Z80I_RES7_mIXd_A	; BC-BF
		dd	Z80I_SET0_mIXd_B, Z80I_SET0_mIXd_C, Z80I_SET0_mIXd_D, Z80I_SET0_mIXd_E	; C0-C3
		dd	Z80I_SET0_mIXd_H, Z80I_SET0_mIXd_L, Z80I_SET0_mIXd, Z80I_SET0_mIXd_A	; C4-C7
		dd	Z80I_SET1_mIXd_B, Z80I_SET1_mIXd_C, Z80I_SET1_mIXd_D, Z80I_SET1_mIXd_E	; C8-CB
		dd	Z80I_SET1_mIXd_H, Z80I_SET1_mIXd_L, Z80I_SET1_mIXd, Z80I_SET1_mIXd_A	; CC-CF
		dd	Z80I_SET2_mIXd_B, Z80I_SET2_mIXd_C, Z80I_SET2_mIXd_D, Z80I_SET2_mIXd_E	; D0-D3
		dd	Z80I_SET2_mIXd_H, Z80I_SET2_mIXd_L, Z80I_SET2_mIXd, Z80I_SET2_mIXd_A	; D4-D7
		dd	Z80I_SET3_mIXd_B, Z80I_SET3_mIXd_C, Z80I_SET3_mIXd_D, Z80I_SET3_mIXd_E	; D8-DB
		dd	Z80I_SET3_mIXd_H, Z80I_SET3_mIXd_L, Z80I_SET3_mIXd, Z80I_SET3_mIXd_A	; DC-DF
		dd	Z80I_SET4_mIXd_B, Z80I_SET4_mIXd_C, Z80I_SET4_mIXd_D, Z80I_SET4_mIXd_E	; E0-E3
		dd	Z80I_SET4_mIXd_H, Z80I_SET4_mIXd_L, Z80I_SET4_mIXd, Z80I_SET4_mIXd_A	; E4-E7
		dd	Z80I_SET5_mIXd_B, Z80I_SET5_mIXd_C, Z80I_SET5_mIXd_D, Z80I_SET5_mIXd_E	; E8-EB
		dd	Z80I_SET5_mIXd_H, Z80I_SET5_mIXd_L, Z80I_SET5_mIXd, Z80I_SET5_mIXd_A	; EC-EF
		dd	Z80I_SET6_mIXd_B, Z80I_SET6_mIXd_C, Z80I_SET6_mIXd_D, Z80I_SET6_mIXd_E	; F0-F3
		dd	Z80I_SET6_mIXd_H, Z80I_SET6_mIXd_L, Z80I_SET6_mIXd, Z80I_SET6_mIXd_A	; F4-F7
		dd	Z80I_SET7_mIXd_B, Z80I_SET7_mIXd_C, Z80I_SET7_mIXd_D, Z80I_SET7_mIXd_E	; F8-FB
		dd	Z80I_SET7_mIXd_H, Z80I_SET7_mIXd_L, Z80I_SET7_mIXd, Z80I_SET7_mIXd_A	; FC-FF
	
	FDCB_Table:
		dd	Z80I_RLC_mIYd_B, Z80I_RLC_mIYd_C, Z80I_RLC_mIYd_D, Z80I_RLC_mIYd_E	; 00-03
		dd	Z80I_RLC_mIYd_H, Z80I_RLC_mIYd_L, Z80I_RLC_mIYd, Z80I_RLC_mIYd_A	; 04-07
		dd	Z80I_RRC_mIYd_B, Z80I_RRC_mIYd_C, Z80I_RRC_mIYd_D, Z80I_RRC_mIYd_E	; 08-0B
		dd	Z80I_RRC_mIYd_H, Z80I_RRC_mIYd_L, Z80I_RRC_mIYd, Z80I_RRC_mIYd_A	; 0C-0F
		dd	Z80I_RL_mIYd_B, Z80I_RL_mIYd_C, Z80I_RL_mIYd_D, Z80I_RL_mIYd_E		; 10-13
		dd	Z80I_RL_mIYd_H, Z80I_RL_mIYd_L, Z80I_RL_mIYd, Z80I_RL_mIYd_A		; 14-17
		dd	Z80I_RR_mIYd_B, Z80I_RR_mIYd_C, Z80I_RR_mIYd_D, Z80I_RR_mIYd_E		; 18-1B
		dd	Z80I_RR_mIYd_H, Z80I_RR_mIYd_L, Z80I_RR_mIYd, Z80I_RR_mIYd_A		; 1C-1F
		dd	Z80I_SLA_mIYd_B, Z80I_SLA_mIYd_C, Z80I_SLA_mIYd_D, Z80I_SLA_mIYd_E	; 20-23
		dd	Z80I_SLA_mIYd_H, Z80I_SLA_mIYd_L, Z80I_SLA_mIYd, Z80I_SLA_mIYd_A	; 24-27
		dd	Z80I_SRA_mIYd_B, Z80I_SRA_mIYd_C, Z80I_SRA_mIYd_D, Z80I_SRA_mIYd_E	; 28-2B
		dd	Z80I_SRA_mIYd_H, Z80I_SRA_mIYd_L, Z80I_SRA_mIYd, Z80I_SRA_mIYd_A	; 2C-2F
		dd	Z80I_SLL_mIYd_B, Z80I_SLL_mIYd_C, Z80I_SLL_mIYd_D, Z80I_SLL_mIYd_E	; 30-33
		dd	Z80I_SLL_mIYd_H, Z80I_SLL_mIYd_L, Z80I_SLL_mIYd, Z80I_SLL_mIYd_A	; 34-37
		dd	Z80I_SRL_mIYd_B, Z80I_SRL_mIYd_C, Z80I_SRL_mIYd_D, Z80I_SRL_mIYd_E	; 38-3B
		dd	Z80I_SRL_mIYd_H, Z80I_SRL_mIYd_L, Z80I_SRL_mIYd, Z80I_SRL_mIYd_A	; 3C-3F
		dd	Z80I_BIT0_B, Z80I_BIT0_C, Z80I_BIT0_D, Z80I_BIT0_E			; 40-43
		dd	Z80I_BIT0_H, Z80I_BIT0_L, Z80I_BIT0_mIYd, Z80I_BIT0_A			; 44-47
		dd	Z80I_BIT1_B, Z80I_BIT1_C, Z80I_BIT1_D, Z80I_BIT1_E			; 48-4B
		dd	Z80I_BIT1_H, Z80I_BIT1_L, Z80I_BIT1_mIYd, Z80I_BIT1_A			; 4C-4F
		dd	Z80I_BIT2_B, Z80I_BIT2_C, Z80I_BIT2_D, Z80I_BIT2_E			; 50-53
		dd	Z80I_BIT2_H, Z80I_BIT2_L, Z80I_BIT2_mIYd, Z80I_BIT2_A			; 54-57
		dd	Z80I_BIT3_B, Z80I_BIT3_C, Z80I_BIT3_D, Z80I_BIT3_E			; 58-5B
		dd	Z80I_BIT3_H, Z80I_BIT3_L, Z80I_BIT3_mIYd, Z80I_BIT3_A			; 5C-5F
		dd	Z80I_BIT4_B, Z80I_BIT4_C, Z80I_BIT4_D, Z80I_BIT4_E			; 60-63
		dd	Z80I_BIT4_H, Z80I_BIT4_L, Z80I_BIT4_mIYd, Z80I_BIT4_A			; 64-67
		dd	Z80I_BIT5_B, Z80I_BIT5_C, Z80I_BIT5_D, Z80I_BIT5_E			; 68-6B
		dd	Z80I_BIT5_H, Z80I_BIT5_L, Z80I_BIT5_mIYd, Z80I_BIT5_A			; 6C-6F
		dd	Z80I_BIT6_B, Z80I_BIT6_C, Z80I_BIT6_D, Z80I_BIT6_E			; 70-73
		dd	Z80I_BIT6_H, Z80I_BIT6_L, Z80I_BIT6_mIYd, Z80I_BIT6_A			; 74-77
		dd	Z80I_BIT7_B, Z80I_BIT7_C, Z80I_BIT7_D, Z80I_BIT7_E			; 78-7B
		dd	Z80I_BIT7_H, Z80I_BIT7_L, Z80I_BIT7_mIYd, Z80I_BIT7_A			; 7C-7F
		dd	Z80I_RES0_mIYd_B, Z80I_RES0_mIYd_C, Z80I_RES0_mIYd_D, Z80I_RES0_mIYd_E	; 80-83
		dd	Z80I_RES0_mIYd_H, Z80I_RES0_mIYd_L, Z80I_RES0_mIYd, Z80I_RES0_mIYd_A	; 84-87
		dd	Z80I_RES1_mIYd_B, Z80I_RES1_mIYd_C, Z80I_RES1_mIYd_D, Z80I_RES1_mIYd_E	; 88-8B
		dd	Z80I_RES1_mIYd_H, Z80I_RES1_mIYd_L, Z80I_RES1_mIYd, Z80I_RES1_mIYd_A	; 8C-8F
		dd	Z80I_RES2_mIYd_B, Z80I_RES2_mIYd_C, Z80I_RES2_mIYd_D, Z80I_RES2_mIYd_E	; 90-93
		dd	Z80I_RES2_mIYd_H, Z80I_RES2_mIYd_L, Z80I_RES2_mIYd, Z80I_RES2_mIYd_A	; 94-97
		dd	Z80I_RES3_mIYd_B, Z80I_RES3_mIYd_C, Z80I_RES3_mIYd_D, Z80I_RES3_mIYd_E	; 98-9B
		dd	Z80I_RES3_mIYd_H, Z80I_RES3_mIYd_L, Z80I_RES3_mIYd, Z80I_RES3_mIYd_A	; 9C-9F
		dd	Z80I_RES4_mIYd_B, Z80I_RES4_mIYd_C, Z80I_RES4_mIYd_D, Z80I_RES4_mIYd_E	; A0-A3
		dd	Z80I_RES4_mIYd_H, Z80I_RES4_mIYd_L, Z80I_RES4_mIYd, Z80I_RES4_mIYd_A	; A4-A7
		dd	Z80I_RES5_mIYd_B, Z80I_RES5_mIYd_C, Z80I_RES5_mIYd_D, Z80I_RES5_mIYd_E	; A8-AB
		dd	Z80I_RES5_mIYd_H, Z80I_RES5_mIYd_L, Z80I_RES5_mIYd, Z80I_RES5_mIYd_A	; AC-AF
		dd	Z80I_RES6_mIYd_B, Z80I_RES6_mIYd_C, Z80I_RES6_mIYd_D, Z80I_RES6_mIYd_E	; B0-B3
		dd	Z80I_RES6_mIYd_H, Z80I_RES6_mIYd_L, Z80I_RES6_mIYd, Z80I_RES6_mIYd_A	; B4-B7
		dd	Z80I_RES7_mIYd_B, Z80I_RES7_mIYd_C, Z80I_RES7_mIYd_D, Z80I_RES7_mIYd_E	; B8-BB
		dd	Z80I_RES7_mIYd_H, Z80I_RES7_mIYd_L, Z80I_RES7_mIYd, Z80I_RES7_mIYd_A	; BC-BF
		dd	Z80I_SET0_mIYd_B, Z80I_SET0_mIYd_C, Z80I_SET0_mIYd_D, Z80I_SET0_mIYd_E	; C0-C3
		dd	Z80I_SET0_mIYd_H, Z80I_SET0_mIYd_L, Z80I_SET0_mIYd, Z80I_SET0_mIYd_A	; C4-C7
		dd	Z80I_SET1_mIYd_B, Z80I_SET1_mIYd_C, Z80I_SET1_mIYd_D, Z80I_SET1_mIYd_E	; C8-CB
		dd	Z80I_SET1_mIYd_H, Z80I_SET1_mIYd_L, Z80I_SET1_mIYd, Z80I_SET1_mIYd_A	; CC-CF
		dd	Z80I_SET2_mIYd_B, Z80I_SET2_mIYd_C, Z80I_SET2_mIYd_D, Z80I_SET2_mIYd_E	; D0-D3
		dd	Z80I_SET2_mIYd_H, Z80I_SET2_mIYd_L, Z80I_SET2_mIYd, Z80I_SET2_mIYd_A	; D4-D7
		dd	Z80I_SET3_mIYd_B, Z80I_SET3_mIYd_C, Z80I_SET3_mIYd_D, Z80I_SET3_mIYd_E	; D8-DB
		dd	Z80I_SET3_mIYd_H, Z80I_SET3_mIYd_L, Z80I_SET3_mIYd, Z80I_SET3_mIYd_A	; DC-DF
		dd	Z80I_SET4_mIYd_B, Z80I_SET4_mIYd_C, Z80I_SET4_mIYd_D, Z80I_SET4_mIYd_E	; E0-E3
		dd	Z80I_SET4_mIYd_H, Z80I_SET4_mIYd_L, Z80I_SET4_mIYd, Z80I_SET4_mIYd_A	; E4-E7
		dd	Z80I_SET5_mIYd_B, Z80I_SET5_mIYd_C, Z80I_SET5_mIYd_D, Z80I_SET5_mIYd_E	; E8-EB
		dd	Z80I_SET5_mIYd_H, Z80I_SET5_mIYd_L, Z80I_SET5_mIYd, Z80I_SET5_mIYd_A	; EC-EF
		dd	Z80I_SET6_mIYd_B, Z80I_SET6_mIYd_C, Z80I_SET6_mIYd_D, Z80I_SET6_mIYd_E	; F0-F3
		dd	Z80I_SET6_mIYd_H, Z80I_SET6_mIYd_L, Z80I_SET6_mIYd, Z80I_SET6_mIYd_A	; F4-F7
		dd	Z80I_SET7_mIYd_B, Z80I_SET7_mIYd_C, Z80I_SET7_mIYd_D, Z80I_SET7_mIYd_E	; F8-FB
		dd	Z80I_SET7_mIYd_H, Z80I_SET7_mIYd_L, Z80I_SET7_mIYd, Z80I_SET7_mIYd_A	; FC-FF
