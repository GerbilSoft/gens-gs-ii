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

// Byteswapping macros.
#include "libgens/Util/byteswap.h"

// GL_UNSIGNED_INT_8_8_8_8_REV is needed for native byte-order on PowerPC.
// When used with GL_BGRA, it's effectively the same as GL_ARGB.
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define SDLGL_UNSIGNED_BYTE GL_UNSIGNED_BYTE
#endif

namespace GensSdl {

SdlGLBackend::SdlGLBackend()
	: m_window(nullptr)
	, m_glContext(nullptr)
{
	// Initialize the SDL window.
	// NOTE: Starting with 2x resolution so the
	// window isn't ridiculously tiny.
	// FIXME: SDL window resizing is broken.
	m_winW = 640; m_winH = 480;
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
	GLBackend::update(fb_dirty);

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

}
