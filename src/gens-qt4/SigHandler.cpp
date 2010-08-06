/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * SigHandler.cpp: Signal handler.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#include "SigHandler.hpp"

// C includes
#include <signal.h>

// LibGens includes.
#include "libgens/macros/log_msg.h"
#include "libgens/macros/git.h"
#include "libgens/Util/siginfo.h"

// Qt includes.
#include <QtCore/QString>
#include <QtGui/QMessageBox>

// OS platform.
// TODO: Move this somewhere else!
#if defined(Q_OS_WIN32)
#define GENS_PLATFORM "Win32"
#elif defined(Q_OS_MAC)
#define GENS_PLATFORM "Mac OS X"
#elif defined(Q_OS_LINUX)
#define GENS_PLATFORM "Linux"
#elif defined(Q_OS_FREEBSD)
#define GENS_PLATFORM "FreeBSD"
#elif defined(Q_OS_OPENBSD)
#define GENS_PLATFORM "OpenBSD"
#elif defined(Q_OS_NETBSD)
#define GENS_PLATFORM "NetBSD"
#elif defined(Q_OS_UNIX)
#define GENS_PLATFORM "Some Unix system"
#else
#define GENS_PLATFORM "Unknown"
#endif

// derp
#define RICKROLL
#ifdef RICKROLL
#include <QtGui/QIcon>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif /* _WIN32 */
#endif /* RICKROLL */

namespace GensQt4
{

/**
 * Init(): Initialize the signal handler.
 */
void SigHandler::Init(void)
{
#ifdef HAVE_SIGACTION
	struct sigaction sa;
	sa.sa_sigaction = SignalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
#endif
	
	for (unsigned int i = 0; gens_signals[i].signum != 0; i++)
	{
#ifdef HAVE_SIGACTION
		sigaction(gens_signals[i].signum, &sa, NULL);
#else
		signal(gens_signals[i].signum, SignalHandler);
#endif
	}
}


/**
 * End(): Shut down the signal handler.
 */
void SigHandler::End(void)
{
	for (unsigned int i = 0; gens_signals[i].signum != 0; i++)
	{
		signal(gens_signals[i].signum, SIG_DFL);
	}
}


#ifdef HAVE_SIGACTION
/**
 * SigHandler_get_siginfo(): Get the signal information from a received signal.
 * @param signum	[in] Signal number.
 * @param si_code	[in] Signal information code.
 * @return Pointer to gens_signal_t, or NULL if not found.
 */
const gens_signal_t *SigHandler::GetSigInfo(int signum, int si_code)
{
	// Check if there's any information associated with the signal.
	const gens_signal_t *siginfo;
	
	switch (signum)
	{
#ifdef SIGILL
		case SIGILL:
			siginfo = &siginfo_SIGILL[0];
			break;
#endif
#ifdef SIGFPE
		case SIGFPE:
			siginfo = &siginfo_SIGFPE[0];
			break;
#endif
#ifdef SIGSEGV
		case SIGSEGV:
			siginfo = &siginfo_SIGSEGV[0];
			break;
#endif
#ifdef SIGBUS
		case SIGBUS:
			siginfo = &siginfo_SIGBUS[0];
			break;
#endif
		default:
			siginfo = NULL;
			break;
	}
	
	if (!siginfo)
		return NULL;
	
	// Check for signal information.
	for (; siginfo->signum != 0; siginfo++)
	{
		if (siginfo->signum == si_code)
			return siginfo;
	}
	
	// No signal information was found.
	return NULL;
}
#endif


/**
 * SignalHandler(): Signal handler.
 * @param signum	[in] Signal number.
 * @param info		[in] Signal information. (ONLY if sigaction() is available.)
 * @param context	[in] Context. (ONLY if sigaction() is available.)
 */
#ifdef HAVE_SIGACTION
void SigHandler::SignalHandler(int signum, siginfo_t *info, void *context)
#else
void SigHandler::SignalHandler(int signum)
#endif
{
#ifdef HAVE_SIGACTION
	// The `context' parameter isn't being used right now.
	((void)context);
#endif
	
	if (
#ifdef SIGHUP
	    signum == SIGHUP ||
#endif
#ifdef SIGUSR1
	    signum == SIGUSR1 ||
#endif
#ifdef SIGUSR2
	    signum == SIGUSR2 ||
#endif
	    0)
	{
		// SIGHUP, SIGUSR1, SIGUSR2. Ignore this signal.
		const char *signame;
		
		switch (signum)
		{
#ifdef SIGHUP
			case SIGHUP:
				signame = "SIGHUP";
				break;
#endif
#ifdef SIGUSR1
			case SIGUSR1:
				signame = "SIGUSR1";
				break;
#endif
#ifdef SIGUSR2
			case SIGUSR2:
				signame = "SIGUSR2";
				break;
#endif
			default:
				signame = "UNKNOWN";
				break;
		}
		
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Signal %d (%s) received; ignoring.", signum, signame);
		return;
	}
	
	// Check what signal this is.
	const char *signame = "SIGUNKNOWN";
	const char *sigdesc = "Unknown signal";
	for (unsigned int i = 0; gens_signals[i].signum != 0; i++)
	{
		if (gens_signals[i].signum == signum)
		{
			signame = gens_signals[i].signame;
			sigdesc = gens_signals[i].sigdesc;
			break;
		}
	}
	
#ifdef HAVE_SIGACTION
	// Note: If context is NULL, then info is invalid.
	// This may happen if SIGILL is sent to the program via kill/pkill/killall.
	const gens_signal_t *siginfo = NULL;
	if (info && context)
		siginfo = GetSigInfo(signum, info->si_code);
#endif
	
	// This uses LOG_MSG_LEVEL_ERROR in order to suppress the message box.
#ifdef HAVE_SIGACTION
	if (siginfo)
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Signal %d (%s: %s) received. Shutting down.",
			signum, signame, siginfo->signame);
	}
	else
