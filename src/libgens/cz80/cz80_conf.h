/********************************************************************************/
/*                                                                              */
/* CZ80: configuration file                                                     */
/* C Z80 emulator version 0.91 	                                                */
/* Copyright 2004 Stephane Dallongeville                                        */
/*                                                                              */
/* Split from cz80.h for Gens/GS II.                                            */
/*                                                                              */
/********************************************************************************/

#ifndef _CZ80_CONF_H_
#define _CZ80_CONF_H_

// Emulator Configuration

// TODO: Set this somewhere.
#ifdef WORDS_BIGENDIAN
#define CZ80_LITTLE_ENDIAN	0
#else
#define CZ80_LITTLE_ENDIAN      1
#endif

// Size of fetch banks.
// Default is 4 (16 banks of 4 KB)
// Gens/GS II uses 6 (64 banks of 1 KB)
#define CZ80_FETCH_BITS         6	// [4-12]   default = 8
#define CZ80_FETCH_SFT          (16 - CZ80_FETCH_BITS)
#define CZ80_FETCH_BANK         (1 << CZ80_FETCH_BITS)

// Optimize for size instead of speed.
#define CZ80_SIZE_OPT           0

// Use separate functions for 16-bit accesses.
// May improve performance.
#define CZ80_USE_WORD_HANDLER   0

// TODO: Remove this.
#define CZ80_DEBUG              0

// Emulate the YF and XF flags exactly.
// This also enables proper WZ register emulation.
#define CZ80_EXACT              1

// Emulate the "R" register exactly.
// Reference: http://www.z80.info/zip/z80-documented.pdf
#define CZ80_EXACT_R            1

#endif /* _CZ80_CONF_H_ */
