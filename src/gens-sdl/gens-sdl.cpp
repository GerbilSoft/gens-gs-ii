/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * gens-sdl.cpp: Entry point and main event loop.                          *
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

#include "SdlHandler.hpp"
using GensSdl::SdlHandler;

// LibGens
#include "libgens/lg_main.hpp"
#include "libgens/Rom.hpp"
#include "libgens/MD/EmuMD.hpp"
#include "libgens/Vdp/VdpPalette.hpp"
#include "libgens/Util/Timing.hpp"
using LibGens::Rom;
using LibGens::EmuContext;
using LibGens::EmuMD;
using LibGens::VdpPalette;
using LibGens::Timing;

// yield(), aka usleep(0) or Sleep(0)
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define yield() do { Sleep(0); } while (0)
#else
#include <unistd.h>
#define yield() do { usleep(0); } while (0)
#endif

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>

#include <SDL.h>

static SdlHandler *sdlHandler = nullptr;
static Rom *rom = nullptr;
static EmuMD *context = nullptr;

static const char *rom_filename = nullptr;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s [rom filename]\n", argv[0]);
		return EXIT_FAILURE;
	}
	rom_filename = argv[1];

	// Initialize SDL.
	int ret = SDL_Init(0);
	if (ret < 0) {
		fprintf(stderr, "SDL initialization failed: %d - %s\n",
			ret, SDL_GetError());
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	// Reference: http://sdl.beuc.net/sdl.wiki/FAQ_Console
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
#endif /* _WIN32 */

	// Initialize LibGens.
	LibGens::Init();

	// Load the ROM image.
	rom = new Rom(rom_filename);
	if (!rom->isOpen()) {
		// Error opening the ROM.
		// TODO: Error code?
		fprintf(stderr, "Error opening ROM file %s: (TODO get error code)\n",
			rom_filename);
		return EXIT_FAILURE;
	}
	if (rom->isMultiFile()) {
		// Select the first file.
		rom->select_z_entry(rom->get_z_entry_list());
	}

	// Create the emulation context.
	context = new EmuMD(rom);
	if (!context->isRomOpened()) {
		// Error loading the ROM into EmuMD.
		// TODO: Error code?
		fprintf(stderr, "Error initializing EmuContext for %s: (TODO get error code)\n",
			rom_filename);
		return EXIT_FAILURE;
	}

	// Initialize SDL handlers.
	sdlHandler = new SdlHandler();
	if (sdlHandler->init_video() < 0)
		return EXIT_FAILURE;
	if (sdlHandler->init_timers() < 0)
		return EXIT_FAILURE;
	if (sdlHandler->init_audio() < 0)
		return EXIT_FAILURE;

	// Start the frame timer.
	// TODO: Region code?
	bool isPal = false;
	sdlHandler->start_timer(isPal);

	// TODO: Close the ROM, or let EmuContext do it?

	// Set the color depth.
	context->m_vdp->m_palette.setBpp(VdpPalette::BPP_32);

	// Set the SDL video source.
	sdlHandler->set_video_source(context->m_vdp->MD_Screen);

	// Start audio.
	SDL_PauseAudio(0);

	// TODO: Initialize I/O manager.

	LibGens::Timing timing;
	bool running = true;
	while (running) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_USEREVENT: {
					char title[1024];
					snprintf(title, sizeof(title), "gens-sdl - %0.1f fps", event.user.code / 10.0);
					SDL_WM_SetCaption(title, NULL);
					break;
				}

				case SDL_QUIT:
					running = 0;
					break;

				default:
					break;
			}
		}

		// Get the high-resolution time for synchronization.
		uint64_t time_start = timing.getTime();

		// Run a frame.
		context->execFrame();
		sdlHandler->update_video();
		sdlHandler->update_audio();

		// Wait some time after the frame is finished:
		// - NTSC: 16ms
		// - PAL: 19.5ms
		uint64_t time_wait = (isPal ? 19500 : 16000);
		while (time_start + time_wait > timing.getTime()) {
			yield();
		}

		// Synchronize.
		sdlHandler->wait_for_frame_sync();
	}

	// Shut. Down. EVERYTHING.
	delete sdlHandler;
	delete context;
	delete rom;
	return EXIT_SUCCESS;
}
