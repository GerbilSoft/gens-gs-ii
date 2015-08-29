/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * cpuflags.h: CPU flag definitions and functions.                         *
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

#ifndef __LIBCOMPAT_CPUFLAGS_H__
#define __LIBCOMPAT_CPUFLAGS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// NOTE: This MUST be kept in sync with MDP's mdp_cpuflags.h.
// We're not adding a dependency on MDP to libcompat, since
// that doesn't make sense.

// Prevent conflicts with MDP's mdp_cpuflags.h.
// If mdp_cpuflags.h is included first, those definitions
// will be preferred. Note that this can cause compile errors
// if either version gets out of sync.
#ifndef __MDP_CPUFLAGS_H
#define __MDP_CPUFLAGS_H

/* CPU flags (IA32/x86_64) */
#if defined(__i386__) || defined(__amd64__) || \
    defined(_M_IX86) || defined(_M_X64)

/*! BEGIN: MDP v1.0 CPU flags. !*/
#define MDP_CPUFLAG_X86_MMX		((uint32_t)(1 << 0))
#define MDP_CPUFLAG_X86_MMXEXT		((uint32_t)(1 << 1))	/* AMD only */
#define MDP_CPUFLAG_X86_3DNOW		((uint32_t)(1 << 2))	/* AMD only */
#define MDP_CPUFLAG_X86_3DNOWEXT	((uint32_t)(1 << 3))	/* AMD only */
#define MDP_CPUFLAG_X86_SSE		((uint32_t)(1 << 4))
#define MDP_CPUFLAG_X86_SSE2		((uint32_t)(1 << 5))
#define MDP_CPUFLAG_X86_SSE3		((uint32_t)(1 << 6))
#define MDP_CPUFLAG_X86_SSSE3		((uint32_t)(1 << 7))
#define MDP_CPUFLAG_X86_SSE41		((uint32_t)(1 << 8))
#define MDP_CPUFLAG_X86_SSE42		((uint32_t)(1 << 9))
#define MDP_CPUFLAG_X86_SSE4A		((uint32_t)(1 << 10))
/*! END: MDP v1.0 CPU flags. !*/
/*! BEGIN: MDP v1.x CPU flags. (TODO: Add to MDP.) */
#define MDP_CPUFLAG_X86_AVX		((uint32_t)(1 << 11))
#define MDP_CPUFLAG_X86_F16C		((uint32_t)(1 << 12))	/* AMD only */
#define MDP_CPUFLAG_X86_XOP		((uint32_t)(1 << 13))	/* AMD only */
#define MDP_CPUFLAG_X86_FMA4		((uint32_t)(1 << 14))	/* AMD only */
#define MDP_CPUFLAG_X86_FMA3		((uint32_t)(1 << 15))
#define MDP_CPUFLAG_X86_AVX2		((uint32_t)(1 << 16))
// TODO: Bit manipulation instructions?
// NOTE: Some implementations of certain instruction sets
// may be slower on older CPUs, e.g. SSE2 on Core 1.
// - If SSE2 is set but SSE2SLOW is not set, SSE2 is fast.
// - If SSE2 is set and SSE2SLOW is set, SSE2 is usually okay,
//   but may be slow in certain cases. (Benchmarking is needed.)
// - If SSE2 is not set and SSE2SLOW is set, SSE2 is almost
//   always slower and should not be used.
#define MDP_CPUFLAG_X86_SSE2SLOW	((uint32_t)(1 << 31))
#define MDP_CPUFLAG_X86_SSE3SLOW	((uint32_t)(1 << 30))
#define MDP_CPUFLAG_X86_ATOM		((uint32_t)(1 << 29))	/* slow SSSE3 */
#define MDP_CPUFLAG_X86_AVXSLOW		((uint32_t)(1 << 28))
/*! END: MDP v1.x CPU flags. !*/

#endif /* defined(__i386__) || defined(__amd64__) */

#endif /* __MDP_CPUFLAGS_H */

extern uint32_t CPU_Flags;
uint32_t LibCompat_GetCPUFlags(void);

/**
 * Get the CPU vendor ID.
 * Equivalent to the 12-char vendor ID on x86.
 * @return Pointer to CPU vendor ID (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUVendorID(void);

/**
 * Get the full CPU name.
 * Equivalent to the "brand string" on x86.
 * @return Pointer to the full CPU name (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUFullName(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCOMPAT_CPUFLAGS_H__ */
