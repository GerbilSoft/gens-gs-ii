/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromDrives.hpp: Find CD-ROM drives: Manager class.                 *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

#ifndef __GENS_QT4_CDROM_FINDCDROMDRIVES_HPP__
#define __GENS_QT4_CDROM_FINDCDROMDRIVES_HPP__

// C includes.
#include <stdint.h>

// LibGensCD includes.
#include "libgenscd/CdDrive.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/QIcon>

namespace GensQt4 {

class FindCdromDrivesPrivate;

class FindCdromDrives : public QObject
{
	Q_OBJECT
	
	public:
		FindCdromDrives(QObject *parent = 0);
		~FindCdromDrives();

	private:
		typedef QObject super;
		FindCdromDrivesPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(FindCdromDrives)
	private:
		Q_DISABLE_COPY(FindCdromDrives)

	public:
		/**
		 * Check if CD-ROM drives are supported on this platform.
		 * @return True if supported; false if not.
		 */
		bool isSupported(void) const;

		/**
		 * Rescan all disc drives.
		 * This clears the QStringList and enumerates all disc drives.
		 */
		void rescan(void);

		/**
		 * Get a list of all available CD-ROM device names.
		 * @return QStringList containing CD-ROM device names.
		 */
		QStringList getDriveNames(void) const;

		/**
		 * Get a libgenscd CdDrive instance for a given CD-ROM device name.
		 * @param deviceName CD-ROM device name.
		 * @return CdDrive instance, or nullptr if unable to open the drive.
		 */
		LibGensCD::CdDrive *getCdDrive(const QString &deviceName);

		/**
		 * Get the icon for the specified CD-ROM drive.
		 * Returns the disc icon if a disc is present,
		 * or the drive icon if no disc is present.
		 * @param deviceName CD_ROM device name.
		 * @return Icon for either the disc or the drive.
		 */
		QIcon getDriveIcon(const QString &deviceName);

		/**
		 * Get the icon for the specified CD-ROM drive.
		 * Returns the disc icon if a disc is present,
		 * or the drive icon if no disc is present.
		 * @param cdDrive LibGensCD::CdDrive.
		 * @return Icon for either the disc or the drive.
		 */
		QIcon getDriveIcon(LibGensCD::CdDrive *cdDrive);

	signals:
		/**
		 * Disc drive added.
		 * This happens if a disc drive was added to the system,
		 * or a rescan() was requested.
		 * @param deviceName Device name of the added CD-ROM drive.
		 */
		void driveAdded(const QString &deviceName);

		/**
		 * Disc drive removed.
		 * This happens if a disc drive was removed from the system,
		 * or a rescan() was requested.
		 * @param deviceName Device name of the removed CD-ROM drive.
		 */
		void driveRemoved(const QString &deviceName);

		/**
		 * Disc drive changed.
		 * This happens if the media in a disc drive was changed.
		 * @param deviceName Device name of the changed CD-ROM drive.
		 */
		void driveChanged(const QString &deviceName);
};

}

#endif /* __GENS_QT4_CDROM_FINDCDROMDRIVES_HPP__ */
