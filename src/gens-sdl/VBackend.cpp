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

#include "VBackend.hpp"

#include "libgens/Util/MdFb.hpp"
using LibGens::MdFb;

// C includes. (C++ namespace)
#include <cstdio>

namespace GensSdl {

VBackend::VBackend()
	: m_dirty(true)
	, m_fullscreen(false)
	, m_fb(nullptr)
	, m_int_fb(nullptr)
	, m_stretchMode(STRETCH_H)
	, m_aspectRatioConstraint(true)
	, m_pausedEffect(false)
	, m_fastBlur(false)
{ }

VBackend::~VBackend()
{
	if (m_fb) {
		m_fb->unref();
		m_fb = nullptr;
	}

	if (m_int_fb) {
		m_int_fb->unref();
		m_int_fb = nullptr;
	}
}

void VBackend::setStretchMode(StretchMode_t stretchMode)
{
	if (m_stretchMode == stretchMode)
		return;
	m_stretchMode = stretchMode;
	setDirty();
}

void VBackend::setAspectRatioConstraint(bool aspectRatioConstraint)
{
	if (m_aspectRatioConstraint == aspectRatioConstraint)
		return;
	m_aspectRatioConstraint = aspectRatioConstraint;
	setDirty();
}

void VBackend::setPausedEffect(bool pausedEffect)
{
	if (m_pausedEffect == pausedEffect)
		return;
	m_pausedEffect = pausedEffect;
	// Framebuffer must be reuploaded.
	// NOTE: If shaders are being used, this isn't true...
	setForceFbDirty();
}

void VBackend::setFastBlur(bool fastBlur)
{
	if (m_fastBlur == fastBlur)
		return;
	m_fastBlur = fastBlur;
	// Framebuffer must be reuploaded.
	// NOTE: If shaders are being used, this isn't true...
	setForceFbDirty();
}

/** Onscreen Display functions. **/

/**
 * Are any OSD messages currently onscreen?
 * @return True if OSD messages are onscreen; false if not.
 */
bool VBackend::has_osd_messages(void) const
{
	// Nothing to do here...
	return false;
}

/**
 * Process OSD messages.
 * This usually only needs to be called if the emulator is paused.
 * @return True if OSD messages were processed; false if not.
 */
bool VBackend::process_osd_messages(void)
{
	// Nothing to do here...
	return false;
}

/**
 * Print a message to the Onscreen Display.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message. (printf-formatted; UTF-8)
 * @param ap Format arguments.
 */
void VBackend::osd_vprintf(int duration, const char *msg, va_list ap)
{
	// TODO: Use vasprintf() or something similar?
	char buf[2048];
	vsnprintf(buf, sizeof(buf), msg, ap);
	osd_print(duration, buf);
}

/**
 * Print a message to the Onscreen Display.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message. (printf-formatted; UTF-8)
 * @params ... Format arguments.
 */
void VBackend::osd_printf(int duration, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	osd_vprintf(duration, msg, ap);
	va_end(ap);
}

/**
 * Print a message to the Onscreen Display.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message. (printf-formatted; UTF-8)
 */
void VBackend::osd_print(int duration, const char *msg)
{
	// Default version prints to the console.
	// TODO: Convert to local codepage on Windows?
	((void)duration);
	puts(msg);
}

/**
 * Display a preview image on the Onscreen Display.
 * @param duration Duration for the preview image to appear, in milliseconds.
 * @param img_data Image data. (If nullptr, or internal data is nullptr, hide the current image.)
 */
void VBackend::osd_preview_image(int duration, const _Zomg_Img_Data_t *img_data)
{
	// Default implementation doesn't support OSD.
	((void)duration);
	((void)img_data);
	return;
}

}
