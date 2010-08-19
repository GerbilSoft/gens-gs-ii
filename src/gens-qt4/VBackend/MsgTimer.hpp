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

#include "VBackend.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace GensQt4
{

class MsgTimer : public QObject
{
	Q_OBJECT
	
	public:
		MsgTimer(VBackend *vBackend)
		{
			m_vBackend = vBackend;
			connect(&m_timer, SIGNAL(timeout()),
				this, SLOT(checkMsg()));
		}
		~MsgTimer() { }
		
		// 100 ms is a decent interval when emulation isn't running.
		static const int MSGTIMER_INTERVAL = 100;
		
		/**
		 * start(): Start the message timer.
		 */
		void start(void)
		{
			m_timer.start(MSGTIMER_INTERVAL);
		}
	
	protected:
		VBackend *m_vBackend;
		QTimer m_timer;
	
	protected slots:
		void checkMsg(void)
		{
			// Message timer tick.
			// Check the VBackend for messages.
			int num = m_vBackend->osd_process();
			
			if (num <= 0)
			{
				// No more messages.
				// Stop the timer.
				m_timer.stop();
			}
		}
};

}

#endif /* __GENS_QT4_VBACKEND_MSGTIMER_HPP__ */
