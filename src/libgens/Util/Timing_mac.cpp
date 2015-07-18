/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Timing_mac.cpp: Timing functions. (Mac OS X)                            *
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

#ifndef __APPLE__
#error This file is only for Windows systems.
#endif

#include <mach/mach_time.h>

namespace LibGens {

class TimingPrivate
{
	public:
		TimingPrivate();

	public:
		// TODO: The following should probably be static,
		// since they're not going to change while the
		// system is running.

		// Mach timebase information.
		mach_timebase_info_data_t timebase_info;
};

TimingPrivate::TimingPrivate()
{
	// TODO: Initialize timebase_info here?
}

/**
 * Initialize the timing subsystem.
 */
Timing::Timing(void)
	: d(new TimingPrivate())
	, m_tMethod(TM_UNKNOWN)
	, m_timer_base(0)
{
	// Get the Mach timebase information.
	mach_timebase_info(&d->timebase_info);
	m_tMethod = TM_MACH_ABSOLUTE_TIME;
}

/**
 * Shut down the timing subsystem.
 */
Timing::~Timing()
{
	delete d;
}

/**
 * Get the current time.
 * @return Current time. (double-precision floating point)
 */
double Timing::getTimeD(void)
{
	// Mach absolute time. (Mac OS X)
	uint64_t abs_time = mach_absolute_time();
	double d_abs_time = (double)abs_time * (double)m_timebase_info.numer / (double)m_timebase_info.denom;
	return (d_abs_time / 1.0e9);
}

/**
 * Get the elapsed time in microseconds.
 * @return Elapsed time, in microseconds.
 */
uint64_t Timing::getTime(void)
{
	// Mach absolute time. (Mac OS X)
	uint64_t abs_time = mach_absolute_time();
	// d_abs_time contains time in nanoseconds.
	double d_abs_time = (double)abs_time * (double)m_timebase_info.numer / (double)m_timebase_info.denom;
	// Convert to microseconds.
	return (uint64_t)(d_abs_time / 1000.0);
}

}
