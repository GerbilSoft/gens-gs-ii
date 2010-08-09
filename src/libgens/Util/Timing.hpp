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

#ifndef __LIBGENS_UTIL_TIMING_HPP__
#define __LIBGENS_UTIL_TIMING_HPP__

#include <config.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
typedef ULONGLONG (WINAPI *GETTICKCOUNT64PROC)(void);
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif

namespace LibGens
{

class Timing
{
	public:
		static void Init(void);
		static void End(void);
		
		enum TimingMethod
		{
			TM_GETTIMEOFDAY,
#ifdef HAVE_LIBRT
			TM_CLOCK_GETTIME,
#endif /* HAVE_LIBRT */
#ifdef _WIN32
			TM_GETTICKCOUNT,
			TM_GETTICKCOUNT64,
			TM_QUERYPERFORMANCECOUNTER,
#endif /* _WIN32 */
#ifdef __APPLE__
			// http://www.wand.net.nz/~smr26/wordpress/2009/01/19/monotonic-time-in-mac-os-x/
			TM_MACH_ABSOLUTE_TIME,
#endif
			TM_MAX
		};
		
		static TimingMethod GetTimingMethod(void) { return ms_TMethod; }
		static const char *GetTimingMethodName(TimingMethod tMethod);
		
		static double GetTimeD(void);
	
	protected:
		static TimingMethod ms_TMethod;
		
#if defined(_WIN32)
		// GetTickCount64() function pointer.
		static HMODULE ms_hKernel32;
		static GETTICKCOUNT64PROC ms_pGetTickCount64;
		
		// Performance Frequency.
		static LARGE_INTEGER ms_PerfFreq;
#elif defined(__APPLE__)
		// Mach timebase information.
		static mach_timebase_info_data_t ms_timebase_info;
#endif
	
	private:
		Timing() { }
		~Timing() { }
};

}

#endif /* __LIBGENS_UTIL_TIMING_HPP__ */
