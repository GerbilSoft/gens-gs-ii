/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromWin32.cpp: Find CD-ROM drives: Win32 version.                  *
 *                                                                         *
 * Copyright (c) 2011-2014 by David Korth.                                 *
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

#include "FindCdromWin32.hpp"

// C includes.
#include <stdio.h>

// Common Controls 6 is required for SHGetImageList().
// TODO: Add an SHGetImageList() definition here in case
// the Windows SDK is pre-XP?
#if !defined(_WIN32_IE) || _WIN32_IE < 0x0600
#undef _WIN32_IE
#define _WIN32_IE 0x0600
#endif /* _WIN32_IE */
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif /* _WIN32_WINNT */

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <shellapi.h>
#include <commctrl.h>
#include <commoncontrols.h>

// NOTE: mingw64-runtime 2010/10/03 does not have IID_IImageList in libuuid.
static const GUID Gens_IID_IImageList = {0x46EB5926, 0x582E, 0x4017, {0x9F, 0xDF, 0xE8, 0x99, 0x8D, 0xAA, 0x09, 0x50}};

#ifndef MAKE_FUNCPTR
#define MAKE_FUNCPTR(f) typeof(f) * p##f
#endif

namespace GensQt4
{

class FindCdromWin32Private
{
	private:
		FindCdromWin32Private() { }
		~FindCdromWin32Private() { }
		Q_DISABLE_COPY(FindCdromWin32Private);

	public:
		static HICON GetShilIcon(int iIcon, int iImageList);
};

/** FindCdromWin32Private **/

/**
 * Get an icon from the shell image list.
 * Requires Windows XP or later.
 * @param iIcon Index in the shell image list.
 * @param iImageList Image list index. (default == SHIL_LARGE == 0)
 * @return Icon from shell image list, or NULL on error.
 */
HICON FindCdromWin32Private::GetShilIcon(int iIcon, int iImageList)
{
	// Open shell32.dll.
	HINSTANCE hShell32 = LoadLibraryA("shell32.dll");
	if (!hShell32)
		return NULL;

	// Attempt to get the process address for shell32.dll::SHGetImageList().
	MAKE_FUNCPTR(SHGetImageList);
	pSHGetImageList = (typeof(pSHGetImageList))GetProcAddress(hShell32, "SHGetImageList");
	if (!pSHGetImageList) {
		// SHGetImageList() not found.
		FreeLibrary(hShell32);
		return NULL;
	}

	// SHGetImageList() found.
	// Get the image list.
	// NOTE: mingw64-runtime 2010/10/03 does not have IID_IImageList in libuuid.
	IImageList *imgl;
	HRESULT hr = pSHGetImageList(iImageList, Gens_IID_IImageList, (void**)&imgl);
	if (FAILED(hr)) {
		// Failed to retrieve the image list.
		FreeLibrary(hShell32);
		return NULL;
	}

	// Image list obtained.
	// Get the requested icon.
	HICON hIcon;
	hr = imgl->GetIcon(iIcon, ILS_NORMAL, &hIcon);
	if (FAILED(hr)) {
		// Failed to retrieve the icon.
		FreeLibrary(hShell32);
		return NULL;
	}

	// Icon retrieved.
	FreeLibrary(hShell32);
	return hIcon;
}

/** FindCdromWin32 **/

FindCdromWin32::FindCdromWin32(QObject *parent)
	: FindCdromBase(parent)
{ }

/**
 * Scan the system for CD-ROM devices.
 * @return QStringList with all detected CD-ROM device names.
 */
QStringList FindCdromWin32::scanDeviceNames(void)
{
	// TODO: We're using GetLogicalDrives() to get drive letters.
	// There's probably a better way to get optical drive information,
	// e.g. via WMI, but this is the most compatible.
	DWORD logical_drives = GetLogicalDrives();
	if (!logical_drives)
		return QStringList();

	// List of CD-ROM device names.
	QStringList cdromDeviceNames;

	// Go through the 26 drive letters and find CD-ROM drives.
	// TODO: Get special information like supported disc types.
	char drive_path[4] = {0, ':', '\\', 0};
	unsigned int drive_type;
	for (int i = 0; i < 26; i++) {
		drive_path[0] = 'A' + i;
		drive_type = GetDriveTypeA(drive_path);
		if (drive_type == DRIVE_CDROM) {
			// Found a CD-ROM drive.
			cdromDeviceNames.append(QLatin1String(drive_path));
		}
	}
	
	return cdromDeviceNames;
}


/**
 * Check if this backend supports OS-specific disc/drive icons.
 * @return True if OS-specific disc/drive icons are supported; false if not.
 */
bool FindCdromWin32::isDriveIconSupported(void) const
{
	// Win32 supports custom disc icons via AUTORUN.INF.
	return true;
}


/**
 * Get the OS-specific disc/drive icon.
 * @param deviceName Device name.
 * @return OS-specific disc/drive icon.
 */
QIcon FindCdromWin32::getDriveIcon(const QString &deviceName) const
{
	// Get the icon using SHGetFileInfo().
	// This requires shell32.dll v4.0 or later.
	// TODO: Check shell32.dll version first!
	QIcon ret_icon;

	SHFILEINFOA sfi;
	memset(&sfi, 0x00, sizeof(sfi));

	// Get the icon information.
	HRESULT hr = SHGetFileInfoA(deviceName.toLocal8Bit().constData(),
					0, &sfi, sizeof(sfi),
					SHGFI_ICON | SHGFI_LARGEICON);
	if (FAILED(hr))
		return QIcon();

	// Icon information retrieved.

	// Attempt to get the SHIL_EXTRALARGE ImageList from the shell.
	// TODO: Get all icons, including SHIL_JUMBO on Vista and later.
	HICON hIcon = FindCdromWin32Private::GetShilIcon(sfi.iIcon, SHIL_EXTRALARGE);
	if (!hIcon) {
		// FindCdromWin32Private::GetShilIcon() isn't usable.
		hIcon = sfi.hIcon;
	} else {
		// getShilIcon() is usable.
		// Delete the icon retrieved by SHGetFileInfoA().
		DestroyIcon(sfi.hIcon);
	}

	// Convert the HICON to a QIcon.
#if QT_VERSION >= 0x040600
	// QPixmap::fromWinHICON() was added in Qt 4.6.
	QPixmap pxm_icon = QPixmap::fromWinHICON(hIcon);
#else
	// Convert the HICON to a QIcon.
	// http://lists.trolltech.com/qt-interest/2007-07/thread00170-0.html
	ICONINFO info;
	if (!GetIconInfo(hIcon, &info)) {
		DestroyIcon(hIcon);
		return QIcon();
	}
	QPixmap pxm_icon = QPixmap::fromWinHBITMAP(info.hbmColor, QPixmap::Alpha);
#endif

	if (pxm_icon.width() != 64 || pxm_icon.height() != 64) {
		// Pixmap is not 64x64.
		// TODO: Does Qt::KeepAspectRatio result in a centered image
		// on a 64x64 pixmap, or will it result in a smaller pixmap?
		// TODO: Scaling QPixmap::fromWinHBITMAP results in horrible transparency.
		// QPixmap::fromWinHICON is good, but is only available in Qt 4.6 and later.
		// Maybe I should copy the function from Qt 4.6's source code?
		pxm_icon = pxm_icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	// Convert the QPixmap to a QIcon.
	ret_icon = QIcon(pxm_icon);

	// Delete the retrieved icon.
	DestroyIcon(hIcon);

	return ret_icon;
}

}
