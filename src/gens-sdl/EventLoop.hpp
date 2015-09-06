/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * EventLoop.hpp: Event loop base class.                                   *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __GENS_SDL_EVENTLOOP_HPP__
#define __GENS_SDL_EVENTLOOP_HPP__

#include <SDL.h>

// from qglobal.h (qt-4.8.6)
// TODO: Move to a common header, along with implementations
// of Q_DISABLE_COPY(), Q_D(), Q_Q(), etc.?
#define EVENT_LOOP_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(d_ptr); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(d_ptr); } \
    friend class Class##Private;

namespace GensSdl {

class VBackend;

class EventLoopPrivate;
class EventLoop
{
	public:
		EventLoop(EventLoopPrivate *d);
		virtual ~EventLoop();

	protected:
		EventLoopPrivate *const d_ptr;
		EVENT_LOOP_DECLARE_PRIVATE(EventLoop)
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		EventLoop(const EventLoop &);
		EventLoop &operator=(const EventLoop &);

	public:
		/**
		 * Run the event loop.
		 * @param rom_filename ROM filename. [TODO: Replace with options struct?]
		 * @return Exit code.
		 */
		virtual int run(const char *rom_filename) = 0;

		/**
		 * Get the VBackend.
		 * @return VBackend.
		 */
		VBackend *vBackend(void) const;

	protected:
		/**
		 * Process an SDL event.
		 * @param event SDL event.
		 * @return 0 if the event was handled; non-zero if it wasn't.
		 */
		virtual int processSdlEvent(const SDL_Event *event);

		/**
		 * Process the SDL event queue.
		 * If emulation is paused and the OSD message,
		 * list is empty, SDL_WaitEvent() will be used
		 * to wait for the next event.
		 */
		void processSdlEventQueue(void);

	protected:
		// TODO: Move to EventLoopPrivate?

		/**
		 * Run a frame.
		 * This function handles frameskip timing.
		 * Call this function from run().
		 */
		void runFrame(void);

		/**
		 * Run a normal frame.
		 * This function is called by runFrame(),
		 * and should be handled by running a full
		 * frame with video and audio updates.
		 */
		virtual void runFullFrame(void) = 0;

		/**
		 * Run a fast frame.
		 * This function is called by runFrame() if the
		 * system is lagging a bit, and should be handled
		 * by running a frame with audio updates only.
		 */
		virtual void runFastFrame(void) = 0;
};

}

#endif /* __GENS_SDL_EVENTLOOP_HPP__ */
