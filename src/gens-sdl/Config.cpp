/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * Config.hpp: Emulator configuration.                                     *
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

#include "Config.hpp"

// LibGens
#include "libgens/Util/MdFb.hpp"
#include "libgens/Rom.hpp"
using LibGens::MdFb;
using LibGens::Rom;

// LibZomg
#include "libzomg/PngWriter.hpp"
#include "libzomg/img_data.h"
#include "libzomg/Metadata.hpp"
using LibZomg::PngWriter;
using LibZomg::Metadata;

#ifdef _WIN32
// Windows
#include <windows.h>
#include <shlobj.h>
#include "libgens/Win32/W32U_mini.h"
#else
// Linux / Unix / Mac OS X.
#include <unistd.h>
#include <pwd.h>
#endif

// C includes.
#include <sys/stat.h>
#include <sys/types.h>

// C includes. (C++ namespace)
#include <cstring>
#include <cerrno>

// C++ includes.
#include <string>
using std::string;

namespace GensSdl {

/**
 * Recursively create a directory.
 * @param dir Directory.
 * @return 0 on success; non-zero on error.
 */
static int mkdir_recursive(const char *dir) {
	// Reference: http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
	// NOTE: We're not adjusting umask, so mkdir()'s 0777
	// will be reduced to 0755 in most cases.
	char tmp[260];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if (tmp[len - 1] == DIR_SEP_CHR) {
		tmp[len - 1] = 0;
	}
        for (p = tmp + 1; *p; p++) {
		if (*p == DIR_SEP_CHR) {
			*p = 0;
			// TODO: Check for errors?
			mkdir(tmp, 0777);
			*p = DIR_SEP_CHR;
		}
	}
	mkdir(tmp, 0777);

	// The path should exist now.
	return (access(dir, F_OK));
}

/**
 * Get the configuration directory.
 * @param subdir [in, opt] If not null, append a subdirectory.
 * @return Configuration directory, or nullptr on error.
 */
const std::string getConfigDir(const utf8_str *subdir)
{
	static string config_dir;
	if (config_dir.empty()) {
		// Determine the directory.
#if defined(_WIN32)
		// Windows.
		WCHAR wpath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, wpath))) {
			// Convert from UTF-16 to UTF-8.
			char *upath = W32U_UTF16_to_mbs(wpath, CP_UTF8);
			if (upath) {
				config_dir = string(upath);
				config_dir += "\\gens-gs-ii";
			}
		}
#else
		// TODO: Mac OS X-specific path.
		char *home = getenv("HOME");
		if (home) {
			// HOME variable found. Use it.
			config_dir = string(home);
		} else {
			// HOME variable not found.
			// Check the user's pwent.
			// TODO: Check for getpwuid_r().
			struct passwd *pw = getpwuid(getuid());
			config_dir = string(pw->pw_dir);
		}
		config_dir += "/.config/gens-gs-ii";
#endif
	}

	string ret = config_dir;
	if (!ret.empty()) {
		if (subdir) {
			// Append the subdirectory.
			ret += DIR_SEP_CHR;
			ret += subdir;
		}

		// Make sure the directory exists.
		if (access(ret.c_str(), F_OK) != 0) {
			// Directory does not exist. Create it.
			mkdir_recursive(ret.c_str());
		}
	}

	return ret;
}

/**
 * Take a screenshot.
 * @param fb	[in] MdFb.
 * @param rom	[in] ROM object.
 * @return 0 on success; non-zero on error.
 */
int doScreenShot(const MdFb *fb, const Rom *rom)
{
	/**
	 * TODO: This code is duplicated in four places:
	 * - GensSdl::Config
	 * - GensQt4::EmuManager
	 * - LibGens::EmuMD
	 * - LibGens::EmuPico
	 *
	 * Consolidate it into a LibGens function?
	 */

	const string configDir = getConfigDir("Screenshots");
	if (configDir.empty() || !fb || !rom)
		return -EINVAL;

	// TODO: Include z_file information?
	string basename = rom->filenameBaseNoExt();
	string romFilename(configDir);
	romFilename += DIR_SEP_CHR;
	romFilename += basename;

	// Add the current directory, number, and .png extension.
	const utf8_str scrFilenameSuffix[] = ".png";
	utf8_str scrFilename[260];
	int scrNumber = -1;
	do {
		// TODO: Figure out how to optimize this!
		scrNumber++;
		snprintf(scrFilename, sizeof(scrFilename), "%s_%03d%s",
			 romFilename.c_str(), scrNumber, scrFilenameSuffix);
	} while (!access(scrFilename, F_OK));

	// Take the screenshot.
	// TODO: Separate function to create an img_data from an MdFb.
	// NOTE: LibZomg doesn't depend on LibGens, so it can't use MdFb directly.
	fb->ref();
	const int imgXStart = fb->imgXStart();
	const int imgYStart = fb->imgYStart();

	// TODO: Option to save the full framebuffer, not just active display?
	// NOTE: Zeroing the struct in case new stuff is added later.
	Zomg_Img_Data_t img_data;
	memset(&img_data, 0, sizeof(img_data));
	img_data.w = fb->imgWidth();
	img_data.h = fb->imgHeight();

	// Aspect ratio.
	// Vertical is always 4.
	// Horizontal is 4 for H40, 5 for H32.
	// TODO: Handle Interlaced mode 2x rendering?
	img_data.phys_y = 4;
	// TODO: Formula to automatically scale for any width?
	img_data.phys_x = (img_data.w == 256 ? 5 : 4);

	const MdFb::ColorDepth bpp = fb->bpp();
	if (bpp == MdFb::BPP_32) {
		img_data.data = (void*)(fb->lineBuf32(imgYStart) + imgXStart);
		img_data.pitch = (fb->pxPitch() * sizeof(uint32_t));
		img_data.bpp = 32;
	} else {
		img_data.data = (void*)(fb->lineBuf16(imgYStart) + imgXStart);
		img_data.pitch = (fb->pxPitch() * sizeof(uint16_t));
		img_data.bpp = (bpp == MdFb::BPP_16 ? 16 : 15);
	}

	// Set up metadata.
	Metadata metadata;
	// TODO: Get system from Rom.
	metadata.setSystemId("MD");	// TODO: Pass MDP system ID directly.
	//metadata.setRegion();		// TODO: Get region code.
	metadata.setRomFilename(basename);	// TODO: With extension; also, z_file?
	metadata.setRomCrc32(rom->rom_crc32());
	//metadata.setRomSize(rom->romSize());	// TODO; also, include SMD header?
	// TODO: Add more metadata.

	// Write the PNG image.
	PngWriter pngWriter;
	int ret = pngWriter.writeToFile(&img_data, scrFilename,
				&metadata, Metadata::MF_Default);

	// Done using the framebuffer.
	fb->unref();

	if (ret == 0) {
		printf("Screenshot %d saved.\n", scrNumber);
	} else {
		// TODO: Print the actual error.
		printf("Error saving screenshot: %s\n", strerror(-ret));
	}

	return ret;
}

}
