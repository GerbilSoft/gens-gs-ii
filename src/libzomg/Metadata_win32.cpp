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
#include "libcompat/W32U/W32U_mini.h"

// Needed for GetUserNameEx();
#define SECURITY_WIN32
#include <security.h>

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

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
 * Get a string from the registry.
 * @param hKey         [in] Opened registry key.
 * @param valueName    [in] Value name.
 * @param str          [out] Output string.
 * @return ERROR_SUCCESS on success; other Windows error code on error.
 */
static LONG getRegValue(HKEY hKey, const char *valueName, string &str)
{
	// TODO: RegOpenKeyExU().
	DWORD type;
	char buf[256];
	DWORD cbBuf = sizeof(buf);
	LONG ret = RegQueryValueExA(hKey, valueName, nullptr, &type, (LPBYTE)buf, &cbBuf);
	if (ret != ERROR_SUCCESS) {
		// Error reading the value.
		// Don't bother trying to handle ERROR_MORE_DATA here.
		return ret;
	} else if (cbBuf > sizeof(buf)) {
		// Somehow we read too much data...
		return ERROR_MORE_DATA;
	} else if (cbBuf == 0) {
		// Empty string.
		str.clear();
		return ERROR_SUCCESS;
	}

	// Check if there are any trailing NULLs in buf[].
	for (; cbBuf > 0; cbBuf--) {
		if (buf[cbBuf-1] != 0)
			break;
	}

	// Return the string.
	str.assign(buf, cbBuf);
	return ERROR_SUCCESS;
}

/**
 * Get the Windows username. (Unicode version)
 * The display name is checked first.
 * If it's empty or invalid, the username is checked.
 * @return Windows username.
 */
static string getUserName_unicode(void)
{
	// GetUserNameExW() was added in Windows 2000.
	// Dynamically load the function to prevent issues
	// with older versions of Windows.
	string ret;
	wchar_t username[256];
	DWORD cchUsername = ARRAY_SIZE(username);
	int wret = 0;

	HMODULE hSecur32 = LoadLibraryW(L"SECUR32.DLL");
	if (hSecur32) {
		typedef BOOLEAN (WINAPI *PFNGETUSERNAMEEXW)(EXTENDED_NAME_FORMAT, LPWSTR, PULONG);
		PFNGETUSERNAMEEXW pfnGetUserNameExW = (PFNGETUSERNAMEEXW)GetProcAddress(hSecur32, "GetUserNameExW");
		if (pfnGetUserNameExW) {
			wret = pfnGetUserNameExW(NameDisplay, username, &cchUsername);
		}
		FreeLibrary(hSecur32);
	}

	if (wret == 0 || cchUsername == 0) {
		// Error retrieving display name.
		cchUsername = ARRAY_SIZE(username);
		wret = GetUserNameW(username, &cchUsername);
		if (wret == 0 || cchUsername == 0) {
			// Error retrieving username.
			// TODO: Check Registered Owner in the registry?
			cchUsername = 0;
		}
	}

	if (cchUsername > 0) {
		// Try to convert from UTF-16 to UTF-8.
		// FIXME: If this fails, do a naive low-byte conversion?

		// Next, try to convert from UTF-16 to UTF-8.
		int cbMbs = WideCharToMultiByte(CP_UTF8, 0, username, cchUsername, nullptr, 0, nullptr, nullptr);
		if (cbMbs > 0) {
			char *mbs = (char*)malloc(cbMbs);
			WideCharToMultiByte(CP_UTF8, 0, username, cchUsername, mbs, cbMbs, nullptr, nullptr);

			// Save the UTF-8 string.
			ret = string(mbs, cbMbs);
			free(mbs);
		}
	}

	return ret;
}

/**
 * Get the Windows username. (ANSI version)
 * The display name is checked first.
 * If it's empty or invalid, the username is checked.
 * @return Windows username.
 */
