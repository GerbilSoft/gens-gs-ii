/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromDrives.cpp: Find CD-ROM drives.                                *
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

#include "FindCdromDrives.hpp"

// Qt includes.
#include <QtCore/QMap>

namespace GensQt4
{

class FindCdromDrivesPrivate
{
	public:
		FindCdromDrivesPrivate(FindCdromDrives *q);
		~FindCdromDrivesPrivate();

	private:
		FindCdromDrives *const q;
		Q_DISABLE_COPY(FindCdromDrivesPrivate)

	public:
		// List of CD-ROM device names.
		QStringList cdromDeviceNames;

		// Map of CD-ROM device names to LibGensCD::CdDrive instances.
		// NOTE: Using QMap instead of QHash because most people will
		// probably have a maximum of 2 physical drives + 1 virtual drive.
		// Reference: http://woboq.com/blog/qmap_qhash_benchmark.html
		QMap<QString, LibGensCD::CdDrive*> mapCdromDevices;

		// FindCdromBase instance.
		// TODO: Create FindCdromBase class.
		void *findCdromBase;

		// Clear all CD-ROM devices.
		void clearCdromDevices(void);

		/**
		 * Add a CD-ROM device.
		 * @param deviceName CD-ROM device name.
		 */
		void addCdromDevice(QString deviceName);

		/**
		 * Remove a CD-ROM device.
		 * @param deviceName CD-ROM device name.
		 */
		void removeCdromDevice(QString deviceName);
};


/*************************************
 * FindCdromDrivesPrivate functions. *
 *************************************/

FindCdromDrivesPrivate::FindCdromDrivesPrivate(FindCdromDrives *q)
	: q(q)
{
	// TODO: Figure out which FindCdromBase class to instantiate
	// in order to find the device names.
	findCdromBase = nullptr;
}

FindCdromDrivesPrivate::~FindCdromDrivesPrivate()
{
	clearCdromDevices();
}


/**
 * Clear all CD-ROM devices.
 */
void FindCdromDrivesPrivate::clearCdromDevices(void)
{
	// Clear the device name string list.
	foreach (QString deviceName, cdromDeviceNames) {
		emit q->driveRemoved(deviceName);
	}
	cdromDeviceNames.clear();

	// Clear the map of LibGensCD::CdDrive instances.
	qDeleteAll(mapCdromDevices);
	mapCdromDevices.clear();
}


/**
 * Add a CD-ROM device.
 * @param deviceName CD-ROM device name.
 */
void FindCdromDrivesPrivate::addCdromDevice(QString deviceName)
{
	// TODO: Use a QSet instead of QStringList?
	if (cdromDeviceNames.contains(deviceName)) {
		// Device is already added.
		// TODO: Refresh the device.
		return;
	}

	// Add the CD-ROM drive.
	cdromDeviceNames.append(deviceName);
	emit q->driveAdded(deviceName);
}


/**
 * Remove a CD-ROM device.
 * @param deviceName CD-ROM device name.
 */
void FindCdromDrivesPrivate::removeCdromDevice(QString deviceName)
{
	// TODO: Use a QSet instead of QStringList?
	if (!cdromDeviceNames.contains(deviceName)) {
		// Device is not present.
		return;
	}

	// Remove the CD-ROM drive.
	cdromDeviceNames.removeOne(deviceName);

	// Delete the LibGensCD::CdDrive instance.
	LibGensCD::CdDrive *cdDrive = mapCdromDevices.value(deviceName);
	if (cdDrive) {
		delete cdDrive;
		mapCdromDevices.remove(deviceName);
	}

	// Drive is removed.
	emit q->driveRemoved(deviceName);
}


/******************************
 * FindCdromDrives functions. *
 ******************************/

FindCdromDrives::FindCdromDrives(QObject *parent)
	: QObject(parent)
	, d(new FindCdromDrivesPrivate(this))
{
	// Rescan disc drives.
	rescan();
}

FindCdromDrives::~FindCdromDrives()
{
	delete d;
}

/**
 * Check if CD-ROM drives are supported on this platform.
 * @return True if supported; false if not.
 */
bool FindCdromDrives::isSupported(void) const
{
	// TODO: Implement this!
	return true;
}

/**
 * Get a list of all available CD-ROM device names.
 * @return QStringList containing CD-ROM device names.
 */
QStringList FindCdromDrives::getDriveNames()
{
	return d->cdromDeviceNames;
}

/**
 * Get a libgenscd CdDrive instance for a given CD-ROM device name.
 * @param deviceName CD-ROM device name.
 * @return CdDrive instance, or nullptr if unable to open the drive.
 */
LibGensCD::CdDrive *FindCdromDrives::getCdDrive(QString deviceName)
{
	// TODO: Use a QSet instead of QStringList?
	if (!d->cdromDeviceNames.contains(deviceName)) {
		// Device is not present.
		return nullptr;
	}

	// Check if we already have an instance of LibGensCD::CdDrive for this device.
	LibGensCD::CdDrive *cdDrive = d->mapCdromDevices.value(deviceName);
	if (cdDrive)
		return cdDrive;

	// Create an instance of LibGensCD::CdDrive.
	cdDrive = new LibGensCD::CdDrive(deviceName.toUtf8().constData());
	if (!cdDrive->isOpen()) {
		// Could not open the CD-ROM drive.
		// TODO: Show an error.
		fprintf(stderr, "FindCdromDrivesPrivate::addCdromDevice(): ERROR opening device %s\n",
			deviceName.toUtf8().constData());
		delete cdDrive;
		return nullptr;
	}

	d->mapCdromDevices.insert(deviceName, cdDrive);
	return cdDrive;
}

/**
 * Rescan all disc drives.
 * This clears the QStringList and enumerates all disc drives.
 */
void FindCdromDrives::rescan(void)
{
	d->clearCdromDevices();

	// Search for drives.
	// TODO: Use d->findCdromBase. For now, hardcode "/dev/sr0".
	QString deviceName = QLatin1String("/dev/sr0");
	d->cdromDeviceNames.append(deviceName);
	emit driveAdded(deviceName);
}

}
