/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Metadata_unix.cpp: Metadata handler. (Windows)                          *
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

#include "Metadata_p.hpp"

// System includes.
#include <windows.h>
// Needed for GetUserNameEx();
#define SECURITY_WIN32
#include <security.h>

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

namespace LibZomg {

/**
 * Get the current time.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::init_ctime(void)
{
	// NOTE: Using FILETIME due to higher accuracy.
	// - SYSTEMTIME only supports milliseconds, and
	//   it's split up into logical date fields.
	// - FILETIME is in units of 100 nanoseconds.
	FILETIME fileTime;
	ULARGE_INTEGER largeInteger;
	GetSystemTimeAsFileTime(&fileTime);
	largeInteger.LowPart = fileTime.dwLowDateTime;
	largeInteger.HighPart = fileTime.dwHighDateTime;

	// Convert to Unix time format.
	// Note that the Windows epoch is January 1, 1601 00:00 UTC.
	// References:
	// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284%28v=vs.85%29.aspx
	// - http://stackoverflow.com/questions/19709580/c-convert-filetime-to-seconds
	ctime.seconds = (largeInteger.QuadPart / 10000000ULL) - 11644473600ULL;
	ctime.nano = (largeInteger.QuadPart % 10000000ULL) * 100;
}

/**
 * Initialize system-specific metadata.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::InitSystemMetadata(void)
{
	// OS version.
	// FIXME: This will return "Windows 8" on 8.1+.
	// Potential workaround is NetWkstaGetInfo().
	// There should also be a kernel function...
	// http://www.codeproject.com/Articles/678606/Part-Overcoming-Windows-s-deprecation-of-GetVe
	ostringstream oss;
	OSVERSIONINFOEXA osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
	if (GetVersionExA((LPOSVERSIONINFOA)&osVersionInfo) != 0) {
		// Version info retrieved.
		// Check if the product name is available in the registry.
		//reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v ProductName
		HKEY hKey;
		LONG ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
		if (ret == ERROR_SUCCESS) {
			// Key opened. Get the ProductName.
			// TODO: Needs testing on 9x, 2000, and XP.
			char productName[256];
			DWORD cbProductName = sizeof(productName);
			// 256 bytes should always be enough for the product name.
			ret = RegQueryValueExA(hKey, "ProductName", nullptr, nullptr, (LPBYTE)productName, &cbProductName);
			if (ret == ERROR_SUCCESS) {
				// Product name retrieved.
				// Make sure it's null-terminated.
				if (cbProductName < sizeof(productName)) {
					productName[cbProductName] = 0;
				}
				oss << string(productName);
			} else {
				// Could not retrieve product name.
				// Use a generic name.
				switch (osVersionInfo.dwPlatformId) {
					case VER_PLATFORM_WIN32s:
						// Yeah, right, like this is ever going to happen...
						oss << "Windows 3.1";
						break;
					case VER_PLATFORM_WIN32_WINDOWS:
						oss << "Windows 9x";
						break;
					case VER_PLATFORM_WIN32_NT:
					default:
						switch (osVersionInfo.wProductType) {
							case VER_NT_WORKSTATION:
							default:
								oss << "Windows NT";
								break;
							case VER_NT_DOMAIN_CONTROLLER:
							case VER_NT_SERVER:
								oss << "Windows NT Server";
								break;
						}
						break;
				}
			}
		}

		// Append service pack version.
		char spver[32];
		if (osVersionInfo.wServicePackMajor > 0 || osVersionInfo.wServicePackMinor > 0) {
			// Service Pack.
			if (osVersionInfo.wServicePackMinor > 0) {
				snprintf(spver, sizeof(spver), " SP%d.%d",
					osVersionInfo.wServicePackMajor,
					osVersionInfo.wServicePackMinor);
			} else {
				snprintf(spver, sizeof(spver), " SP%d",
					osVersionInfo.wServicePackMajor);
			}
			oss << string(spver);
		}

		// Append version number.
		snprintf(spver, sizeof(spver), " (%d.%d.%d)",
			osVersionInfo.dwMajorVersion,
			osVersionInfo.dwMinorVersion,
			osVersionInfo.dwBuildNumber);
		oss << string(spver);
	}
	sysInfo.osVersion = oss.str();

	// Get the username.
	// TODO: Use Unicode versions if available.
	char username_buf[256];
	DWORD cbUsername_buf = sizeof(username_buf);
	if (GetUserNameExA(NameDisplay, username_buf, &cbUsername_buf)) {
		// User's display name retrieved.
		sysInfo.username = string(username_buf, cbUsername_buf);
	} else if (GetUserNameA(username_buf, &cbUsername_buf)) {
		// User's login username retrieved.
		if (cbUsername_buf > 0) {
			sysInfo.username = string(username_buf, cbUsername_buf);
		} else {
			// Invalid size...
			sysInfo.username = string();
		}
	} else {
		// Error retrieving username.
		// TODO: Check Registered Owner in the registry?
		sysInfo.username = string();
	}

	// TODO: CPU information.
}

}
