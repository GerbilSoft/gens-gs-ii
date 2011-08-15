/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * siginfo.h: signal(2) information.                                       *
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

#include "gens_siginfo.h"

// C includes.
#include <stdlib.h>

/**
 * gens_signals[]: Signal information.
 */
const gens_signal_t gens_signals[] =
{
	// Signal ordering from /usr/include/bits/signum.h (glibc-2.9_p20081201-r2)
	
#ifdef SIGHUP
	{SIGHUP, "SIGHUP", "Hangup"},
#endif
#ifdef SIGILL
	{SIGILL, "SIGILL", "Illegal instruction"},
#endif
#ifdef SIGABRT
	{SIGABRT, "SIGABRT", "Aborted"},
#endif
#ifdef SIGBUS
	{SIGBUS, "SIGBUS", "Bus error"},
#endif
#ifdef SIGFPE
	{SIGFPE, "SIGFPE", "Floating-point exception"},
#endif
#ifdef SIGUSR1
	{SIGUSR1, "SIGUSR1", "User-defined signal 1"},
#endif
#ifdef SIGSEGV
	{SIGSEGV, "SIGSEGV", "Segmentation fault"},
#endif
#ifdef SIGUSR2
	{SIGUSR2, "SIGUSR2", "User-defined signal 2"},
#endif
#ifdef SIGPIPE
	{SIGPIPE, "SIGPIPE", "Broken pipe"},
#endif
#ifdef SIGALRM
	{SIGALRM, "SIGALRM", "Alarm clock"},
#endif
#ifdef SIGSTKFLT
	{SIGSTKFLT, "SIGSTKFLT", "Stack fault"},
#endif
#ifdef SIGXCPU
	{SIGXCPU, "SIGXCPU", "CPU limit exceeded"},
#endif
#ifdef SIGXFSZ
	{SIGXFSZ, "SIGXFSZ", "File size limit exceeded"},
#endif
#ifdef SIGVTALRM
	{SIGVTALRM, "SIGVTALRM", "Virtual alarm clock"},
#endif
#ifdef SIGPWR
	{SIGPWR, "SIGPWR", "Power failure"},
#endif
#ifdef SIGSYS
	{SIGSYS, "SIGSYS", "Bad system call"},
#endif
	{0, NULL, NULL}
};


#ifdef HAVE_SIGACTION

#ifdef SIGILL
// SIGILL information.
const gens_signal_t gens_siginfo_SIGILL[] =
{
#ifdef ILL_ILLOPC
	{ILL_ILLOPC,	"ILL_ILLOPC",	"Illegal opcode"},
#endif
#ifdef ILL_ILLOPN
	{ILL_ILLOPN,	"ILL_ILLOPN",	"Illegal operand"},
#endif
#ifdef ILL_ILLADR
	{ILL_ILLADR,	"ILL_ILLADR",	"Illegal addressing mode"},
#endif
#ifdef ILL_ILLTRP
	{ILL_ILLTRP,	"ILL_ILLTRP",	"Illegal trap"},
#endif
#ifdef ILL_PRVOPC
	{ILL_PRVOPC,	"ILL_PRVOPC",	"Privileged opcode"},
#endif
#ifdef ILL_PRVREG
	{ILL_PRVREG,	"ILL_PRVREG",	"Privileged register"},
#endif
#ifdef ILL_COPROC
	{ILL_COPROC,	"ILL_COPROC",	"Coprocessor error"},
#endif
#ifdef ILL_BADSTK
	{ILL_BADSTK,	"ILL_BADSTK",	"Internal stack error"},
#endif
	{0, NULL, NULL}
};
#endif
	
#ifdef SIGFPE
// SIGFPE information.
const gens_signal_t gens_siginfo_SIGFPE[] =
{
#ifdef FPE_INTDIV
	{FPE_INTDIV,	"FPE_INTDIV",	"Integer divide by zero"},
#endif
#ifdef FPE_INTOVF
	{FPE_INTOVF,	"FPE_INTOVF",	"Integer overflow"},
#endif
#ifdef FPE_FLTDIV
	{FPE_FLTDIV,	"FPE_FLTDIV",	"Floating-point divide by zero"},
#endif
#ifdef FPE_FLTOVF
	{FPE_FLTOVF,	"FPE_FLTOVF",	"Floating-point overflow"},
#endif
#ifdef FPE_FLTUND
	{FPE_FLTUND,	"FPE_FLTUND",	"Floating-point underflow"},
#endif
#ifdef FPE_FLTRES
	{FPE_FLTRES,	"FPE_FLTRES",	"Floating-point inexact result"},
#endif
#ifdef FPE_FLTINV
	{FPE_FLTINV,	"FPE_FLTINV",	"Floating-point invalid operation"},
#endif
#ifdef FPE_FLTSUB
	{FPE_FLTSUB,	"FPE_FLTSUB",	"Subscript out of range"},
#endif
	{0, NULL, NULL}
};
#endif
	
#ifdef SIGSEGV
// SIGSEGV information.
const gens_signal_t gens_siginfo_SIGSEGV[] =
{
#ifdef SEGV_MAPERR
	{SEGV_MAPERR,	"SEGV_MAPERR",	"Address not mapped to object"},
#endif
#ifdef SEGV_ACCERR
	{SEGV_ACCERR,	"SEGV_ACCERR",	"Invalid permissions for mapped object"},
#endif
	{0, NULL, NULL}
};
#endif
	
#ifdef SIGBUS
// SIGBUS information.
const gens_signal_t gens_siginfo_SIGBUS[] =
{
#ifdef BUS_ADRALN
	{BUS_ADRALN,	"BUS_ADRALN",	"Invalid address alignment"},
#endif
#ifdef BUS_ADRERR
	{BUS_ADRERR,	"BUS_ADRERR",	"Nonexistent physical address"},
#endif
#ifdef BUS_OBJERR
	{BUS_OBJERR,	"BUS_OBJERR",	"Object-specific hardware error"},
#endif
	{0, NULL, NULL}
};
#endif

#endif /* HAVE_SIGACTION */