static string getUserName_ansi(void)
{
	// GetUserNameExA() was added in Windows 2000.
	// As such, there's no point in attempting to
	// use it on ANSI Windows, so we'll go directly
	// to GetUserNameA() instead.
	string ret;
	char username[256];
	DWORD cbUsername = ARRAY_SIZE(username);
	int wret = GetUserNameA(username, &cbUsername);
	if (wret == 0 || cbUsername == 0) {
		// Error retrieving username.
		// TODO: Check Registered Owner in the registry?
		cbUsername = 0;
	}

	if (cbUsername > 0) {
		// Try to convert from ANSI to UTF-8.
		// FIXME: If this fails, use the ANSI text as-is?

		// First, convert from ANSI to UTF-16
		int cchWcs = MultiByteToWideChar(CP_ACP, 0, username, cbUsername, nullptr, 0);
		if (cchWcs > 0) {
			wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, username, cbUsername, wcs, cchWcs);

			// Next, try to convert from UTF-16 to UTF-8.
			int cbMbs = WideCharToMultiByte(CP_UTF8, 0, wcs, cchWcs, nullptr, 0, nullptr, nullptr);
			if (cbMbs > 0) {
				char *mbs = (char*)malloc(cbMbs);
				WideCharToMultiByte(CP_UTF8, 0, wcs, cchWcs, mbs, cbMbs, nullptr, nullptr);

				// Save the UTF-8 string.
				ret = string(mbs, cbMbs);
				free(mbs);
			}
			free(wcs);
		}
	}

	return ret;
}

/**
 * Get the OS version. (Windows 8)
 * @return OS version.
 */
static string getOSVersion_8(void)
{
	// ostringstream for the OS version.
	ostringstream oss;

	// Check if the product name is available in the registry.
	//reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v ProductName
	HKEY hKey;
	LONG ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
	if (ret == ERROR_SUCCESS) {
		// Key opened. Get the ProductName.
		// TODO: Needs testing on 9x, 2000, and XP.
		string regData;
		ret = getRegValue(hKey, "ProductName", regData);
		if (ret == ERROR_SUCCESS) {
			// Product name retrieved.
			// As of Windows 7 (or Vista), this always says
			// "Windows (version) (edition)", so no MS or edition
			// check is necessary.
			oss << regData;
		} else {
			// Could not retrieve the product name.
			return string();
		}

		// Get the service pack version.
		ret = getRegValue(hKey, "CSDVersion", regData);
		if (ret == ERROR_SUCCESS) {
			// Service Pack version found.
			// Only take numeric characters (or dots),
			// starting from the end.
			int pos = (int)regData.size() - 1;
			for (; pos >= 0; pos--) {
				if (!isdigit(regData[pos]) && regData[pos] != '.') {
					pos++;
					break;
				}
			}
			if (pos >= 0 && pos < (int)regData.size()) {
				oss << " SP" << regData.substr(pos);
			}
		}

		// Get the version number.
		ret = getRegValue(hKey, "CurrentVersion", regData);
		if (ret == ERROR_SUCCESS) {
			// Major/minor version number retrieved.
			oss << " (" << regData;
			// Try to get the build number.
			// NOTE: On Windows 7+ (and probably Vista),
			// "CurrentBuildNumber" and "CurrentBuild" both have
			// the build number. On Windows XP, "CurrentBuildNumber"
			// has the build number, but "CurrentBuild" has weird
			// data: "1.511.1 () (Obsolete data - do not use)"
			// Only fall back to "CurrentBuild" if "CurrentBuildNumber" fails.
			// TODO: Check older versions.
			ret = getRegValue(hKey, "CurrentBuildNumber", regData);
			if (ret != ERROR_SUCCESS) {
				ret = getRegValue(hKey, "CurrentBuild", regData);
			}
			if (ret == ERROR_SUCCESS) {
				// Build number retrieved.
				oss << '.' << regData;
			}
			oss << ')';
		}

		// Close the registry key.
		// COMMIT NOTE: Forgot to close the key...
		RegCloseKey(hKey);
	}

	return oss.str();
}

/**
 * Get the OS version.
 * @return OS version.
 */
