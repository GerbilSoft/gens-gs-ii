/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * cpuflags.c: CPU flag definitions and functions. (generic version)       *
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
#include <string.h>

// CPU flags.
// This variable is publicly accessible for performance reasons.
uint32_t CPU_Flags = 0;

/**
 * Get the CPU vendor ID.
 * Equivalent to the 12-char vendor ID on x86.
 * @return Pointer to CPU vendor ID (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUVendorID(void)
{
	// No vendor ID in the generic version...
	return NULL;
}

/**
 * Get the full CPU name.
 * Equivalent to the "brand string" on x86.
 * @return Pointer to the full CPU name (null-terminated string), or NULL on error.
 */
const char *LibCompat_GetCPUFullName(void)
{
	// No full CPU name in the generic version...
	return NULL;
}
