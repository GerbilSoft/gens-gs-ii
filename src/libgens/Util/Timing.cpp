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

#include "Timing.hpp"

namespace LibGens {

/**
 * Initialize the timing subsystem.
 */
Timing::Timing(void)
	: m_tMethod(TM_UNKNOWN)
	, m_tv_sec_base(0)
#ifdef _WIN32
	, m_hKernel32(nullptr)
	, m_pGetTickCount64(nullptr)
#endif /* WIN32 */
{
	// TODO: Initialize once?

#ifdef _WIN32
	// Determine which method to use for the timing subsystem.
	BOOL bRet;
	bRet = QueryPerformanceFrequency(&m_perfFreq);
	if (bRet != 0 && m_perfFreq.QuadPart > 0) {
		// System has a high-resolution performance counter.
		m_tMethod = TM_QUERYPERFORMANCECOUNTER;
	} else {
		// System does not have a high-resolution performance counter.
		// Fall back to GetTickCount64() or GetTickCount().
		m_hKernel32 = LoadLibrary("kernel32.dll");
		if (m_hKernel32) {
			m_pGetTickCount64 = (GETTICKCOUNT64PROC)GetProcAddress(m_hKernel32, "GetTickCount64");
			if (!m_pGetTickCount64) {
				FreeLibrary(m_hKernel32);
				m_hKernel32 = nullptr;
			}
		}

		if (m_pGetTickCount64) {
			// GetTickCount64() was found.
			m_tMethod = TM_GETTICKCOUNT64;
		} else {
			// GetTickCount64() was not found.
			// Fall back to GetTickCount().
			m_tMethod = TM_GETTICKCOUNT;
		}
	}
#elif defined(__APPLE__)
	// Get the Mach timebase information.
	mach_timebase_info(&m_timebase_info);
	m_tMethod = TM_MACH_ABSOLUTE_TIME;
#elif defined(HAVE_LIBRT)
	m_tMethod = TM_CLOCK_GETTIME;

	// Initialize the base value for seconds.
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	m_tv_sec_base = ts.tv_sec;
#else
	m_tMethod = TM_GETTIMEOFDAY;

	// Initialize the base value for seconds.
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	m_tv_sec_base = tv.tv_sec;
#endif /* _WIN32 */
}

/**
 * Shut down the timing subsystem.
 */
Timing::~Timing()
{
#ifdef _WIN32
	// Close any module handles that were opened.
	if (m_hKernel32) {
		FreeLibrary(m_hKernel32);
	}
#endif /* _WIN32 */
}

/**
 * Get the name of a timing method.
 * @param tMethod Timing method.
 * @return Timing method name. (ASCII)
 */
const char *Timing::GetTimingMethodName(TimingMethod tMethod)
{
	switch (tMethod) {
		case TM_GETTIMEOFDAY:
		default:
			// TODO: Default timing method on Win32 is currently GetTickCount().
			return "gettimeofday";
#ifdef HAVE_LIBRT
		case TM_CLOCK_GETTIME:
			return "clock_gettime";
#endif /* HAVE_LIBRT */
#ifdef _WIN32
		case TM_GETTICKCOUNT:
			return "GetTickCount";
		case TM_GETTICKCOUNT64:
			return "GetTickCount64";
		case TM_QUERYPERFORMANCECOUNTER:
			return "QueryPerformanceCounter";
#endif /* _WIN32 */
#ifdef __APPLE__
		case TM_MACH_ABSOLUTE_TIME:
			return "mach_absolute_time";
#endif /* __APPLE__ */
	}
}

/**
 * Get the current time.
 * @return Current time. (double-precision floating point)
 */
double Timing::getTimeD(void)
{
	// TODO: Use integer arithmetic instead of floating-point.
#if defined(_WIN32)
	// Win32-specific timer functions.
	// TODO: Should TM_GETTIMEOFDAY / TM_CLOCK_GETTIME be supported here?
	LARGE_INTEGER perf_ctr;
	switch (m_tMethod) {
		case TM_GETTICKCOUNT:
		default:
			// GetTickCount().
			return ((double)GetTickCount() / 1000.0);
		case TM_GETTICKCOUNT64:
			// GetTickCount64().
			return ((double)m_pGetTickCount64() / 1000.0);
		case TM_QUERYPERFORMANCECOUNTER:
			// QueryPerformanceCounter().
			QueryPerformanceCounter(&perf_ctr);
			return (((double)(perf_ctr.QuadPart)) / ((double)(m_perfFreq.QuadPart)));
	}
#elif defined(__APPLE__)
	// Mach absolute time. (Mac OS X)
	uint64_t abs_time = mach_absolute_time();
	double d_abs_time = (double)abs_time * (double)m_timebase_info.numer / (double)m_timebase_info.denom;
	return (d_abs_time / 1.0e9);
#elif defined(HAVE_LIBRT)
	// librt is available: use clock_gettime().
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec -= m_tv_sec_base;
	return ((double)ts.tv_sec + ((double)ts.tv_nsec / 1.0e9));
#else
	// Fall back to gettimeofday().
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tv.tv_sec -= m_tv_sec_base;
	return ((double)tv.tv_sec + ((double)tv.tv_usec / 1.0e6));
#endif
}

/**
 * Get the elapsed time in microseconds.
 * @return Elapsed time, in microseconds.
 */
uint64_t Timing::getTime(void)
{
#if defined(_WIN32)
	// Win32-specific timer functions.
	// TODO: Should TM_GETTIMEOFDAY / TM_CLOCK_GETTIME be supported here?
	// TODO: Prevent overflow.
	LARGE_INTEGER perf_ctr;
	switch (m_tMethod) {
		case TM_GETTICKCOUNT:
		default:
			// GetTickCount().
			return ((uint64_t)GetTickCount() * 1000);
		case TM_GETTICKCOUNT64:
			// GetTickCount64().
			return (m_pGetTickCount64() * 1000.0);
		case TM_QUERYPERFORMANCECOUNTER:
			// QueryPerformanceCounter().
			QueryPerformanceCounter(&perf_ctr);
			const uint64_t divisor = m_perfFreq.QuadPart / 1000000;
			return (perf_ctr.QuadPart / divisor);
	}
#elif defined(__APPLE__)
	// Mach absolute time. (Mac OS X)
	// TODO: http://stackoverflow.com/questions/23378063/how-can-i-use-mach-absolute-time-without-overflowing
#if 0
	uint64_t abs_time = mach_absolute_time();
	double d_abs_time = (double)abs_time * (double)m_timebase_info.numer / (double)m_timebase_info.denom;
	return (d_abs_time / 1.0e9);
#endif
	return 0;
#elif defined(HAVE_LIBRT)
	// librt is available: use clock_gettime().
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec -= m_tv_sec_base;
	return ((ts.tv_sec * 1000000) + (ts.tv_nsec / 1000));
#else
	// Fall back to gettimeofday().
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	tv.tv_sec -= m_tv_sec_base;
	return ((tv.tv_sec * 1000000) + tv.tv_usec);
#endif
}

}
