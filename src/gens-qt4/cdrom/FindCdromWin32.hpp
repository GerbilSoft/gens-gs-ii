/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromWin32.hpp: Find CD-ROM drives: Win32 version.                  *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMWIN32_HPP__
#define __GENS_QT4_CDROM_FINDCDROMWIN32_HPP__

#include "FindCdromBase.hpp"

namespace GensQt4
{

class FindCdromWin32 : public FindCdromBase
{
	Q_OBJECT

	public:
		FindCdromWin32(QObject *parent = 0);

	private:
		Q_DISABLE_COPY(FindCdromWin32);

	public:
		/**
		 * Determine if this CD-ROM backend is usable.
		 * @return True if this CD-ROM backend is usable; false if not.
		 */
		bool isUsable(void) const final
			{ return true; }

		/**
		 * Scan the system for CD-ROM devices.
		 * @return QStringList with all detected CD-ROM device names.
		 */
		QStringList scanDeviceNames(void) final;

		/**
		 * Check if this backend supports OS-specific disc/drive icons.
		 * @return True if OS-specific disc/drive icons are supported; false if not.
		 */
		bool isDriveIconSupported(void) const final;

		/**
		 * Get the OS-specific disc/drive icon.
		 * @param deviceName Device name.
		 * @return OS-specific disc/drive icon.
		 */
		QIcon getDriveIcon(const QString &deviceName) const final;
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMWIN32_HPP__ */
