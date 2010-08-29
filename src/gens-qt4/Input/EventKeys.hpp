/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EventKeys.hpp: Event key handler.                                       *
 * Used for mapping keys to non-controller events, e.g. savestates.        *
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

#ifndef __GENS_QT4_INPUT_EVENTKEYS_HPP__
#define __GENS_QT4_INPUT_EVENTKEYS_HPP__

// Gens Keys.
#include "libgens/GensInput/GensKey_t.h"

// Qt includes.
#include <QtCore/QObject>

class EventKeys : public QObject
{
	Q_OBJECT
	
	public:
		EventKeys();
		~EventKeys();
		
		bool checkEventKey(GensKey_t key, int mod);
	
	signals:
		void eventToggleFastBlur(void);
		void eventTogglePaused(void);
		
		/**
		 * eventResetEmulator(): Reset the emulator.
		 * @param hardReset If true, do a hard reset; otherwise, do a soft reset.
		 */
		void eventResetEmulator(bool hardReset);
};

#endif /* __GENS_QT4_INPUT_EVENTKEYS_HPP__ */
