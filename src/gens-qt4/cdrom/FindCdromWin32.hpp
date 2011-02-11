/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromWin32.hpp: Find CD-ROM drives using Win32 API.                 *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMWIN32_HPP__
#define __GENS_QT4_CDROM_FINDCDROMWIN32_HPP__

#include "FindCdrom.hpp"

// Qt includes.
#include <QtGui/QIcon>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace GensQt4
{

class FindCdromWin32 : public FindCdrom
{
	Q_OBJECT
	
	public:
		QIcon getDriveIcon(const CdromDriveEntry& drive);
	
	protected:
		/**
		 * query_int(): Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
		 * The driveUpdated() signal will be emitted once for each detected drive.
		 * @return 0 on success; non-zero on error.
		 */
		int query_int(void);
	
	private:
		/**
		 * GetVolumeLabel(): Get the volume label of the specified drive.
		 * @param drive_letter Drive letter.
		 * @return Volume label, or empty string on error.
		 */
		static QString GetVolumeLabel(char drive_letter);
		
		/**
		 * getShilIcon(): Get an icon from the shell image list.
		 * Requires Windows XP or later.
		 * @param iIcon Index in the shell image list.
		 * @param iImageList Image list index. (default == SHIL_LARGE == 0)
		 * @return Icon from shell image list, or NULL on error.
		 */
		HICON getShilIcon(int iIcon, int iImageList = 0);
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMWIN32_HPP__ */
