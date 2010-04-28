/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * MtQueue.cpp: Multithreaded queue class for IPC.                         *
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

#include "MtQueue.hpp"

#include <queue>
using std::queue;

namespace LibGens
{

MtQueue::MtQueue(MtQ_callback_fn callback)
{
	m_callback = callback;
	
	// Create the SDL mutex.
	// TODO: Throw an exception if the mutex couldn't be created.
	m_mutex = SDL_CreateMutex();
}

MtQueue::~MtQueue()
{
	// Destroy the SDL mutex.
	SDL_DestroyMutex(m_mutex);
}


/**
 * push(): Push a message onto the MtQueue.
 * @param type Message type.
 * @param param Parameter.
 * @param 0 on success; non-zero on error.
 */
int MtQueue::push(MtQueue::MtQ_type type, void *param)
{
	if (type <= MtQueue::MTQ_NONE || type >= MtQueue::MTQ_MAX)
	{
		// Invalid message type.
		return -1;
	}
	
	SDL_LockMutex(m_mutex);
	m_queue.push(MtQueue::MtQ_elem(type, param));
	SDL_UnlockMutex(m_mutex);
	
	if (m_callback)
		m_callback(this);
	
	// Message pushed successfully.
	return 0;
}


/**
 * pop(): Pop a message from MtQueue.
 * @param r_param Return variable for the parameter.
 * @return Message type. (MTQ_NONE if no messages are available.)
 */
MtQueue::MtQ_type MtQueue::pop(void **r_param)
{
	MtQueue::MtQ_type r_type;
	
	SDL_LockMutex(m_mutex);
	if (m_queue.empty())
	{
		// Queue is empty.
		r_type = MtQueue::MTQ_NONE;
	}
	else
	{
		// Queue is not empty.
		const MtQueue::MtQ_elem& elem = m_queue.front();
		r_type = elem.type;
		*r_param = elem.param;
		m_queue.pop();
	}
	SDL_UnlockMutex(m_mutex);
	
	return r_type;
}

}
