/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromUDisks2.cpp: Find CD-ROM drives: UDisks2 version.              *
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

#include "FindCdromUDisks2.hpp"

// C includes.
#include <paths.h>

// C++ includes.
#include <memory>
using std::auto_ptr;

// Qt includes.
#include <QtCore/QList>

// QtDBus includes.
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

// UDisks interfaces.
#include "udisksinterface.h"
#include "deviceinterface.h"

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

namespace GensQt4
{

class FindCdromUDisks2Private
{
	public:
		FindCdromUDisks2Private(FindCdromUDisks2 *q);

	private:
		FindCdromUDisks2 *const q;
		Q_DISABLE_COPY(FindCdromUDisks2Private)

	public:
		// D-BUS interface to UDisks.
		OrgFreedesktopUDisksInterface *ifUDisks;
};

FindCdromUDisks2Private::FindCdromUDisks2Private(FindCdromUDisks2 *q)
	: q(q)
{ }


/** FindCdromUDisks **/


FindCdromUDisks2::FindCdromUDisks2(QObject *parent)
	: FindCdromBase(parent)
	, d(new FindCdromUDisks2Private(this))
{
	// Connect to UDisks2 over D-BUS.
	QDBusConnection bus = QDBusConnection::systemBus();
	d->ifUDisks = new OrgFreedesktopUDisksInterface(
					QLatin1String("org.freedesktop.UDisks"),
					QLatin1String("/org/freedesktop/UDisks"),
					bus, this);
	if (!d->ifUDisks->isValid()) {
		// Error connecting to D-BUS.
		delete d->ifUDisks;
		d->ifUDisks = NULL;
		return;
	}

	// Run a simple query.
	// If the returned string is empty, UDisks2 isn't working.
	// Otherwise, UDisks2 is working.
	QString daemonVersion = d->ifUDisks->daemonVersion();
	if (d->ifUDisks->lastError().isValid() || daemonVersion.isEmpty()) {
		// UDisks is not available.
		delete d->ifUDisks;
		d->ifUDisks = NULL;
		return;
	}

	// TODO: Notification signals for LibGensCD.
#if 0
	// D-BUS is initialized.
	// Connect the UDisks signals.
	// * DeviceChanged(): A device has changed.
	// * DeviceRemoved(): A device has been removed.
	// * NOTE: DeviceAdded() is not needed, since UDisks emits
	//   DeviceChanged() immediately after the device is added.
	connect(d->ifUDisks, SIGNAL(DeviceChanged(QDBusObjectPath)),
		this, SLOT(deviceChanged(QDBusObjectPath)));
#if 0
	connect(d->ifUDisks, SIGNAL(DeviceAdded(QDBusObjectPath)),
		this, SLOT(deviceChanged(QDBusObjectPath)));
#endif
	connect(d->ifUDisks, SIGNAL(DeviceRemoved(QDBusObjectPath)),
		this, SLOT(deviceRemoved(QDBusObjectPath)));
#endif
}

FindCdromUDisks2::~FindCdromUDisks2()
	{ delete d; }

bool FindCdromUDisks2::isUsable(void) const
	{ return (d->ifUDisks != nullptr); }


/**
 * Scan the system for CD-ROM devices.
 * @return QStringList with all detected CD-ROM device names.
 */
QStringList FindCdromUDisks2::scanDeviceNames(void)
{
	if (!isUsable())
		return QStringList();
	
	// NOTE: QDBusConnection is not thread-safe.
	// See http://bugreports.qt.nokia.com/browse/QTBUG-11413
	
	// Attempt to get all disk devices.
	// Method: EnumerateDevices
	// Return type: ao (QList<QDBusObjectPath>)
	QDBusReply<QList<QDBusObjectPath> > reply_EnumerateDevices = d->ifUDisks->EnumerateDevices();
	if (!reply_EnumerateDevices.isValid()) {
		LOG_MSG(cd, LOG_MSG_LEVEL_ERROR,
			"FindCdromUDisks2: EnumerateDevices failed: %s",
			d->ifUDisks->lastError().message().toLocal8Bit().constData());
		
		// TODO: Emit an error signal instead?
		//emit driveQueryFinished();
		//return -2;
		return QStringList();
	}

	// Received disk devices.
	// Determine which ones are CD-ROM drives.
	const QList<QDBusObjectPath>& disks = reply_EnumerateDevices.value();
	QStringList cdromDeviceNames;
	QDBusConnection bus = QDBusConnection::systemBus();
	foreach (const QDBusObjectPath& cur_disk, disks) {
		auto_ptr<OrgFreedesktopUDisksDeviceInterface> drive_if(
			new OrgFreedesktopUDisksDeviceInterface(
				QLatin1String("org.freedesktop.UDisks"),
				cur_disk.path(), bus));
		if (!drive_if->isValid()) {
			// Drive interface is invalid.
			LOG_MSG(cd, LOG_MSG_LEVEL_ERROR,
				"FindCdromUDisks: Error attaching interface %s: %s",
				cur_disk.path().toLocal8Bit().constData(),
				drive_if->lastError().message().toLocal8Bit().constData());
			continue;
		}

		// Verify that this drive is a CD-ROM drive.
		if (drive_if->deviceIsRemovable() &&
		    drive_if->deviceIsOpticalDisc())
		{
			// This is a CD-ROM drive.
			cdromDeviceNames.append(drive_if->deviceFile());
		}
	}
	
	// Devices queried.
	//emit driveQueryFinished();
	return cdromDeviceNames;
}


// TODO: Notification support for LibGensCD.
#if 0
/**
 * deviceChanged(): A device has changed.
 * @param objectPath Device object path.
 */
void FindCdromUDisks::deviceChanged(const QDBusObjectPath& objectPath)
{
	// Query the device for changes.
	d->queryUDisksDevice(objectPath);
}


/**
 * deviceRemoved(): A device has been removed.
 * @param objectPath Device object path.
 */
void FindCdromUDisks::deviceRemoved(const QDBusObjectPath& objectPath)
{
	// NOTE: The device file cannot be retrieved, since the object is deleted.
	// Assume the device file is _PATH_DEV + the last component of objectPath.
	// TODO: Store the device filenames locally.
	// TODO: Use QDir::separator()?
	QString devFile = QLatin1String(_PATH_DEV) + objectPath.path().section(QChar(L'/'), -1);
	emit driveRemoved(devFile);
}
#endif

}
