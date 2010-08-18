; asm wrapper for Starscream
; because Starscream is kinda broken

; Determine the output format.
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
	
	; Mac OS X requires 16-byte aligned stacks.
	; Otherwise, the program will randomly crash in
	; __dyld_misaligned_stack_error().
	; (The crash might not even be immediately after
	; calling the C function!)
	%define __FORCE_STACK_ALIGNMENT
%endif

%ifdef __OBJ_ELF
	; Mark the stack as non-executable on ELF.
	section .note.GNU-stack noalloc noexec nowrite progbits
%endif

; Symbol declaration.
%ifdef __OBJ_ELF
	%define SYM(x) x
%else
	%define SYM(x) _ %+ x
%endif

section .data align=64
	
	extern SYM(Gens_M68K_RB)
	extern SYM(Gens_M68K_RW)
	extern SYM(Gens_M68K_WB)
	extern SYM(Gens_M68K_WW)

section .text align=64

arg_1	equ 8
arg_2	equ 12

; uint8_t M68K_RB(uint32_t address)
global SYM(M68K_RB)
SYM(M68K_RB):
	; Set up the frame pointer.
	push	ebp
	mov	ebp, esp
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	and	esp, ~0x0F
%endif
	
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	sub	esp, 8
%endif
	
	; Call the function.
	mov	eax, [ebp + arg_1]
	push	eax
	call	SYM(Gens_M68K_RB)
%ifdef __FORCE_STACK_ALIGNMENT
	add	esp, 12
%else
	add	esp, 4
%endif
	
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	
	; Reset the frame pointer.
	mov	esp, ebp
	pop	ebp
	ret

; uint16_t M68K_RW(uint32_t address)
global SYM(M68K_RW)
SYM(M68K_RW):
	; Set up the frame pointer.
	push	ebp
	mov	ebp, esp
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	and	esp, ~0x0F
%endif
	
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	sub	esp, 8
%endif
	
	; Call the function.
	mov	eax, [ebp + arg_1]
	push	eax
	call	SYM(Gens_M68K_RW)
%ifdef __FORCE_STACK_ALIGNMENT
	add	esp, 12
%else
	add	esp, 4
%endif
	
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	
	; Reset the frame pointer.
	mov	esp, ebp
	pop	ebp
	ret

; void M68K_WB(uint32_t address, uint8_t data)
global SYM(M68K_WB)
SYM(M68K_WB):
	; Set up the frame pointer.
	push	ebp
	mov	ebp, esp
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	and	esp, ~0x0F
%endif
	pushad
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	sub	esp, 8
%endif
	
	; Call the function.
	mov	eax, [ebp + arg_1]
	mov	edx, [ebp + arg_2]
	push	edx
	push	eax
	call	SYM(Gens_M68K_WB)
%ifdef __FORCE_STACK_ALIGNMENT
	add	esp, 16
%else
	add	esp, 8
%endif
	
	; Reset the frame pointer.
	popad
	mov	esp, ebp
	pop	ebp
	ret

; void M68K_WW(uint32_t address, uint16_t data)
global SYM(M68K_WW)
SYM(M68K_WW):
	; Set up the frame pointer.
	push	ebp
	mov	ebp, esp
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	and	esp, ~0x0F
%endif
	pushad
%ifdef __FORCE_STACK_ALIGNMENT
	; Enforce 16-byte stack alignment.
	sub	esp, 8
%endif
	
	; Call the function.
	mov	eax, [ebp + arg_1]
	mov	edx, [ebp + arg_2]
	push	edx
	push	eax
	call	SYM(Gens_M68K_WW)
%ifdef __FORCE_STACK_ALIGNMENT
	add	esp, 16
%else
	add	esp, 8
%endif
	
	; Reset the frame pointer.
	popad
	mov	esp, ebp
	pop	ebp
	ret
