/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Timing_unix.cpp: Timing functions. (Unix/Linux, but not Mac OS X)       *
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

#if defined(_WIN32) || defined(__APPLE__) || \
    (!defined(__unix__) && !defined(__unix) && !defined(unix) || \
     !defined(__linux__) && !defined(__linux) && !defined(linux))
#error This file is only for Unix and Linux systems, excluding Mac OS X.
#endif

#include <config.libgens.h>

#include <time.h>
#include <sys/time.h>

namespace LibGens {

/**
 * Initialize the timing subsystem.
 */
Timing::Timing()
	: d(nullptr)
	, m_tMethod(TM_UNKNOWN)
	, m_timer_base(0)
{
#if defined(HAVE_CLOCK_GETTIME)
	// Use clock_gettime().
	m_tMethod = TM_CLOCK_GETTIME;
#else
	// clock_gettime() is not available.
	// Fall back to gettimeofday().
	// NOTE: gettimeofday() is not guaranteed to be monotonic.
	m_tMethod = TM_GETTIMEOFDAY;
#endif

	// Reset the timer base.
	resetBase();
}

/**
 * Shut down the timing subsystem.
 */
Timing::~Timing()
{ }

/**
 * Reset the timer base.
 * This will invalidate all previous timer values
 * when compared to new timer values.
 */
void Timing::resetBase(void)
{
#if defined(HAVE_CLOCK_GETTIME)
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	m_timer_base = ts.tv_sec;
#else
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	m_timer_base = tv.tv_sec;
#endif
}

/**
 * Get the current time.
 * @return Current time. (double-precision floating point)
 */
double Timing::getTimeD(void)
{
#if defined(HAVE_CLOCK_GETTIME)
	// Use clock_gettime().
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec -= m_timer_base;
	return ((double)ts.tv_sec + ((double)ts.tv_nsec / 1.0e9));
#else
	// Fall back to gettimeofday().
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tv.tv_sec -= m_timer_base;
	return ((double)tv.tv_sec + ((double)tv.tv_usec / 1.0e6));
#endif
}

/**
 * Get the elapsed time in microseconds.
 * @return Elapsed time, in microseconds.
 */
uint64_t Timing::getTime(void)
{
#if defined(HAVE_CLOCK_GETTIME)
	// Use clock_gettime().
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec -= m_timer_base;
	return ((ts.tv_sec * 1000000) + (ts.tv_nsec / 1000));
#else
	// Fall back to gettimeofday().
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	tv.tv_sec -= m_timer_base;
	return ((tv.tv_sec * 1000000) + tv.tv_usec);
#endif
}

}
