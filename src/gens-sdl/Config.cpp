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

#ifdef _WIN32
// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
 * @return Configuration directory, or nullptr on error.
 */
const utf8_str *getConfigDir(void)
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

	if (!config_dir.empty()) {
		// Make sure the directory exists.
		if (access(config_dir.c_str(), F_OK) != 0) {
			// Directory does not exist. Create it.
			mkdir_recursive(config_dir.c_str());
		}
	}

	return (!config_dir.empty() ? config_dir.c_str() : nullptr);
}

}
