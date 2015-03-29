/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlHandler.hpp: SDL library handler.                                    *
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

#ifndef __GENS_SDL_SDLHANDLER_HPP__
#define __GENS_SDL_SDLHANDLER_HPP__

#include <stdint.h>

// SDL
#include <SDL.h>

namespace LibGens {
	class MdFb;
}

namespace GensSdl {

class SdlHandler {
	public:
		SdlHandler();
		~SdlHandler();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		SdlHandler(const SdlHandler &);
		SdlHandler &operator=(const SdlHandler &);

	public:
		/**
		 * Initialize SDL video.
		 * TODO: Parameter for GL rendering.
		 * @return 0 on success; non-zero on error.
		 */
		int init_video(void);

		/**
		 * Shut down SDL video.
		 */
		void end_video(void);

		/**
		 * Set the SDL video source to an MdFb.
		 * If nullptr, removes the SDL video source.
		 * @param fb MdFb.
		 */
		void set_video_source(LibGens::MdFb *fb);

		/**
		 * Update SDL video.
		 */
		void update_video(void);

		/**
		 * Initialize SDL timers and threads.
		 * @return 0 on success; non-zero on error.
		 */
		int init_timers(void);

		/**
		 * Shut down SDL timers and threads.
		 */
		void end_timers(void);

		/**
		 * Start and/or restart the synchronization timer.
		 * @param isPal If true, use PAL timing.
		 */
		void start_timer(bool isPal);

		/**
		 * Wait for frame synchronization.
		 * On every third frame, wait for the timer.
		 */
		void wait_for_frame_sync(void);

		/**
		 * Initialize SDL audio.
		 * @return 0 on success; non-zero on error.
		 */
		int init_audio(void);

		/**
		 * Shut down SDL audio.
		 */
		void end_audio(void);

		/**
		 * Update SDL audio using SoundMgr.
		 */
		void update_audio(void);

	private:
		/**
		 * SDL synchronization timer callback.
		 * @param interval Timer interval.
		 * @param param SdlHandler class pointer.
		 * @return Timer interval to use.
		 */
		static uint32_t sdl_timer_callback(uint32_t interval, void *param);

		/**
		 * SDL audio callback.
		 * @param userdata SdlHandler class pointer.
		 * @param stream SDL audio stream.
		 * @param len Number of bytes requested.
		 */
		static void sdl_audio_callback(void *userdata, uint8_t *stream, int len);

	private:
		// Screen buffer.
		SDL_Surface *m_screen;
		// MdFb object.
		LibGens::MdFb *m_fb;
		// MD screen buffer.
		// Points to data on an MdFb.
		SDL_Surface *m_md;

		// Timers and threads.
		SDL_sem *m_sem;
		unsigned int m_ticks;
		SDL_TimerID m_timer;
		bool m_isPal;

		// Frames rendered.
		int m_framesRendered;

		// Audio.
		static const int SEGMENTS_TO_BUFFER = 4;
		uint8_t *m_audioBuffer;
		int m_audioBufferLen;		// Length of the audio buffer.
		int m_audioBufferUsed;		// Current number of bytes used in the buffer.
		int m_sampleSize;		// Sample size. (Should be 4)
		uint8_t *m_audioWritePos;	// Write position within the buffer.
};

}

#endif /* __GENS_SDL_SDLHANDLER_HPP__ */
