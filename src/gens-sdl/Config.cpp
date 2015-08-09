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

#include <gens-sdl/config.gens-sdl.h>

#include "Config.hpp"

// LibGens
#include "libgens/Rom.hpp"
#include "libgens/Util/MdFb.hpp"
#include "libgens/Util/Screenshot.hpp"
using LibGens::Rom;
using LibGens::MdFb;
using LibGens::Screenshot;

#ifdef _WIN32
// Windows
#include <windows.h>
#include "libW32U/W32U_mini.h"
#else
// Linux / Unix / Mac OS X.
#include <unistd.h>
#include <pwd.h>
#endif

// C includes.
#include <sys/stat.h>
#include <sys/types.h>

// C includes. (C++ namespace)
#include <cstdio>
#include <cstring>
#include <cerrno>

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

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
std::string getConfigDir(const utf8_str *subdir)
{
	static string config_dir;
	if (config_dir.empty()) {
		// Determine the directory.
#if defined(_WIN32)
		// Windows.
		// NOTE: Reserving more than MAX_PATH due to the
		// expansion of UTF-16 characters to UTF-8.
		char appData[MAX_PATH*3];
		if (SHGetSpecialFolderPathU(nullptr, appData, sizeof(appData), CSIDL_APPDATA, FALSE)) {
			// Path retrieved.
			config_dir = string(appData);
			config_dir += "\\gens-gs-ii";
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
			struct passwd *pwd;
#ifdef HAVE_GETPWUID_R
#define PWD_FN "getpwuid_r"
			char buf[2048];
			struct passwd pwd_r;
			// TODO: Check for ENOMEM?
			getpwuid_r(getuid(), &pwd_r, buf, sizeof(buf), &pwd);
#else
#define PWD_FN "getpwuid"
			pwd = getpwuid(getuid());
#endif
			if (!pwd) {
				// getpwuid() failed.
				fprintf(stderr, PWD_FN "() failed; cannot get user's home directory.\n");
				exit(EXIT_FAILURE);
			}

			config_dir = string(pwd->pw_dir);
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
 * Get a savestate filename.
 * @param rom ROM for the savestate.
 * @param saveSlot Save slot number. (0-9)
 * @return Savestate filename.
 */
std::string getSavestateFilename(const LibGens::Rom *rom, int saveSlot)
{
	assert(saveSlot >= 0 && saveSlot <= 9);
	if (!rom || saveSlot < 0 || saveSlot > 9)
		return string();

	const string configDir = getConfigDir("Savestates");
	if (configDir.empty())
		return string();

	ostringstream oss;
	oss << configDir;
	oss << DIR_SEP_CHR;
	oss << rom->filename_baseNoExt();
	oss << '.';
	oss << saveSlot;
	oss << ".zomg";

	return oss.str();
}

/**
 * Take a screenshot.
 * @param fb	[in] MdFb.
 * @param rom	[in] ROM object.
 * @return 0 on success; non-zero on error.
 */
int doScreenShot(const MdFb *fb, const Rom *rom)
{
	const string configDir = getConfigDir("Screenshots");
	if (configDir.empty() || !fb || !rom)
		return -EINVAL;

	// TODO: Include z_file information?
	string basename = rom->filename_baseNoExt();
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
	int ret = Screenshot::toFile(fb, rom, scrFilename);
	if (ret == 0) {
		printf("Screenshot %d saved.\n", scrNumber);
	} else {
		// TODO: Print the actual error.
		printf("Error saving screenshot: %s\n", strerror(-ret));
	}

	return ret;
}

}
