/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlGLBackend.hpp: SDL OpenGL rendeirng backend.                         *
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

#include "SdlGLBackend.hpp"

#include "libgens/Util/MdFb.hpp"
using LibGens::MdFb;

// C includes.
#include <stdlib.h>

// System byte order.
#include "libcompat/byteorder.h"

#include <SDL.h>
#include <SDL_syswm.h>

// GL_UNSIGNED_INT_8_8_8_8_REV is needed for native byte-order on PowerPC.
// When used with GL_BGRA, it's effectively the same as GL_ARGB.
#if SYS_BYTEORDER == SYS_BIG_ENDIAN
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_INT_8_8_8_8_REV
#else /* SYS_BYTEORDER == SYS_LIL_ENDIAN */
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_BYTE
#endif

// Windows icon.
#ifdef _WIN32
#include <windows.h>
#include "win32/gens-sdl.h"
#endif

namespace GensSdl {

SdlGLBackend::SdlGLBackend()
	: super()
	, m_window(nullptr)
	, m_glContext(nullptr)
{
	// Initialize the SDL window.
	// NOTE: Starting with 2x resolution so the
	// window isn't ridiculously tiny.
	// FIXME: SDL window resizing is broken.
	m_winW = 640; m_winH = 480;

	// Set the GL attributes.
	// NOTE: We're always setting GL color depth to 888,
	// since most modern video cards prefer it compared
	// to 15-bit/16-bit.
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

	// TODO: Make sure m_window, m_glContext, etc. were created successfully.
	// TODO: Rename m_window to m_window?
	m_window = SDL_CreateWindow("Gens/GS II [SDL]",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_winW, m_winH,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	// TODO: Split icon setting into a common function?
#ifdef _WIN32
	// Set the window icon.
	HICON hIcon = LoadIcon(GetModuleHandleA(nullptr), MAKEINTRESOURCE(IDI_GENS_APP));
	if (hIcon) {
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(m_window, &info)) {
			SetClassLongPtr(info.info.win.window, GCL_HICON, (LONG_PTR)hIcon);
		}
	}
#else
	// TODO: Non-Windows icon.
#endif

	// Create the OpenGL context.
	m_glContext = SDL_GL_CreateContext(m_window);

	// Initialize OpenGL.
	initGL();
}

SdlGLBackend::~SdlGLBackend()
{
	// Shut down OpenGL.
	endGL();

	// Close the SDL/GL contexts.
	if (m_glContext) {
		SDL_GL_DeleteContext(m_glContext);
	}
	if (m_window) {
		SDL_DestroyWindow(m_window);
	}
}

/**
 * Set the window title.
 * TODO: Set based on "paused" and fps values?
 * @param title Window title.
 */
void SdlGLBackend::set_window_title(const char *title)
{
	SDL_SetWindowTitle(m_window, title);
}

/**
 * Update video.
 * @param fb_dirty If true, MdFb was updated.
 */
void SdlGLBackend::update(bool fb_dirty)
{
	// Run the GLBackend update first.
	super::update(fb_dirty);

	// Swap the GL buffers.
	SDL_GL_SwapWindow(m_window);
}

/**
 * Toggle fullscreen.
 */
void SdlGLBackend::toggle_fullscreen(void)
{
	m_fullscreen = !m_fullscreen;
	if (m_fullscreen) {
		// Switched to windowed fullscreen.
		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		// Switch to windowed mode.
		SDL_SetWindowFullscreen(m_window, 0);
	}
}

/** Absolute Mouse Movement functions. **/
/** Used for Pico emulation. **/

/**
 * Translate absolute mouse coordinates into tablet coordinates.
 * Absolute mouse coordinates are window-relative.
 * Tablet coordinates are scaled to 1280x240.
 * 1280 allows for easy conversion to 320px or 240px.
 * NOTE: If the mouse is offscreen, coordinates (-1,-1) will be returned.
 * @param x	[in, out] Mouse X coordinate.
 * @param y	[in, out] Mouse Y coordinate.
 * @return 0 if mouse is onscreen; -1 if mouse is offscreen.
 */
int SdlGLBackend::translateAbsMouseCoords(int *x, int *y) const
{
	// Tablet size.
	static const int MAX_X = 1280;
	static const int MAX_Y = 240;

	if (*x < 0 || *y < 0) {
		// One of the coordinates is offscreen.
		// Set both to offscreen.
		*x = -1;
		*y = -1;
		return -1;
	}

	if (aspectRatioConstraint()) {
		// Aspect ratio constraints are enabled.
		// NOTE: Since this is low-level OpenGL,
		// SDL does *not* scale the mouse position.
		// We have to do that ourselves.
		int tmp_win_w, tmp_win_h;
		int offset_x, offset_y;

		int expected_w = m_winH * 4;
		int expected_h = m_winW * 3;
		if (expected_h > expected_w) {
			// Window is wider than it should be.
			tmp_win_h = m_winH;
			tmp_win_w = expected_w / 3;
			offset_x = (m_winW - tmp_win_w) / 2;
			*x -= offset_x;
		} else if (expected_w > expected_h) {
			// Window is taller than it should be.
			tmp_win_w = m_winW;
			tmp_win_h = expected_h / 4;
			offset_y = (m_winH - tmp_win_h) / 2;
			*y -= offset_y;
		} else {
			// Window is 4:3.
			tmp_win_w = m_winW;
			tmp_win_h = m_winH;
			offset_x = 0;
			offset_y = 0;
		}

		// Scale the mouse coordinates to 1280x240.
		// 1280 is divisible by both 320 and 256, so it
		// can be used by both Pico and the SMS tablet.
		// TODO: Handle aspect ratio and stretch mode?
		*x = (int)((*x * MAX_X) / (float)tmp_win_w);
		*y = (int)((*y * MAX_Y) / (float)tmp_win_h);

		if (*x < 0 || *y < 0 || *x >= MAX_X || *y >= MAX_Y) {
			// Mouse pointer is onscreen, but not
			// in the visible image area.
			*x = -1;
			*y = -1;
		}
	} else {
		// Aspect ratio constraints are disabled.
		*x = (int)((*x * MAX_X) / (float)m_winW);
		*y = (int)((*y * MAX_Y) / (float)m_winH);
	}

	if (*x < 0 || *y < 0)
		return -1;
	return 0;
}

}