#endif
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Signal %d (%s) received. Shutting down.",
			signum, signame);
	}
	
	// Show a message box.
#ifdef RICKROLL
	QString sMsg =
		"Gens/GS II has given you up. (Signal " + QString::number(signum) + ")\n";
#else
	QString sMsg =
		"Gens/GS II has crashed with Signal " + QString::number(signum) + ".\n";
#endif
	
	if (signame)
	{
		sMsg += QString(signame);
		if (sigdesc)
			sMsg += ": " + QString(sigdesc);
		sMsg += "\n";
	}
	
#ifdef HAVE_SIGACTION
	if (siginfo && siginfo->signame)
	{
		sMsg += QString(siginfo->signame);
		if (siginfo->sigdesc)
			sMsg += ": " + QString(sigdesc);
		sMsg += "\n";
	}
#endif
	
	sMsg += "\n"
		"Build Information:\n"
		"- Platform: " GENS_PLATFORM "\n"
		"- Version: 0.0.0 (Development Build)\n"	/* TODO: Add some #define for this! */
#ifdef GENS_GIT_VERSION
		"- " GENS_GIT_VERSION "\n"
#endif
		"\n"
		"Please report this error to GerbilSoft.\n"
		"- E-mail: gerbilsoft@verizon.net\n\n"
		"Be sure to include detailed information about what you were "
		"doing when this error occurred.";
	
	// Display the message box.
	QMessageBox dialog(QMessageBox::Critical, "Gens/GS II Fatal Error", sMsg);
	dialog.setTextFormat(Qt::PlainText);
#ifdef RICKROLL
	QPixmap pxmRickRoll(":/gens/rick-roll.png");
	if (!pxmRickRoll.isNull())
	{
		dialog.setIconPixmap(pxmRickRoll);
#ifdef _WIN32
		// Qt on Win32 doesn't play a message sound if a
		// custom icon is used. Let's play it outselves.
		MessageBeep(MB_ICONSTOP);
#endif
	}
#endif
	dialog.exec();
	
	exit(EXIT_FAILURE);
}

}
