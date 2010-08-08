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

// Message logging.
#include "macros/log_msg.h"

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

// Static class variables.
Timing::TimingMethod Timing::ms_TMethod;
		
#ifdef _WIN32
HMODULE Timing::ms_hKernel32;
GETTICKCOUNT64PROC Timing::ms_pGetTickCount64;
LARGE_INTEGER Timing::ms_PerfFreq;
#endif /* _WIN32 */


/**
 * Init(): Initialize the timing subsystem.
 */
void Timing::Init(void)
{
#ifdef _WIN32
	// NULL out the DLL and function pointers initially.
	ms_hKernel32 = NULL;
	ms_pGetTickCount64 = NULL;
	
	// Determine which method to use for the timing subsystem.
	BOOL bRet;
	bRet = QueryPerformanceFrequency(&ms_PerfFreq);
	if (bRet != 0 && ms_PerfFreq.QuadPart > 0)
	{
		// System has a high-resolution performance counter.
		ms_TMethod = TM_QUERYPERFORMANCECOUNTER;
	}
	else
	{
		// System does not have a high-resolution performance counter.
		// Fall back to GetTickCount64() or GetTickCount().
		ms_hKernel32 = LoadLibrary("kernel32.dll");
		if (ms_hKernel32)
		{
			ms_pGetTickCount64 = (GETTICKCOUNT64PROC)GetProcAddress(ms_hKernel32, "GetTickCount64");
			if (!ms_pGetTickCount64)
			{
				FreeLibrary(ms_hKernel32);
				ms_hKernel32 = NULL;
			}
		}
		
		if (ms_pGetTickCount64)
		{
			// GetTickCount64() was found.
			ms_TMethod = TM_GETTICKCOUNT64;
		}
		else
		{
			// GetTickCount64() was not found.
			// Fall back to GetTickCount().
			ms_TMethod = TM_GETTICKCOUNT;
		}
	}
#elif defined(HAVE_LIBRT)
	ms_TMethod = TM_CLOCK_GETTIME;
#else
	ms_TMethod = TM_GETTIMEOFDAY;
#endif /* _WIN32 */
	
	// Log the timing method.
	const char *sTMethod;
	switch (ms_TMethod)
	{
		case TM_GETTIMEOFDAY:
		default:
			sTMethod = "gettimeofday";
			break;
#ifdef HAVE_LIBRT
		case TM_CLOCK_GETTIME:
			sTMethod = "clock_gettime";
			break;
#endif /* HAVE_LIBRT */
#ifdef _WIN32
		case TM_GETTICKCOUNT:
			sTMethod = "GetTickCount";
			break;
		case TM_GETTICKCOUNT64:
			sTMethod = "GetTickCount64";
			break;
		case TM_QUERYPERFORMANCECOUNTER:
			sTMethod = "QueryPerformanceCounter";
			break;
#endif /* _WIN32 */
	}
	LOG_MSG(gens, LOG_MSG_LEVEL_INFO,
		"Using %s() for timing.", sTMethod);
}

/**
 * End(): Shut down the timing subsystem.
 */
void Timing::End(void)
{
	// Reset the timing method to gettimeofday().
	ms_TMethod = TM_GETTIMEOFDAY;
	
#ifdef _WIN32
	// Close any module handles that were opened.
	if (ms_hKernel32)
	{
		ms_pGetTickCount64 = NULL;
		FreeLibrary(ms_hKernel32);
		ms_hKernel32 = NULL;
	}
#endif
}


/**
 * GetTimeD(): Get the current time.
 * @return Current time. (double-precision floating point)
 */
double Timing::GetTimeD(void)
{
	// TODO: Use integer arithmetic instead of floating-point.
#if defined(_WIN32)
	// Win32-specific timer functions.
	// TODO: Should TM_GETTIMEOFDAY / TM_CLOCK_GETTIME be supported here?
	LARGE_INTEGER perf_ctr;
	switch (ms_TMethod)
	{
		case TM_GETTICKCOUNT:
		default:
			// GetTickCount().
			return ((double)GetTickCount() / 1000.0);
		
		case TM_GETTICKCOUNT64:
			// GetTickCount64().
			return ((double)ms_pGetTickCount64() / 1000.0);
		
		case TM_QUERYPERFORMANCECOUNTER:
			// QueryPerformanceCounter().
			QueryPerformanceCounter(&perf_ctr);
			return (((double)(perf_ctr.QuadPart)) / ((double)(ms_PerfFreq.QuadPart)));
	}
#elif defined(HAVE_LIBRT)
	// librt is available: use clock_gettime().
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec + (ts.tv_nsec / 1000000000.0));
#else
	// Fall back to gettimeofday().
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + (tv.tv_usec / 1000000.0));
#endif
}

}
