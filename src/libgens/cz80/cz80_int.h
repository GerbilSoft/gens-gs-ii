/********************************************************************************/
/*                                                                              */
/* CZ80: internal definitions                                                   */
/* C Z80 emulator version 0.91 	                                                */
/* Copyright 2004 Stephane Dallongeville                                        */
/*                                                                              */
/* Split from cz80.h for Gens/GS II.                                            */
/*                                                                              */
/********************************************************************************/

#ifndef _CZ80_INT_H_
#define _CZ80_INT_H_

#include "cz80_conf.h"

// use zR8 for B/C/D/E/H/L registers only
// use zR16 for BC/DE/HL registers only

#if CZ80_LITTLE_ENDIAN
#define zR8(A)          CPU->creg.r8[(A) ^ 1]
#else
#define zR8(A)          CPU->creg.r8[(A)]
#endif
#define zR16(A)         CPU->creg.r16[A].W
#define pzR16(A)        &(CPU->creg.r16[A])

#define pzFA            &(CPU->creg.name.FA)
#define zFA             CPU->creg.name.FA.W
#define zlFA            CPU->creg.name.FA.B.L
#define zhFA            CPU->creg.name.FA.B.H
#define zA              zlFA
#define zF              zhFA

#define pzBC            &(CPU->creg.name.BC)
#define zBC             CPU->creg.name.BC.W
#define zlBC            CPU->creg.name.BC.B.L
#define zhBC            CPU->creg.name.BC.B.H
#define zB              zhBC
#define zC              zlBC

#define pzDE            &(CPU->creg.name.DE)
#define zDE             CPU->creg.name.DE.W
#define zlDE            CPU->creg.name.DE.B.L
#define zhDE            CPU->creg.name.DE.B.H
#define zD              zhDE
#define zE              zlDE

#define pzHL            &(CPU->creg.name.HL)
#define zHL             CPU->creg.name.HL.W
#define zlHL            CPU->creg.name.HL.B.L
#define zhHL            CPU->creg.name.HL.B.H
#define zH              zhHL
#define zL              zlHL

#define zFA2            CPU->FA2.W
#define zlFA2           CPU->FA2.B.L
#define zhFA2           CPU->FA2.B.H
#define zA2             zlFA2
#define zF2             zhFA2

#define zBC2            CPU->BC2.W
#define zDE2            CPU->DE2.W
#define zHL2            CPU->HL2.W

#define pzIX            &(CPU->IX)
#define zIX             CPU->IX.W
#define zlIX            CPU->IX.B.L
#define zhIX            CPU->IX.B.H

#define pzIY            &(CPU->IY)
#define zIY             CPU->IY.W
#define zlIY            CPU->IY.B.L
#define zhIY            CPU->IY.B.H

#define pzSP            &(CPU->SP)
#define zSP             CPU->SP.W
#define zlSP            CPU->SP.B.L
#define zhSP            CPU->SP.B.H

// WZ register. Internal register used for address calculations.
// This register must be emulated correctly in order to emulate
// flag bits 3 and 5 for the BIT n,(HL) instruction.
// NOTE: Stored as local variable for performance reasons.
#define zWZ		WZ

// Program Counter.
// NOTE: Stored as local variable for performance reasons.
#define zPC             PC

#define zI              CPU->I
#define zIM             CPU->IM

// R register.
// For performance reasons, the high bit is stored
// in zR2, since the Z80 only modifies the low 7 bits
// during the refresh cycle.
#define zwR             CPU->R.W
#define zR1             CPU->R.B.L
#define zR2             CPU->R.B.H
#define zR              zR1

#define zIFF            CPU->IFF.W
#define zIFF1           CPU->IFF.B.L
#define zIFF2           CPU->IFF.B.H

#endif /* _CZ80_INT_H_ */
