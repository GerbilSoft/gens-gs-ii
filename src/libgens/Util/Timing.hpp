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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
typedef ULONGLONG (WINAPI *GETTICKCOUNT64PROC)(void);
#endif /* _WIN32 */

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
			TM_MAX
		};
		
		static TimingMethod GetTimingMethod(void) { return ms_TMethod; }
		static const char *GetTimingMethodName(TimingMethod tMethod);
		
		static double GetTimeD(void);
	
	protected:
		static TimingMethod ms_TMethod;
		
#ifdef _WIN32
		// GetTickCount64() function pointer.
		static HMODULE ms_hKernel32;
		static GETTICKCOUNT64PROC ms_pGetTickCount64;
		
		// Performance Frequency.
		static LARGE_INTEGER ms_PerfFreq;
#endif
	
	private:
		Timing() { }
		~Timing() { }
};

}

#endif /* __LIBGENS_UTIL_TIMING_HPP__ */
