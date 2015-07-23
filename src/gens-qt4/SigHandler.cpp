/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * SigHandler.cpp: Signal handler.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2015 by David Korth                                  *
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
#include "gqt4_main.hpp"

// C includes
#include <signal.h>

// LibGens includes.
#include "libgens/lg_main.hpp"
#include "libgens/macros/log_msg.h"

// Qt includes.
#include <QtCore/QString>
#include <QtGui/QMessageBox>

// Gens QApplication.
#include "GensQApplication.hpp"

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

// HANG() macro.
#ifdef Q_OS_WIN32
#include <windows.h>
#define HANG() do { Sleep(INFINITE); } while (1)
#else
#include <unistd.h>
#define HANG() do { sleep(1000); } while (1)
#endif

// derp
#define RICKROLL
#ifdef RICKROLL
#include <QtGui/QIcon>
#endif /* RICKROLL */

namespace GensQt4 {

/**
 * Initialize the signal handler.
 */
void SigHandler::Init(void)
{
#ifdef HAVE_SIGACTION
	struct sigaction sa;
	sa.sa_sigaction = SignalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
#endif
	
	for (int i = 0; gens_signals[i].signum != 0; i++) {
#ifdef HAVE_SIGACTION
		sigaction(gens_signals[i].signum, &sa, nullptr);
#else
		signal(gens_signals[i].signum, SignalHandler);
#endif
	}
}

/**
 * Shut down the signal handler.
 */
void SigHandler::End(void)
{
	// TODO: If using sigaction(), restore oldact?
	for (int i = 0; gens_signals[i].signum != 0; i++) {
		signal(gens_signals[i].signum, SIG_DFL);
	}
}

#ifdef HAVE_SIGACTION
/**
 * Get the signal information from a received signal.
 * @param signum	[in] Signal number.
 * @param si_code	[in] Signal information code.
 * @return Pointer to gens_signal_t, or nullptr if not found.
 */
const gens_signal_t *SigHandler::GetSigInfo(int signum, int si_code)
{
	// Check if there's any information associated with the signal.
	const gens_signal_t *siginfo;

	switch (signum) {
#ifdef SIGILL
		case SIGILL:
			siginfo = &gens_siginfo_SIGILL[0];
			break;
#endif
#ifdef SIGFPE
		case SIGFPE:
			siginfo = &gens_siginfo_SIGFPE[0];
			break;
#endif
#ifdef SIGSEGV
		case SIGSEGV:
			siginfo = &gens_siginfo_SIGSEGV[0];
			break;
#endif
#ifdef SIGBUS
		case SIGBUS:
			siginfo = &gens_siginfo_SIGBUS[0];
			break;
#endif
		default:
			siginfo = nullptr;
			break;
	}

	if (!siginfo)
		return nullptr;

	// Check for signal information.
	for (; siginfo->signum != 0; siginfo++) {
		if (siginfo->signum == si_code)
			return siginfo;
	}

	// No signal information was found.
	return nullptr;
}
#endif

/**
 * Signal handler.
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
	Q_UNUSED(context)
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
		
		switch (signum) {
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
	for (int i = 0; gens_signals[i].signum != 0; i++) {
		if (gens_signals[i].signum == signum) {
			signame = gens_signals[i].signame;
			sigdesc = gens_signals[i].sigdesc;
			break;
		}
	}

#ifdef HAVE_SIGACTION
	// Note: If context is nullptr, then info is invalid.
	// This may happen if SIGILL is sent to the program via kill/pkill/killall.
	const gens_signal_t *siginfo = nullptr;
	if (info && context) {
		siginfo = GetSigInfo(signum, info->si_code);
	}
#endif

	// Make sure this is the GUI thread.
	if (!GensQt4::gqt4_app->isGuiThread()) {
		// This isn't the GUI thread.
		// This uses LOG_MSG_LEVEL_ERROR in order to suppress the message box.
#ifdef HAVE_SIGACTION
		if (siginfo) {
			LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
				"Signal %d (%s: %s) received in a non-GUI thread. Hanging...",
				signum, signame, siginfo->signame);
		} else
#endif
		{
			LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
				"Signal %d (%s) received in a non-GUI thread. Hanging...",
				signum, signame);
		}

		// Signal the GUI thread that we crashed.
		// TODO: Include the ID of the thread that crashed?
		// (e.g. emulation thread, PortAudio thread)
#ifdef HAVE_SIGACTION
		gqt4_app->doCrash(signum, info, context);
#else /* HAVE_SIGACTION */
		gqt4_app->doCrash(signum);
#endif /* HAVE_SIGACTION */

		// Hang here.
		HANG();
	}

	// This uses LOG_MSG_LEVEL_ERROR in order to suppress the message box.
#ifdef HAVE_SIGACTION
	if (siginfo) {
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Signal %d (%s: %s) received. Shutting down.",
			signum, signame, siginfo->signame);
	} else
#endif
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Signal %d (%s) received. Shutting down.",
			signum, signame);
	}

	// Show a message box.
#ifdef RICKROLL
	QString sMsg = QString::fromLatin1("Gens/GS II has given you up. (Signal %1)\n").arg(signum);
#else
	QString sMsg = QString::fromLatin1("Gens/GS II has crashed with Signal %1.\n").arg(signum);
#endif

	if (signame) {
		sMsg += QString::fromLatin1(signame);
		if (sigdesc)
			sMsg += QLatin1String(": ") + QLatin1String(sigdesc);
		sMsg += QChar(L'\n');
	}

#ifdef HAVE_SIGACTION
	if (siginfo && siginfo->signame) {
		sMsg += QString::fromLatin1(siginfo->signame);
		if (siginfo->sigdesc)
			sMsg += QLatin1String(": ") + QLatin1String(siginfo->sigdesc);
		sMsg += QChar(L'\n');
	}
#endif

	sMsg += QLatin1String("\n"
			"Build Information:\n"
			"- Platform: " GENS_PLATFORM "\n"
			);

	// TODO: Use MDP version number macros.
	sMsg += QLatin1String("- Version: ") +
			QString::number((LibGens::version >> 24) & 0xFF) + QChar(L'.') +
			QString::number((LibGens::version >> 16) & 0xFF) + QChar(L'.') +
			QString::number(LibGens::version & 0xFFFF);

	if (LibGens::version_desc) {
		sMsg += QLatin1String(" (") +
				QLatin1String(LibGens::version_desc) + QChar(L')');
	}
	sMsg += QChar(L'\n');

	// VCS revision.
	if (LibGens::version_vcs) {
		sMsg += QLatin1String("- ") +
				QLatin1String(LibGens::version_vcs) + QChar(L'\n');
	}

	sMsg += QLatin1String("\n"
			"Please report this error to GerbilSoft.\n"
			"- E-mail: gerbilsoft@gerbilsoft.com\n\n"
			"Be sure to include detailed information about what you were "
			"doing when this error occurred."
			);

	// Display the message box.
	QMessageBox dialog(QMessageBox::Critical,
				QLatin1String("Gens/GS II Fatal Error"), sMsg);
	dialog.setTextFormat(Qt::PlainText);
#ifdef RICKROLL
	QPixmap pxmRickRoll(QLatin1String(":/gens/rick-roll.png"));
	if (!pxmRickRoll.isNull()) {
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
