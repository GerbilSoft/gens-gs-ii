/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQApplication.hpp: QApplication subclass.                            *
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

#ifndef __GENS_QT4_GENSQAPPLICATION_HPP__
#define __GENS_QT4_GENSQAPPLICATION_HPP__

#include <config.h>

#include <QtGui/QApplication>
#include <QtCore/QThread>

#include "SigHandler.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "gqt4_win32.hpp"
#endif

namespace GensQt4
{

class GensQApplication : public QApplication
{
	Q_OBJECT
	
	public:
		GensQApplication(int &argc, char **argv)
			: QApplication(argc, argv)
		{
			gqaInit();
		}
		
		GensQApplication(int &argc, char **argv, bool GUIenabled)
			: QApplication(argc, argv, GUIenabled)
		{
			gqaInit();
		}
		
		GensQApplication(int &argc, char **argv, Type type)
			: QApplication(argc, argv, type)
		{
			gqaInit();
		}
		
		~GensQApplication() { }
		
		/**
		 * isGuiThread(): Check if the current thread is the GUI thread.
		 * @return True if it is; false if it isn't.
		 */
		inline bool isGuiThread(void)
		{
			return (QThread::currentThread() == m_guiThread);
		}
		
#ifdef _WIN32
		/**
		 * winEventFilter(): Win32 event filter.
		 * @param msg Win32 message.
		 * @param result Return value for the window procedure.
		 * @return True if we're handling the message; false if we should let Qt handle the message.
		 */
		bool winEventFilter(MSG *msg, long *result)
		{
			if (msg->message != WM_SETTINGCHANGE)
				return false;
			if (msg->wParam != SPI_SETNONCLIENTMETRICS)
				return false;
			
			// WM_SETTINGCHANGE / SPI_SETNONCLIENTMETRICS.
			// Update the Qt font.
			Win32_SetFont();
			return false;
		}
#endif
		
		/**
		 * HACK: The following mess is a hack to get the
		 * custom signal handler dialog to work across
		 * multiple threads. Don't mess around with it!
		 */
		
	signals:
#ifdef HAVE_SIGACTION
		void signalCrash(int signum, siginfo_t *info, void *context);
#else /* HAVE_SIGACTION */
		void signalCrash(int signum);
#endif /* HAVE_SIGACTION */
	
	protected:
		QThread *m_guiThread;
		
		/**
		 * gqaInit(): GensQApplication initialization function.
		 * The same code is used in all three GensQApplication() constructors.
		 */
		inline void gqaInit(void)
		{
			// Save the GUI thread pointer for later.
			m_guiThread = QThread::currentThread();
			
			// Connect the crash handler.
#ifdef HAVE_SIGACTION
			connect(this, SIGNAL(signalCrash(int, siginfo_t*, void*)),
				this, SLOT(slotCrash(int, siginfo_t*, void*)));
#else /* HAVE_SIGACTION */
			connect(this, SIGNAL(signalCrash(int)),
				this, SLOT(slotCrash(int)));
#endif /* HAVE_SIGACTION */
		}
		
		friend class SigHandler; // Allow SigHandler to call doCrash().
#ifdef HAVE_SIGACTION
		inline void doCrash(int signum, siginfo_t *info, void *context)
		{
			emit signalCrash(signum, info, context);
		}
#else /* HAVE_SIGACTION */
		inline void doCrash(int signum)
		{
			emit signalCrash(signum);
		}
#endif /* HAVE_SIGACTION */
	
	protected slots:
#ifdef HAVE_SIGACTION
		inline void slotCrash(int signum, siginfo_t *info, void *context)
		{
			SigHandler::SignalHandler(signum, info, context);
		}
#else /* HAVE_SIGACTION */
		inline void slotCrash(int signum)
		{
			SigHandler::SignalHandler(signum);
		}
#endif /* HAVE_SIGACTION */
};

}

#endif /* __GENS_QT4_GENSQAPPLICATION_HPP__ */
