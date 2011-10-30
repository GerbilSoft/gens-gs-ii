/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * FindCdromUDisks.cpp: Find CD-ROM drives using UDisks.                   *
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

#include "FindCdromUDisks.hpp"

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

class FindCdromUDisksPrivate
{
	public:
		FindCdromUDisksPrivate(FindCdromUDisks *q);
	
	private:
		FindCdromUDisks *const q;
		Q_DISABLE_COPY(FindCdromUDisksPrivate)
	
	public:
		/**
		 * UDisks media type IDs.
		 */
		static const char *const UDisks_MediaTypeID[20];
		
		// D-BUS interface to UDisks.
		OrgFreedesktopUDisksInterface *ifUDisks;
		
		/**
		 * Query a UDisks Device object.
		 * @param objectPath UDisks Device object path.
		 * @return Error code: (TODO: Use an enum?)
		 * - 0: success
		 * - 1: not an optical drive
		 * - negative: error
		 */
		int queryUDisksDevice(const QDBusObjectPath& objectPath);
};

FindCdromUDisksPrivate::FindCdromUDisksPrivate(FindCdromUDisks *q)
	: q(q)
{ }


/**
 * UDisks media type IDs.
 */
const char *const FindCdromUDisksPrivate::UDisks_MediaTypeID[20] =
{
	"optical_cd", "optical_cd_r", "optical_cd_rw", "optical_dvd",
	"optical_dvd_r", "optical_dvd_rw", "optical_dvd_ram",
	"optical_dvd_plus_r", "optical_dvd_plus_rw",
	"optical_dvd_plus_r_dl", "optical_dvd_plus_rw_dl",
	"optical_bd", "optical_bd_r", "optical_bd_re",
	"optical_hddvd", "optical_hddvd_r", "optical_hddvd_rw",
	"optical_mo", "optical_mrw", "optical_mrw_w"
};


/**
 * Query a UDisks Device object.
 * @param objectPath UDisks Device object path.
 * @return Error code: (TODO: Use an enum?)
 * - 0: success
 * - 1: not an optical drive
 * - negative: error
 */
int FindCdromUDisksPrivate::queryUDisksDevice(const QDBusObjectPath& objectPath)
{
	QDBusConnection bus = QDBusConnection::systemBus();
	auto_ptr<OrgFreedesktopUDisksDeviceInterface> drive_if(
					new OrgFreedesktopUDisksDeviceInterface(
								QLatin1String("org.freedesktop.UDisks"),
								objectPath.path(), bus));
	if (!drive_if->isValid())
	{
		// Drive interface is invalid.
		LOG_MSG(cd, LOG_MSG_LEVEL_ERROR,
			"FindCdromUDisks: Error attaching interface %s: %s",
			objectPath.path().toLocal8Bit().constData(),
			drive_if->lastError().message().toLocal8Bit().constData());
		return -2;
	}
	
	// Verify that this drive is removable.
	if (!drive_if->deviceIsRemovable())
	{
		// This drive does not support removable media.
		// Hence, it isn't an optical drive.
		return 1;
	}
	
	// Construct the CdromDriveEntry.
	CdromDriveEntry drive;
	drive.discs_supported = 0;
	drive.drive_type = DRIVE_TYPE_NONE;
	drive.disc_type = 0;
	
	// Get various properties.
	drive.path		= drive_if->deviceFile();
	drive.drive_vendor	= drive_if->driveVendor();
	drive.drive_model	= drive_if->driveModel();
	drive.drive_firmware	= drive_if->driveRevision();
	drive.disc_label	= drive_if->idLabel();
	drive.disc_blank	= drive_if->opticalDiscIsBlank();
	
	// Determine the drive media support.
	// TODO: Convert ms_UDisks_DriveID[] to a QMap or QHash?
	QStringList driveMediaCompatibility = drive_if->driveMediaCompatibility();
	foreach (const QString& drive_media_id, driveMediaCompatibility)
	{
		// Check the drive media table.
		for (size_t i = 0; i < sizeof(UDisks_MediaTypeID)/sizeof(UDisks_MediaTypeID[0]); i++)
		{
			if (drive_media_id == QLatin1String(UDisks_MediaTypeID[i]))
			{
				// Found a match.
				drive.discs_supported |= (1 << i);
				break;
			}
		}
	}
	if (drive.discs_supported == 0)
	{
		// This is not an optical drive.
		return 1;
	}
	
	// Get the drive type.
	drive.drive_type = q->GetDriveType(drive.discs_supported);
	
	// Determine the type of disc in the drive.
	QString DriveMedia = drive_if->driveMedia();
	if (!DriveMedia.isEmpty())
	{
		for (size_t i = 0; i < sizeof(UDisks_MediaTypeID)/sizeof(UDisks_MediaTypeID[0]); i++)
		{
			if (DriveMedia == QLatin1String(UDisks_MediaTypeID[i]))
			{
				// Found a match.
				drive.disc_type = (1 << i);
				break;
			}
		}
	}
	
	// TODO: Determine if UDisks is checking what type of disc is present.
	
	// If the disc is blank, set the disc label to "Blank [disc_type]".
	// TODO: Make this a common FindCdromBase function?
	if (drive.disc_type != DISC_TYPE_NONE && drive.disc_blank)
		drive.disc_label = FindCdromUDisks::tr("Blank %1").arg(q->GetDiscTypeName(drive.disc_type));
	
	// Emit the driveUpdated() signal for this drive.
	emit q->driveUpdated(drive);
	return 0;
}


