/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * VBackend.hpp: Video Backend base class.                                 *
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

#ifndef __GENS_SDL_VBACKEND_HPP__
#define __GENS_SDL_VBACKEND_HPP__

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cstdarg>

// utf8_str
#include "libgens/macros/common.h"

// TODO: Minimum gcc version, other compilers?
// TODO: Move to libgens/macros/common.h?
#ifdef __GNUC__
#define ATTR_FORMAT_PRINTF(fmt, varargs) \
	__attribute__ ((format (printf, (fmt), (varargs))))
#else
#define ATTR_FORMAT_PRINTF(fmt, varargs)
#endif

namespace LibGens {
	class MdFb;
	class EmuContext;
}

namespace GensSdl {

class VBackend {
	public:
		VBackend();
		virtual ~VBackend();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		VBackend(const VBackend &);
		VBackend &operator=(const VBackend &);

	public:
		/**
		 * Set the window title.
		 * TODO: Set based on "paused" and fps values?
		 * @param title Window title.
		 */
		virtual void set_window_title(const char *title) = 0;

		/**
		 * Set the video source to an MdFb.
		 * If nullptr, removes the video source.
		 * @param fb MdFb.
		 */
		virtual void set_video_source(LibGens::MdFb *fb) = 0;

		/**
		 * Update video.
		 * @param fb_dirty If true, MdFb was updated.
		 */
		virtual void update(bool fb_dirty) = 0;

		/**
		 * Viewing area has been resized.
		 * @param width Width.
		 * @param height Height.
		 */
		virtual void resize(int width, int height) = 0;

		/**
		 * Toggle fullscreen.
		 */
		virtual void toggle_fullscreen(void) = 0;

		/**
		 * Is the VBackend dirty?
		 * This is true if any of the properties have changed
		 * and the image hasn't been updated.
		 * TODO: Also OSD?
		 * @return True if dirty; false if not.
		 */
		bool isDirty(void) const;

	public:
		/** Properties. **/

		// Stretch mode.
		enum StretchMode_t {
			STRETCH_NONE = 0,
			STRETCH_H,
			STRETCH_V,
			STRETCH_FULL,

			STRETCH_MAX
		};
		StretchMode_t stretchMode(void) const;
		void setStretchMode(StretchMode_t stretchMode);

	public:
		/**
		 * Print a message to the Onscreen Display.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message. (printf-formatted; UTF-8)
		 * @param ap Format arguments.
		 */
		virtual void osd_vprintf(const int duration, const utf8_str *msg, va_list ap)
			ATTR_FORMAT_PRINTF(3, 0);

		/**
		 * Print a message to the Onscreen Display.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message. (printf-formatted; UTF-8)
		 * @params ... Format arguments.
		 */
		void osd_printf(const int duration, const utf8_str *msg, ...)
			ATTR_FORMAT_PRINTF(3, 4);

	private:
		// Dirty flag.
		bool m_dirty;

	protected:
		// Is fullscreen?
		// TODO: Is there a way to check if SDL is fullscreen
		// without maintaining the state here?
		bool m_fullscreen;

	protected:
		// MdFb object.
		LibGens::MdFb *m_fb;

		// Dirty flag functions.
		void setDirty(void);
		void clearDirty(void);

		// Properties.
		StretchMode_t m_stretchMode;
};

/** Property accessors. **/

inline bool VBackend::isDirty(void) const
	{ return m_dirty; }
inline void VBackend::setDirty(void)
	{ m_dirty = true; }
inline void VBackend::clearDirty(void)
	{ m_dirty = false; }
inline VBackend::StretchMode_t VBackend::stretchMode(void) const
	{ return m_stretchMode; }

}

#endif /* __GENS_SDL_VBACKEND_HPP__ */
