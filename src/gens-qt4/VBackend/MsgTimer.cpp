/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * MsgTimer.cpp: Message timer class.                                      *
 * This class ensures that OSD messages are removed from the screen after  *
 * their duration expires when emulation isn't running.                    *
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

#include "MsgTimer.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QTimer>

// Video Backend.
#include "VBackend.hpp"

namespace GensQt4
{

class MsgTimerPrivate
{
	public:
		VBackend *vBackend;
		QTimer *timer;
};

MsgTimer::MsgTimer(VBackend *vBackend)
	: d(new MsgTimerPrivate())
{
	d->vBackend = vBackend;
	
	// Initialize the QTimer.
	d->timer = new QTimer(this);
	connect(d->timer, SIGNAL(timeout()),
		this, SLOT(checkMsg()));
}

MsgTimer::~MsgTimer()
	{ delete d; }

/**
 * Start the message timer.
 */
void MsgTimer::start(void)
	{ d->timer->start(MSGTIMER_INTERVAL); }

/**
 * Check the OSD messages.
 */
void MsgTimer::checkMsg(void)
{
	// Message timer tick.
	// Check the VBackend for messages.
	int num = d->vBackend->osd_process();
	
	if (num <= 0)
	{
		// No more messages.
		// Stop the timer.
		d->timer->stop();
	}
}

}
