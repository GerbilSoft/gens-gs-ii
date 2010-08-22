/***************************************************************************
 * Gens: CPU Flags.                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

// MDP CPU flag definitions.
// TODO: Switch to MDP!
//#include "mdp/mdp_cpuflags.h"

// IA32 CPU flags
// Intel: http://download.intel.com/design/processor/applnots/24161832.pdf
// AMD: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf

// CR0.EM: FPU emulation.
#define IA32_CR0_EM (1 << 2)

// CPUID function 1: Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EDX_MMX		((uint32_t)(1 << 23))
#define CPUFLAG_IA32_EDX_FXSAVE		((uint32_t)(1 << 24))
#define CPUFLAG_IA32_EDX_SSE		((uint32_t)(1 << 25))
#define CPUFLAG_IA32_EDX_SSE2		((uint32_t)(1 << 26))

// Flags stored in the ecx register.
#define CPUFLAG_IA32_ECX_SSE3		((uint32_t)(1 << 0))
#define CPUFLAG_IA32_ECX_SSSE3		((uint32_t)(1 << 9))
#define CPUFLAG_IA32_ECX_SSE41		((uint32_t)(1 << 19))
#define CPUFLAG_IA32_ECX_SSE42		((uint32_t)(1 << 20))

// CPUID function 0x80000001: Extended Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EXT_EDX_MMXEXT	((uint32_t)(1 << 22))
#define CPUFLAG_IA32_EXT_EDX_3DNOW	((uint32_t)(1 << 31))
#define CPUFLAG_IA32_EXT_EDX_3DNOWEXT	((uint32_t)(1 << 30))

// Flags stored in the ecx register.
#define CPUFLAG_IA32_EXT_ECX_SSE4A	((uint32_t)(1 << 6))

// CPUID functions.
#define CPUID_MAX_FUNCTIONS		((uint32_t)(0x00000000))
#define CPUID_FAMILY_FEATURES		((uint32_t)(0x00000001))
#define CPUID_MAX_EXT_FUNCTIONS		((uint32_t)(0x80000000))
#define CPUID_EXT_FAMILY_FEATURES	((uint32_t)(0x80000001))

// CPUID macro with PIC support.
// See http://gcc.gnu.org/ml/gcc-patches/2007-09/msg00324.html
#if defined(__i386__) && defined(__PIC__)
#define __cpuid(level, a, b, c, d)				\
	__asm__ (						\
		"xchgl	%%ebx, %1\n"				\
		"cpuid\n"					\
		"xchgl	%%ebx, %1\n"				\
		: "=a" (a), "=r" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		)
#else
#define __cpuid(level, a, b, c, d)				\
	__asm__ (						\
		"cpuid\n"					\
		: "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		)
#endif

// CPU flags.
uint32_t CPU_Flags = 0;

/**
 * LibGens_GetCPUFlags(): Get the CPU flags.
 * Stores the CPU flags in the global variable CPU_Flags.
 * @return CPU flags.
 */
#include <stdio.h>
uint32_t LibGens_GetCPUFlags(void)
{
#if defined(__i386__) || defined(__amd64__)
	// IA32/x86_64.
	
	// Check if cpuid is supported.
	unsigned int _eax, _ebx, _ecx, _edx;
	
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
		:	"=a" (_eax)	// Output
		);
	
	if (!_eax)
	{
		// CPUID is not supported.
		// This CPU must be an early 486 or older.
		return 0;
	}
#endif
	
	// CPUID is supported.
	// Check if the CPUID Features function (Function 1) is supported.
	unsigned int maxFunc;
	__cpuid(CPUID_MAX_FUNCTIONS, maxFunc, _ebx, _ecx, _edx);
	
	if (!maxFunc)
	{
		// No CPUID functions are supported.
		return 0;
	}
	
	// Get the CPU feature flags.
	__cpuid(CPUID_FAMILY_FEATURES, _eax, _ebx, _ecx, _edx);
	
	// Check the feature flags.
	CPU_Flags = 0;
	
	if (_edx & CPUFLAG_IA32_EDX_MMX)
		CPU_Flags |= MDP_CPUFLAG_X86_MMX;
	
	int can_FXSAVE = 0;
	
	if (_edx & CPUFLAG_IA32_EDX_SSE)
	{
		// Check if this CPU supports FXSAVE with SSE.
		if (_edx & CPUFLAG_IA32_EDX_FXSAVE)
		{
			// CPU supports FXSAVE. Does the OS?
			// TODO: smsw causes problems with Valgrind.
			unsigned int smsw;
#if 0
			__asm__ (
				"smsw	%0"
				:	"=r" (smsw)
				);
#endif
			smsw = 0;
			if (!(smsw & IA32_CR0_EM))
			{
				// FPU emulation is disabled. This CPU supports FXSAVE with SSE.
				can_FXSAVE = 1;
			}
		}
		
		if (can_FXSAVE)
		{
			CPU_Flags |= MDP_CPUFLAG_X86_SSE;
			
			// MMXext is a subset of SSE.
			// See http://www.x86-64.org/pipermail/patches/2005-March/003261.html
			CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;
		}
	}
	
	if (can_FXSAVE)
	{
		if (_edx & CPUFLAG_IA32_EDX_SSE2)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE2;
		if (_ecx & CPUFLAG_IA32_ECX_SSE3)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE3;
		if (_ecx & CPUFLAG_IA32_ECX_SSSE3)
			CPU_Flags |= MDP_CPUFLAG_X86_SSSE3;
		if (_ecx & CPUFLAG_IA32_ECX_SSE41)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE41;
		if (_ecx & CPUFLAG_IA32_ECX_SSE42)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE42;
	}
	
	// Check if the CPUID Extended Features function (Function 0x80000001) is supported.
	__cpuid(CPUID_MAX_EXT_FUNCTIONS, maxFunc, _ebx, _ecx, _edx);
	if (maxFunc >= CPUID_EXT_FAMILY_FEATURES)
	{
		// CPUID Extended Features are supported.
		__cpuid(CPUID_EXT_FAMILY_FEATURES, _eax, _ebx, _ecx, _edx);
		
		// Check the extended feature flags.
		if (_edx & CPUFLAG_IA32_EXT_EDX_MMXEXT)
			CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOW)
			CPU_Flags |= MDP_CPUFLAG_X86_3DNOW;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOWEXT)
			CPU_Flags |= MDP_CPUFLAG_X86_3DNOWEXT;
		if (can_FXSAVE && (_ecx & CPUFLAG_IA32_EXT_ECX_SSE4A))
			CPU_Flags |= MDP_CPUFLAG_X86_SSE4A;
	}
	
	// Return the CPU flags.
	return CPU_Flags;
	
#else
	// No flags for this CPU.
	return 0;
#endif
}
