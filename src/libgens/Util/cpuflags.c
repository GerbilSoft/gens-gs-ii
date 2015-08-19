/***************************************************************************
 * Gens: CPU Flags.                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
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

#include "cpuflags.h"

// C includes.
#include <stdint.h>

// CPU flags.
uint32_t CPU_Flags = 0;

// MDP CPU flag definitions.
// TODO: Switch to MDP!
//#include "mdp/mdp_cpuflags.h"

#if defined(__i386__) || defined(__amd64__)
// IA32 CPU flags
// References:
// - Intel: http://download.intel.com/design/processor/applnots/24161832.pdf
// - AMD: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
// - Wikipedia:
//   - https://en.wikipedia.org/wiki/CPUID
//   - https://en.wikipedia.org/wiki/Control_register

// CR0.EM: FPU emulation.
#define IA32_CR0_EM		(1 << 2)

// CPUID function 1: Processor Info and Feature Bits

// Flags stored in the %edx register.
#define CPUFLAG_IA32_EDX_MMX		((uint32_t)(1 << 23))
#define CPUFLAG_IA32_EDX_FXSAVE		((uint32_t)(1 << 24))
#define CPUFLAG_IA32_EDX_SSE		((uint32_t)(1 << 25))
#define CPUFLAG_IA32_EDX_SSE2		((uint32_t)(1 << 26))

// Flags stored in the %ecx register.
#define CPUFLAG_IA32_ECX_SSE3		((uint32_t)(1 << 0))
#define CPUFLAG_IA32_ECX_SSSE3		((uint32_t)(1 << 9))
#define CPUFLAG_IA32_ECX_SSE41		((uint32_t)(1 << 19))
#define CPUFLAG_IA32_ECX_SSE42		((uint32_t)(1 << 20))
#define CPUFLAG_IA32_ECX_XSAVE		((uint32_t)(1 << 26))
#define CPUFLAG_IA32_ECX_OSXSAVE	((uint32_t)(1 << 27))
#define CPUFLAG_IA32_ECX_AVX		((uint32_t)(1 << 28))
#define CPUFLAG_IA32_ECX_FMA3		((uint32_t)(1 << 12))

// CPUID function 7: Extended Features

// Flags stored in the %ebx register.
#define CPUFLAG_IA32_FN7_EBX_AVX2	((uint32_t)(1 << 5))

// CPUID function 0x80000001: Extended Processor Info and Feature Bits

// Flags stored in the %edx register.
#define CPUFLAG_IA32_EXT_EDX_MMXEXT	((uint32_t)(1 << 22))
#define CPUFLAG_IA32_EXT_EDX_3DNOW	((uint32_t)(1 << 31))
#define CPUFLAG_IA32_EXT_EDX_3DNOWEXT	((uint32_t)(1 << 30))

// Flags stored in the %ecx register.
#define CPUFLAG_IA32_EXT_ECX_SSE4A	((uint32_t)(1 << 6))
#define CPUFLAG_IA32_EXT_ECX_F16C	((uint32_t)(1 << 29))
#define CPUFLAG_IA32_EXT_ECX_XOP	((uint32_t)(1 << 11))
#define CPUFLAG_IA32_EXT_ECX_FMA4	((uint32_t)(1 << 16))

// CPUID functions.
#define CPUID_MAX_FUNCTIONS			((uint32_t)(0x00000000))
#define CPUID_PROC_INFO_FEATURE_BITS		((uint32_t)(0x00000001))
#define CPUID_EXT_FEATURES			((uint32_t)(0x00000007))
#define CPUID_MAX_EXT_FUNCTIONS			((uint32_t)(0x80000000))
#define CPUID_EXT_PROC_INFO_FEATURE_BITS	((uint32_t)(0x80000001))

// CPUID macro with PIC support.
// See http://gcc.gnu.org/ml/gcc-patches/2007-09/msg00324.html
#if defined(__i386__) && defined(__PIC__)
#define __cpuid(level, a, b, c, d) do {				\
	__asm__ (						\
		"xchgl	%%ebx, %1\n"				\
		"cpuid\n"					\
		"xchgl	%%ebx, %1\n"				\
		: "=a" (a), "=r" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		);						\
	} while (0)
#else
#define __cpuid(level, a, b, c, d) do {				\
	__asm__ (						\
		"cpuid\n"					\
		: "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		);						\
	} while (0)
#endif

#endif /* defined(__i386__) || defined(__amd64__) */

