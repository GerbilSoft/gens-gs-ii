/********************************************************************************/
/*                                                                              */
/* CZ80: byteorder definitions                                                  */
/* C Z80 emulator version 0.91 	                                                */
/* Copyright 2004 Stephane Dallongeville                                        */
/*                                                                              */
/* Copied from libcompat/byteorder.h from Gens/GS II.                           */
/*                                                                              */
/********************************************************************************/

#ifndef _CZ80_BYTEORDER_H_
#define _CZ80_BYTEORDER_H_

#if (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || \
    defined(__hppa__) || defined(__HPPA__) || \
    defined(__m68k__) || defined(__MC68K__) || defined(_M_M68K) || \
    defined(mc68000) || defined(M68000) || \
    ((defined(__mips) || defined(__mips__) || defined(__mips) || defined(__MIPS__)) && \
     (defined(__mipseb) || defined(__mipseb__) || defined(__MIPSEB) || defined(__MIPSEB__))) || \
    defined(__powerpc__) || defined(__POWERPC__) || \
    defined(__ppc__) || defined(__PPC__) || defined(_M_PPC) || \
    defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
    defined(__sparc) || defined(__sparc__)

/* System is big-endian. */
#define CZ80_LITTLE_ENDIAN 0

#else

/* System is little-endian. */
// TODO: Set this somewhere.
#define CZ80_LITTLE_ENDIAN 1

#endif

#endif /* _CZ80_BYTEORDER_H_ */