/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * MsgTimer.hpp: Message timer class.                                      *
 * This class ensures that OSD messages are removed from the screen after  *
 * their duration expires when emulation isn't running.                    *
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

#ifndef __GENS_QT4_VBACKEND_MSGTIMER_HPP__
#define __GENS_QT4_VBACKEND_MSGTIMER_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace GensQt4
{

// VBackend.
class VBackend;

class MsgTimerPrivate;

class MsgTimer : public QObject
{
	Q_OBJECT
	
	public:
		MsgTimer(QObject *parent);
		MsgTimer(VBackend *vBackend, QObject *parent);
		~MsgTimer();
	
	private:
		MsgTimerPrivate *d;
		Q_DISABLE_COPY(MsgTimer);
	
	public:
		// 100 ms is a decent interval when emulation isn't running.
		static const int MSGTIMER_INTERVAL = 100;
		static const int MSGTIMER_INTERVAL_QUICK = 20;
		
		/**
		 * Start the message timer.
		 */
		void start(void);
		
		/**
		 * Start the message timer with the quick interval initially.
		 */
		void startQuick(void);
	
	private slots:
		/**
		 * Check the OSD messages.
		 */
		void checkMsg(void);
};

}

#endif /* __GENS_QT4_VBACKEND_MSGTIMER_HPP__ */
