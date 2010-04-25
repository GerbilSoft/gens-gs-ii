/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * MtQueue.hpp: Multithreaded queue class for IPC.                         *
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

#ifndef __LIBGENS_MTQUEUE_HPP__
#define __LIBGENS_MTQUEUE_HPP__

#include <queue>
#include <SDL/SDL.h>

// Used for notifying SDL.
#define SDL_EVENT_MTQ 0x51

namespace LibGens
{

class MtQueue
{
	public:
		MtQueue(bool notifySDL = false);
		~MtQueue();
		
		/**
		* MtQ_type: MtQueue message type.
		*/
		enum MtQ_type
		{
			MTQ_NONE = 0,
			
			/**
			 * MTQ_LG_SETBGCOLOR: Test message to set the LibGens background color.
			 * @param param 24-bit RGB color value. (BGR)
			 */
			MTQ_LG_SETBGCOLOR,
			
			/**
			 * MTQ_LG_UPDATE: Update the SDL window.
			 * @param param Unused.
			 */
			MTQ_LG_UPDATE,
			
			MTQ_MAX
		};

		struct MtQ_elem
		{
			MtQ_type type;
			void *param;
			
			MtQ_elem(MtQ_type n_type, void *n_param)
			{
				this->type = n_type;
				this->param = n_param;
			}
		};
		
		int push(MtQ_type type, void *param);
		MtQ_type pop(void **r_param);
	
	private:
		std::queue<MtQ_elem> m_queue;
		bool m_notifySDL;
		SDL_mutex *m_mutex;
};

}

#endif /* __LIBGENS_MTQUEUE_HPP__ */
