/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Timing.hpp: Timing functions.                                           *
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

#ifndef __LIBGENS_UTIL_TIMING_HPP__
#define __LIBGENS_UTIL_TIMING_HPP__

#include <libgens/config.libgens.h>

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <ctime>

// OS-specific headers.
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

namespace LibGens {

class Timing
{
	public:
		Timing();
		~Timing();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Timing(const Timing &);
		Timing &operator=(const Timing &);

	public:
		enum TimingMethod {
			TM_UNKNOWN,
			TM_GETTIMEOFDAY,
#ifdef HAVE_CLOCK_GETTIME
			TM_CLOCK_GETTIME,
#endif /* HAVE_CLOCK_GETTIME */
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

		inline TimingMethod getTimingMethod(void)
			{ return m_tMethod; }

		/**
		 * Get the name of a timing method.
		 * @param tMethod Timing method.
		 * @return Timing method name. (ASCII)
		 */
		static const char *GetTimingMethodName(TimingMethod tMethod);

		/**
		 * Get the elapsed time in seconds.
		 * @return Elapsed time, in seconds.
		 */
		double getTimeD(void);

		/**
		 * Get the elapsed time in microseconds.
		 * @return Elapsed time, in microseconds.
		 */
		uint64_t getTime(void);

	protected:
		TimingMethod m_tMethod;

		// Base value for seconds.
		// Needed to prevent overflow.
		time_t m_tv_sec_base;

#if defined(_WIN32)
		// GetTickCount64() function pointer.
		HMODULE m_hKernel32;
		GETTICKCOUNT64PROC m_pGetTickCount64;
		// Performance Frequency.
		LARGE_INTEGER m_perfFreq;
#elif defined(__APPLE__)
		// Mach timebase information.
		mach_timebase_info_data_t m_timebase_info;
#endif
};

}

#endif /* __LIBGENS_UTIL_TIMING_HPP__ */