static string getOSVersion(void)
{
	// OS version.
	ostringstream oss;
	OSVERSIONINFOEXA osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
	if (GetVersionExA((LPOSVERSIONINFOA)&osVersionInfo) == 0) {
		// Error retrieving OS version info.
		return string();
	}

	// Version info retrieved.
	if (osVersionInfo.dwMajorVersion > 6 ||
	    (osVersionInfo.dwMajorVersion == 6 && osVersionInfo.dwMinorVersion >= 2))
	{
		// Windows 8 or later.
		// As of Windows 8.1, GetVersionEx() always returns
		// Windows 8 unless the program has a version-specific
		// manifest. We aren't targetting Windows 8, so we'll
		// need to use an alternative method.
		string os8 = getOSVersion_8();
		if (!os8.empty())
			return os8;
	}

	// Build number is a bit screwy on Win9x.
	// The low 16 bits map to the correct number;
	// the high 16 bits are something else entirely.
	// On Win98SE (build 2222), the build number is:
	// - 67766446 (0x040A08AE)
	// This translates to 0x040A (1,034) and 0x08AE (2,222).
	DWORD dwBuildNumber = osVersionInfo.dwBuildNumber;

	// Check if the product name is available in the registry.
	//reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v ProductName
	HKEY hKey;
	LONG ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
	if (ret == ERROR_SUCCESS) {
		// Key opened. Get the ProductName.
		// TODO: Needs testing on 9x, 2000, and XP.
		string productName;
		ret = getRegValue(hKey, "ProductName", productName);
		if (ret == ERROR_SUCCESS) {
			// Product name retrieved.
			// TODO: Remove "Microsoft"; append edition if not present.
			oss << productName;
		} else {
			// Could not retrieve product name.
			// Use a generic name.
			switch (osVersionInfo.dwPlatformId) {
				case VER_PLATFORM_WIN32s:
					// Yeah, right, like this is ever going to happen...
					dwBuildNumber &= 0xFFFF;
					oss << "Windows 3.1";
					break;
				case VER_PLATFORM_WIN32_WINDOWS:
					// Ignore the high word of the build number.
					dwBuildNumber &= 0xFFFF;
					if (osVersionInfo.dwMajorVersion == 4) {
						// Check if this is a specific version of 9x.
						switch (osVersionInfo.dwMinorVersion) {
							case 0:
								// Windows 95.
								// TODO: OSR versions?
								oss << "Windows 95";
								break;
							case 10:
								// Windows 98.
								oss << "Windows 98";
								if (dwBuildNumber == 2222) {
									// Windows 98SE.
									oss << "SE";
								}
								break;
							case 90:
								// Windows Me.
								oss << "Windows Me";
								break;
							default:
								// Unknown...
								oss << "Windows 9x";
								break;
						}
					} else {
						// Unknown...
						oss << "Windows 9x";
					}
					break;
				case VER_PLATFORM_WIN32_NT:
				default:
					// Append the OS version.
					oss << "Windows NT" << ' ' << osVersionInfo.dwMajorVersion << '.';
					if (osVersionInfo.dwMinorVersion % 10 == 0) {
						// NT 3.1 and 3.5 are internally
						// 3.10 and 3.50. Remove the extra 0.
						oss << (osVersionInfo.dwMinorVersion / 10);
					} else {
						// Use the full minor version.
						// Needed for e.g. NT 3.51.
						oss << osVersionInfo.dwMinorVersion;
					}

					// Append the edition.
					// TODO: Advanced Server, etc.?
					oss << ' ';
					switch (osVersionInfo.wProductType) {
						case VER_NT_WORKSTATION:
						default:
							oss << "Workstation";
							break;
						case VER_NT_DOMAIN_CONTROLLER:
						case VER_NT_SERVER:
							oss << "Server";
							break;
					}
					break;
			}
		}

		// Close the registry key.
		RegCloseKey(hKey);
	}

	// Append the service pack version.
	char spver[32];
	if (osVersionInfo.wServicePackMajor > 0 || osVersionInfo.wServicePackMinor > 0) {
		// Service Pack.
		if (osVersionInfo.wServicePackMinor > 0) {
			snprintf(spver, sizeof(spver), " SP%u.%u",
				osVersionInfo.wServicePackMajor,
				osVersionInfo.wServicePackMinor);
		} else {
			snprintf(spver, sizeof(spver), " SP%u",
				osVersionInfo.wServicePackMajor);
		}
		oss << string(spver);
	}

	// Append the version number.
	// NOTE: MinGW-w64 defines DWORD as unsigned long.
	// We're casting it to plain old unsigned int
	// to eliminate some warnings.
	snprintf(spver, sizeof(spver), " (%u.%u.%u)",
		(unsigned int)osVersionInfo.dwMajorVersion,
		(unsigned int)osVersionInfo.dwMinorVersion,
		(unsigned int)dwBuildNumber);
	oss << string(spver);

	// Return the OS version.
	return oss.str();
}

/**
 * Initialize system-specific metadata.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::InitSystemMetadata(void)
{
	// OS version.
	sysInfo.osVersion = getOSVersion();

	// Get the username.
	if (W32U_IsUnicode()) {
		// OS supports Unicode.
		sysInfo.username = getUserName_unicode();
	} else {
		// OS does not support Unicode.
		sysInfo.username = getUserName_ansi();
	}

	// TODO: CPU information.
}

}
