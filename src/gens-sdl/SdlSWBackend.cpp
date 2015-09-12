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

class SdlSWBackendPrivate
{
	public:
		SdlSWBackendPrivate(SdlSWBackend *q);
		~SdlSWBackendPrivate();

	private:
		friend class SdlSWBackend;
		SdlSWBackend *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		SdlSWBackendPrivate(const SdlSWBackendPrivate &);
		SdlSWBackendPrivate &operator=(const SdlSWBackendPrivate &);

	public:
		// Screen context.
		SDL_Window *window;
		SDL_Renderer *renderer;
		SDL_Texture *texture;

		// Last color depth.
		MdFb::ColorDepth lastBpp;

		// Window size.
		int winW, winH;

		// Previous aspect ratio constraint.
		bool prevAspectRatioConstraint;

	public:
		/**
		 * (Re-)Initialize the texture.
		 * If m_fb is set, uses m_fb's color depth.
		 * Otherwise, BPP_32 is used.
		 */
		void reinitTexture(void);
};

/** SdlSWBackendPrivate **/

SdlSWBackendPrivate::SdlSWBackendPrivate(SdlSWBackend *q)
	: q(q)
	, window(nullptr)
	, renderer(nullptr)
	, texture(nullptr)
	, lastBpp(MdFb::BPP_MAX)
	, winW(320), winH(240)
	, prevAspectRatioConstraint(false)
{
	// lastBpp is initialized to MdFb::BPP_MAX in order to
	// ensure that the texture is initialized. If it's set
	// to MdFb::BPP_32, then the texture wouldn't get
	// initialized if m_fb was also set to MdFb::BPP_32.

	// prevAspectRatioConstraint is initialized to false, since
	// SDL doesn't enable constraints by default.
}

SdlSWBackendPrivate::~SdlSWBackendPrivate()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

/**
 * (Re-)Initialize the texture.
 * If m_fb is set, uses m_fb's color depth.
 * Otherwise, BPP_32 is used.
 */
void SdlSWBackendPrivate::reinitTexture(void)
{
	if (!q->m_fb) {
		// No framebuffer. Don't do anything.
		return;
	}

	const MdFb::ColorDepth bpp = q->m_fb->bpp();
	if (lastBpp == bpp) {
		// Color depth hasn't changed.
		return;
	}

	// Determine the SDL texture format.
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

	if (texture) {
		// Destroy the existing texture.
		SDL_DestroyTexture(texture);
	}

	// Create the texture.
	texture = SDL_CreateTexture(renderer, format,
			SDL_TEXTUREACCESS_STREAMING,
			320, 240);
	// Save the last color depth.
	lastBpp = bpp;
}

/** SdlSWBackend **/

SdlSWBackend::SdlSWBackend()
	: super()
	, d(new SdlSWBackendPrivate(this))
{
	// Initialize the SDL window.
	d->window = SDL_CreateWindow("Gens/GS II [SDL]",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				d->winW, d->winH,
				SDL_WINDOW_RESIZABLE);

	// TODO: Split icon setting into a common function?
#ifdef _WIN32
	// Set the window icon.
	HICON hIcon = LoadIcon(GetModuleHandleA(nullptr), MAKEINTRESOURCE(IDI_GENS_APP));
	if (hIcon) {
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(d->window, &info)) {
			SetClassLongPtr(info.info.win.window, GCL_HICON, (LONG_PTR)hIcon);
		}
	}
#else
	// TODO: Non-Windows icon.
#endif

	// Create a renderer.
	// TODO: Parameter for enabling/disabling VSync?
	d->renderer = SDL_CreateRenderer(d->window, -1, SDL_RENDERER_PRESENTVSYNC);

	// Clear the screen.
	SDL_SetRenderDrawColor(d->renderer, 0, 0, 0, 255);
	SDL_RenderClear(d->renderer);
	SDL_RenderPresent(d->renderer);

	// Initialize the texture.
	d->reinitTexture();
}

SdlSWBackend::~SdlSWBackend()
{
	delete d;
}

/**
 * Set the window title.
 * TODO: Set based on "paused" and fps values?
 * @param title Window title.
 */
void SdlSWBackend::set_window_title(const char *title)
{
	SDL_SetWindowTitle(d->window, title);
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
		d->reinitTexture();
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
	SDL_RenderClear(d->renderer);

	// Check if the Aspect Ratio Constraint needs to be updated.
	if (d->prevAspectRatioConstraint != m_aspectRatioConstraint) {
		// Update the Aspect Ratio Constraint.
		d->prevAspectRatioConstraint = m_aspectRatioConstraint;
		if (m_aspectRatioConstraint) {
			// TODO: Use MdFb size?
			SDL_RenderSetLogicalSize(d->renderer, 320, 240);
		} else {
			SDL_RenderSetLogicalSize(d->renderer, 0, 0);
		}
	}

	if (m_fb) {
		// Source surface is available.
		const MdFb::ColorDepth bpp = m_fb->bpp();
		if (bpp != d->lastBpp) {
			// Color depth has changed.
			d->reinitTexture();
		}

		// Update the texture.
		if (bpp == MdFb::BPP_32) {
			SDL_UpdateTexture(d->texture, nullptr,
				m_fb->fb32(), m_fb->pxPitch() * sizeof(uint32_t));
		} else {
			SDL_UpdateTexture(d->texture, nullptr,
				m_fb->fb16(), m_fb->pxPitch() * sizeof(uint16_t));
		}
		SDL_RenderCopy(d->renderer, d->texture, nullptr, nullptr);
	}

	// Update the screen.
	SDL_RenderPresent(d->renderer);

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
	// Save the window size for later.
	d->winW = width;
	d->winH = height;

	// Aspect ratio constraints will be updated in the
	// next frame update.
}

/**
 * Toggle fullscreen.
 */
void SdlSWBackend::toggle_fullscreen(void)
{
	m_fullscreen = !m_fullscreen;
	if (m_fullscreen) {
		// Switched to windowed fullscreen.
		SDL_SetWindowFullscreen(d->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		// Switch to windowed mode.
		SDL_SetWindowFullscreen(d->window, 0);
	}
}

}
