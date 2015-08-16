/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Metadata_unix.cpp: Metadata handler. (Unix/Linux, including Mac OS X)   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include <libzomg/config.libzomg.h>

// Reentrant functions.
// MUST be included before everything else due to
// _POSIX_SOURCE and _POSIX_C_SOURCE definitions.
#include "libcompat/reentrant.h"

#include "Metadata_p.hpp"

// System includes.
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/utsname.h>

// C includes. (C++ namespace)
#include <cstring>

// C++ includes.
#include <string>
using std::string;

namespace LibZomg {

/**
 * Get the current time.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::init_ctime(void)
{
	// TODO: Is there a way to get 64-bit time_t on 32-bit Linux?
	// TODO: Use clock_gettime() for nanoseconds.
	ctime.seconds = time(nullptr);
	ctime.nano = 0;
}

/**
 * Initialize system-specific metadata.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::InitSystemMetadata(void)
{
	// OS version.
	// TODO: Mac OS X-specific version.
	// TODO: Check for lsb_release.
	// For now, just use uname.
	struct utsname sys;
	int ret = uname(&sys);
	if (!ret) {
		sysInfo.osVersion = string(sys.sysname) + ' ' + string(sys.release);
	} else {
		sysInfo.osVersion = "Unknown Unix-like OS";
	}

	// Username.
	// Assuming OS X is compatible with the POSIX version.
	// NOTE: Assuming UTF-8 encoding.
	char buf[2048];
	struct passwd pwd;
	struct passwd *pwd_result;
	// TODO: Check for ENOMEM?
	ret = getpwuid_r(getuid(), &pwd, buf, sizeof(buf), &pwd_result);
	if (!ret && pwd_result) {
		// User information retrieved.
		// Check for a display name.
		if (pwd.pw_gecos && pwd.pw_gecos[0]) {
			// Find the first comma.
			char *comma = strchr(pwd.pw_gecos, ',');
			if (!comma) {
				// No comma. Use the entire field.
				sysInfo.username = string(pwd.pw_gecos);
			} else {
				// Found a comma.
				sysInfo.username = string(pwd.pw_gecos, (comma - pwd.pw_gecos));
			}
		} else {
			// No display name.
			// Check the username.
			if (pwd.pw_name && pwd.pw_name[0]) {
				// Username is valid.
				sysInfo.username = string(pwd.pw_name);
			}
		}
	} else {
		// Could not retrieve user information.
		sysInfo.username = string();
	}

	// TODO: CPU information.
}

}
