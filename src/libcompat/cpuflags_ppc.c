/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * cpuflags.c: CPU flag definitions and functions. (PowerPC version)       *
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

#if !defined(HW_RVL) && !defined(HW_DOL)
#error cpuflags_ppc.c currently only supports HW_RVL and HW_DOL.
#endif

#include "cpuflags.h"

// C includes.
#include <string.h>
#include <stdio.h>

#if defined(HW_RVL) || defined(HW_DOL)
// libogc includes.
#include <gccore.h>
#include <ogcsys.h>
#endif

// CPU flags.
// This variable is publicly accessible for performance reasons.
uint32_t CPU_Flags = 0;

// Full CPU name. (NULL-terminated)
static char CPU_FullName[64] = {0};

/**
 * Get the CPU vendor ID.
 * Equivalent to the 12-char vendor ID on x86.
 * @return Pointer to CPU vendor ID (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUVendorID(void)
{
#if defined(HW_RVL) || defined(HW_DOL)
	// libogc (Nintendo Wii or Nintendo GameCube)
	// Both CPUs were manufactured by IBM.
	return "IBM";
#else
	return NULL;
#endif
}

/**
 * Get the full CPU name.
 * Equivalent to the "brand string" on x86.
 * @return Pointer to the full CPU name (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUFullName(void)
{
	if (!CPU_FullName[0]) {
#if defined(HW_RVL) || defined(HW_DOL)
		// libogc (Nintendo Wii or Nintendo GameCube)
		// TODO: Check CPU version?
		// http://wiibrew.org/wiki/Hardware/Broadway
		// - Broadway == 87102
		// - Gekko == 83410
#if defined(HW_RVL)
		const char cpu_name[] = "PowerPC 750CL \"Broadway\"";
#elif defined(HW_DOL)
		const char cpu_name[] = = "PowerPC 750CXe \"Gekko\"";
#endif

		// FIXME: There should be a way to retrieve the
		// actual CPU frequency instead of hard-coding it.
		snprintf(CPU_FullName, sizeof(CPU_FullName), "%s @ %d MHz",
			cpu_name, TB_CORE_CLOCK / 1000000);
#endif
	}

	if (!CPU_FullName[0]) {
		// No full name.
		return NULL;
	}

	return CPU_FullName;
}
