/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Timing_win32.cpp: Timing functions. (Windows)                           *
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

#ifndef _WIN32
#error This file is only for Windows systems.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

typedef ULONGLONG (WINAPI *GETTICKCOUNT64PROC)(void);

namespace LibGens {

class TimingPrivate
{
	public:
		TimingPrivate();

	public:
		// TODO: The following should probably be static,
		// since they're not going to change while the
		// system is running.

		// GetTickCount64() function pointer.
		HMODULE hKernel32;
		GETTICKCOUNT64PROC pGetTickCount64;
		// Performance Frequency.
		LARGE_INTEGER perfFreq;
};

TimingPrivate::TimingPrivate()
	: hKernel32(nullptr)
	, pGetTickCount64(nullptr)
{
	perfFreq.QuadPart = 0;
}

/** Timing **/

/**
 * Initialize the timing subsystem.
 */
Timing::Timing()
	: d(new TimingPrivate())
	, m_tMethod(TM_UNKNOWN)
	, m_timer_base(0)
{
	// TODO: Initialize once?

	// Determine which method to use for the timing subsystem.
	BOOL bRet;
	bRet = QueryPerformanceFrequency(&d->perfFreq);
	if (bRet != 0 && d->perfFreq.QuadPart > 0) {
		// System has a high-resolution performance counter.
		m_tMethod = TM_QUERYPERFORMANCECOUNTER;
	} else {
		// System does not have a high-resolution performance counter.
		// Fall back to GetTickCount64() or GetTickCount().
		d->hKernel32 = LoadLibraryA("kernel32.dll");
		if (d->hKernel32) {
			d->pGetTickCount64 = (GETTICKCOUNT64PROC)GetProcAddress(d->hKernel32, "GetTickCount64");
			if (!d->pGetTickCount64) {
				FreeLibrary(d->hKernel32);
				d->hKernel32 = nullptr;
			}
		}

		if (d->pGetTickCount64) {
			// GetTickCount64() was found.
			m_tMethod = TM_GETTICKCOUNT64;
		} else {
			// GetTickCount64() was not found.
			// Fall back to GetTickCount().
			m_tMethod = TM_GETTICKCOUNT;
		}
	}

	// Reset the timer base.
	resetBase();
}

/**
 * Shut down the timing subsystem.
 */
Timing::~Timing()
{
	// Close any module handles that were opened.
	if (d->hKernel32) {
		FreeLibrary(d->hKernel32);
	}

	delete d;
}

/**
 * Reset the timer base.
 * This will invalidate all previous timer values
 * when compared to new timer values.
 */
void Timing::resetBase(void)
{
	switch (m_tMethod) {
		case TM_GETTICKCOUNT:
		default:
			m_timer_base = GetTickCount();
			break;
		case TM_GETTICKCOUNT64:
			m_timer_base = d->pGetTickCount64();
			break;
		case TM_QUERYPERFORMANCECOUNTER: {
			LARGE_INTEGER perf_ctr;
			QueryPerformanceCounter(&perf_ctr);
			m_timer_base = perf_ctr.QuadPart;
			break;
		}
	}
}

/**
 * Get the current time.
 * @return Current time. (double-precision floating point)
 */
double Timing::getTimeD(void)
{
	double timer;
	switch (m_tMethod) {
		case TM_GETTICKCOUNT:
		default:
			// FIXME: Prevent overflow.
			timer = (double)(GetTickCount() - m_timer_base) / 1000.0;
			break;

		case TM_GETTICKCOUNT64:
			timer = (double)(d->pGetTickCount64() - m_timer_base) / 1000.0;
			break;

		case TM_QUERYPERFORMANCECOUNTER: {
			LARGE_INTEGER perf_ctr;
			QueryPerformanceCounter(&perf_ctr);
			timer = (((double)(perf_ctr.QuadPart - m_timer_base)) /
				 ((double)(d->perfFreq.QuadPart)));
			break;
		}
	}

	return timer;
}

/**
 * Get the elapsed time in microseconds.
 * @return Elapsed time, in microseconds.
 */
uint64_t Timing::getTime(void)
{
	uint64_t timer;
	switch (m_tMethod) {
		case TM_GETTICKCOUNT:
		default:
			// FIXME: Prevent overflow.
			timer = (uint64_t)(GetTickCount() - m_timer_base) * 1000;
			break;

		case TM_GETTICKCOUNT64:
			timer = (d->pGetTickCount64() - m_timer_base) * 1000;
			break;

		case TM_QUERYPERFORMANCECOUNTER: {
			LARGE_INTEGER perf_ctr;
			QueryPerformanceCounter(&perf_ctr);
			const uint64_t divisor = d->perfFreq.QuadPart / 1000000;
			timer = (perf_ctr.QuadPart - m_timer_base) / divisor;
			break;
		}
	}

	return timer;
}

}
