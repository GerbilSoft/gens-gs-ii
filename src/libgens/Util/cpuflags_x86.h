/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * cpuflags_x86.h: i386/amd64 internal CPU flag definitions and functions. *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBGENS_UTIL_CPUFLAGS_X86_H__
#define __LIBGENS_UTIL_CPUFLAGS_X86_H__

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

/**
 * Check if CPUID is supported on this CPU.
 * @return 0 if not supported; non-zero if supported.
 */
static __inline int is_cpuid_supported(void)
{
	int __eax;
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
		: "=a" (__eax)	// Output
		:		// Input
		: "edx", "cc"	// Clobber
		);

	return __eax;
#else
	// AMD64. CPUID is always supported.
	return 1;
#endif	
}

#endif /* defined(__i386__) || defined(__amd64__) */

#endif /* __LIBGENS_UTIL_CPUFLAGS_X86_H__ */
