/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * SigHandler.hpp: Signal handler.                                         *
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

#ifndef __GENS_QT4_SIGHANDLER_HPP__
#define __GENS_QT4_SIGHANDLER_HPP__

#include <config.h>

#ifdef HAVE_SIGACTION
// C includes.
#include <signal.h>
#endif

// LibGens includes
#include "libgens/Util/gens_siginfo.h"

namespace GensQt4
{

// Forward declaration of GensQApplication.
class GensQApplication;

class SigHandler
{
	public:
		static void Init(void);
		static void End(void);
	
	private:
		// Static class; prevent instantiation.
		SigHandler() { }
		~SigHandler() { }
		
#ifdef HAVE_SIGACTION
		static const gens_signal_t *GetSigInfo(int signum, int si_code);
		static void SignalHandler(int signum, siginfo_t *info, void *context);
#else
		static void SignalHandler(int signum);
#endif
		friend class GensQApplication;
};

}

#endif /* __GENS_QT4_SIGHANDLER_HPP__ */
