/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * lg_win32.c: Win32 compatibility functions.                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "lg_win32.h"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

/**
 * setenv(): Win32 setenv() wrapper.
 * @param name Variable name.
 * @param value Variable value.
 * @param overwrite If non-zero, overwrites the variable if it exists.
 * @return 0 on success; non-zero on error.
 * TODO: Proper error handling.
 */
int setenv(const char *name, const char *value, int overwrite)
{
	if (!overwrite)
	{
		// If the environment variable exists, don't overwrite it.
		int ret = GetEnvironmentVariable(name, NULL, 0);
		if (ret != 0)
		{
			// Variable exists. Don't overwrite it.
			return 0;
		}
	}
	
	// Set the environment variable.
	// NOTE: Win32 functions return non-zero on success and zero on error.
	// POSIX uses the other way around.
	return !SetEnvironmentVariable(name, value);
}

/**
 * unsetenv(): Win32 unsetenv() wrapper.
 * @param name Variable to unset.
 * @return 0 on success; non-zero on error.
 * TODO: Proper error handling.
 */
int unsetenv(const char *name)
{
	// NOTE: Win32 functions return non-zero on success and zero on error.
	// POSIX uses the other way around.
	return !SetEnvironmentVariable(name, NULL);
}
