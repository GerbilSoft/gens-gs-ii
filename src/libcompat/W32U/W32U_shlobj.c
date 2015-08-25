/***************************************************************************
 * libcompat/W32U: Win32 Unicode Translation Layer. (Mini Version)         *
 * W32U_shlobj.c: ShellAPI functions.                                      *
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

#include <config.libcompat.h>

#define __IN_W32U__
#include "W32U_mini.h"
#include "W32U_shlobj.h"

// W32U_IsUnicode()
#include "is_unicode.h"

/**
 * Get a special folder path.
 *
 * NOTE: MSDN marks this function is marked as deprecated,
 * since SHGetFolderPath() is available in Windows 2000+ and
 * has some extra functionality, but special handling is
 * needed to use that function on ANSI Windows. We don't need
 * the extra functionality of SHGetFolderPath(), though.
 *
 * NOTE 2: lpszPath may need to be larger than MAX_PATH due to
 * the expansion of UTF-16 characters to UTF-8.
 *
 * @param hwndOwner	[in] Reserved. (Set to NULL)
 * @param lpszPath	[out] Output buffer.
 * @param cbPath	[in] Size of lpszPath, in bytes.
 * @param csidl	        [in] CSIDL value that identifies the folder.
 * @param fCreate	[in] If non-zero the folder is created if it does not already exist.
 * @return TRUE on success; FALSE on error.
 */
BOOL SHGetSpecialFolderPathU(HWND hwndOwner, char *lpszPath, int cbPath, int csidl, BOOL fCreate)
{
	// UTF-16 buffer.
	wchar_t pathW[MAX_PATH];
	BOOL ret;
	int wc_ret;

	// TODO: Rewrite this to dynamically load a function:
	// - SHGetKnownFolderPath() (Vista+)
	// - SHGetFolderPath() (Win2000+; Win9x with shfolder.dll)
	// - SHGetSpecialFolderPath() (Win9x)
	if (!W32U_IsUnicode()) {
#ifdef ENABLE_ANSI_WINDOWS
		// Call the ANSI function and convert from ANSI first.
		char pathA[MAX_PATH];
		ret = SHGetSpecialFolderPathA(hwndOwner, pathA, csidl, fCreate);
		if (!ret) {
			// Error retrieving the special folder path.
			return ret;
		}

		// Convert from ANSI to UTF-16.
		pathA[MAX_PATH-1] = 0;
		wc_ret = MultiByteToWideChar(CP_ACP, 0, pathA, -1,
			pathW, sizeof(pathW)/sizeof(pathW[0]));
		if (wc_ret == 0) {
			// Error converting the string from ANSI to UTF-16.
			return FALSE;
		}
		pathW[MAX_PATH-1] = 0;
#else /* !ENABLE_ANSI_WINDOWS */
		// ANSI is not supported in this build.
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
#endif /* ENABLE_ANSI_WINDOWS */
	} else {
		// Call the Unicode function.
		ret = SHGetSpecialFolderPathW(hwndOwner, pathW, csidl, fCreate);
		if (!ret) {
			// Error retrieving the special folder path.
			return ret;
		}
		pathW[MAX_PATH-1] = 0;
	}

	// Convert the path from UTF-16 to UTF-8.
	wc_ret = WideCharToMultiByte(CP_UTF8, 0, pathW, -1, lpszPath, cbPath, nullptr, nullptr);
	return (wc_ret != 0);
}
