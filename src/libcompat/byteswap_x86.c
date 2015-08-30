/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * byteswap_x86.c: Byteswapping functions. (i386/amd64 optimized)          *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth                                  *
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

#if !defined(__i386__) && !defined(__amd64__) && \
    !defined(_M_IX86) && !defined(_M_X64)
#error Do not compile byteswap_x86.c on non-x86 CPUs!
#endif

#include "byteswap.h"
#include "cpuflags.h"

// C includes.
#include <assert.h>

#ifdef _MSC_VER
#define inline __inline
#endif

/**
 * Byteswap two 16-bit WORDs in a 32-bit DWORD.
 * @param dword DWORD containing two 16-bit WORDs.
 * @return DWORD containing the byteswapped 16-bit WORDs.
 */
static inline uint32_t swap_two_16_in_32(uint32_t dword)
{
	// NOTE: The 'rol 8' version was slightly faster on
	// Core 2 T7200 in 64-bit mode when compiled with -O2;
	// in 32-bit mode, the swap_two_16_in_32() version
	// is always faster.
	uint32_t tmp1 = (dword >> 8) & 0x00FF00FF;
	uint32_t tmp2 = (dword << 8) & 0xFF00FF00;
	return (tmp1 | tmp2);
}

/**
 * 16-bit byteswap function.
 * @param ptr Pointer to array to swap. (MUST be 16-bit aligned!)
 * @param n Number of bytes to swap. (Must be divisible by 2; an extra odd byte will be ignored.)
 */
void __byte_swap_16_array(uint16_t *ptr, unsigned int n)
{
	uint32_t *dwptr;

	// Verify the block is 16-bit aligned
	// and is a multiple of 2 bytes.
	assert(((uintptr_t)ptr & 1) == 0);
	assert((n & 1) == 0);
	n &= ~1;

	// TODO: Don't bother with MMX or SSE2
	// if n is below a certain size?

#if defined(__GNUC__)
	if (CPU_Flags & MDP_CPUFLAG_X86_SSE2) {
		// If wptr isn't 16-byte aligned, swap words
		// manually until we get to 16-byte alignment.
		for (; ((uintptr_t)ptr % 16 != 0) && n > 0;
		     n -= 2, ptr++)
		{
			*ptr = __swab16(*ptr);
		}

		// SSE2: Swap 16 bytes (8 words) at a time.
		for (; n >= 16; n -= 16, ptr += 8) {
			__asm__ (
				"movdqa	(%[ptr]), %%xmm0\n"
				"movdqa	%%xmm0, %%xmm1\n"
				"psllw	$8, %%xmm0\n"
				"psrlw	$8, %%xmm1\n"
				"por	%%xmm0, %%xmm1\n"
				"movdqa	%%xmm1, (%[ptr])\n"
				:
				: [ptr] "r" (ptr)
				// FIXME: gcc complains xmm? registers are unknown.
				// May need to compile with -msse...
				//: "xmm0", "xmm1"
			);
		}

		// If the block isn't a multiple of 8 bytes,
		// the C implementation will handle the rest.
	} else if (CPU_Flags & MDP_CPUFLAG_X86_MMX) {
		// MMX: Swap 8 bytes (4 words) at a time.
		for (; n >= 8; n -= 8, ptr += 4) {
			__asm__ (
				"movq	(%[ptr]), %%mm0\n"
				"movq	%%mm0, %%mm1\n"
				"psllw	$8, %%mm0\n"
				"psrlw	$8, %%mm1\n"
				"por	%%mm0, %%mm1\n"
				"movq	%%mm1, (%[ptr])\n"
				:
				: [ptr] "r" (ptr)
				// FIXME: gcc complains mm? registers are unknown.
				// May need to compile with -mmmx...
				//: "mm0", "mm1"
			);
		}

		// Reset the FPU state.
		__asm__ ("emms");

		// If the block isn't a multiple of 8 bytes,
		// the C implementation will handle the rest.
	}
#endif /* defined(__GNUC__) */

	// C version. Used if optimized asm isn't available,
	// or if we have a block that isn't a multiple of
	// 16 (SSE2) or 8 (MMX) bytes.

	// Process 8 WORDs per iteration,
	// using 32-bit accesses.
	dwptr = (uint32_t*)ptr;
	for (; n >= 16; n -= 16, dwptr += 4) {
		*(dwptr+0) = swap_two_16_in_32(*(dwptr+0));
		*(dwptr+1) = swap_two_16_in_32(*(dwptr+1));
		*(dwptr+2) = swap_two_16_in_32(*(dwptr+2));
		*(dwptr+3) = swap_two_16_in_32(*(dwptr+3));
	}
	ptr = (uint16_t*)dwptr;

	// Process remaining WORDs.
	for (; n > 0; n -= 2, ptr++) {
		*ptr = __swab16(*ptr);
	}	
}

/**
 * 32-bit byteswap function.
 * @param ptr Pointer to array to swap. (MUST be 32-bit aligned!)
 * @param n Number of bytes to swap. (Must be divisible by 4; extra bytes will be ignored.)
 */
void __byte_swap_32_array(uint32_t *ptr, unsigned int n)
{
	// Verify the block is 32-bit aligned
	// and is a multiple of 4 bytes.
	assert(((uintptr_t)ptr & 3) == 0);
	assert((n & 3) == 0);
	n &= ~3;

	// Process 4 DWORDs per iteration.
	for (; n >= 16; n -= 16, ptr += 4) {
		*(ptr+0) = __swab32(*(ptr+0));
		*(ptr+1) = __swab32(*(ptr+1));
		*(ptr+2) = __swab32(*(ptr+2));
		*(ptr+3) = __swab32(*(ptr+3));
	}

	// Process remaining DWORDs.
	for (; n > 0; n -= 4, ptr++) {
		*ptr = __swab32(*ptr);
	}
}
