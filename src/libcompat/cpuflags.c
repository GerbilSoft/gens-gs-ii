/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * cpuflags.c: CPU flag definitions and functions.                         *
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

#include "cpuflags.h"

// C includes.
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// CPU flags.
// This variable is publicly accessible for performance reasons.
uint32_t CPU_Flags = 0;

// CPU vendor ID. (NULL-terminated)
// NOTE: x86 has 12 characters; others may have more.
static char CPU_VendorID[16] = {0};

// Full CPU name. (NULL-terminated)
// NOTE: x86 has 48 characters; others may have more.
static char CPU_FullName[64] = {0};

// CPU-specific flag definitions.
// These are for internal use by LibGens_GetCPUFlags() only.
#include "cpuflags_x86.h"

/**
 * Get the CPU flags.
 * Stores the CPU flags in the global variable CPU_Flags.
 * @return CPU flags.
 */
uint32_t LibCompat_GetCPUFlags(void)
{
#if defined(__i386__) || defined(__amd64__) || \
    defined(_M_IX86) || defined(_M_X64)
	// IA32/x86_64.

	// Check if cpuid is supported.
	unsigned int __eax, __ebx, __ecx, __edx;
	int family = 0, model = 0;
	union { uint32_t i[3]; char c[12]; } vendor;
	unsigned int maxFunc;
	uint8_t can_FXSAVE = 0;
	uint8_t can_XSAVE = 0;

	if (CPU_Flags != 0) {
		// CPU_Flags was already set.
		// NOTE: We're assuming the case where CPU_Flags == 0
		// after initialization is rare.
		return CPU_Flags;
	}

	// Clear CPU flags, vendor ID, and full name.
	CPU_Flags = 0;
	CPU_VendorID[0] = 0;
	CPU_FullName[0] = 0;

	if (!is_cpuid_supported()) {
		// CPUID is not supported.
		// This CPU must be an early 486 or older.
		return 0;
	}
	// CPUID is supported.
	// Check if the CPUID Features function (Function 1) is supported.
	// This also retrieves the CPU vendor string.
	CPUID(CPUID_MAX_FUNCTIONS, maxFunc, vendor.i[0], vendor.i[2], vendor.i[1]);
	if (!maxFunc) {
		// No CPUID functions are supported.
		return 0;
	}

	// Save the vendor string.
	memcpy(CPU_VendorID, vendor.c, sizeof(vendor.c));
	CPU_VendorID[12] = 0;

	// Get the CPU feature flags.
	CPUID(CPUID_PROC_INFO_FEATURE_BITS, __eax, __ebx, __ecx, __edx);

	// Get the CPU family and model.
	family = (__eax >> 8) & 0xF;
	if (family == 15) {
		family += ((__eax >> 20) & 0xFF);
	}
	model = (__eax >> 4) & 0xF;
	if (family == 6 || family == 15) {
		model += ((__eax >> 12) & 0xF0);
	}

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
			// Windows NT 4.0 supports SSE if the
			// appropriate driver is installed.

			// Check if CR0.EM == 0.
			unsigned int __smsw;
#if defined(__GNUC__)
			__asm__ (
				"smsw	%0"
				: "=r" (__smsw)
				);
#elif defined(_MSC_VER)
			// TODO: Optimize the MSVC version to
			// not use the stack?
			__asm {
				smsw	__smsw
			}
#else
#error Missing 'smsw' asm implementation for this compiler.
#endif
			if (!(__smsw & IA32_CR0_EM)) {
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
	}

	// Check for other SSE instruction sets.
	if (can_FXSAVE) {
		CPU_Flags |= MDP_CPUFLAG_X86_SSE;
		// MMXext is a subset of SSE.
		// See http://www.x86-64.org/pipermail/patches/2005-March/003261.html
		CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;

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
		CPUID(CPUID_EXT_FEATURES, __eax, __ebx, __ecx, __edx);

		// Check the extended features.
		if (can_XSAVE) {
			if (__ebx & CPUFLAG_IA32_FN7_EBX_AVX2)
				CPU_Flags |= MDP_CPUFLAG_X86_AVX2;
		}
	}

	// Get the highest extended function supported by the CPU.
	CPUID(CPUID_MAX_EXT_FUNCTIONS, maxFunc, __ebx, __ecx, __edx);

	// Check if the CPUID Extended Processor Info and Feature Bits function
	// (0x80000001) is supported.
	if (maxFunc >= CPUID_EXT_PROC_INFO_FEATURE_BITS) {
		// CPUID Extended Processor Info and Feature Bits are supported.
		CPUID(CPUID_EXT_PROC_INFO_FEATURE_BITS, __eax, __ebx, __ecx, __edx);

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

	// Check if the CPUID Processor Brand String functions
	// (0x80000002, 0x80000003, 0x80000004) are supported.
	if (maxFunc >= CPUID_EXT_PROC_BRAND_STRING_3) {
		// CPUID Processor Brand String functions are supported.
		// TODO: Generic string for unsupported CPUs?
		// TODO: Space elimination algorithm?
		union { uint32_t i[12]; char c[48]; } brand_string;
		CPUID(CPUID_EXT_PROC_BRAND_STRING_1,
			brand_string.i[0], brand_string.i[1],
			brand_string.i[2], brand_string.i[3]);
		CPUID(CPUID_EXT_PROC_BRAND_STRING_2,
			brand_string.i[4], brand_string.i[5],
			brand_string.i[6], brand_string.i[7]);
		CPUID(CPUID_EXT_PROC_BRAND_STRING_3,
			brand_string.i[8], brand_string.i[9],
			brand_string.i[10], brand_string.i[11]);

		// Simple space elimination algorithm.
		// We can't use libgenstext here because:
		// 1. libcompat shouldn't depend on other Gens/GS II libraries.
		// 2. StringManip is C++; this is C.
		char *cpuFullName = &CPU_FullName[0];
		int wasLastSpace = 1;
		for (const char *brandChr = &brand_string.c[0];
		     brandChr < &brand_string.c[48]; brandChr++)
		{
			if (isspace(*brandChr)) {
				if (!wasLastSpace) {
					// Last character was not a space.
					// Append this character, then mark
					// the last character as a space.
					*cpuFullName++ = *brandChr;
					wasLastSpace = 1;
				}
			} else {
				// This character is not a space.
				*cpuFullName++ = *brandChr;
				wasLastSpace = 0;
			}
		}

		// Make sure the string is NULL-terminated.
		*cpuFullName = 0;

		// Check for any trailing spaces.
		for (cpuFullName--; cpuFullName >= &CPU_FullName[0]; cpuFullName--) {
			if (isspace(*cpuFullName)) {
				// Found a trailing space.
				*cpuFullName = 0;
			} else {
				// Not a trailing space.
				// We're done here.
				break;
			}
		}
	}

	// Check for SSE2SLOW, SSE3SLOW, and AVXSLOW.
	// These require vendor-specific checks.
	// Based on ffmpeg's libavutil/x86/cpu.c:
	// - https://github.com/FFmpeg/FFmpeg/blob/7206b94fb893c63b187bcdfe26422b4e026a3ea0/libavutil/x86/cpu.c
	if (!strncmp(vendor.c, "AuthenticAMD", 12)) {
		// AMD CPUs that support SSE2 but don't support SSE4a
		// typically perform faster using MMX, SSE, or 3DNow!
		// instructions than SSE2. This includes Athlon 64,
		// some Opteron, and some Sempron processors.
		// Note that SSE2 *can* be fast on these CPUs in some
		// cases. Benchmarks are required to determine this.
		if ((CPU_Flags & MDP_CPUFLAG_X86_SSE2) &&
		    !(CPU_Flags & MDP_CPUFLAG_X86_SSE4A))
		{
			CPU_Flags |= MDP_CPUFLAG_X86_SSE2SLOW;
		}

		// Similarly, AMD's Bulldozer CPUs support AVX, but they
		// don't have 256-bit execution units, so 128-bit SSE
		// instructions are usually faster.
		if (family == 0x15 && (CPU_Flags & MDP_CPUFLAG_X86_AVX)) {
			CPU_Flags |= MDP_CPUFLAG_X86_AVXSLOW;
		}
	} else if (!strncmp(vendor.c, "GenuineIntel", 12)) {
		if (family == 6 && (model == 9 || model == 13 || model == 14)) {
			// Pentium M and Core 1 CPUs support SSE2, but it's almost
			// always slower than MMX. As such, we're going to disable
			// the SSE2 flag while marking SSE2 as "slow". This is what
			// FFmpeg does. Note that for the AMD cases above, SSE2 is
			// left enabled, since SSE2 can be fast in some cases.
			// Same for SSE3 on Core 1.
			// https://ffmpeg.org/pipermail/ffmpeg-devel/2011-February/102911.html
			// https://github.com/FFmpeg/FFmpeg/blob/440fa7758b687bbb0007f85497eed8bb9aec96bd/libavutil/cpu.h
			// https://github.com/FFmpeg/FFmpeg/blob/7206b94fb893c63b187bcdfe26422b4e026a3ea0/libavutil/x86/cpu.c
			if (CPU_Flags & MDP_CPUFLAG_X86_SSE2)
				CPU_Flags ^= (MDP_CPUFLAG_X86_SSE2 | MDP_CPUFLAG_X86_SSE2SLOW);
			if (CPU_Flags & MDP_CPUFLAG_X86_SSE3)
				CPU_Flags ^= (MDP_CPUFLAG_X86_SSE3 | MDP_CPUFLAG_X86_SSE3SLOW);
		}

		if (family == 6 && model == 28) {
			// Intel Atom supports SSSE3, but it's usually slower than
			// the equivalent SSE2 code.
			CPU_Flags |= MDP_CPUFLAG_X86_ATOM;
		}
	}

	// Return the CPU flags.
	return CPU_Flags;

#else
	// No flags for this CPU.
	CPU_Flags = 0;
	return 0;
#endif
}

/**
 * Get the CPU vendor ID.
 * Equivalent to the 12-char vendor ID on x86.
 * @return Pointer to CPU vendor ID (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUVendorID(void)
{
	if (CPU_Flags == 0) {
		// CPU flags might not have been obtained yet.
		LibCompat_GetCPUFlags();
	}

	if (!CPU_VendorID[0]) {
		// No vendor ID.
		return NULL;
	}

	return CPU_VendorID;
}

/**
 * Get the full CPU name.
 * Equivalent to the "brand string" on x86.
 * @return Pointer to the full CPU name (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUFullName(void)
{
	if (CPU_Flags == 0) {
		// CPU flags might not have been obtained yet.
		LibCompat_GetCPUFlags();
	}

	if (!CPU_FullName[0]) {
		// No full name.
		return NULL;
	}

	return CPU_FullName;
}