/**
 * Get the CPU flags.
 * Stores the CPU flags in the global variable CPU_Flags.
 * @return CPU flags.
 */
uint32_t LibGens_GetCPUFlags(void)
{
#if defined(__i386__) || defined(__amd64__)
	// IA32/x86_64.

	// Check if cpuid is supported.
	unsigned int __eax, __ebx, __ecx, __edx;
	uint8_t can_FXSAVE = 0;
	uint8_t can_XSAVE = 0;

#if defined(__i386__)
	__asm__ (
		"pushfl\n"
		"popl %%eax\n"
		"movl %%eax, %%edx\n"
		"xorl $0x200000, %%eax\n"
		"pushl %%eax\n"
		"popfl\n"
		"pushfl\n"
		"popl %%eax\n"
		"xorl %%edx, %%eax\n"
		"andl $0x200000, %%eax"
		:	"=a" (__eax)	// Output
		);

	if (!__eax) {
		// CPUID is not supported.
		// This CPU must be an early 486 or older.
		return 0;
	}
#endif

	// CPUID is supported.
	// Check if the CPUID Features function (Function 1) is supported.
	unsigned int maxFunc;
	__cpuid(CPUID_MAX_FUNCTIONS, maxFunc, __ebx, __ecx, __edx);

	if (!maxFunc) {
		// No CPUID functions are supported.
		return 0;
	}

	// Get the CPU feature flags.
	__cpuid(CPUID_PROC_INFO_FEATURE_BITS, __eax, __ebx, __ecx, __edx);

	// Check the feature flags.
	CPU_Flags = 0;

	if (__edx & CPUFLAG_IA32_EDX_MMX) {
		// MMX is supported.
		CPU_Flags |= MDP_CPUFLAG_X86_MMX;
	}

#if defined(__i386__) || defined(_M_IX86)
	if (__edx & CPUFLAG_IA32_EDX_SSE) {
		// CPU reports that it supports SSE, but the OS
		// might not support FXSAVE.

		// Check if this CPU supports FXSAVE with SSE.
		if (__edx & CPUFLAG_IA32_EDX_FXSAVE) {
			// CPU supports FXSAVE.

#ifdef _WIN32
			// Windows 95 does not support SSE.
			// Windows NT 4.0 supports SSE with the appropriate driver.
			// Check if CR0.EM == 0.
			unsigned int smsw;
			__asm__ (
				"smsw	%0"
				: "=r" (smsw)
				);
			if (!(smsw & IA32_CR0_EM)) {
				// FPU emulation is disabled.
				// SSE is enabled by the OS.
				can_FXSAVE = 1;
			}
#else /* !_WIN32 */
			// For non-Windows operating systems, we'll assume
			// the OS supports SSE. Valgrind doesn't like the
			// 'smsw' instruction, so we can't do memory debugging
			// with Valgrind if we use 'smsw'.
			can_FXSAVE = 1;
#endif /* _WIN32 */
		}

		if (can_FXSAVE) {
			CPU_Flags |= MDP_CPUFLAG_X86_SSE;
			// MMXext is a subset of SSE.
			// See http://www.x86-64.org/pipermail/patches/2005-March/003261.html
			CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;
		}
	}

	// Check for other SSE instruction sets.
	if (can_FXSAVE) {
		if (__edx & CPUFLAG_IA32_EDX_SSE2)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE2;
		if (__ecx & CPUFLAG_IA32_ECX_SSE3)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE3;
		if (__ecx & CPUFLAG_IA32_ECX_SSSE3)
			CPU_Flags |= MDP_CPUFLAG_X86_SSSE3;
		if (__ecx & CPUFLAG_IA32_ECX_SSE41)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE41;
		if (__ecx & CPUFLAG_IA32_ECX_SSE42)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE42;
	}
#else /* !(defined(__i386__) || defined(_M_IX86)) */
	// AMD64: SSE2 and lower are always supported.
	can_FXSAVE = 1;
	CPU_Flags |= (MDP_CPUFLAG_X86_SSE |
		      MDP_CPUFLAG_X86_MMXEXT |
		      MDP_CPUFLAG_X86_SSE2);

	// Check for other SSE instruction sets.
	if (__ecx & CPUFLAG_IA32_ECX_SSSE3)
		CPU_Flags |= MDP_CPUFLAG_X86_SSSE3;
	if (__ecx & CPUFLAG_IA32_ECX_SSE41)
		CPU_Flags |= MDP_CPUFLAG_X86_SSE41;
	if (__ecx & CPUFLAG_IA32_ECX_SSE42)
		CPU_Flags |= MDP_CPUFLAG_X86_SSE42;
#endif /* defined(__i386__) || defined(_M_IX86) */

	// Check for XSAVE.
	if (__ecx & CPUFLAG_IA32_ECX_XSAVE) {
		// CPU supports XSAVE. Does the OS?
		/* FIXME: Detect XSAVE. */
		can_XSAVE = 0;
	}

	// Check for AVX.
	if (can_XSAVE) {
		if (__ecx & CPUFLAG_IA32_ECX_AVX)
			CPU_Flags |= MDP_CPUFLAG_X86_AVX;
		if (__ecx & CPUFLAG_IA32_ECX_FMA3)
			CPU_Flags |= MDP_CPUFLAG_X86_FMA3;
	}

	// Check if the CPUID Extended Features function (0x00000007) is supported.
	if (maxFunc >= CPUID_EXT_FEATURES) {
		// CPUID Extended Features are supported.
		__cpuid(CPUID_EXT_FEATURES, __eax, __ebx, __ecx, __edx);

		// Check the extended features.
		if (can_XSAVE) {
			if (__ebx & CPUFLAG_IA32_FN7_EBX_AVX2)
				CPU_Flags |= MDP_CPUFLAG_X86_AVX2;
		}
	}

	// Get the highest extended function supported by the CPU.
	__cpuid(CPUID_MAX_EXT_FUNCTIONS, maxFunc, __ebx, __ecx, __edx);

	// Check if the CPUID Extended Processor Info and Feature Bits function
	// (0x80000001) is supported.
	if (maxFunc >= CPUID_EXT_PROC_INFO_FEATURE_BITS) {
		// CPUID Extended Processor Info and Feature Bits are supported.
		__cpuid(CPUID_EXT_PROC_INFO_FEATURE_BITS, __eax, __ebx, __ecx, __edx);

		// Check the extended processor info and feature bits.
#if defined(__i386__) || defined(_M_IX86)
		// MMXEXT is always enabled on amd64, so
		// only check it on i386.
		if (__edx & CPUFLAG_IA32_EXT_EDX_MMXEXT)
			CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;
#endif /* defined(__i386__) || defined(_M_IX86) */
		if (__edx & CPUFLAG_IA32_EXT_EDX_3DNOW)
			CPU_Flags |= MDP_CPUFLAG_X86_3DNOW;
		if (__edx & CPUFLAG_IA32_EXT_EDX_3DNOWEXT)
			CPU_Flags |= MDP_CPUFLAG_X86_3DNOWEXT;

		// More SSE.
		if (can_FXSAVE) {
			if (__ecx & CPUFLAG_IA32_EXT_ECX_SSE4A)
				CPU_Flags |= MDP_CPUFLAG_X86_SSE4A;
			if (__ecx & CPUFLAG_IA32_EXT_ECX_F16C)
				CPU_Flags |= MDP_CPUFLAG_X86_F16C;
			if (__ecx & CPUFLAG_IA32_EXT_ECX_XOP)
				CPU_Flags |= MDP_CPUFLAG_X86_XOP;
		}

		// AVX.
		if (can_XSAVE) {
			if (__ecx & CPUFLAG_IA32_EXT_ECX_FMA4)
				CPU_Flags |= MDP_CPUFLAG_X86_FMA4;
		}
	}

	// Return the CPU flags.
	return CPU_Flags;

#else
	// No flags for this CPU.
	return 0;
#endif
}
