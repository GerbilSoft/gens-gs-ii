/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * SdlSWBackend.cpp: SDL software rendeirng backend.                       *
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

#include "SdlSWBackend.hpp"

#include "libgens/Util/MdFb.hpp"
using LibGens::MdFb;

// C includes. (C++ namespace)
#include <cassert>

#include <SDL.h>
#include <SDL_syswm.h>

// Windows icon.
#ifdef _WIN32
#include <windows.h>
#include "win32/gens-sdl.h"
#endif

namespace GensSdl {

SdlSWBackend::SdlSWBackend(MdFb::ColorDepth bpp)
	: super(bpp)
	, m_window(nullptr)
	, m_renderer(nullptr)
	, m_texture(nullptr)
{
	// Initialize the SDL window.
	m_window = SDL_CreateWindow("Gens/GS II [SDL]",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				320, 240, SDL_WINDOW_RESIZABLE);

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

	// Create a renderer.
	// TODO: Parameter for enabling/disabling VSync?
	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);

	// Clear the screen.
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_renderer);
	SDL_RenderPresent(m_renderer);

	// Create a texture.
	uint32_t format;
	switch (bpp) {
		case MdFb::BPP_15:
			format = SDL_PIXELFORMAT_RGB555;
			break;
		case MdFb::BPP_16:
			format = SDL_PIXELFORMAT_RGB565;
			break;
		case MdFb::BPP_32:
		default:
			format = SDL_PIXELFORMAT_ARGB8888;
			break;
	}
	m_texture = SDL_CreateTexture(m_renderer, format,
			SDL_TEXTUREACCESS_STREAMING,
			320, 240);
}

SdlSWBackend::~SdlSWBackend()
{
	if (m_fb) {
		m_fb->unref();
		m_fb = nullptr;
	}

	SDL_DestroyTexture(m_texture);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
}

/**
 * Set the window title.
 * TODO: Set based on "paused" and fps values?
 * @param title Window title.
 */
void SdlSWBackend::set_window_title(const char *title)
{
	SDL_SetWindowTitle(m_window, title);
}

/**
 * Set the SDL video source to an MdFb.
 * If nullptr, removes the SDL video source.
 * @param fb MdFb.
 */
void SdlSWBackend::set_video_source(LibGens::MdFb *fb)
{
	// Free the existing MD surface first.
	if (m_fb) {
		m_fb->unref();
		m_fb = nullptr;
	}

	if (fb) {
		m_fb = fb->ref();
	}
}


/**
 * Update video.
 * @param fb_dirty If true, MdFb was updated.
 */
void SdlSWBackend::update(bool fb_dirty)
{
	// We always have to draw the MdFb in software mode.
	// TODO: Is this true with SDL2?
	((void)fb_dirty);

	// Clear the screen before doing anything else.
	SDL_RenderClear(m_renderer);
	if (m_fb) {
		// Source surface is available.
		// TODO: Verify color depth.
		if (m_bpp == MdFb::BPP_32) {
			SDL_UpdateTexture(m_texture, nullptr, m_fb->fb32(), m_fb->pxPitch() * sizeof(uint32_t));
		} else {
			SDL_UpdateTexture(m_texture, nullptr, m_fb->fb16(), m_fb->pxPitch() * sizeof(uint16_t));
		}
		SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	}

	// Update the screen.
	SDL_RenderPresent(m_renderer);

	// VBackend is no longer dirty.
	clearDirty();
}

/**
 * Viewing area has been resized.
 * @param width Width.
 * @param height Height.
 */
void SdlSWBackend::resize(int width, int height)
{
	// SDL 2.0 automatically handles resize for standard rendering.
	// TODO: Aspect ratio constraints?
	((void)width);
	((void)height);
	return;
}

/**
 * Toggle fullscreen.
 */
void SdlSWBackend::toggle_fullscreen(void)
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
