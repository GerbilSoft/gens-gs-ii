/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * GLBackend.hpp: OpenGL rendeirng backend.                                *
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

#ifndef __GENS_SDL_GLBACKEND_HPP__
#define __GENS_SDL_GLBACKEND_HPP__

// C includes.
#include <stdint.h>

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// Video Backend.
#include "VBackend.hpp"
// LibGens includes.
#include "libgens/Util/MdFb.hpp"

namespace GensSdl {

class GLBackendPrivate;
class GLBackend : public VBackend {
	public:
		GLBackend();
		virtual ~GLBackend();

	private:
		friend class GLBackendPrivate;
		GLBackendPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		GLBackend(const GLBackend &);
		GLBackend &operator=(const GLBackend &);

	public:
		/**
		 * Set the video source to an MdFb.
		 * If nullptr, removes the video source.
		 * @param fb MdFb.
		 */
		virtual void set_video_source(LibGens::MdFb *fb) final;

		/**
		 * Update video.
		 * @param fb_dirty If true, MdFb was updated.
		 */
		virtual void update(bool fb_dirty) override;

		/**
		 * Viewing area has been resized.
		 * @param width Width.
		 * @param height Height.
		 */
		virtual void resize(int width, int height) override;

	public:
		/** Onscreen Display functions. **/

		/**
		 * Are any OSD messages currently onscreen?
		 * @return True if OSD messages are onscreen; false if not.
		 */
		virtual bool has_osd_messages(void) const final;

		/**
		 * Process OSD messages.
		 * This usually only needs to be called if the emulator is paused.
		 * @return True if OSD messages were processed; false if not.
		 */
		virtual bool process_osd_messages(void) final;

		/**
		 * Print a message to the Onscreen Display.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message. (printf-formatted; UTF-8)
		 * @param ap Format arguments.
		 */
		virtual void osd_vprintf(int duration, const utf8_str *msg, va_list ap) final
			ATTR_FORMAT_PRINTF(3, 0);

		/**
		 * Display a preview image on the Onscreen Display.
		 * @param duration Duration for the preview image to appear, in milliseconds.
		 * @param img_data Image data. (If nullptr, or internal data is nullptr, hide the current image.)
		 */
		virtual void osd_preview_image(int duration, const _Zomg_Img_Data_t *img_data) final;

	protected:
		// Window size.
		// TODO: Accessors?
		int m_winW, m_winH;

	protected:
		/**
		 * Initialize OpenGL.
		 * This must be called by the subclass constructor.
		 */
		void initGL(void);

		/**
		 * Shut down OpenGL.
		 * This must be called by the subclass destructor.
		 */
		void endGL(void);
};

}

#endif /* __GENS_SDL_GLBACKEND_HPP__ */
