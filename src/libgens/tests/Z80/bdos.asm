; Z80 BDOS for Gens ZEX Machine.
; Limited BDOS with init, text printing, and halt.
; Assemble with Macro Assembler AS:
; asmx -C Z80 -o bdos.bin -b F000h-FFFFh -e -w bdos.asm

; Gens ZEX Machine I/O ports:
; - 01h: stdout
; - 02h: stderr
; - FFh: halt

	CPU Z80

; F000h: Warm boot code.
	ORG	F000h

	; Replace the warm boot vector with a jump to F100h.
	; ZEXALL jumps to 0000h on completion.
	; F100h halts the system.
	LD	BC,F100h
	LD	(0001h),BC

	; Initialize the syscall vector.
	LD	A,C3h		; JP NN
	LD	(0005h),A
	LD	BC,F400h
	LD	(0006h),BC

	; Start the program.
	JP	0100h

; F100h: HALT vector.
	ORG	F100h
$$loop:	LD	A,1
	OUT	(FFh),a
	HALT
	JR	$$loop

; F400h: System call handler.
	ORG	F400h

	; Syscall number is in C.
	LD	A,0
	CP	C
	JP	Z,P_TERMCPM
	LD	A,2
	CP	C
	JP	Z,C_WRITE
	LD	A,9
	CP	C
	JP	Z,C_WRITESTR

	; Invalid syscall.
	; Halt the system.
	; TODO: Print an error message with the syscall ID?
	JP	F100h

P_TERMCPM:	; Halt the system.
	JP	0000h

C_WRITE:	; Write a character to stdout.
	LD	C,01h
	OUT	(C),E
	RET

C_WRITESTR:	; Write a string to stdout. (Ends in '$')
	LD	C,01h
$$nextchr:
	LD	A,(DE)
	CP	'$'
	JP	Z,$$endofstr
	OUT	(C),A
	INC	DE
	JP	$$nextchr
$$endofstr:
	RET
