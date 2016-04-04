/********************************************************************************/
/*                                                                              */
/* CZ80: context definition                                                     */
/* C Z80 emulator version 0.91 	                                                */
/* Copyright 2004 Stephane Dallongeville                                        */
/*                                                                              */
/* Split from cz80.h for Gens/GS II.                                            */
/*                                                                              */
/********************************************************************************/

#ifndef _CZ80_CONTEXT_H_
#define _CZ80_CONTEXT_H_

#include "cz80_conf.h"

typedef union _union16 {
	struct {
#if CZ80_LITTLE_ENDIAN
		uint8_t L;
		uint8_t H;
#else
		uint8_t H;
		uint8_t L;
#endif
	} B;
	uint16_t W;
} union16;

typedef struct _cz80_struc {
	union {
		uint8_t r8[8];
		union16 r16[4];
		struct {
			union16 BC;
			union16 DE;
			union16 HL;
			union16 FA;
		} name;
	} creg;

	union16 IX;
	union16 IY;
	union16 SP;

#if CZ80_EXACT
	uint16_t WZ;
#endif
	uint16_t PC;

	union16 BC2;
	union16 DE2;
	union16 HL2;
	union16 FA2;

	union16 R;
	union16 IFF;

	uint8_t I;
	uint8_t IM;
	uint8_t IntVect;
	uint8_t Status;

	uintptr_t BasePC;
	int CycleIO;

	int CycleToDo;
	int CycleSup;

	void *ctx;

	CZ80_READ *Read_Byte;
	CZ80_WRITE *Write_Byte;
#if CZ80_USE_WORD_HANDLER
	CZ80_READ_WORD *Read_Word;
	CZ80_WRITE_WORD *Write_Word;
#endif

	CZ80_READ *IN_Port;
	CZ80_WRITE *OUT_Port;

	CZ80_RETI_CALLBACK *RetI;
	CZ80_INT_CALLBACK *Interrupt_Ack;

	uint8_t *Fetch[CZ80_FETCH_BANK];
} cz80_struc;

#endif /* _CZ80_CONTEXT_H_ */
