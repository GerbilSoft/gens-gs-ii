/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * IdleThread.hpp: Idle thread.                                            *
 * This thread runs the Intro Effect when the emulator is idle.            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __GENS_QT4_IDLETHREAD_HPP__
#define __GENS_QT4_IDLETHREAD_HPP__

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

namespace GensQt4
{

class IdleThread : public QThread
{
	Q_OBJECT
	
	public:
		IdleThread(QObject *parent = 0);
		~IdleThread();
		
		bool isStopRequested(void);
	
	signals:
		void frameDone(void);
	
	public slots:
		void resume(void);
		void stop(void);
	
	protected:
		void run(void);
		QWaitCondition m_wait;
		QMutex m_mutex;
		
		bool m_stop;
};


/**
 * isStopRequested(): Check if a stop is requested.
 * @return True if a stop is requested; false otherwise.
 */
inline bool IdleThread::isStopRequested(void)
{
	m_mutex.lock();
	bool ret = m_stop;
	m_mutex.unlock();
	return ret;
}

}

#endif /* __GENS_QT4_EMUTHREAD_HPP__ */
