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

namespace LibGens {

class TimingPrivate;
class Timing
{
	public:
		Timing();
		~Timing();

	protected:
		friend class TimingPrivate;
		TimingPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Timing(const Timing &);
		Timing &operator=(const Timing &);

	public:
		enum TimingMethod {
			TM_UNKNOWN,

			// POSIX
			TM_GETTIMEOFDAY,
			TM_CLOCK_GETTIME,

			// Windows-specific.
			TM_GETTICKCOUNT,
			TM_GETTICKCOUNT64,
			TM_QUERYPERFORMANCECOUNTER,

			// Mac OS X-specific.
			// http://www.wand.net.nz/~smr26/wordpress/2009/01/19/monotonic-time-in-mac-os-x/
			TM_MACH_ABSOLUTE_TIME,

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
		 * Reset the timer base.
		 * This will invalidate all previous timer values
		 * when compared to new timer values.
		 */
		void resetBase(void);

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

		// Timer base value.
		// Needed to prevent overflow.
#if !defined(_WIN32) && !defined(__APPLE__)
		// Timer may be 32-bit on Linux.
		// TODO: Fix that.
		time_t m_timer_base;
#else
		// Timer is always 64-bit on Windows and Mac OS X.
		uint64_t m_timer_base;
#endif
};

}

#endif /* __LIBGENS_UTIL_TIMING_HPP__ */
