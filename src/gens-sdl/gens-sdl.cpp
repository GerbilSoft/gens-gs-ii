/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * gens-sdl.hpp: Entry point.                                              *
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

// Reentrant functions.
// MUST be included before everything else due to
// _POSIX_SOURCE and _POSIX_C_SOURCE definitions.
#include "libcompat/reentrant.h"

#include <config.gens-sdl.h>
#include "gens-sdl.hpp"
#include <SDL.h>

// Configuration.
#include "Config.hpp"

#include "SdlHandler.hpp"
#include "VBackend.hpp"
using GensSdl::SdlHandler;
using GensSdl::VBackend;

// LibGens
#include "libgens/lg_main.hpp"
#include "libgens/lg_osd.h"

// Main event loops.
#include "EmuLoop.hpp"
#include "CrazyEffectLoop.hpp"

// Command line parameters.
#include "Options.hpp"

// OS-specific includes.
#ifdef _WIN32
// Windows
#include <windows.h>
// Win32 Unicode Translation Layer.
// Needed for proper Unicode filename support on Windows.
#include "libcompat/W32U/W32U_mini.h"
#include "libcompat/W32U/W32U_argv.h"
// DEP policy (requires _WIN32_WINNT >= 0x0600)
#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE 0x1
#endif
#ifndef PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION
#define PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION 0x2
#endif
#else
// Linux, Unix, Mac OS X
#include <unistd.h>
#endif

// C includes. (C++ namespace)
#include <cstdio>
#include <cstdlib>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace GensSdl {

/** Command line parameters. **/
static Options *options = nullptr;

// Event loop.
static EventLoop *eventLoop = nullptr;

// Startup OSD message queue.
struct OsdStartup {
	int duration;
	const char *msg;
	int param;
};
static vector<OsdStartup> startup_queue;

/**
 * Onscreen Display handler.
 * @param osd_type OSD type.
 * @param param Parameter.
 */
static void gsdl_osd(OsdType osd_type, int param)
{
	// NOTE: We're not using any sort of translation system
	// in the SDL frontend, so we'll just use the plural form.
	// Most SRAM/EEPROM chips are larger than 1 byte, after all...
	const char *msg;
	switch (osd_type) {
		case OSD_SRAM_LOAD:
			msg = "SRAM loaded. (%d bytes)";
			break;
		case OSD_SRAM_SAVE:
			msg = "SRAM saved. (%d bytes)";
			break;
		case OSD_SRAM_AUTOSAVE:
			msg = "SRAM autosaved. (%d bytes)";
			break;
		case OSD_EEPROM_LOAD:
			msg = "EEPROM loaded. (%d bytes)";
			break;
		case OSD_EEPROM_SAVE:
			msg = "EEPROM saved. (%d bytes)";
			break;
		case OSD_EEPROM_AUTOSAVE:
			msg = "EEPROM autosaved. (%d bytes)";
			break;
		case OSD_PICO_PAGESET:
			msg = "Pico: Page set to page %d.";
			break;
		case OSD_PICO_PAGEUP:
			msg = "Pico: PgUp to page %d.";
			break;
		case OSD_PICO_PAGEDOWN:
			msg = "Pico: PgDn to page %d.";
			break;
		default:
			// Unknown OSD type.
			msg = nullptr;
			break;
	}

	if (msg != nullptr) {
		VBackend *vBackend = eventLoop->vBackend();
		if (vBackend) {
			vBackend->osd_printf(1500, msg, param);
		} else {
			// SDL handler hasn't been created yet.
			// Store the message for later.
			OsdStartup startup;
			startup.duration = 1500;
			startup.msg = msg;
			startup.param = param;
			startup_queue.push_back(startup);
		}
	}
}

/**
 * Check for any OSD messages that were printed during startup.
 * These messages would have been printed before VBackend
 * was initialized, so they had to be temporarily stored.
 */
void checkForStartupMessages(void)
{
	if (!startup_queue.empty()) {
		VBackend *vBackend = eventLoop->vBackend();
		for (int i = 0; i < (int)startup_queue.size(); i++) {
			const OsdStartup &startup = startup_queue.at(i);
			vBackend->osd_printf(startup.duration, startup.msg, startup.param);
		}
	}
}

