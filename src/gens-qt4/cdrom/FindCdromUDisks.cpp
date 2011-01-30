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
#include <stdio.h>

// Qt includes.
#include <QtCore/QList>
#include <QtCore/QScopedPointer>

// QtDBus includes.
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusObjectPath>

namespace GensQt4
{

// Common DBus types.
typedef QList<QDBusObjectPath> QtDBus_ao_t;

// UDisks drive IDs.
const char *FindCdromUDisks::ms_UDisks_DriveID[20] =
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
 * getStringProperty(): Get a string property from the given DBus interface.
 * @param dbus-if	[in] QDBusInterface.
 * @param prop		[in] Property name.
 * @return String property. (If an error occurred, the string will be empty.)
 */
QString FindCdromUDisks::getStringProperty(QDBusInterface *dbus_if, const char *prop)
{
	QVariant dbus_reply = dbus_if->property(prop);
	if (dbus_reply.isValid())
		return dbus_reply.toString();
	
	// DBus reply is invalid.
	// Return an empty string.
	return QString();
}


/**
 * getBoolProperty(): Get a bool property from the given DBus interface.
 * @param dbus-if	[in] QDBusInterface.
 * @param prop		[in] Property name.
 * @return Bool property. (If an error occurred, the value will be false.)
 */
bool FindCdromUDisks::getBoolProperty(QDBusInterface *dbus_if, const char *prop)
{
	QVariant dbus_reply = dbus_if->property(prop);
	if (dbus_reply.isValid())
		return dbus_reply.toBool();
	
	// DBus reply is invalid.
	// Return false.
	return false;
}


/**
 * query(): Query for CD-ROM drives.
 * @return 0 on success; non-zero on error.
 */
int FindCdromUDisks::query(void)
{
	// Find all CD-ROM devices.
	// TODO: Make a base class and return standard values.
	// TODO: Get qdbusxml2cpp working with UDisks.
	
	// Clear the existing list of CD-ROM devices.
	m_drives.clear();
	
	QDBusConnection bus = QDBusConnection::systemBus();
	QScopedPointer<QDBusInterface> interface(new QDBusInterface("org.freedesktop.UDisks",
									"/org/freedesktop/UDisks",
									QString(), bus));
	if (!interface->isValid())
	{
		// Interface is invalid.
		printf("Error attaching interface: %s\n", interface->lastError().message().toLocal8Bit().constData());
		return -1;
	}
	
	// Attempt to get all disk devices.
	// Method: EnumerateDevices
	// Return type: ao (QList<QDBusObjectPath>)
	QDBusReply<QtDBus_ao_t> reply_EnumerateDevices = interface->call("EnumerateDevices");
	if (!reply_EnumerateDevices.isValid())
	{
		printf("EnumerateDevices failed: %s\n", interface->lastError().message().toLocal8Bit().constData());
		return -2;
	}
	
	// Received disk devices.
	// Query each disk device to see if it's a CD-ROM drive.
	const QtDBus_ao_t& disks = reply_EnumerateDevices.value();
	QDBusObjectPath cur_disk;
	foreach(cur_disk, disks)
	{
		QScopedPointer<QDBusInterface> drive_if(new QDBusInterface("org.freedesktop.UDisks",
										cur_disk.path(),
										"org.freedesktop.UDisks.Device", bus));
		if (!interface->isValid())
		{
			// Drive interface is invalid.
			printf("Error attaching interface %s: %s\n",
			       cur_disk.path().toLocal8Bit().constData(),
			       drive_if->lastError().message().toLocal8Bit().constData());
			continue;
		}
		
		// Verify that this drive is removable.
		bool DeviceIsRemovable = getBoolProperty(drive_if.data(), "DeviceIsRemovable");
		if (!DeviceIsRemovable)
			continue;
		
		// Get the media compatibility.
		// Method: DriveMediaCompatibility
		// Return type: as (QStringList)
		QVariant reply_DriveMediaCompatibility = drive_if->property("DriveMediaCompatibility");
		if (!reply_DriveMediaCompatibility.isValid())
		{
			printf("DriveMediaCompatibility failed for %s: %s\n",
			       cur_disk.path().toLocal8Bit().constData(),
			       drive_if->lastError().message().toLocal8Bit().constData());
			continue;
		}
		
		// Construct the drive_entry_t.
		drive_entry_t drive;
		drive.discs_supported = 0;
		drive.drive_type = DRIVE_TYPE_NONE;
		drive.disc_type = 0;
		
		// Get various properties.
		drive.path		= getStringProperty(drive_if.data(), "DeviceFile");
		drive.drive_vendor	= getStringProperty(drive_if.data(), "DriveVendor");
		drive.drive_model	= getStringProperty(drive_if.data(), "DriveModel");
		drive.drive_firmware	= getStringProperty(drive_if.data(), "DriveRevision");
		drive.disc_label	= getStringProperty(drive_if.data(), "IdLabel");
		
		// Determine the drive media support.
		// TODO: Convert ms_UDisks_DriveID[] to a QMap.
		// TODO: Verify that reply_DriveMediaCompatibility is a QStringList.
		const QStringList& DriveMediaCompatibility = reply_DriveMediaCompatibility.toStringList();
		QString drive_media_id;
		foreach (drive_media_id, DriveMediaCompatibility)
		{
			// Check the drive media table.
			for (size_t i = 0; i < sizeof(ms_UDisks_DriveID)/sizeof(ms_UDisks_DriveID[0]); i++)
			{
				if (drive_media_id == ms_UDisks_DriveID[i])
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
			// TODO: Should we check the "Removable" property first?
			continue;
		}
		
		// Get the drive type.
		drive.drive_type = GetDriveType(drive.discs_supported);
		
		// Determine the type of disc in the drive.
		QString DriveMedia = getStringProperty(drive_if.data(), "DriveMedia");
		if (!DriveMedia.isEmpty())
		{
			for (size_t i = 0; i < sizeof(ms_UDisks_DriveID)/sizeof(ms_UDisks_DriveID[0]); i++)
			{
				if (DriveMedia == ms_UDisks_DriveID[i])
				{
					// Found a match.
					drive.disc_type = (1 << i);
					break;
				}
			}
		}
		
		// Set the device icon.
		if (drive.disc_type == 0)
			drive.icon = GetDriveTypeIcon(drive.drive_type);
		else
			drive.icon = GetDiscTypeIcon(drive.disc_type);
		
		// Add the drive to m_drives.
		m_drives.append(drive);
		
		// Print drive information.
		printf("Drive: %s - drive is type %d, disc is 0x%08X\n",
		       drive.path.toLocal8Bit().constData(), (int)drive.drive_type, drive.disc_type);
	}
	
	// Devices queried.
	return 0;
}

}
