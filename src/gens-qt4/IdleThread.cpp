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

#include "IdleThread.hpp"
#include "gqt4_main.hpp"

// LibGens effects.
// TODO: Gens Logo effect.
// TODO: Do the Gens Logo effect in GL?
#include "libgens/Effects/CrazyEffect.hpp"

namespace GensQt4
{

IdleThread::IdleThread(QObject *parent)
	: QThread(parent)
{
	m_stop = false;
}

IdleThread::~IdleThread()
{
	// NOTE: I don't think this works in the destructor...
#if 0
	// Stop the thread and wait for it to finish.
	this->stop();
	this->wait();
#endif
}

void IdleThread::resume(void)
{
	m_mutex.lock();
	m_wait.wakeAll();
	m_mutex.unlock();
}

void IdleThread::stop(void)
{
	m_mutex.lock();
	m_stop = true;
	m_wait.wakeAll();
	m_mutex.unlock();
}

void IdleThread::run(void)
{
	// TODO: Use an enum for the Intro Effect Style.
	
	// Run the idle thread.
	m_mutex.lock();
	while (!m_stop)
	{
		// Run the intro effect.
		switch (gqt4_config->introStyle())
		{
			case 0:
			default:
				// No intro effect.
				// Stop the thread.
				m_stop = true;
				break;
			
			case 1:
				// Gens Logo effect.
				// TODO
				m_stop = true;
				break;
			
			case 2:
				// "Crazy" effect.
				LibGens::CrazyEffect::DoCrazyEffect(
					(LibGens::CrazyEffect::ColorMask)gqt4_config->introColor());
				emit frameDone();
				usleep(20000);
				break;
		}
		
		if (m_stop)
			break;
		
		// Wait for a resume command.
		m_wait.wait(&m_mutex);
	}
	m_mutex.unlock();
}

}