/**
 * Run the emulator.
 */
int run(void)
{
	// Initialize LibGens.
	LibGens::Init();

	// Register the LibGens OSD handler.
	lg_set_osd_fn(gsdl_osd);

	if (options->run_crazy_effect()) {
		// Run the Crazy Effect.
		eventLoop = new CrazyEffectLoop();
	} else {
		// Start the emulation loop.
		eventLoop = new EmuLoop();
	}
	int ret = 0;
	if (eventLoop) {
		ret = eventLoop->run(options);
	}

	// Unregister the LibGens OSD handler.
	lg_set_osd_fn(nullptr);

	// ...and we're done here.
	return ret;
}

}

#ifdef _WIN32
/**
 * Enable extra security options.
 * Reference: http://msdn.microsoft.com/en-us/library/bb430720.aspx
 * @return 0 on success; non-zero on error.
 */
static int SetSecurityOptions(void)
{
	HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
	if (hKernel32 == nullptr) {
		return -1;
	}

	// Enable DEP/NX. (WinXP SP3, Vista, and later.)
	// NOTE: DEP/NX should be specified in the PE header
	// using ld's --nxcompat, but we'll set it manually here,
	// just in case the linker doesn't support it.
	typedef BOOL (WINAPI *PFNSETDEP)(DWORD dwFlags);
	PFNSETDEP pfnSetDep = (PFNSETDEP)GetProcAddress(hKernel32, "SetProcessDEPPolicy");
	if (pfnSetDep) {
		pfnSetDep(PROCESS_DEP_ENABLE);
	}

	// Remove the current directory from the DLL search path.
	typedef BOOL (WINAPI *PFNSETDLLDIRW)(LPCWSTR lpPathName);
	PFNSETDLLDIRW pfnSetDllDirectoryW = (PFNSETDLLDIRW)GetProcAddress(hKernel32, "SetDllDirectoryW");
	if (pfnSetDllDirectoryW) {
		pfnSetDllDirectoryW(L"");
	}

	// Terminate the process if heap corruption is detected.
	// NOTE: Parameter 2 is usually type enum HEAP_INFORMATION_CLASS,
	// but this type isn't present in older versions of MinGW, so we're
	// using int instead.
	typedef BOOL (WINAPI *PFNHSI)
		(HANDLE HeapHandle, int HeapInformationClass,
		 PVOID HeapInformation, SIZE_T HeapInformationLength);
	PFNHSI pfnHeapSetInformation = (PFNHSI)GetProcAddress(hKernel32, "HeapSetInformation");
	if (pfnHeapSetInformation) {
		// HeapEnableTerminationOnCorruption == 1
		pfnHeapSetInformation(nullptr, 1, nullptr, 0);
	}

	if (hKernel32) {
		FreeLibrary(hKernel32);
	}

	return 0;
}
#endif

// Don't use SDL_main.
#undef main
int main(int argc, char *argv[])
{
#ifdef _WIN32
	SetSecurityOptions();
	// Convert command line parameters to UTF-8.
	if (W32U_GetArgvU(&argc, &argv, nullptr) != 0) {
		// ERROR!
		return EXIT_FAILURE;
	}
#endif /* _WIN32 */

	// Parse command line options.
	GensSdl::options = new GensSdl::Options();
	GensSdl::options->set_rom_filename_required(true);
	int ret = GensSdl::options->parse(argc, (const char**)argv);
	if (ret != 0) {
		// Error parsing command line options.
		// Options::parse() already printed an error message.
		delete GensSdl::options;
		return ret;
	}

	// Make sure we have a valid configuration directory.
	if (GensSdl::getConfigDir().empty()) {
		fprintf(stderr, "*** WARNING: Could not find a usable configuration directory.\n"
				"Save functionality will be disabled.\n\n");
	}

	// Initialize SDL.
	ret = SDL_Init(0);
	if (ret < 0) {
		fprintf(stderr, "SDL initialization failed: %d - %s\n",
			ret, SDL_GetError());
		return EXIT_FAILURE;
	}

	ret = GensSdl::run();
	// Command line options are no longer needed.
	// TODO: Should we bother deleting it? The program's
	// exiting here anyway...
	delete GensSdl::options;
	return ret;
}
