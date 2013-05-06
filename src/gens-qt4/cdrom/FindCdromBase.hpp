/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromBase.hpp: Find CD-ROM drives: OS-specific base class.          *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMBASE_HPP__
#define __GENS_QT4_CDROM_FINDCDROMBASE_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/QIcon>

namespace GensQt4
{

class FindCdromBase : public QObject
{
	Q_OBJECT

	public:
		FindCdromBase(QObject *parent = 0);
		virtual ~FindCdromBase();
		
		/**
		 * Determine if this CD-ROM backend is usable.
		 * @return True if this CD-ROM backend is usable; false if not.
		 */
		virtual bool isUsable(void) const = 0;

		/**
		 * Scan the system for CD-ROM devices.
		 * @return QStringList with all detected CD-ROM device names.
		 */
		virtual QStringList scanDeviceNames(void) = 0;

		/**
		 * Check if this backend supports OS-specific disc/drive icons.
		 * @return True if OS-specific disc/drive icons are supported; false if not.
		 */
		virtual bool isDriveIconSupported(void) const;

		/**
		 * Get the OS-specific disc/drive icon.
		 * @param deviceName Device name.
		 * @return OS-specific disc/drive icon.
		 */
		virtual QIcon getDriveIcon(QString deviceName) const;

	// TODO: Update for FindCdromDrives.
#if 0
	signals:
		void driveUpdated(const CdromDriveEntry& drive);
		void driveQueryFinished(void);
		void driveRemoved(QString path);
#endif
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMBASE_HPP__ */