/** FindCdromUDisks **/


FindCdromUDisks::FindCdromUDisks()
	: d(new FindCdromUDisksPrivate(this))
{
	// Connect to UDisks over D-BUS.
	QDBusConnection bus = QDBusConnection::systemBus();
	d->ifUDisks = new OrgFreedesktopUDisksInterface(
					QLatin1String("org.freedesktop.UDisks"),
					QLatin1String("/org/freedesktop/UDisks"),
					bus, this);
	if (!d->ifUDisks->isValid())
	{
		// Error connecting to D-BUS.
		delete d->ifUDisks;
		d->ifUDisks = NULL;
		return;
	}
	
	// Run a simple query.
	// If the returned string is empty, UDisks isn't working.
	// Otherwise, UDisks is working.
	QString daemonVersion = d->ifUDisks->daemonVersion();
	if (d->ifUDisks->lastError().isValid() || daemonVersion.isEmpty())
	{
		// UDisks is not available.
		delete d->ifUDisks;
		d->ifUDisks = NULL;
		return;
	}
	
	// D-BUS is initialized.
	// Connect the UDisks signals.
	// * DeviceChanged(): A device has changed.
	// * DeviceRemoved(): A device has been removed.
	// * NOTE: DeviceAdded() is not needed, since UDisks emits
	//   DeviceChanged() immediately after the device is added.
	connect(d->ifUDisks, SIGNAL(DeviceChanged(QDBusObjectPath)),
		this, SLOT(deviceChanged(QDBusObjectPath)));
#if 0
	connect(m_ifUDisks, SIGNAL(DeviceAdded(QDBusObjectPath)),
		this, SLOT(deviceChanged(QDBusObjectPath)));
#endif
	connect(d->ifUDisks, SIGNAL(DeviceRemoved(QDBusObjectPath)),
		this, SLOT(deviceRemoved(QDBusObjectPath)));
}

FindCdromUDisks::~FindCdromUDisks()
	{ delete d; }

bool FindCdromUDisks::isUsable(void) const
	{ return (d->ifUDisks != NULL); }


/**
 * Asynchronously query for CD-ROM drives.
 * The driveUpdated() signal will be emitted once for each detected drive.
 * @return 0 on success; non-zero on error.
 */
int FindCdromUDisks::query(void)
{
	if (!isUsable())
		return -1;
	
	// NOTE: QDBusConnection is not thread-safe.
	// See http://bugreports.qt.nokia.com/browse/QTBUG-11413
	
	// Override the thread mechanism for now.
	return query_int();
}


/**
 * Asynchronously query for CD-ROM drives. (INTERNAL FUNCTION)
 * The driveUpdated() signal will be emitted once for each detected drive.
 * @return 0 on success; non-zero on error.
 */
int FindCdromUDisks::query_int(void)
{
	// Find all CD-ROM devices.
	
	// NOTE: QDBusConnection is not thread-safe.
	// See http://bugreports.qt.nokia.com/browse/QTBUG-11413
	if (!d->ifUDisks)
		return -1;
	
	// Attempt to get all disk devices.
	// Method: EnumerateDevices
	// Return type: ao (QList<QDBusObjectPath>)
	QDBusReply<QList<QDBusObjectPath> > reply_EnumerateDevices = d->ifUDisks->EnumerateDevices();
	if (!reply_EnumerateDevices.isValid())
	{
		LOG_MSG(cd, LOG_MSG_LEVEL_ERROR,
			"FindCdromUdisks: EnumerateDevices failed: %s",
			d->ifUDisks->lastError().message().toLocal8Bit().constData());
		
		// TODO: Emit an error signal instead?
		emit driveQueryFinished();
		return -2;
	}
	
	// Received disk devices.
	// Query each disk device to see if it's a CD-ROM drive.
	const QList<QDBusObjectPath>& disks = reply_EnumerateDevices.value();
	foreach (const QDBusObjectPath& cur_disk, disks)
		d->queryUDisksDevice(cur_disk);
	
	// Devices queried.
	emit driveQueryFinished();
	return 0;
}


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

}
