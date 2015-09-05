/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EmuThread.hpp: Emulation thread.                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __GENS_QT4_EMUTHREAD_HPP__
#define __GENS_QT4_EMUTHREAD_HPP__

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

namespace GensQt4 {

class EmuThread : public QThread
{
	Q_OBJECT

	public:
		EmuThread(QObject *parent = 0);
		~EmuThread();

	private:
		typedef QThread super;
	private:
		Q_DISABLE_COPY(EmuThread)

	public:
		inline bool isStopRequested(void);

	signals:
		void frameDone(bool wasFastFrame);

	public slots:
		void resume(bool doFastFrame = false);
		void stop(void);

	protected:
		void run(void);
		QWaitCondition m_wait;
		QMutex m_mutex;

		bool m_stop;
		bool m_doFastFrame;
};

/**
 * Check if a stop is requested.
 * @return True if a stop is requested; false otherwise.
 */
inline bool EmuThread::isStopRequested(void)
{
	m_mutex.lock();
	bool ret = m_stop;
	m_mutex.unlock();
	return ret;
}

}

#endif /* __GENS_QT4_EMUTHREAD_HPP__ */
