/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Timing.hpp: Timing functions.                                           *
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

#include "Timing.hpp"
#include <config.h>

// C includes. (string.h is needed for NULL)
#include <string.h>

// Timing functions.
// TODO: Mac OS X "Mach-O" monotonic clock support.
#ifdef HAVE_LIBRT
#include <time.h>
#else
#include <sys/time.h>
#endif


namespace LibGens
{

/**
 * GetTimeD(): Get the current time.
 * @return Current time. (double-precision floating point)
 */
double Timing::GetTimeD(void)
{
	// TODO: Use integer arithmetic instead of floating-point.
	// TODO: Add Win32-specific versions that use QueryPerformanceCounter() and/or GetTickCount().
#ifdef HAVE_LIBRT
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec + (ts.tv_nsec / 1000000000.0));
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + (tv.tv_usec / 1000000.0));
#endif
}

}
