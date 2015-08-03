/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdGL.hpp: Onscreen Display for OpenGL.                                 *
 *                                                                         *
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

#ifndef __GENS_SDL_OSDGL_HPP__
#define __GENS_SDL_OSDGL_HPP__

// C includes.
#include <stdint.h>

// utf8_str
#include "libgens/macros/common.h"

// ZOMG image data.
struct _Zomg_Img_Data_t;

namespace GensSdl {

class OsdGLPrivate;
class OsdGL {
	public:
		OsdGL();
		~OsdGL();

	private:
		friend class OsdGLPrivate;
		OsdGLPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		OsdGL(const OsdGL &);
		OsdGL &operator=(const OsdGL &);

	public:
		/**
		 * Initialize the Onscreen Display.
		 * This must be called from a valid GL context.
		 */
		void init(void);

		/**
		 * Shut down the Onscreen Display.
		 * This must be called from a valid GL context.
		 */
		void end(void);

		/**
		 * Draw the Onscreen Display.
		 * This must be called from a valid GL context.
		 */
		void draw(void);

		/**
		 * Set the display offset.
		 * This is used for aspect ratio constraints.
		 * @param x X offset.
		 * @param y Y offset.
		 */
		void setDisplayOffset(double x, double y);

		/**
		 * Are messages present in the message queue?
		 * This should be queried to determine if the
		 * video backend needs to be updated.
		 * @return True if messages are present; false if not.
		 */
		bool hasMessages(void) const;

		/**
		 * Process messages.
		 * This usually only needs to be called if the emulator is paused.
		 * @return True if messages were processed; false if not.
		 */
		bool processMessages(void);

	public:
		// NOTE: printf() functions won't be added here.
		// They can be implemented by VBackend.

		/**
		 * Add a message to the OSD queue.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message. (UTF-8)
		 */
		void print(unsigned int duration, const utf8_str *msg);

		// TODO: Implement this.
		// strchr() isn't available for UTF-16...
		/**
		 * Add a message to the OSD queue.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message. (UTF-16)
		 */
		void print(unsigned int duration, const char16_t *msg);

		/**
		 * Display a preview image.
		 * @param duration Duration for the preview image to appear, in milliseconds.
		 * @param img_data Preview image.
		 */
		void preview_image(int duration, const _Zomg_Img_Data_t *img_data);

	public:
		/** Properties. **/

		bool isFpsEnabled(void) const;
		void setFpsEnabled(bool fpsEnabled);

		bool isMsgEnabled(void) const;
		void setMsgEnabled(bool msgEnabled);

		// Colors are in the same format as VdpPalette.
		// 32-bit ARGB

		uint32_t fpsColor(void) const;
		void setFpsColor(uint32_t fpsColor);

		uint32_t msgColor(void) const;
		void setMsgColor(uint32_t msgColor);
};

}

#endif /* __GENS_SDL_OSDGL_HPP__ */
