/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
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

#ifndef __LIBW32U_W32U_SHLOBJ_H__
#define __LIBW32U_W32U_SHLOBJ_H__

#ifndef _WIN32
#error W32U_shlobj.h should only be included on Win32!
#endif

#ifndef __IN_W32U__
#error Do not include W32U_shlobj.h directly, include W32U_shlobj.h!
#endif

// Win32 includes.
#include <windows.h>
#include <shlobj.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SHGetFolderPathU() **/

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
BOOL SHGetSpecialFolderPathU(HWND hwndOwner, char *lpszPath, int cbPath, int csidl, BOOL fCreate);

#ifdef __cplusplus
}
#endif

#endif /* __LIBW32U_W32U_SHLOBJ_H__ */
