/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * gens_siginfo.h: signal(2) information.                                  *
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

#include <config.h>

#ifndef __LIBGENS_GENS_SIGINFO_H__
#define __LIBGENS_GENS_SIGINFO_H__

// C includes.
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _gens_signal_t
{
	int signum;
	const char *signame;
	const char *sigdesc;
} gens_signal_t;

extern const gens_signal_t gens_signals[];

#ifdef HAVE_SIGACTION

#ifdef SIGILL
extern const gens_signal_t gens_siginfo_SIGILL[];
#endif
	
#ifdef SIGFPE
// SIGFPE information.
extern const gens_signal_t gens_siginfo_SIGFPE[];
#endif
	
#ifdef SIGSEGV
extern const gens_signal_t gens_siginfo_SIGSEGV[];
#endif
	
#ifdef SIGBUS
extern const gens_signal_t gens_siginfo_SIGBUS[];
#endif

#endif /* HAVE_SIGACTION */

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_GENS_SIGINFO_H__ */
