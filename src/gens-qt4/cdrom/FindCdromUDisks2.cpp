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

// QtDBus metatypes.
#include "dbus/DBusMetatypes.hpp"

// UDisks2 interfaces.
#include "udisks2interface.h"
#include "objectmanagerinterface.h"

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

namespace GensQt4
{

class FindCdromUDisks2Private
{
	public:
		FindCdromUDisks2Private(FindCdromUDisks2 *q);
		~FindCdromUDisks2Private();

	private:
		FindCdromUDisks2 *const q;
		Q_DISABLE_COPY(FindCdromUDisks2Private)

	public:
		// UDisks2's ObjectManager.
		OrgFreedesktopDBusObjectManagerInterface *ifObjMgr;
};

FindCdromUDisks2Private::FindCdromUDisks2Private(FindCdromUDisks2 *q)
	: q(q)
	, ifObjMgr(nullptr)
{
	// Make sure the DBus metatypes are registered.
	registerDBusMetatypes();

	// Connect to the UDisks2 Manager over D-BUS.
	QDBusConnection bus = QDBusConnection::systemBus();
	OrgFreedesktopUDisks2ManagerInterface *ifUDisks2Mgr;
	ifUDisks2Mgr = new OrgFreedesktopUDisks2ManagerInterface(
					QLatin1String("org.freedesktop.UDisks2"),
					QLatin1String("/org/freedesktop/UDisks2/Manager"),
					bus, q);
	if (!ifUDisks2Mgr->isValid()) {
		// Error connecting to the UDisks2 Manager.
		delete ifUDisks2Mgr;
		ifUDisks2Mgr = nullptr;
		return;
	}

	// Run a simple query.
	// If the returned string is empty, UDisks isn't working.
	// Otherwise, UDisks is working.
	QString daemonVersion = ifUDisks2Mgr->version();
	if (ifUDisks2Mgr->lastError().isValid() || daemonVersion.isEmpty()) {
		// UDisks2 is not available.
		delete ifUDisks2Mgr;
		ifUDisks2Mgr = nullptr;
		return;
	}

	// We don't need the UDisks2 Manager anymore.
	delete ifUDisks2Mgr;
	ifUDisks2Mgr = nullptr;

	// Connect to the UDisks2 Object Manager over D-BUS.
	ifObjMgr = new OrgFreedesktopDBusObjectManagerInterface(
					QLatin1String("org.freedesktop.UDisks2"),
					QLatin1String("/org/freedesktop/UDisks2"),
					bus, q);
	if (!ifObjMgr->isValid()) {
		// Error connecting to the UDisks2 Object Manager.
		delete ifObjMgr;
		ifObjMgr = nullptr;
		return;
	}

	// Object manager is connected.
}

FindCdromUDisks2Private::~FindCdromUDisks2Private()
{
	delete ifObjMgr;
}


/** FindCdromUDisks **/


FindCdromUDisks2::FindCdromUDisks2(QObject *parent)
	: FindCdromBase(parent)
	, d(new FindCdromUDisks2Private(this))
{
	// We should have a UDisk2 Object Manager connection.
	if (!d->ifObjMgr)
		return;

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
	{ return (d->ifObjMgr != nullptr); }


/**
 * Scan the system for CD-ROM devices.
 * @return QStringList with all detected CD-ROM device names.
 */
QStringList FindCdromUDisks2::scanDeviceNames(void)
{
	if (!isUsable())
		return QStringList();

	// Get a list of all objects managed by UDisks2.
	QDBusConnection bus = QDBusConnection::systemBus();
	QDBusPendingReply<DBusManagerStruct> reply = d->ifObjMgr->GetManagedObjects();
	reply.waitForFinished();
	if (!reply.isValid())
		return QStringList();

	DBusManagerStruct dbus_mgr = reply.value();
	QStringList cdromDeviceNames;

	// Search for objects that have a "org.freedesktop.UDisks2.Block" interface.
	// NOTE: "org.freedesktop.UDisks2.Drive" does not have the device name.
	// "org.freedesktop.UDisks2.Block" has a "device" property.
	const QString udisks2_block_interface = QLatin1String("org.freedesktop.UDisks2.Block");
	QList<QDBusObjectPath> block_devices;
	foreach (QDBusObjectPath obj_path, dbus_mgr.keys()) {
		QVariantMapMap vmm_ifs = dbus_mgr.value(obj_path);
		if (vmm_ifs.contains(udisks2_block_interface))
			block_devices.append(obj_path);
	}

	// Get the drive interfaces for each block device.
	// Key: "org.freedesktop.UDisks2.Drive" object path.
	// Value: Device name.
	QMap<QDBusObjectPath, QString> device_paths;

	foreach (QDBusObjectPath obj_path, block_devices) {
		QScopedPointer<OrgFreedesktopUDisks2BlockInterface> ifBlock(
			new OrgFreedesktopUDisks2BlockInterface(
						QLatin1String("org.freedesktop.UDisks2"),
						obj_path.path(),
						bus, this));
		if (!ifBlock->isValid()) {
			// Block device is invalid.
			continue;
		}

		// Ignore "partitionable" devices.
		// CD-ROM drives aren't partitionable.
		if (ifBlock->hintPartitionable())
			continue;

		// Get the device filename and object path.
		QString dev_filename = QString::fromLocal8Bit(ifBlock->device().constData());
		QDBusObjectPath dev_path = ifBlock->drive();
		device_paths.insert(dev_path, dev_filename);
	}

	// Check the drive interfaces.
	foreach (QDBusObjectPath obj_path, device_paths.keys()) {
		QScopedPointer<OrgFreedesktopUDisks2DriveInterface> ifDrive(
			new OrgFreedesktopUDisks2DriveInterface(
						QLatin1String("org.freedesktop.UDisks2"),
						obj_path.path(),
						bus, this));

		printf("checking %s\n", obj_path.path().toUtf8().constData());
		if (!ifDrive->isValid()) {
			// Drive is invalid.
			continue;
		}

		// Make sure this is a removable drive.
		// NOTE: ifDrive->optical() only returns true if a
		// disc is inserted. Empty drives will return false.
		if (ifDrive->mediaRemovable()) {
			// Check if "optical_cd" is in the list of supported media.
			QStringList mediaCompatibility = ifDrive->mediaCompatibility();
			const QString optical_cd = QLatin1String("optical_cd");
			foreach (QString discTypeId, mediaCompatibility) {
				if (discTypeId == optical_cd) {
					// Found "optical_cd".
					cdromDeviceNames.append(device_paths.value(obj_path));
					break;
				}
			}
		}
	}

	// TODO: Add signal handlers for property changes.
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
